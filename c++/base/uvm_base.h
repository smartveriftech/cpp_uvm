//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Cadence Design Systems, Inc.
// Copyright 2007-2011 Mentor Graphics Corporation
// Copyright 2010 Synopsys, Inc.
// Copyright 2013 NVIDIA Corporation
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

//
//----------------------------------------------------------------------
#ifndef UVM_BASE_H
#define UVM_BASE_H

  class uvm_cmdline_processor;

  // Miscellaneous classes and functions. uvm_void is defined in uvm_misc,
  // along with some auxillary functions that UVM needs but are not really
  // part of UVM.
  #include "base/uvm_version.h"
  #include "base/uvm_object_globals.h"
  #include "base/uvm_misc.h"

//#ifdef UVM_USE_RESOURCE_CONVERTER
  //#include "deprecated/uvm_type_utils.h"
//#endif

  // The base object element. Contains data methods (<copy>, <compare> etc) and
  // factory creation methods (<create>). Also includes control classes.
  #include "base/uvm_object.h"

  #include "base/uvm_pool.h"
  #include "base/uvm_queue.h"

  #include "base/uvm_factory.h"
  #include "base/uvm_registry.h"


  // Resources/configuration facility
  //#include "base/uvm_spell_chkr.h"
  //#include "base/uvm_resource.h"

//#ifdef UVM_USE_RESOURCE_CONVERTER
//  #include "deprecated/uvm_resource_converter.h"
//#endif

  //#include "base/uvm_resource_specializations.h"
  #include "base/uvm_resource_db.h"
  #include "base/uvm_config_db.h"

  // Policies
  #include "base/uvm_printer.h"
  #include "base/uvm_comparer.h"
  #include "base/uvm_packer.h"
  #include "base/uvm_recorder.h"

  // Event interface
  //#include "base/uvm_event_callback.h"
  //#include "base/uvm_event.h"
  //#include "base/uvm_barrier.h"

  // Callback interface
  //#include "base/uvm_callback.h"

  // Reporting interface
  //#include "base/uvm_report_catcher.h"
  //#include "base/uvm_report_server.h"
  //#include "base/uvm_report_handler.h"
  //#include "base/uvm_report_object.h"

  // Base transaction object
  #include "base/uvm_transaction.h"

  // The phase declarations
  //#include "base/uvm_phase.h"
  //#include "base/uvm_domain.h"
  //#include "base/uvm_bottomup_phase.h"
  //#include "base/uvm_topdown_phase.h"
  //#include "base/uvm_task_phase.h"
  //#include "base/uvm_common_phases.h"
  //#include "base/uvm_runtime_phases.h"

  #include "base/uvm_component.h"

  // Objection interface
  //#include "base/uvm_objection.h"
  #include "base/uvm_heartbeat.h"

  #include "base/uvm_globals.h"

  // Command Line Processor
  #include "base/uvm_cmdline_processor.h"
  
#endif // UVM_BASE_H