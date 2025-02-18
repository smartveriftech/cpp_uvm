//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Mentor Graphics Corporation
// Copyright 2007-2011 Cadence Design Systems, Inc. 
// Copyright 2010-2011 Synopsys, Inc.
// Copyright 2013-2014 NVIDIA Corporation
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

#ifndef UVM_VERSION_H
#define UVM_VERSION_H

#include <string>

constexpr const char* uvm_mgc_copyright = "(C) 2007-2012 Mentor Graphics Corporation";
constexpr const char* uvm_cdn_copyright = "(C) 2007-2012 Cadence Design Systems, Inc.";
constexpr const char* uvm_snps_copyright = "(C) 2006-2012 Synopsys, Inc.";
constexpr const char* uvm_cy_copyright = "(C) 2011-2012 Cypress Semiconductor Corp.";

constexpr const char* uvm_revision = "UVM-1.2 CPP";

inline std::string uvm_revision_string(){
  return std::string(uvm_revision);
};

#endif // UVM_VERSION_H