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

#ifndef UVM_MISC_H
#define UVM_MISC_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <random>

#include "base/uvm_object_globals.h"

// The uvm_void class is the base class for all UVM classes in C++.
// It is an abstract class with no data members or functions.
class uvm_void {
public:
    virtual ~uvm_void() = default;  // Virtual destructor to ensure proper cleanup of derived classes

    virtual bool randomize(){
        return false;
    }
};

// Define the uvm_apprepend enum
enum uvm_apprepend {
    UVM_APPEND,
    UVM_PREPEND
};

// Assuming these classes are defined elsewhere
class uvm_object;
class uvm_scope_stack;
class uvm_comparer;
class uvm_packer;
class uvm_recorder;
class uvm_printer;

class uvm_status_container {
public:
    // The clone setting is used by the set/get config to know if cloning is on.
    bool clone = true;

    // Information variables used by the macro functions for storage.
    bool warning;
    bool status;
    uvm_bitstream_t bitstream;
    int intv;
    int element;
    std::string stringv;
    std::string scratch1;
    std::string scratch2;
    std::string key;
    uvm_object* object = nullptr;
    bool array_warning_done;

    static std::unordered_map<std::string, bool> field_array;
    static bool print_matches;

    void do_field_check(const std::string &field, uvm_object *obj);
    std::string get_function_type(int what);

    // The scope stack is used for messages that are emitted by policy classes.
    uvm_scope_stack* scope = nullptr;

    std::string get_full_scope_arg();

    // Used for checking cycles. When a data function is entered, if the depth is
    // non-zero, then the existence of the object in the map means that a
    // cycle has occurred and the function should immediately exit. When the
    // function exits, it should reset the cycle map so that there is no memory
    // leak.
    std::unordered_map<uvm_object*, bool> cycle_check;

    // These are the policy objects currently in use. The policy object gets set
    // when a function starts up. The macros use this.
    uvm_comparer* comparer = nullptr;
    uvm_packer* packer = nullptr;
    uvm_recorder* recorder = nullptr;
    uvm_printer* printer = nullptr;

    // Utility function used to perform a cycle check when config settings are pushed
    // to uvm_objects. The function has to look at the current object stack representing
    // the call stack of all __m_uvm_field_automation() invocations.
    // It is only a cycle if the previous __m_uvm_field_automation call scope
    // is not identical with the current scope AND the scope is already present in the
    // object stack.
    std::vector<uvm_object*> m_uvm_cycle_scopes;

    bool m_do_cycle_check(uvm_object *scope);
};

//------------------------------------------------------------------------------
//
// CLASS- uvm_copy_map
//
// Internal class used to map rhs to lhs so when a cycle is found in the rhs,
// the correct lhs object can be bound to it.
//------------------------------------------------------------------------------
class uvm_copy_map {
public:
    void set(uvm_object* key, uvm_object* obj);
    uvm_object* get(uvm_object* key);
    void clear();
    void remove(uvm_object* v);

private:
    std::unordered_map<uvm_object*, uvm_object*> m_map;
};

//------------------------------------------------------------------------------
//
// CLASS- uvm_seed_map
//
// This map is a seed map that can be used to update seeds. The update
// is done automatically by the seed hashing routine. The seed_table_lookup
// uses an instance name lookup and the seed_table inside a given map
// uses a type name for the lookup.
//------------------------------------------------------------------------------
class uvm_seed_map {
public:
    std::unordered_map<std::string, unsigned int> seed_table;
    std::unordered_map<std::string, unsigned int> count;
};

extern std::unordered_map<std::string, uvm_seed_map> uvm_random_seed_table_lookup;

//------------------------------------------------------------------------------
// Internal utility functions
//------------------------------------------------------------------------------
std::string uvm_instance_scope();
unsigned int uvm_oneway_hash(const std::string& string_in, unsigned int seed = 0);
unsigned int uvm_create_random_seed(const std::string& type_id, const std::string& inst_id = "");
std::string uvm_object_value_str(uvm_object* v);
std::string uvm_leaf_scope(const std::string& full_name, char scope_separator = '.');
std::string uvm_vector_to_string(u_int64_t value, int size, uvm_radix_enum radix = UVM_NORADIX, const std::string& radix_str = "");
int uvm_get_array_index_int(const std::string& arg, bool& is_wildcard);
std::string uvm_get_array_index_string(const std::string& arg, bool& is_wildcard);
bool uvm_is_array(const std::string& arg);
bool uvm_has_wildcard(const std::string& arg);

//----------------------------------------------------------------------------
//
// CLASS- uvm_scope_stack
//
//----------------------------------------------------------------------------
class uvm_scope_stack {
public:
    // Constructor
    uvm_scope_stack();

    // Get the depth of the stack
    int depth() const;

    // Get the current scope as a string
    std::string get() const;

    // Get the argument string
    std::string get_arg() const;

    // Set the scope to a new value
    void set(const std::string& s);

    // Move down the scope hierarchy
    void down(const std::string& s);

    // Move down to a specific element in the scope hierarchy
    void down_element(int element);

    // Move up from an element in the scope hierarchy
    void up_element();

    // Move up the scope hierarchy
    void up(const std::string separator = std::string("."));

    // Set the argument string
    void set_arg(const std::string& arg);

    // Set the argument string with a specific element
    void set_arg_element(const std::string& arg, int ele);

    // Unset the argument string
    void unset_arg(const std::string& arg);

private:
    std::string m_arg;
    std::vector<std::string> m_stack;
};

#endif // UVM_MISC_H
