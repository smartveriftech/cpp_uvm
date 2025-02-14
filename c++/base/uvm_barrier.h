// Copyright (c) 2025-2035 Smart Verification Technology Corporation (智验科技)
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

#ifndef UVM_BARRIER_H
#define UVM_BARRIER_H

#include <string>
#include "base/uvm_object.h"

class uvm_printer;
class uvm_event;

//-----------------------------------------------------------------------------
//
// CLASS: uvm_barrier
//
// The uvm_barrier class provides a multiprocess synchronization mechanism. 
// It enables a set of processes to block until the desired number of processes
// get to the synchronization point, at which time all of the processes are
// released.
//-----------------------------------------------------------------------------
class uvm_barrier : public uvm_object {
public:
    uvm_barrier(const std::string& name = "", int threshold = 0);
    ~uvm_barrier();

    // Task: wait_for
    //
    // Waits for enough processes to reach the barrier before continuing. 
    //
    // The number of processes to wait for is set by the <set_threshold> method.
    virtual void wait_for();

    // Function: reset
    //
    // Resets the barrier. This sets the waiter count back to zero. 
    //
    // The threshold is unchanged. After reset, the barrier will force processes
    // to wait for the threshold again. 
    //
    // If the ~wakeup~ bit is set, any currently waiting processes will
    // be activated.
    virtual void reset(bool wakeup = true);

    // Function: set_auto_reset
    //
    // Determines if the barrier should reset itself after the threshold is
    // reached. 
    //
    // The default is on, so when a barrier hits its threshold it will reset, and
    // new processes will block until the threshold is reached again. 
    //
    // If auto reset is off, then once the threshold is achieved, new processes
    // pass through without being blocked until the barrier is reset.
    virtual void set_auto_reset(bool value = true);

    // Function: set_threshold
    //
    // Sets the process threshold. 
    //
    // This determines how many processes must be waiting on the barrier before
    // the processes may proceed. 
    //
    // Once the ~threshold~ is reached, all waiting processes are activated. 
    //
    // If ~threshold~ is set to a value less than the number of currently
    // waiting processes, then the barrier is reset and waiting processes are
    // activated.
    virtual void set_threshold(int threshold);

    // Function: get_threshold
    //
    // Gets the current threshold setting for the barrier.
    virtual int get_threshold() const;

    // Function: get_num_waiters
    //
    // Returns the number of processes currently waiting at the barrier.
    virtual int get_num_waiters() const;

    // Function: cancel
    //
    // Decrements the waiter count by one. This is used when a process that is
    // waiting on the barrier is killed or activated by some other means.
    virtual void cancel();

    static const std::string type_name;

    virtual uvm_object* create(const std::string& name = "");
    virtual std::string get_type_name();

protected:
    virtual void do_print(uvm_printer* printer);
    virtual void do_copy(uvm_object* rhs);

private:
    void trigger();

    int threshold;
    int num_waiters;
    bool at_threshold;
    bool auto_reset;
    uvm_event* m_event;
};

#endif // UVM_BARRIER_H