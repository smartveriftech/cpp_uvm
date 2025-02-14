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

#ifndef UVM_REPORT_HANDLER_H
#define UVM_REPORT_HANDLER_H

#include "base/uvm_pool.h"
#include "base/uvm_report_object.h"
#include "base/uvm_object_globals.h"

#include <string>
#include <map>

//FIXME

// Forward declaration
class uvm_report_server;

typedef uvm_pool<std::string, uvm_action> uvm_id_actions_array;
typedef uvm_pool<std::string, UVM_FILE> uvm_id_file_array;
typedef uvm_pool<std::string, int> uvm_id_verbosities_array;
typedef uvm_pool<uvm_severity, uvm_severity> uvm_sev_override_array;

//------------------------------------------------------------------------------
//
// CLASS: uvm_report_handler
//
// The uvm_report_handler is the class to which most methods in
// <uvm_report_object> delegate. It stores the maximum verbosity, actions,
// and files that affect the way reports are handled. 
//
// The report handler is not intended for direct use. See <uvm_report_object>
// for information on the UVM reporting mechanism.
//
// The relationship between <uvm_report_object> (a base class for uvm_component)
// and uvm_report_handler is typically one to one, but it can be many to one
// if several uvm_report_objects are configured to use the same
// uvm_report_handler_object. See <uvm_report_object::set_report_handler>.
//
// The relationship between uvm_report_handler and <uvm_report_server> is many
// to one. 
//
//------------------------------------------------------------------------------
class uvm_report_handler {
public:

    // Function: new
    // 
    // Creates and initializes a new uvm_report_handler object.
    uvm_report_handler();

    // Function- get_server
    //
    // Internal method called by <uvm_report_object::get_report_server>.
    uvm_report_server* get_server();

    // Function- set_max_quit_count
    //
    // Internal method called by <uvm_report_object::set_report_max_quit_count>.
    void set_max_quit_count(int max_count);

    // Function- summarize
    //
    // Internal method called by <uvm_report_object::report_summarize>.
    void summarize(UVM_FILE file = 0);

    // Function- report_relnotes_banner
    //
    // Internal method called by <uvm_report_object::report_header>.
    void report_relnotes_banner(UVM_FILE file = 0);

    // Function- report_header
    //
    // Internal method called by <uvm_report_object::report_header>
    void report_header(UVM_FILE file = 0);

    // Function- initialize
    // 
    // This method is called by the constructor to initialize the arrays and
    // other variables described above to their default values.
    void initialize();

    // Function: run_hooks
    //
    // The run_hooks method is called if the <UVM_CALL_HOOK> action is set for a
    // report. It first calls the client's <uvm_report_object::report_hook> method, 
    // followed by the appropriate severity-specific hook method. If either 
    // returns 0, then the report is not processed.
    bool run_hooks(uvm_report_object* client, uvm_severity severity, const std::string& id, const std::string& message, int verbosity, const std::string& filename, int line);
    UVM_FILE get_severity_id_file(uvm_severity severity, const std::string& id);
    void set_verbosity_level(int verbosity_level);
    int get_verbosity_level(uvm_severity severity = UVM_INFO, const std::string& id = "");
    uvm_action get_action(uvm_severity severity, const std::string& id);
    UVM_FILE get_file_handle(uvm_severity severity, const std::string& id);
    void report(uvm_severity severity, const std::string& name, const std::string& id, const std::string& message, int verbosity_level = UVM_MEDIUM, const std::string& filename = "", int line = 0, uvm_report_object* client = nullptr);
    std::string format_action(uvm_action action);
    void set_defaults();
    void set_severity_action(uvm_severity severity, uvm_action action);
    void set_id_action(const std::string& id, uvm_action action);
    void set_severity_id_action(uvm_severity severity, const std::string& id, uvm_action action);
    void set_id_verbosity(const std::string& id, int verbosity);
    void set_severity_id_verbosity(uvm_severity severity, const std::string& id, int verbosity);
    void set_default_file(UVM_FILE file);
    void set_severity_file(uvm_severity severity, UVM_FILE file);
    void set_id_file(const std::string& id, UVM_FILE file);
    void set_severity_id_file(uvm_severity severity, const std::string& id, UVM_FILE file);
    void set_severity_override(uvm_severity cur_severity, uvm_severity new_severity);
    void set_severity_id_override(uvm_severity cur_severity, const std::string& id, uvm_severity new_severity);
    void dump_state();

    int m_max_verbosity_level;

private:
    std::map<uvm_severity, uvm_action> severity_actions;

    uvm_id_actions_array id_actions;
    std::map<uvm_severity, uvm_id_actions_array> severity_id_actions;

    // id verbosity settings : default and severity
    uvm_id_verbosities_array id_verbosities;
    std::map<uvm_severity, uvm_id_verbosities_array> severity_id_verbosities;

    // severity overrides
    uvm_sev_override_array sev_overrides;
    std::map<std::string, uvm_sev_override_array> sev_id_overrides;

    // file handles : default, severity, action, (severity,id)
    UVM_FILE default_file_handle;
    std::map<uvm_severity, UVM_FILE> severity_file_handles;
    uvm_id_file_array id_file_handles;
    std::map<uvm_severity, uvm_id_file_array> severity_id_file_handles;

    static bool m_relnotes_done;
};

#endif // UVM_REPORT_HANDLER_H