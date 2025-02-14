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

#ifndef UVM_HEARTBEAT_H
#define UVM_HEARTBEAT_H

#include "uvm_root.h"
#include "uvm_component.h"
#include "uvm_report_handler.h"

#include "time_proc/uvm_time.h"

#include <vector>
#include <string>

class uvm_heartbeat : public uvm_object {
public:
    // Singleton access
    static uvm_heartbeat& get_inst();

    // Set the precision for heartbeat intervals
    void set_precision(uvm_time_unit unit);

    // Set the runtime for the heartbeat (duration to run the heartbeat)
    void set_runtime(double duration, uvm_time_unit unit);

    // Run the heartbeat for the specified runtime
    void run();

    // Function to get current simulation time in a specified unit
    uint64_t get_cur_time() const;

    // Execute `run_1precision` on all components in the simulation
    void run_1precision();

    // Constructor and Destructor
    uvm_heartbeat(const std::string& name = "uvm_heartbeat");
    virtual ~uvm_heartbeat();

private:
    // Singleton instance
    static uvm_heartbeat* m_instance;

    // Precision and runtime as uvm_time objects
    uvm_time time;
    uint32_t m_runtime_cnt;        // Total runtime counter
    uint32_t m_elapsed_cnt;        // elapsed counter

    // List of uvm_component pointers
    std::vector<uvm_component*> m_components;

    // Helper function to traverse uvm_components
    void collect_all_components();
};

/////////////////////////////////////////////////////////////////////////
// below are direct translate from systemverilog c++
/////////////////////////////////////////////////////////////////////////
/*

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <chrono>

#include "base/uvm_object.h"
#include "base/uvm_object_globals.h"
#include "base/uvm_globals.h"
#include "base/uvm_component.h"
#include "base/uvm_event.h"
#include "base/uvm_objection.h"

enum uvm_heartbeat_modes {
    UVM_ALL_ACTIVE,
    UVM_ONE_ACTIVE,
    UVM_ANY_ACTIVE,
    UVM_NO_HB_MODE
};

class uvm_heartbeat_callback;

class uvm_heartbeat : public uvm_object {
public:
    uvm_heartbeat(const std::string& name, uvm_component* cntxt, uvm_callbacks_objection* objection = nullptr);

    uvm_heartbeat_modes set_mode(uvm_heartbeat_modes mode = UVM_NO_HB_MODE);

    void set_heartbeat(uvm_event* e, std::vector<uvm_component*>& comps);

    void add(uvm_component* comp);

    void remove(uvm_component* comp);

    void start(uvm_event* e = nullptr);

    void stop();

private:
    uvm_callbacks_objection* m_objection;
    uvm_heartbeat_callback* m_cb;
    uvm_component* m_cntxt;
    uvm_heartbeat_modes m_mode;
    std::vector<uvm_component*> m_hblist;
    uvm_event* m_event;
    bool m_started;
    bool m_added;

    void m_start_hb_process();

    void m_enable_cb();

    void m_disable_cb();

    void m_hb_process();
};


class uvm_heartbeat_callback : public uvm_objection_callback {
public:
    std::unordered_map<uvm_object*, int> cnt;
    std::unordered_map<uvm_object*, time_t> last_trigger;
    uvm_object* target;

    uvm_heartbeat_callback(const std::string& name, uvm_object* target);

    virtual void raised(uvm_objection* objection, uvm_object* obj, uvm_object* source_obj, const std::string& description, int count) override;

    virtual void dropped(uvm_objection* objection, uvm_object* obj, uvm_object* source_obj, const std::string& description, int count) override;

    void reset_counts();

    int objects_triggered();
};

using uvm_heartbeat_cbs_t = uvm_callbacks<uvm_callbacks_objection, uvm_heartbeat_callback>;
*/

#endif // UVM_HEARTBEAT_H