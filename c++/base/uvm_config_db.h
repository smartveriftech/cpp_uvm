//
//------------------------------------------------------------------------------
// Copyright 2010-2011 Mentor Graphics Corporation
// Copyright 2011 Cypress Semiconductor
// Copyright 2014 NVIDIA Corporation
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

#ifndef UVM_CONFIG_DB_H
#define UVM_CONFIG_DB_H

#include <string>
#include <list>
#include <unordered_map>
#include <regex>
#include <iostream>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include <cstring> // For std::strchr

#include "base/uvm_resource_db.h"
#include "base/uvm_event.h"
#include "base/uvm_component.h"
#include "base/uvm_cmdline_processor.h"
#include "base/uvm_phase.h"
#include "base/uvm_root.h"

//----------------------------------------------------------------------
// Title: UVM Configuration Database
//
// Topic: Intro
//
// The <uvm_config_db> class provides a convenience interface 
// on top of the <uvm_resource_db> to simplify the basic interface
// that is used for configuring <uvm_component> instances.
//
// If the run-time ~+UVM_CONFIG_DB_TRACE~ command line option is specified,
// all configuration DB accesses (read and write) are displayed.
//----------------------------------------------------------------------

// Internal class for config waiters
class m_uvm_waiter {
public:
    std::string inst_name;
    std::string field_name;
    std::condition_variable trigger;

    m_uvm_waiter(const std::string& inst_name, const std::string& field_name)
        : inst_name(inst_name), field_name(field_name) {}
};

//----------------------------------------------------------------------
// Class: uvm_config_db_options
//
// Provides a namespace for managing options for the
// configuration DB facility.  The only thing allowed in this class is static
// local data members and static functions for manipulating and
// retrieving the value of the data members.  The static local data
// members represent options and settings that control the behavior of
// the configuration DB facility.
//
// Options include:
//
//  * tracing:  on/off
//
//    The default for tracing is off.
//
//----------------------------------------------------------------------
class uvm_config_db_options {
public:

    // Function: turn_on_tracing
    //
    // Turn tracing on for the configuration database. This causes all
    // reads and writes to the database to display information about
    // the accesses. Tracing is off by default.
    //
    // This method is implicitly called by the ~+UVM_CONFIG_DB_TRACE~.
    static void turn_on_tracing();

    // Function: turn_off_tracing
    //
    // Turn tracing off for the configuration database.
    static void turn_off_tracing();

    // Function: is_tracing
    //
    // Returns 1 if the tracing facility is on and 0 if it is off.
    static bool is_tracing();

    static void init();

private:
    static bool ready;
    static bool tracing;
};

//----------------------------------------------------------------------
// class: uvm_config_db
//
// All of the functions in uvm_config_db#(T) are static, so they
// must be called using the :: operator. For example:
//
//|  uvm_config_db#(int)::set(this, "*", "A");
//
// The parameter value "int" identifies the configuration type as
// an int property.  
//
// The <set> and <get> methods provide the same API and
// semantics as the set/get_config_* functions in <uvm_component>.
//----------------------------------------------------------------------
template <typename T>
class uvm_config_db : public uvm_resource_db<T> {
public:
    static bool get(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name, T& value);
    static void set(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name, const T& value);
    static bool exists(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name, bool spell_chk = false);
    static void wait_modified(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name);

private:
    static uvm_resource<T>* m_get_resource_match(uvm_component* cntxt, const std::string& field_name, const std::string& scope);
    static std::unordered_map<uvm_component*, std::unordered_map<std::string, uvm_resource<T>*>> m_rsc;
    static std::unordered_map<std::string, std::list<m_uvm_waiter*>> m_waiters;
    static std::mutex m_mtx;
};

// Static member definitions
template <typename T>
std::unordered_map<uvm_component*, std::unordered_map<std::string, uvm_resource<T>*>> uvm_config_db<T>::m_rsc;

template <typename T>
std::unordered_map<std::string, std::list<m_uvm_waiter*>> uvm_config_db<T>::m_waiters;

template <typename T>
std::mutex uvm_config_db<T>::m_mtx;

