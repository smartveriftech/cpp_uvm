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

#ifndef UVM_COMMON_PHASES_H
#define UVM_COMMON_PHASES_H

#include "base/uvm_phase.h"
#include "base/uvm_topdown_phase.h"
#include "base/uvm_bottomup_phase.h"
#include "base/uvm_task_phase.h"

class uvm_build_phase : public uvm_topdown_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_build_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_build_phase(const std::string& name = "build");
    static uvm_build_phase* m_inst;
};

class uvm_connect_phase : public uvm_bottomup_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_connect_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_connect_phase(const std::string& name = "connect");
    static uvm_connect_phase* m_inst;
};

class uvm_end_of_elaboration_phase : public uvm_bottomup_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_end_of_elaboration_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_end_of_elaboration_phase(const std::string& name = "end_of_elaboration");
    static uvm_end_of_elaboration_phase* m_inst;
};

class uvm_start_of_simulation_phase : public uvm_bottomup_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_start_of_simulation_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_start_of_simulation_phase(const std::string& name = "start_of_simulation");
    static uvm_start_of_simulation_phase* m_inst;
};

class uvm_run_phase : public uvm_task_phase {
public:
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);
    static uvm_run_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_run_phase(const std::string& name = "run");
    static uvm_run_phase* m_inst;
};

class uvm_extract_phase : public uvm_bottomup_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_extract_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_extract_phase(const std::string& name = "extract");
    static uvm_extract_phase* m_inst;
};

class uvm_check_phase : public uvm_bottomup_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_check_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_check_phase(const std::string& name = "check");
    static uvm_check_phase* m_inst;
};

class uvm_report_phase : public uvm_bottomup_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_report_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_report_phase(const std::string& name = "report");
    static uvm_report_phase* m_inst;
};

class uvm_final_phase : public uvm_topdown_phase {
public:
    virtual void exec_func(uvm_component* comp, uvm_phase* phase);
    static uvm_final_phase* get();
    virtual std::string get_type_name();
    static const std::string type_name;
private:
    uvm_final_phase(const std::string& name = "final");
    static uvm_final_phase* m_inst;
};

#endif // UVM_COMMON_PHASES_H