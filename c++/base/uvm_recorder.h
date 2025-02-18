//
//------------------------------------------------------------------------------
// Copyright 2007-2011 Cadence Design Systems, Inc.
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

#ifndef UVM_RECORDER_H
#define UVM_RECORDER_H

#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include "base/uvm_object.h"
#include "base/uvm_factory.h"
#include "base/uvm_registry.h"

//------------------------------------------------------------------------------
//
// CLASS: uvm_recorder
//
// The uvm_recorder class provides a policy object for recording <uvm_objects>.
// The policies determine how recording should be done. 
//
// A default recorder instance, <uvm_default_recorder>, is used when the
// <uvm_object::record> is called without specifying a recorder.
//
//------------------------------------------------------------------------------
class uvm_recorder : public uvm_object {
public:

    UVM_OBJECT_UTILS(uvm_recorder)

    int recording_depth;
    UVM_FILE file = 0;
    std::string filename = "tr_db.log";

    // Variable: tr_handle
    //
    // This is an integral handle to a transaction object. Its use is vendor
    // specific. 
    //
    // A handle of 0 indicates there is no active transaction object. 
    int tr_handle = 0;

    // Variable: default_radix
    //
    // This is the default radix setting if <record_field> is called without
    // a radix.
    uvm_radix_enum default_radix = UVM_HEX;

    // Variable: physical
    //
    // This bit provides a filtering mechanism for fields. 
    //
    // The <abstract> and physical settings allow an object to distinguish between
    // two different classes of fields. 
    //
    // It is up to you, in the <uvm_object::do_record> method, to test the
    // setting of this field if you want to use the physical trait as a filter.
    bool physical = true;

    // Variable: abstract
    //
    // This bit provides a filtering mechanism for fields. 
    //
    // The abstract and physical settings allow an object to distinguish between
    // two different classes of fields. 
    //
    // It is up to you, in the <uvm_object::do_record> method, to test the
    // setting of this field if you want to use the abstract trait as a filter.
    bool abstract = true;

    // Variable: identifier
    //
    // This bit is used to specify whether or not an object's reference should be
    // recorded when the object is recorded. 
    bool identifier = true;

    // Variable: recursion_policy
    //
    // Sets the recursion policy for recording objects. 
    //
    // The default policy is deep (which means to recurse an object).
    uvm_recursion_policy_enum policy = UVM_DEFAULT_POLICY;

    uvm_recorder(const std::string& name = "uvm_recorder");

    // Function: record_field
    //
    // Records an integral field (less than or equal to 4096 bits). ~name~ is the
    // name of the field. 
    //
    // ~value~ is the value of the field to record. ~size~ is the number of bits
    // of the field which apply. ~radix~ is the <uvm_radix_enum> to use.
    virtual void record_field(const std::string& name, uvm_bitstream_t value, int size, uvm_radix_enum radix = UVM_NORADIX);

    // Function: record_field_real
    //
    // Records an real field. ~value~ is the value of the field to record. 
    virtual void record_field_real(const std::string& name, double value);

    // Function: record_object
    //
    // Records an object field. ~name~ is the name of the recorded field. 
    //
    // This method uses the <recursion_policy> to determine whether or not to
    // recurse into the object.
    virtual void record_object(const std::string& name, uvm_object* value);

    // Function: record_string
    //
    // Records a string field. ~name~ is the name of the recorded field.
    virtual void record_string(const std::string& name, const std::string& value);

    // Function: record_time
    //
    // Records a time value. ~name~ is the name to record to the database.
    virtual void record_time(const std::string& name, time_t value);

    // Function: record_generic
    //
    // Records the ~name~-~value~ pair, where ~value~ has been converted
    // to a string. For example:
    //
    //| recorder.record_generic("myvar",$sformatf("%0d",myvar));
    virtual void record_generic(const std::string& name, const std::string& value);

    //------------------------------
    // Group- Vendor-Independent API
    //------------------------------


    // UVM provides only a text-based default implementation.
    // Vendors provide subtype implementations and overwrite the
    // <uvm_default_recorder> handle.


    // Function- open_file
    //
    // Opens the file in the <filename> property and assigns to the
    // file descriptor <file>.
    //
    bool open_file();

    int create_stream(const std::string& name, const std::string& type, const std::string& scope);
    void set_attribute(int txh, const std::string& name, const std::string& value);
    void set_attribute(int txh, const std::string& name, const uvm_bitstream_t& value, uvm_radix_enum radix, int numbits);
    int check_handle_kind(const std::string& htype, int handle);
    int begin_tr(const std::string& txtype, int stream, const std::string& name, const std::string& label = "", const std::string& desc = "", time_t begin_time = 0);
    void end_tr(int handle, time_t end_time = 0);
    void link_tr(int h1, int h2, const std::string& relation = "");
    void free_tr(int handle);

    uvm_scope_stack scope;

    static std::unordered_map<int, bool> m_handles;
    static int handle;
};

#endif // UVM_RECORDER_H