// Function: get
//
// Get the value for ~field_name~ in ~inst_name~, using component ~cntxt~ as 
// the starting search point. ~inst_name~ is an explicit instance name 
// relative to ~cntxt~ and may be an empty string if the ~cntxt~ is the
// instance that the configuration object applies to. ~field_name~
// is the specific field in the scope that is being searched for.
//
// The basic get_config_* methods from <uvm_component> are mapped to 
// this function as:
//
//| get_config_int(...) => uvm_config_db#(int)::get(cntxt,...)
//| get_config_string(...) => uvm_config_db#(std::string)::get(cntxt,...)
//| get_config_object(...) => uvm_config_db#(uvm_object)::get(cntxt,...)
template <typename T>
bool uvm_config_db<T>::get(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name, T& value) {
    uvm_resource_pool* rp = uvm_resource_pool::get();
    std::vector<uvm_resource_base*> lookup_result;
    uvm_resource<T>* r = nullptr;

    if (cntxt == nullptr) 
        cntxt = uvm_root::get();
    
    std::string full_inst_name = cntxt->get_full_name();
    if (!inst_name.empty()) 
        full_inst_name += "." + inst_name;

    std::string scope = full_inst_name;
    std::string name = field_name;

    uvm_info("CFG_DB_GET", "Attempting to get configuration with scope: " + scope + " and name: " + name, UVM_FULL);

    // Perform the lookup using scope and name
    lookup_result = rp->lookup_regex_names(scope, name, uvm_resource<T>::get_type());

    for (auto& res : lookup_result) {
        if ((r = dynamic_cast<uvm_resource<T>*>(res)) != nullptr)
            break;
    }

    if (uvm_config_db_options::is_tracing())
        uvm_resource_db<T>::m_show_msg("CFGDB/GET", "Configuration", "read", scope, name, cntxt, r);

    if (r == nullptr) {
        uvm_info("CFG_DB_GET_FAIL", "Configuration lookup failed for: " + scope + "." + name, UVM_FULL);
        return false;
    }

    // Read the value from the resource
    try {
        value = r->read(cntxt);
    } catch (const std::out_of_range& e) {
        uvm_error("CFG_DB_READ_EXCEPTION", "Exception during read: " + std::string(e.what()));
        return false;
    }
    return true;
}

// Function: set 
//
// Create a new or update an existing configuration setting for
// ~field_name~ in ~inst_name~ from ~cntxt~.
// The setting is made at ~cntxt~, with the full name of ~cntxt~ 
// added to the ~inst_name~. If ~cntxt~ is null then ~inst_name~
// provides the complete scope information of the setting.
// ~field_name~ is the target field. Both ~inst_name~ and ~field_name~
// may be glob style or regular expression style expressions.
//
// If a setting is made at build time, the ~cntxt~ hierarchy is
// used to determine the setting's precedence in the database.
// Settings from hierarchically higher levels have higher
// precedence. Settings from the same level of hierarchy have
// a last setting wins semantic. A precedence setting of 
// <uvm_resource_base::default_precedence>  is used for uvm_top, and 
// each hierarcical level below the top is decremented by 1.
//
// After build time, all settings use the default precedence and thus
// have a last wins semantic. So, if at run time, a low level 
// component makes a runtime setting of some field, that setting 
// will have precedence over a setting from the test level that was 
// made earlier in the simulation.
//
// The basic set_config_* methods from <uvm_component> are mapped to 
// this function as:
//
//| set_config_int(...) => uvm_config_db#(int)::set(cntxt,...)
//| set_config_string(...) => uvm_config_db#(std::string)::set(cntxt,...)
//| set_config_object(...) => uvm_config_db#(uvm_object)::set(cntxt,...)
template <typename T>
void uvm_config_db<T>::set(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name, const T& value) {
    uvm_root* top = uvm_root::get();
    uvm_phase* curr_phase = top->m_current_phase;
    uvm_resource<T>* r = nullptr;
    bool exists = false;

    if (cntxt == nullptr) 
        cntxt = top;
    
    std::string full_inst_name = cntxt->get_full_name();
    if (!inst_name.empty()) 
        full_inst_name += "." + inst_name;

    std::string scope = full_inst_name;
    std::string name = field_name;

    uvm_info("CFG_DB_SET", "Storing configuration with scope: " + scope + " and name: " + name, UVM_FULL);

    r = m_get_resource_match(cntxt, name, scope);
    
    if (r == nullptr) {
        auto& pool = m_rsc[cntxt];
        r = new uvm_resource<T>(field_name, scope); // Pass scope without field name
        pool[name] = r;
        std::string full_key = scope + "." + name;
        uvm_info("CFG_DB_SET_NEW", "Configuration stored with key: " + full_key, UVM_FULL);
        
        // Escape the full_key for regex
        std::string escaped_key = uvm_escape_regex(full_key);
        
        // Register the resource in the resource pool with escaped regex
        uvm_resource_pool::get()->register_resource(escaped_key, r);
    } else {
        exists = true;
    }

    if (curr_phase != nullptr && curr_phase->get_name() == "build")
        r->precedence -= cntxt->get_depth();

    r->write(value, cntxt);

    if (exists) {
        uvm_resource_pool* rp = uvm_resource_pool::get();
        rp->set_priority_name(r, uvm_resource_types::PRI_HIGH);
    } else {
        r->set_override();
    }

    if (uvm_config_db_options::is_tracing())
        uvm_resource_db<T>::m_show_msg("CFGDB/SET", "Configuration", "set", scope, name, cntxt, r);
}

