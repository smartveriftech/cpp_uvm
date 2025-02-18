//
//------------------------------------------------------------------------------
// Copyright 2007-2011 Cadence Design Systems, Inc. 
// Copyright 2007-2011 Mentor Graphics Corporation
// Copyright 2010-2011 Synopsys, Inc.
// Copyright 2013      NVIDIA Corporation
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

#ifndef UVM_OBJECTION_H
#define UVM_OBJECTION_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <functional>
#include <fstream>
#include <cassert>
#include <thread>
#include <condition_variable>

#include "base/uvm_report_object.h"
#include "base/uvm_callback.h"

class uvm_root;

class uvm_objection_context_object;
class uvm_objection;
class uvm_objection_callback;
class uvm_callbacks_objection;
class uvm_objection_events;

class uvm_objection_events {
public:
    int waiters;
    std::condition_variable raised;
    std::condition_variable dropped;
    std::condition_variable all_dropped;
};

//------------------------------------------------------------------------------
// Title: Objection Mechanism
//------------------------------------------------------------------------------
// The following classes define the objection mechanism and end-of-test
// functionality, which is based on <uvm_objection>.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Class: uvm_objection
//
//------------------------------------------------------------------------------
// Objections provide a facility for coordinating status information between
// two or more participating components, objects, and even module-based IP.
//
// Tracing of objection activity can be turned on to follow the activity of
// the objection mechanism. It may be turned on for a specific objection
// instance with <uvm_objection::trace_mode>, or it can be set for all 
// objections from the command line using the option +UVM_OBJECTION_TRACE.
//------------------------------------------------------------------------------

class uvm_objection : public uvm_report_object {
public:
    uvm_objection(const std::string& name = "uvm_objection");

    virtual void clear(uvm_object* obj = nullptr);
    bool trace_mode(int mode = -1);
    void raise_objection(uvm_object* obj = nullptr, const std::string& description = "", int count = 1);
    void drop_objection(uvm_object* obj = nullptr, const std::string& description = "", int count = 1);
    void set_drain_time(uvm_object* obj, int drain);
    int get_objection_count(uvm_object* obj = nullptr);
    int get_objection_total(uvm_object* obj = nullptr);
    int get_drain_time(uvm_object* obj = nullptr);
    void display_objections(uvm_object* obj = nullptr, bool show_header = true);
    void wait_for(uvm_objection_event objt_event, uvm_object* obj = nullptr);
    void wait_for_total_count(uvm_object* obj = nullptr, int count = 0);

    static void m_init_objections();

//protected:
    void m_report(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count, const std::string& action);
    void m_propagate(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count, bool raise, bool in_top_thread);
    uvm_object* m_get_parent(uvm_object* obj);
    void m_raise(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);
    void m_drop(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count, bool in_top_thread);
    void m_set_hier_mode(uvm_object* obj);
    void m_forked_drop(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count, bool in_top_thread);
    void m_execute_scheduled_forks();
    std::string m_display_objections(uvm_object* obj = nullptr, bool show_header = true);
    virtual void raised(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);
    virtual void dropped(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);
    virtual void all_dropped(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);

//private:
    static std::vector<uvm_objection*> m_objections;
    std::unordered_map<uvm_object*, int> m_source_count;
    std::unordered_map<uvm_object*, int> m_total_count;
    std::unordered_map<uvm_object*, int> m_drain_time;
    std::unordered_map<uvm_object*, int> m_draining;
    std::unordered_map<uvm_object*, uvm_objection_events> m_events;
    std::deque<uvm_objection_context_object*> m_context_pool;
    std::deque<uvm_objection_context_object*> m_scheduled_list;

    bool m_trace_mode;
    bool m_top_all_dropped;
    bool m_hier_mode;
    bool m_cleared;
    uvm_root* m_top;
    std::thread m_background_proc;

    static uvm_root* get_uvm_top();

protected:
    void f_display(UVM_FILE file, const std::string& str);
};

class uvm_objection_context_object {
public:
    uvm_object* obj;
    uvm_object* source_obj;
    std::string description;
    int count;
};

