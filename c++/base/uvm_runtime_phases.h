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

#ifndef UVM_RUNTIME_PHASES_H
#define UVM_RUNTIME_PHASES_H

#include <string>
#include "base/uvm_task_phase.h"
#include "base/uvm_component.h"
#include "base/uvm_phase.h"

class uvm_pre_reset_phase : public uvm_task_phase {
public:
    static uvm_pre_reset_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_pre_reset_phase(const std::string& name = "pre_reset");
    static uvm_pre_reset_phase* m_inst;
    static const std::string type_name;
};

class uvm_reset_phase : public uvm_task_phase {
public:
    static uvm_reset_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_reset_phase(const std::string& name = "reset");
    static uvm_reset_phase* m_inst;
    static const std::string type_name;
};

class uvm_post_reset_phase : public uvm_task_phase {
public:
    static uvm_post_reset_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_post_reset_phase(const std::string& name = "post_reset");
    static uvm_post_reset_phase* m_inst;
    static const std::string type_name;
};

class uvm_pre_configure_phase : public uvm_task_phase {
public:
    static uvm_pre_configure_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_pre_configure_phase(const std::string& name = "pre_configure");
    static uvm_pre_configure_phase* m_inst;
    static const std::string type_name;
};

class uvm_configure_phase : public uvm_task_phase {
public:
    static uvm_configure_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_configure_phase(const std::string& name = "configure");
    static uvm_configure_phase* m_inst;
    static const std::string type_name;
};

class uvm_post_configure_phase : public uvm_task_phase {
public:
    static uvm_post_configure_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_post_configure_phase(const std::string& name = "post_configure");
    static uvm_post_configure_phase* m_inst;
    static const std::string type_name;
};

class uvm_pre_main_phase : public uvm_task_phase {
public:
    static uvm_pre_main_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_pre_main_phase(const std::string& name = "pre_main");
    static uvm_pre_main_phase* m_inst;
    static const std::string type_name;
};

class uvm_main_phase : public uvm_task_phase {
public:
    static uvm_main_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_main_phase(const std::string& name = "main");
    static uvm_main_phase* m_inst;
    static const std::string type_name;
};

class uvm_post_main_phase : public uvm_task_phase {
public:
    static uvm_post_main_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_post_main_phase(const std::string& name = "post_main");
    static uvm_post_main_phase* m_inst;
    static const std::string type_name;
};

class uvm_pre_shutdown_phase : public uvm_task_phase {
public:
    static uvm_pre_shutdown_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_pre_shutdown_phase(const std::string& name = "pre_shutdown");
    static uvm_pre_shutdown_phase* m_inst;
    static const std::string type_name;
};

class uvm_shutdown_phase : public uvm_task_phase {
public:
    static uvm_shutdown_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_shutdown_phase(const std::string& name = "shutdown");
    static uvm_shutdown_phase* m_inst;
    static const std::string type_name;
};

class uvm_post_shutdown_phase : public uvm_task_phase {
public:
    static uvm_post_shutdown_phase* get();
    virtual void exec_task(uvm_component* comp, uvm_phase* phase);

private:
    uvm_post_shutdown_phase(const std::string& name = "post_shutdown");
    static uvm_post_shutdown_phase* m_inst;
    static const std::string type_name;
};

#endif // UVM_RUNTIME_PHASES_H