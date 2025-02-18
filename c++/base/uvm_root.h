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

#ifndef UVM_ROOT_H
#define UVM_ROOT_H

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include "base/uvm_component.h"
#include "base/uvm_printer.h"
#include "base/uvm_cmdline_processor.h"
#include "base/uvm_report_handler.h"
#include "time_proc/uvm_delay_process.h"

class uvm_phase;

// Custom exception for scope locking errors
class uvm_scope_error : public std::runtime_error {
public:
    explicit uvm_scope_error(const std::string& msg) : std::runtime_error(msg) {}
};

//------------------------------------------------------------------------------
//
// CLASS: uvm_root
//
// The ~uvm_root~ class serves as the implicit top-level and phase controller for
// all UVM components. Users do not directly instantiate ~uvm_root~. The UVM 
// automatically creates a single instance of <uvm_root> that users can
// access via the global (uvm_pkg-scope) variable, ~uvm_top~. 
// 
// (see uvm_ref_root.gif)
// 
// The ~uvm_top~ instance of ~uvm_root~ plays several key roles in the UVM.
// 
// Implicit top-level - The ~uvm_top~ serves as an implicit top-level component.
// Any component whose parent is specified as NULL becomes a child of ~uvm_top~. 
// Thus, all UVM components in simulation are descendants of ~uvm_top~.
//
// Phase control - ~uvm_top~ manages the phasing for all components.
//
// Search - Use ~uvm_top~ to search for components based on their
// hierarchical name. See <find> and <find_all>.
//
// Report configuration - Use ~uvm_top~ to globally configure
// report verbosity, log files, and actions. For example,
// ~uvm_top.set_report_verbosity_level_hier(UVM_FULL)~ would set
// full verbosity for all components in simulation.
//
// Global reporter - Because ~uvm_top~ is globally accessible (in uvm_pkg
// scope), UVM's reporting mechanism is accessible from anywhere
// outside ~uvm_component~, such as in modules and sequences.
// See <uvm_report_error>, <uvm_report_warning>, and other global
// methods.
//
//
// The ~uvm_top~ instance checks during the end_of_elaboration phase if any errors have 
// been generated so far. If errors are found an UVM_FATAL error is being generated as result 
// so that the simulation will not continue to the start_of_simulation_phase.
// 
//------------------------------------------------------------------------------

class uvm_root : public uvm_component {
protected:
    int sim_step;

public:
    static uvm_root* get();

    ////////////////////////////////////////////////////////////
    // Seems like no need any more, might delete later
    ////////////////////////////////////////////////////////////
    /*
    // Process management methods with scope control
    bool lock(const std::string& scope);
    void add_process(std::shared_ptr<uvm_delay_process> process);
    void run_1step(const std::string& scope);

    virtual void set_sim_step(int sim_step);
    */
    
    // Task: run_test
    //
    // Phases all components through all registered phases. If the optional
    // test_name argument is provided, or if a command-line plusarg,
    // +UVM_TESTNAME=TEST_NAME, is found, then the specified component is created
    // just prior to phasing. The test may contain new verification components or
    // the entire testbench, in which case the test and testbench can be chosen from
    // the command line without forcing recompilation. If the global (package)
    // variable, finish_on_completion, is set, then $finish is called after
    // phasing completes.
    virtual void run_test(const std::string& test_name = "");

    // Variable: top_levels
    //
    // This variable is a list of all of the top level components in UVM. It
    // includes the uvm_test_top component that is created by <run_test> as
    // well as any other top level components that have been instantiated
    // anywhere in the hierarchy.
    std::vector<uvm_component*> top_levels;

    uvm_component* find(const std::string& comp_match);

    // Function: find_all
    //
    // Returns the component handle (find) or list of components handles
    // (find_all) matching a given string. The string may contain the wildcards,
    // * and ?. Strings beginning with '.' are absolute path names. If optional
    // comp arg is provided, then search begins from that component down
    // (default=all components).
    void find_all(const std::string& comp_match, std::vector<uvm_component*>& comps, uvm_component* comp = nullptr);

    // Function: print_topology
    //
    // Print the verification environment's component topology. The
    // ~printer~ is a <uvm_printer> object that controls the format
    // of the topology printout; a ~null~ printer prints with the
    // default output.
    void print_topology(uvm_printer* printer = nullptr);

    // Variable: enable_print_topology
    //
    // If set, then the entire testbench topology is printed just after completion
    // of the end_of_elaboration phase.
    bool enable_print_topology = false;

    // Variable: finish_on_completion
    //
    // If set, then run_test will call $finish after all phases are executed. 
    bool finish_on_completion = true;

    // Variable- phase_timeout
    //
    // Specifies the timeout for task-based phases. Default is 0, or no timeout.
    int phase_timeout = UVM_DEFAULT_TIMEOUT;

    // Function: set_timeout
    //
    // Specifies the timeout for task-based phases. Default is 0, i.e. no timeout.
    void set_timeout(time_t timeout, bool overridable = true);

#ifndef UVM_NO_DEPRECATED
    // stop_request
    // ------------

    // backward compat only 
    // call global_stop_request() or uvm_test_done.stop_request() instead
    void stop_request();
#endif

    void build_phase(uvm_phase* phase);
    virtual void run_phase(uvm_phase* phase);
    virtual void phase_started(uvm_phase* phase);

protected:
    uvm_root();
    virtual ~uvm_root() = default;

private:
    // Process management queue
    std::queue<std::shared_ptr<uvm_delay_process>> m_process_queue;
    
    // Scope control
    std::string m_locked_scope;
    bool m_is_locked = false;
    
    void m_find_all_recurse(const std::string& comp_match, std::vector<uvm_component*>& comps, uvm_component* comp = nullptr);
    bool m_add_child(uvm_component* child);
    void m_do_verbosity_settings();
    void m_do_timeout_settings();
    void m_do_factory_settings();
    void m_process_inst_override(const std::string& ovr);
    void m_process_type_override(const std::string& ovr);
    void m_do_config_settings();
    void m_do_max_quit_settings();
    void m_do_dump_args();
    void m_process_config(const std::string& cfg, bool is_int);
    void m_check_verbosity();

    static uvm_root* m_inst;
    
    //bool m_phase_all_done;

    uvm_cmdline_processor* clp;
};

//------------------------------------------------------------------------------
// Variable: uvm_top
//
// This is the top-level that governs phase execution and provides component
// search interface. See <uvm_root> for more information.
//------------------------------------------------------------------------------
extern uvm_root* uvm_top;

// for backward compatibility
extern uvm_root* _global_reporter;

//-----------------------------------------------------------------------------
//
// Class- uvm_root_report_handler
//
//-----------------------------------------------------------------------------
// Root report has name "reporter"
class uvm_root_report_handler : public uvm_report_handler {
public:
    uvm_root_report_handler();
    virtual void report(uvm_severity severity,
                        const std::string& name,
                        const std::string& id,
                        const std::string& message,
                        int verbosity_level = UVM_MEDIUM,
                        const std::string& filename = "",
                        int line = 0,
                        uvm_report_object* client = nullptr);
};

#endif // UVM_ROOT_H