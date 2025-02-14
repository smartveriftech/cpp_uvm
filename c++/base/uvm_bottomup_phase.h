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

#ifndef UVM_BOTTOMUP_PHASE_H
#define UVM_BOTTOMUP_PHASE_H

#include <string>

#include "base/uvm_phase.h"
#include "base/uvm_object_globals.h"

class uvm_component;

class uvm_bottomup_phase : public uvm_phase {
public:
    uvm_bottomup_phase(const std::string& name);

    virtual void traverse(uvm_component* comp, uvm_phase* phase, uvm_phase_state state);

protected:
    virtual void execute(uvm_component* comp, uvm_phase* phase);
};

#endif // UVM_BOTTOMUP_PHASE_H