// Function: exists
//
// Check if a configuration exists for ~field_name~ in ~inst_name~ 
// relative to ~cntxt~. If ~spell_chk~ is true, perform a spell check
// to suggest possible corrections if the exact name is not found.
//
// The basic exists_config_* methods from <uvm_component> are mapped to 
// this function as:
//
//| exists_config_int(...) => uvm_config_db#(int)::exists(cntxt,...)
//| exists_config_string(...) => uvm_config_db#(std::string)::exists(cntxt,...)
//| exists_config_object(...) => uvm_config_db#(uvm_object)::exists(cntxt,...)
template <typename T>
bool uvm_config_db<T>::exists(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name, bool spell_chk) {
    if (cntxt == nullptr)
        cntxt = uvm_root::get();
    
    std::string full_inst_name = cntxt->get_full_name();
    if (!inst_name.empty()) 
        full_inst_name += "." + inst_name;

    bool config_exists = uvm_resource_db<T>::get_by_name(full_inst_name, field_name, spell_chk) != nullptr;

    if (uvm_config_db_options::is_tracing()) {
        if (config_exists) {
            uvm_info("CFG_DB_EXISTS", "Configuration exists for: " + full_inst_name + "." + field_name, UVM_NONE);
        } else {
            uvm_info("CFG_DB_NOT_EXISTS", "Configuration does not exist for: " + full_inst_name + "." + field_name, UVM_NONE);
        }
    }

    return config_exists;
}

// Function: wait_modified
//
// Wait until the configuration for ~field_name~ in ~inst_name~
// is modified. This is useful for synchronization between different 
// components that depend on the configuration settings.
//
// The basic wait_config_modified_* methods from <uvm_component> are mapped to 
// this function as:
//
//| wait_config_modified_int(...) => uvm_config_db#(int)::wait_modified(cntxt,...)
//| wait_config_modified_string(...) => uvm_config_db#(std::string)::wait_modified(cntxt,...)
//| wait_config_modified_object(...) => uvm_config_db#(uvm_object)::wait_modified(cntxt,...)
template <typename T>
void uvm_config_db<T>::wait_modified(uvm_component* cntxt, const std::string& inst_name, const std::string& field_name) {
    std::unique_lock<std::mutex> lk(m_mtx);
    std::string full_inst_name = cntxt->get_full_name();
    if (!inst_name.empty()) 
        full_inst_name += "." + inst_name;

    m_uvm_waiter waiter(full_inst_name, field_name);
    m_waiters[field_name].push_back(&waiter);
    waiter.trigger.wait(lk);

    auto& waiters = m_waiters[field_name];
    for (auto it = waiters.begin(); it != waiters.end(); ++it) {
        if (*it == &waiter) {
            waiters.erase(it);
            break;
        }
    }

    uvm_info("CFG_DB_WAIT_MODIFIED", "Configuration modified for: " + full_inst_name + "." + field_name, UVM_FULL);
}

// Helper function to match resources
template <typename T>
uvm_resource<T>* uvm_config_db<T>::m_get_resource_match(uvm_component* cntxt, const std::string& field_name, const std::string& scope) {
    auto it = m_rsc.find(cntxt);
    if (it != m_rsc.end()) {
        auto& pool = it->second;
        std::string key = field_name; // Correct key formation as per set function
        auto resource_it = pool.find(field_name);
        if (resource_it != pool.end()) {
            return resource_it->second; // Return the raw pointer to the resource
        }
    }
    return nullptr; // Resource not found
}

#endif // UVM_CONFIG_DB_H