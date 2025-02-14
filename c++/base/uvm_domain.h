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

#ifndef UVM_DOMAIN_H
#define UVM_DOMAIN_H

#include <string>
#include <unordered_map>
#include <vector>
#include "base/uvm_phase.h"

class uvm_build_phase;
class uvm_connect_phase;
class uvm_end_of_elaboration_phase;
class uvm_start_of_simulation_phase;
class uvm_run_phase;
class uvm_extract_phase;
class uvm_check_phase;
class uvm_report_phase;
class uvm_final_phase;

class uvm_pre_reset_phase;
class uvm_reset_phase;
class uvm_post_reset_phase;
class uvm_pre_configure_phase;
class uvm_configure_phase;
class uvm_post_configure_phase;
class uvm_pre_main_phase;
class uvm_main_phase;
class uvm_post_main_phase;
class uvm_pre_shutdown_phase;
class uvm_shutdown_phase;
class uvm_post_shutdown_phase;

extern uvm_phase* build_ph;
extern uvm_phase* connect_ph;
extern uvm_phase* end_of_elaboration_ph;
extern uvm_phase* start_of_simulation_ph;
extern uvm_phase* run_ph;
extern uvm_phase* extract_ph;
extern uvm_phase* check_ph;
extern uvm_phase* report_ph;

//------------------------------------------------------------------------------
//
// Class: uvm_domain
//
//------------------------------------------------------------------------------
//
// Phasing schedule node representing an independent branch of the schedule.
// Handle used to assign domains to components or hierarchies in the testbench
//

class uvm_domain : public uvm_phase {
public:

    // Function: get_domains
    //
    // Provides a list of all domains in the provided ~domains~ argument. 
    //
    static void get_domains(std::unordered_map<std::string, uvm_domain*>& domains);

    // Function: get_uvm_schedule
    //
    // Get the "UVM" schedule, which consists of the run-time phases that
    // all components execute when participating in the "UVM" domain.
    //
    static uvm_phase* get_uvm_schedule();

    // Function: get_common_domain
    //
    // Get the "common" domain, which consists of the common phases that
    // all components execute in sync with each other. Phases in the "common"
    // domain are build, connect, end_of_elaboration, start_of_simulation, run,
    // extract, check, report, and final.
    //
    static uvm_domain* get_common_domain();


    // Function: add_uvm_phases
    //
    // Appends to the given ~schedule~ the built-in UVM phases.
    //
    static void add_uvm_phases(uvm_phase* schedule);

    // Function: get_uvm_domain
    //
    // Get a handle to the singleton ~uvm~ domain
    //
    static uvm_domain* get_uvm_domain();
    
    // Function: new
    //
    // Create a new instance of a phase domain.
    uvm_domain(const std::string& name);

    // Function: jump
    //
    // jumps all active phases of this domain to to-phase if
    // there is a path between active-phase and to-phase
    void jump(uvm_phase* phase);

    // jump_all
    // --------
    static void jump_all(uvm_phase* phase);

private:
    static uvm_domain* m_common_domain;
    static uvm_domain* m_uvm_domain; // run-time phases
    static std::unordered_map<std::string, uvm_domain*> m_domains;
    static uvm_phase* m_uvm_schedule;
};

#endif // UVM_DOMAIN_H