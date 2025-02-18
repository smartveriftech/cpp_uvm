//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Mentor Graphics Corporation
// Copyright 2007-2011 Cadence Design Systems, Inc.
// Copyright 2010 Synopsys, Inc.
// Copyright 2014 NVIDIA Corportation
// Copyright 2025-2035 Smart Verification Technology Corporation (智验科技)
// All Rights Reserved Worldwide
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef UVM_EVENT_H
#define UVM_EVENT_H

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "base/uvm_object.h"
#include "base/uvm_event_callback.h"

class uvm_printer;

//------------------------------------------------------------------------------
// CLASS: uvm_event
//
// The uvm_event class is a wrapper class around the SystemVerilog event
// construct.  It provides some additional services such as setting callbacks
// and maintaining the number of waiters.
//------------------------------------------------------------------------------

class uvm_event : public uvm_object {
public:
    static const std::string type_name;

    uvm_event(const std::string& name = "");
    virtual ~uvm_event() = default;

    //---------//
    // waiting //
    //---------//

    // Task: wait_on
    //
    // Waits for the event to be activated for the first time.
    //
    // If the event has already been triggered, this task returns immediately.
    // If delta is set, the caller will be forced to wait a single delta #0
    // before returning. This prevents the caller from returning before
    // previously waiting processes have had a chance to resume.
    //
    // Once an event has been triggered, it will be remain "on" until the event
    // is reset.
    virtual void wait_on(bool delta = false);

    // Task: wait_off
    //
    // If the event has already triggered and is "on", this task waits for the
    // event to be turned "off" via a call to reset.
    //
    // If the event has not already been triggered, this task returns immediately.
    // If delta is set, the caller will be forced to wait a single delta #0
    // before returning. This prevents the caller from returning before
    // previously waiting processes have had a chance to resume.
    virtual void wait_off(bool delta = false);

    // Task: wait_trigger
    //
    // Waits for the event to be triggered. 
    //
    // If one process calls wait_trigger in the same delta as another process
    // calls trigger, a race condition occurs. If the call to wait occurs
    // before the trigger, this method will return in this delta. If the wait
    // occurs after the trigger, this method will not return until the next
    // trigger, which may never occur and thus cause deadlock.
    virtual void wait_trigger();

    // Task: wait_ptrigger
    //
    // Waits for a persistent trigger of the event. Unlike wait_trigger, this
    // views the trigger as persistent within a given time-slice and thus avoids
    // certain race conditions. If this method is called after the trigger but
    // within the same time-slice, the caller returns immediately.
    virtual void wait_ptrigger();

    // Task: wait_trigger_data
    //
    // This method calls wait_trigger followed by get_trigger_data.
    virtual void wait_trigger_data(uvm_object*& data);

    // Task: wait_ptrigger_data
    //
    // This method calls wait_ptrigger followed by get_trigger_data.
    virtual void wait_ptrigger_data(uvm_object*& data);

    //------------//
    // triggering //
    //------------//

    // Function: trigger
    //
    // Triggers the event, resuming all waiting processes.
    //
    // An optional data argument can be supplied with the enable to provide
    // trigger-specific information.
    virtual void trigger(uvm_object* data = nullptr);

    // Function: get_trigger_data
    //
    // Gets the data, if any, provided by the last call to trigger.
    virtual uvm_object* get_trigger_data() const;

    // Function: get_trigger_time
    //
    // Gets the time that this event was last triggered. If the event has not been
    // triggered, or the event has been reset, then the trigger time will be 0.
    virtual std::time_t get_trigger_time() const;

    //-------//
    // state //
    //-------//

    // Function: is_on
    //
    // Indicates whether the event has been triggered since it was last reset. 
    //
    // A return of 1 indicates that the event has triggered.
    virtual bool is_on() const;

    // Function: is_off
    //
    // Indicates whether the event has been triggered or been reset.
    //
    // A return of 1 indicates that the event has not been triggered.
    virtual bool is_off() const;

    // Function: reset
    //
    // Resets the event to its off state. If wakeup is set, then all processes
    // currently waiting for the event are activated before the reset.
    //
    // No callbacks are called during a reset.
    virtual void reset(bool wakeup = false);

    //-----------//
    // callbacks //
    //-----------//

    // Function: add_callback
    //
    // Registers a callback object, cb, with this event. The callback object
    // may include pre_trigger and post_trigger functionality. If append is set
    // to 1, the default, cb is added to the back of the callback list. Otherwise,
    // cb is placed at the front of the callback list.
    virtual void add_callback(uvm_event_callback* cb, bool append = true);

    // Function: delete_callback
    //
    // Unregisters the given callback, cb, from this event. 
    virtual void delete_callback(uvm_event_callback* cb);

    //--------------//
    // waiters list //
    //--------------//

    // Function: cancel
    //
    // Decrements the number of waiters on the event. 
    //
    // This is used if a process that is waiting on an event is disabled or
    // activated by some other means.
    virtual void cancel();

    // Function: get_num_waiters
    //
    // Returns the number of processes waiting on the event.
    virtual int get_num_waiters() const;

    virtual uvm_object* create(const std::string& name = "");
    virtual std::string get_type_name();
    virtual void do_print(uvm_printer* printer);
    virtual void do_copy(uvm_object* rhs);

private:
    bool on;
    std::time_t trigger_time;
    uvm_object* trigger_data;
    int num_waiters;
    std::vector<uvm_event_callback*> callbacks;
    
    // Synchronization primitives
    mutable std::mutex mtx;
    std::condition_variable cv;
};

#endif // UVM_EVENT_H