class uvm_callbacks_objection : public uvm_objection {
public:
    uvm_callbacks_objection(const std::string& name = "uvm_callbacks_objection");

    virtual void raised(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count) override;
    virtual void dropped(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count) override;
    virtual void all_dropped(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count) override;
};

class uvm_objection_callback : public uvm_callback {
public:
    uvm_objection_callback(const std::string& name);

    virtual void raised(uvm_objection* objection, uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);
    virtual void dropped(uvm_objection* objection, uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);
    virtual void all_dropped(uvm_objection* objection, uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);
};

#ifdef UVM_USE_CALLBACKS_OBJECTION_FOR_TEST_DONE
typedef uvm_callbacks_objection m_uvm_test_done_objection_base;
#else
typedef uvm_objection m_uvm_test_done_objection_base;
#endif

//------------------------------------------------------------------------------
// Class- uvm_test_done_objection DEPRECATED
//
// Provides built-in end-of-test coordination
//------------------------------------------------------------------------------

class uvm_test_done_objection : public m_uvm_test_done_objection_base {
public:
    static uvm_test_done_objection* get();

    // Function- qualify DEPRECATED
    //
    // Checks that the given ~object~ is derived from either <uvm_component> or
    // <uvm_sequence_base>.
    virtual void qualify(uvm_object* obj, bool is_raise, const std::string& description);

//#ifndef UVM_NO_DEPRECATED
    // Function- raise_objection DEPRECATED
    //
    // Calls <uvm_objection::raise_objection> after calling <qualify>. 
    // If the ~object~ is not provided or is ~null~, then the implicit top-level
    // component, ~uvm_top~, is chosen.
    virtual void raise_objection(uvm_object* obj = nullptr, const std::string& description = "", int count = 1);

    // Function- drop_objection DEPRECATED
    //
    // Calls <uvm_objection::drop_objection> after calling <qualify>. 
    // If the ~object~ is not provided or is ~null~, then the implicit top-level
    // component, ~uvm_top~, is chosen.
    virtual void drop_objection(uvm_object* obj = nullptr, const std::string& description = "", int count = 1);

    // Task- force_stop DEPRECATED
    //
    // Forces the propagation of the all_dropped() callback, even if there are still
    // outstanding objections. The net effect of this action is to forcibly end
    // the current phase.
    virtual void force_stop(uvm_object* obj = nullptr);

//protected:
    uvm_test_done_objection(const std::string& name = "uvm_test_done");

    static uvm_test_done_objection* m_inst;
    bool m_forced;
    bool m_executing_stop_processes;
    int m_n_stop_threads;

    // Variable- stop_timeout DEPRECATED
    //
    // These set watchdog timers for task-based phases and stop tasks. You can not
    // disable the timeouts. When set to 0, a timeout of the maximum time possible
    // is applied. A timeout at this value usually indicates a problem with your
    // testbench. You should lower the timeout to prevent "never-ending"
    // simulations. 
    time_t stop_timeout;

    void stop_request();

    // Function- stop_request DEPRECATED
    //
    // Calling this function triggers the process of shutting down the currently
    // running task-based phase. This process involves calling all components'
    // stop tasks for those components whose enable_stop_interrupt bit is set.
    // Once all stop tasks return, or once the optional global_stop_timeout
    // expires, all components' kill method is called, effectively ending the
    // current phase. The uvm_top will then begin execution of the next phase,
    // if any.
    void m_stop_request();

    void m_do_stop_all(uvm_component* comp);

    // Task- all_dropped DEPRECATED
    //
    // This callback is called when the given ~object's~ objection count reaches
    // zero; if the ~object~ is the implicit top-level, <uvm_root> then it means
    // there are no more objections raised for the ~uvm_test_done~ objection.
    // Thus, after calling <uvm_objection::all_dropped>, this method will call
    // <global_stop_request> to stop the current task-based phase (e.g. run).
    virtual void all_dropped(uvm_object* obj, uvm_object* source_obj, const std::string& description, int count);

//#endif //UVM_NO_DEPRECATED

};

#endif // UVM_OBJECTION_H