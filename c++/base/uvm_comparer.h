//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Mentor Graphics Corporation
// Copyright 2007-2011 Cadence Design Systems, Inc.
// Copyright 2010 Synopsys, Inc.
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

#ifndef UVM_COMPARER_H
#define UVM_COMPARER_H

#include <string>
#include <iostream>
#include <unordered_map>

#include "base/uvm_misc.h"
#include "base/uvm_globals.h"
#include "base/uvm_object_globals.h"

class uvm_object;

class uvm_comparer {
public:
    // Variable: policy
    //
    // Determines whether comparison is UVM_DEEP, UVM_REFERENCE, or UVM_SHALLOW.
    uvm_recursion_policy_enum policy = UVM_DEFAULT_POLICY;

    // Variable: show_max
    //
    // Sets the maximum number of messages to send to the messenger for miscompares
    // of an object.
    unsigned int show_max = 1;

    // Variable: verbosity
    //
    // Sets the verbosity for printed messages.
    //
    // The verbosity setting is used by the messaging mechanism to determine
    // whether messages should be suppressed or shown.
    unsigned int verbosity = UVM_LOW;

    // Variable: sev
    //
    // Sets the severity for printed messages.
    //
    // The severity setting is used by the messaging mechanism for printing and
    // filtering messages.
    uvm_severity sev = UVM_INFO;

    // Variable: miscompares
    //
    // This string is reset to an empty string when a comparison is started.
    //
    // The string holds the last set of miscompares that occurred during a
    // comparison.
    std::string miscompares;

    // Variable: physical
    //
    // This bit provides a filtering mechanism for fields.
    //
    // The abstract and physical settings allow an object to distinguish between
    // two different classes of fields.
    //
    // It is up to you, in the <uvm_object::do_compare> method, to test the
    // setting of this field if you want to use the physical trait as a filter.
    bool physical = true;

    // Variable: abstract
    //
    // This bit provides a filtering mechanism for fields.
    //
    // The abstract and physical settings allow an object to distinguish between
    // two different classes of fields.
    //
    // It is up to you, in the <uvm_object::do_compare> method, to test the
    // setting of this field if you want to use the abstract trait as a filter.
    bool abstract = true;

    // Variable: check_type
    //
    // This bit determines whether the type, given by <uvm_object::get_type_name>,
    // is used to verify that the types of two objects are the same.
    //
    // This bit is used by the <compare_object> method. In some cases it is useful
    // to set this to 0 when the two operands are related by inheritance but are
    // different types.
    bool check_type = true;

    // Variable: result
    //
    // This bit stores the number of miscompares for a given compare operation.
    // You can use the result to determine the number of miscompares that
    // were found.
    unsigned int result = 0;

    uvm_scope_stack* scope = nullptr;
    uvm_copy_map* compare_map = nullptr;
    int depth = 0; // current depth of objects

    // Constructor
    uvm_comparer();

    // Destructor
    ~uvm_comparer();

    // Function: compare_field
    //
    // Compares two integral values.
    //
    // The ~name~ input is used for purposes of storing and printing a miscompare.
    //
    // The left-hand-side ~lhs~ and right-hand-side ~rhs~ objects are the two
    // objects used for comparison.
    //
    // The size variable indicates the number of bits to compare; size must be
    // less than or equal to 4096.
    //
    // The radix is used for reporting purposes, the default radix is hex.
    virtual bool compare_field(const std::string& name, uvm_bitstream_t lhs, uvm_bitstream_t rhs, int size, uvm_radix_enum radix = UVM_NORADIX);

    // Function: compare_field_int
    //
    // This method is the same as <compare_field> except that the arguments are
    // small integers, less than or equal to 64 bits. It is automatically called
    // by <compare_field> if the operand size is less than or equal to 64.
    virtual bool compare_field_int(const std::string& name, uint64_t lhs, uint64_t rhs, int size, uvm_radix_enum radix = UVM_NORADIX);

    // Function: compare_field_real
    //
    // This method is the same as <compare_field> except that the arguments are
    // real numbers.
    virtual bool compare_field_real(const std::string& name, double lhs, double rhs);

    // Function: compare_object
    //
    // Compares two class objects using the <policy> knob to determine whether the
    // comparison should be deep, shallow, or reference.
    //
    // The name input is used for purposes of storing and printing a miscompare.
    //
    // The ~lhs~ and ~rhs~ objects are the two objects used for comparison.
    //
    // The ~check_type~ determines whether or not to verify the object
    // types match (the return from ~lhs.get_type_name()~ matches
    // ~rhs.get_type_name()~).
    virtual bool compare_object(const std::string& name, uvm_object* lhs, uvm_object* rhs);

    // Function: compare_string
    //
    // Compares two string variables.
    //
    // The ~name~ input is used for purposes of storing and printing a miscompare.
    //
    // The ~lhs~ and ~rhs~ objects are the two objects used for comparison.
    virtual bool compare_string(const std::string& name, const std::string& lhs, const std::string& rhs);

    // Function: print_msg
    //
    // Causes the error count to be incremented and the message, ~msg~, to be
    // appended to the <miscompares> string (a newline is used to separate
    // messages).
    //
    // If the message count is less than the <show_max> setting, then the message
    // is printed to standard-out using the current verbosity and severity
    // settings. See the <verbosity> and <sev> variables for more information.
    void print_msg(const std::string& msg);

    // Internal methods - do not call directly

    // print_rollup
    // Need this function because sformat doesn't support objects
    void print_rollup(uvm_object* rhs, uvm_object* lhs);

    // print_msg_object
    void print_msg_object(uvm_object* lhs, uvm_object* rhs);

    // init ??
    static uvm_comparer* init();
};

#endif // UVM_COMPARER_H