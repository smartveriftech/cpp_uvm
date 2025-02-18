//
//------------------------------------------------------------------------------
// Copyright 2010 Mentor Graphics Corporation
// Copyright 2011 Cadence Design Systems, Inc. 
// Copyright 2011 Cypress Semiconductor
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

#ifndef UVM_RESOURCE_H
#define UVM_RESOURCE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>
#include <ctime>
#include <memory>

#include "base/uvm_object.h"
#include "base/uvm_regex.h"

//----------------------------------------------------------------------
// Title: Resources
//
// Topic: Intro
//
// A resource is a parameterized container that holds arbitrary data.
// Resources can be used to configure components, supply data to
// sequences, or enable sharing of information across disparate parts of
// a testbench.  They are stored using scoping information so their
// visibility can be constrained to certain parts of the testbench.
// Resource containers can hold any type of data, constrained only by
// the data types available in SystemVerilog.  Resources can contain
// scalar objects, class handles, queues, lists, or even virtual
// interfaces.
//
// Resources are stored in a resource database so that each resource can
// be retrieved by name or by type. The databse has both a name table
// and a type table and each resource is entered into both. The database
// is globally accessible.
//
// Each resource has a set of scopes over which it is visible.  The set
// of scopes is represented as a regular expression.  When a resource is
// looked up the scope of the entity doing the looking up is supplied to
// the lookup function.  This is called the ~current scope~.  If the
// current scope is in the set of scopes over which a resource is
// visible then the resource can be retuned in the lookup.
//
// Resources can be looked up by name or by type. To support type lookup
// each resource has a static type handle that uniquely identifies the
// type of each specialized resource container.
//
// Mutliple resources that have the same name are stored in a queue.
// Each resource is pushed into a queue with the first one at the front
// of the queue and each subsequent one behind it.  The same happens for
// multiple resources that have the same type.  The resource queues are
// searched front to back, so those placed earlier in the queue have
// precedence over those placed later.
//
// The precedence of resources with the same name or same type can be
// altered.  One way is to set the ~precedence~ member of the resource
// container to any arbitrary value.  The search algorithm will return
// the resource with the highest precedence.  In the case where there
// are multiple resources that match the search criteria and have the
// same (highest) precedence, the earliest one located in the queue will
// be one returned.  Another way to change the precedence is to use the
// set_priority function to move a resource to either the front or back
// of the queue.
//
// The classes defined here form the low level layer of the resource
// database.  The classes include the resource container and the database
// that holds the containers.  The following set of classes are defined
// here:
//
// <uvm_resource_types>: A class without methods or members, only
// typedefs and enums. These types and enums are used throughout the
// resources facility.  Putting the types in a class keeps them confined
// to a specific name space.
//
// <uvm_resource_options>: policy class for setting options, such
// as auditing, which effect resources.
//
// <uvm_resource_base>: the base (untyped) resource class living in the
// resource database.  This class includes the interface for setting a
// resource as read-only, notification, scope management, altering
// search priority, and managing auditing.
//
// <uvm_resource#(T)>: parameterized resource container.  This class
// includes the interfaces for reading and writing each resource.
// Because the class is parameterized, all the access functions are type
// safe.
//
// <uvm_resource_pool>: the resource database. This is a singleton
// class object.
//----------------------------------------------------------------------

// Forward reference
class uvm_resource_base;

//----------------------------------------------------------------------
// Class: uvm_resource_types
//
// Provides typedefs and enums used throughout the resources facility.
// This class has no members or methods, only typedefs. It's used in
// lieu of package-scope types. When needed, other classes can use
// these types by prefixing their usage with uvm_resource_types::.
//
//----------------------------------------------------------------------
class uvm_resource_types {
public:
    // types used for setting overrides
    typedef uint8_t override_t;
    enum override_e { TYPE_OVERRIDE = 0x01, NAME_OVERRIDE = 0x02 };

    // general-purpose queue of resources
    typedef std::vector<uvm_resource_base*> rsrc_q_t;

    // enum for setting resource search priority
    enum priority_e { PRI_HIGH, PRI_LOW };

    // access record for resources. A set of these is stored for each
    // resource by accessing object. It's updated for each read/write.
    struct access_t {
        time_t read_time;
        time_t write_time;
        unsigned int read_count;
        unsigned int write_count;
    };
};

//----------------------------------------------------------------------
// Class: uvm_resource_options
//
// Provides a namespace for managing options for the
// resources facility.  The only thing allowed in this class is static
// local data members and static functions for manipulating and
// retrieving the value of the data members.  The static local data
// members represent options and settings that control the behavior of
// the resources facility.
//
// Options include:
//
//  * auditing:  on/off
//
//    The default for auditing is on.  You may wish to turn it off to
//    for performance reasons.  With auditing off memory is not
//    consumed for storage of auditing information and time is not
//    spent collecting and storing auditing information.  Of course,
//    during the period when auditing is off no audit trail information
//    is available
//
//----------------------------------------------------------------------
class uvm_resource_options {
private:
    static bool auditing;

public:
    // Turn auditing on for the resource database. This causes all
    // reads and writes to the database to store information about
    // the accesses. Auditing is turned on by default.
    static void turn_on_auditing() { auditing = true; }

    // Turn auditing off for the resource database. If auditing is turned off,
    // it is not possible to get extra information about resource
    // database accesses.
    static void turn_off_auditing() { auditing = false; }

    // Returns true if the auditing facility is on and false if it is off.
    static bool is_auditing() { return auditing; }
};

//----------------------------------------------------------------------
// Class: uvm_resource_base
//
// Non-parameterized base class for resources. Supports interfaces for
// scope matching, and virtual functions for printing the resource and
// for printing the accessor list
//----------------------------------------------------------------------
class uvm_resource_base {
protected:
    std::string scope;
    std::string name; // Added to store the resource's name
    bool modified;
    bool read_only;

public:
    unsigned int precedence;
    static unsigned int default_precedence;
    bool m_is_regex_name;
    std::unordered_map<std::string, uvm_resource_types::access_t> access;

    uvm_resource_base(std::string name = "", std::string s = "*")
        : name(name), scope(s), modified(false), read_only(false),
          precedence(default_precedence),
          m_is_regex_name(false) {
        set_scope(s);
        if (uvm_has_wildcard(name)) m_is_regex_name = true;
    }

    virtual ~uvm_resource_base() {};

    virtual uvm_resource_base* get_type_handle() const = 0;

    void set_read_only() { read_only = true; }

    void set_read_write() { read_only = false; }

    bool is_read_only() const { return read_only; }

    void wait_modified() {
        while (!modified);
        modified = false;
    }

    void set_scope(const std::string& s) { scope = uvm_glob_to_re(s); }

    std::string get_scope() const { return scope; }

    bool match_scope(const std::string& s) const {
        try {
            std::regex regex_pattern(scope);
            bool match = std::regex_search(s, regex_pattern);
            uvm_info("DBG_CFG_DB", "Matching scope '" + s + "' against regex '" + scope + "': " +
                                         (match ? "true" : "false"), UVM_FULL);
            return match;
        } catch (const std::regex_error& e) {
            uvm_error("DBG_CFG_DB", PSPRINTF("Regex error in match_scope: %s", e.what()));
            return false;
        }
    }

    /*
    bool match_scope(const std::string& s) const {
        try {
            return std::regex_match(s, std::regex(scope));
        } catch (const std::regex_error& e) {
            uvm_error("DBG_CFG_DB", PSPRINTF("Regex error in match_scope: %s", e.what()));
            return false;
        }
    }*/

    virtual void set_priority(uvm_resource_types::priority_e pri) = 0;

    virtual std::string convert2string() const { return "?"; }

    void do_print() const {
        uvm_info("RESOURCE_PRINT", get_name() + " [" + get_scope() + "] : " + convert2string(), UVM_LOW);
    }

    void record_read_access(uvm_object* accessor = nullptr) {
        if (!uvm_resource_options::is_auditing()) return;
        std::string str = accessor ? accessor->get_full_name() : "<empty>";
        auto& access_record = access[str];
        access_record.read_count++;
        access_record.read_time = std::time(nullptr);
    }

    void record_write_access(uvm_object* accessor = nullptr) {
        if (!uvm_resource_options::is_auditing()) return;
        std::string str = accessor ? accessor->get_full_name() : "<empty>";
        auto& access_record = access[str];
        access_record.write_count++;
        access_record.write_time = std::time(nullptr);
    }

    virtual void print_accessors() const {
        if (access.empty()) return;
        uvm_info("ACCESSORS", "  --------", UVM_LOW);
        for (const auto& [str, access_record] : access) {
            uvm_info("ACCESSORS", "  " + str + " reads: " + std::to_string(access_record.read_count) +
                                           " @ " + std::to_string(access_record.read_time) +
                                           "  writes: " + std::to_string(access_record.write_count) +
                                           " @ " + std::to_string(access_record.write_time), UVM_LOW);
        }
    }

    void init_access_record(uvm_resource_types::access_t& access_record) {
        access_record.read_time = 0;
        access_record.write_time = 0;
        access_record.read_count = 0;
        access_record.write_count = 0;
    }

    virtual std::string get_name() const = 0; // Now returns the actual name
};

//----------------------------------------------------------------------
// Class - get_t
//
// Instances of get_t are stored in the history list as a record of each
// get. Failed gets are indicated with rsrc set to null. This is part
// of the audit trail facility for resources.
//----------------------------------------------------------------------
class get_t {
public:
    std::string name;
    std::string scope;
    uvm_resource_base* rsrc;
    time_t t;
};

//----------------------------------------------------------------------
// Class: uvm_resource_pool
//
// The global (singleton) resource database.
//----------------------------------------------------------------------
class uvm_resource_pool {
private:
    static bool m_has_wildcard_names;
    static uvm_resource_pool* rp;

    // Mapping from regex patterns to resource queues
    std::unordered_map<std::string, uvm_resource_types::rsrc_q_t> rtab;
    std::unordered_map<uvm_resource_base*, uvm_resource_types::rsrc_q_t> ttab;
    std::vector<get_t> get_record;

    bool m_has_wildcard_names_flag;

    uvm_resource_pool() : m_has_wildcard_names_flag(true) {}

public:
    static uvm_resource_pool* get() {
        if (!rp) rp = new uvm_resource_pool();
        return rp;
    }

    bool spell_check(const std::string& s) const {
        return rtab.find(s) != rtab.end();
    }

    using rsrc_q_t = uvm_resource_types::rsrc_q_t;

    // Registers a resource with an escaped regex pattern
    void register_resource(const std::string& regex_pattern, uvm_resource_base* resource) {
        rtab[regex_pattern].push_back(resource);
        uvm_info("RESOURCE_REGISTRATION", "Resource registered with regex pattern: " + regex_pattern, UVM_LOW);
    }

    void set(uvm_resource_base* rsrc, uvm_resource_types::override_t override = 0) {
        if (!rsrc) return;

        // Insert into the name map
        std::string name = rsrc->get_name();
        if (!name.empty()) {
            auto& rq = rtab[name];
            if (override & uvm_resource_types::NAME_OVERRIDE)
                rq.insert(rq.begin(), rsrc);
            else
                rq.push_back(rsrc);

            rtab[name] = rq;
        }

        // Insert into the type map
        uvm_resource_base* type_handle = rsrc->get_type_handle();
        auto& tq = ttab[type_handle];
        if (override & uvm_resource_types::TYPE_OVERRIDE)
            tq.insert(tq.begin(), rsrc);
        else
            tq.push_back(rsrc);

        ttab[type_handle] = tq;

        if (rsrc->m_is_regex_name)
            m_has_wildcard_names_flag = true;
    }

    void set_override(uvm_resource_base* rsrc) {
        set(rsrc, uvm_resource_types::NAME_OVERRIDE | uvm_resource_types::TYPE_OVERRIDE);
    }

    void set_name_override(uvm_resource_base* rsrc) {
        set(rsrc, uvm_resource_types::NAME_OVERRIDE);
    }

    void set_type_override(uvm_resource_base* rsrc) {
        set(rsrc, uvm_resource_types::TYPE_OVERRIDE);
    }

    void push_get_record(const std::string& name, const std::string& scope, uvm_resource_base* rsrc) {
        if (!uvm_resource_options::is_auditing()) return;

        get_t record;
        record.name = name;
        record.scope = scope;
        record.rsrc = rsrc;
        record.t = std::time(nullptr);
        get_record.push_back(record);
    }

    void dump_get_records() const {
        uvm_info("RESOURCE_GET_RECORDS", "--- resource get records ---", UVM_NONE);
        for (const auto& record : get_record) {
            bool success = (record.rsrc != nullptr);
            uvm_info("RESOURCE_GET_RECORDS", "get: name=" + record.name + " scope=" + record.scope +
                                           " " + (success ? "success" : "fail") + " @ " + std::to_string(record.t), UVM_NONE);
        }
    }

    uvm_resource_types::rsrc_q_t lookup_name(const std::string& scope, const std::string& name,
                                             uvm_resource_base* type_handle = nullptr, bool rpterr = true) {
        uvm_resource_types::rsrc_q_t q;
        if (name.empty()) return q;

        if ((rpterr && !spell_check(name)) || (!rpterr && rtab.find(name) == rtab.end())) {
            return q;
        }

        auto& rq = rtab[name];
        for (auto* r : rq) {
            if ((!type_handle || r->get_type_handle() == type_handle) && r->match_scope(scope))
                q.push_back(r);
        }

        return q;
    }

    uvm_resource_base* get_highest_precedence(uvm_resource_types::rsrc_q_t& q) {
        if (q.empty()) return nullptr;

        uvm_resource_base* rsrc = q.front();
        unsigned int prec = rsrc->precedence;

        for (auto* r : q) {
            if (r->precedence > prec) {
                rsrc = r;
                prec = r->precedence;
            }
        }

        return rsrc;
    }

    void sort_by_precedence(uvm_resource_types::rsrc_q_t& q) {
        std::unordered_map<unsigned int, uvm_resource_types::rsrc_q_t> all;
        for (auto* r : q) {
            all[r->precedence].push_back(r);
        }
        q.clear();
        for (auto& [_, rq] : all) {
            for (auto* r : rq) {
                q.push_back(r);
            }
        }
    }

    uvm_resource_base* get_by_name(const std::string& scope, const std::string& name, uvm_resource_base* type_handle, bool rpterr = true) {
        auto q = lookup_name(scope, name, type_handle, rpterr);
        if (q.empty()) {
            push_get_record(name, scope, nullptr);
            return nullptr;
        }

        auto* rsrc = get_highest_precedence(q);
        push_get_record(name, scope, rsrc);
        return rsrc;
    }

    uvm_resource_types::rsrc_q_t lookup_type(const std::string& scope, uvm_resource_base* type_handle) {
        uvm_resource_types::rsrc_q_t q;
        if (!type_handle || ttab.find(type_handle) == ttab.end()) return q;

        auto& rq = ttab[type_handle];
        for (auto* r : rq) {
            if (r->match_scope(scope))
                q.push_back(r);
        }

        return q;
    }

    uvm_resource_base* get_by_type(const std::string& scope = "", uvm_resource_base* type_handle = nullptr) {
        if (!type_handle)
            return nullptr;

        uvm_resource_types::rsrc_q_t q = lookup_type(scope, type_handle);
        if (q.empty()) {
            push_get_record("<type>", scope, nullptr);
            return nullptr;
        }

        auto* rsrc = q.front();
        push_get_record("<type>", scope, rsrc);
        return rsrc;
    }

    uvm_resource_types::rsrc_q_t lookup_regex_names(const std::string& scope, const std::string& name,
                                                    uvm_resource_base* type_handle = nullptr) {
        rsrc_q_t result_q;

        // For the simple case where no wildcard names exist, just return the queue associated with name.
        if (!m_has_wildcard_names_flag) {
            result_q = lookup_name(scope, name, type_handle, false); // Assuming this method is defined elsewhere
            return result_q;
        }

        uvm_info("REGEX_LOOKUP", "Looking up full key: " + scope + "." + name + " with regex patterns", UVM_FULL);

        // Iterate over the resource table, checking each pattern against the provided name
        for (auto& [glob_pattern, rq] : rtab) {
            std::string regex_pattern_str = uvm_glob_to_re(glob_pattern);

            uvm_info("REGEX_MATCH", "Attempting to match name: " + name + " against regex: " + regex_pattern_str, UVM_FULL);

            for (auto* r : rq) {
                // Use the uvm_is_match helper function to perform the matching
                if (uvm_is_match(regex_pattern_str, name)) {
                    // Check if the type and scope match
                    if ((!type_handle || r->get_type_handle() == type_handle) && r->match_scope(scope)) {
                        uvm_info("REGEX_MATCH_SUCCESS", "Match found for key: " + name + " with regex: " + regex_pattern_str, UVM_HIGH);
                        result_q.push_back(r);
                    }
                } else {
                    uvm_info("REGEX_MATCH_FAIL", "No match for name: " + name + " with regex: " + regex_pattern_str, UVM_FULL);
                }
            }
        }

        if (result_q.empty()) {
            uvm_info("REGEX_LOOKUP", "No matching resources found for key: " + scope + "." + name, UVM_HIGH);
        }

        return result_q;
    }

    uvm_resource_types::rsrc_q_t lookup_regex(const std::string& re, const std::string& scope) {
        uvm_resource_types::rsrc_q_t result_q;
        std::regex regex_pattern(re);

        for (auto& [name, rq] : rtab) {
            if (!std::regex_match(name, regex_pattern)) continue;
            for (auto* r : rq) {
                if (r->match_scope(scope))
                    result_q.push_back(r);
            }
        }
        return result_q;
    }

    uvm_resource_types::rsrc_q_t lookup_scope(const std::string& scope) {
        uvm_resource_types::rsrc_q_t q;

        for (auto& [name, rq] : rtab) {
            for (auto* r : rq) {
                if (r->match_scope(scope)) {
                    q.push_back(r);
                }
            }
        }

        return q;
    }

    void set_priority_queue(uvm_resource_base* rsrc, uvm_resource_types::rsrc_q_t& q,
                            uvm_resource_types::priority_e pri) {
        auto it = std::find(q.begin(), q.end(), rsrc);
        if (it == q.end()) {
            uvm_error("PRIORITY_CHANGE", "Handle for resource named " + rsrc->get_name() + " is not in the name map; cannot change its priority");
            return;
        }
        q.erase(it);
        if (pri == uvm_resource_types::PRI_HIGH) {
            q.insert(q.begin(), rsrc);
        } else {
            q.push_back(rsrc);
        }
    }

    void set_priority_type(uvm_resource_base* rsrc, uvm_resource_types::priority_e pri) {
        if (!rsrc) {
            uvm_error("PRIORITY_CHANGE", "attempting to change the search priority of a null resource");
            return;
        }
        uvm_resource_base* type_handle = rsrc->get_type_handle();
        auto it = ttab.find(type_handle);
        if (it == ttab.end()) {
            uvm_error("PRIORITY_CHANGE", "Type handle for resource named " + rsrc->get_name() + " not found in type map; cannot change its search priority");
            return;
        }
        set_priority_queue(rsrc, it->second, pri);
    }

    void set_priority_name(uvm_resource_base* rsrc, uvm_resource_types::priority_e pri) {
        if (!rsrc) {
            uvm_error("PRIORITY_CHANGE", "attempting to change the search priority of a null resource");
            return;
        }
        std::string name = rsrc->get_name();
        auto it = rtab.find(name);
        if (it == rtab.end()) {
            uvm_error("PRIORITY_CHANGE", "Resource named " + name + " not found in name map; cannot change its search priority");
            return;
        }
        set_priority_queue(rsrc, it->second, pri);
    }

    void set_priority(uvm_resource_base* rsrc, uvm_resource_types::priority_e pri) {
        set_priority_type(rsrc, pri);
        set_priority_name(rsrc, pri);
    }

    uvm_resource_types::rsrc_q_t find_unused_resources() {
        uvm_resource_types::rsrc_q_t q;
        for (auto& [name, rq] : rtab) {
            for (auto* r : rq) {
                int reads = 0;
                int writes = 0;
                for (auto& [_, access_record] : r->access) {
                    reads += access_record.read_count;
                    writes += access_record.write_count;
                }
                if (writes > 0 && reads == 0)
                    q.push_back(r);
            }
        }
        return q;
    }

    void print_resources(const uvm_resource_types::rsrc_q_t& rq, bool audit = false) const {
        if (rq.empty()) {
            uvm_info("PRINT_RESOURCES", "<none>", UVM_NONE);
            return;
        }
        for (const auto* r : rq) {
            r->do_print();
            if (audit)
                r->print_accessors();
        }
    }

    void dump(bool audit = false) const {
        uvm_info("DUMP", "\n=== resource pool ===", UVM_NONE);
        for (const auto& [name, rq] : rtab) {
            print_resources(rq, audit);
        }
        uvm_info("DUMP", "=== end of resource pool ===", UVM_NONE);
    }
};

//------------------------------------------------------------------------------
//
// CLASS- m_uvm_resource_converter<T>
//
// The m_uvm_resource_converter class provides a policy object for doing
// conversion from resource value to string.
//
//------------------------------------------------------------------------------
template <typename T = int>
class m_uvm_resource_converter {
public:

    // Constructor
    m_uvm_resource_converter() {
        // Initialization code, if needed
    }

    // Destructor
    virtual ~m_uvm_resource_converter() {
        // Cleanup code, if needed
    }

    // Function- convert2string
    // Convert a value of type ~T~ to a string that can be displayed.
    //
    // By default, returns the name of the type
    //
    // Base template function for convert2string
    std::string convert2string(const T& val) {
        std::string str;
        if constexpr (std::is_integral_v<T>) {
            // Handle all integral types, including unsigned int
            str = std::to_string(val);
        } 
        else if constexpr (std::is_same_v<T, std::string>) {
            // If T is a std::string, return it enclosed in quotes
            str = "\"" + val + "\"";
        } 
        else if constexpr (std::is_base_of_v<uvm_object, T>) {
            // If T is derived from uvm_object, return its full name
            str = val.get_full_name();
        } 
        else {
            // Default case for other types
            str = "(" + std::string(typeid(val).name()) + ") ?";
        }
        // Debug statement to verify the conversion
        uvm_info("CONVERT2STRING", "convert2string(" + str + ") = " + str, UVM_FULL);
        return str;
    }

};

//----------------------------------------------------------------------
// Class: uvm_resource
//
// Parameterized resource. Provides essential access methods to read
// from and write to the resource database.
//----------------------------------------------------------------------
template <typename T = int>
class uvm_resource : public uvm_resource_base {
public:
    using this_type = uvm_resource<T>;

private:
    static this_type* my_type;
    T val;
    static std::unique_ptr<m_uvm_resource_converter<T>> m_r2s;

public:
    uvm_resource(const std::string& name = "", const std::string& scope = "*")
        : uvm_resource_base(name, scope), val(T()) {}

    static m_uvm_resource_converter<T>* m_get_converter() {
        if (!m_r2s)
            m_r2s.reset(new m_uvm_resource_converter<T>());
        return m_r2s.get();
    }

    static void m_set_converter(m_uvm_resource_converter<T>* r2s) {
        m_r2s.reset(r2s);
    }

    std::string convert2string() const override {
        return m_get_converter()->convert2string(val);
    }

    static this_type* get_type() {
        static this_type* my_type_instance = new this_type();
        return my_type_instance;
    }

    uvm_resource_base* get_type_handle() const override {
        return get_type();
    }

    void set() {
        uvm_resource_pool::get()->set(this);
    }

    void set_override(uvm_resource_types::override_t override = 0x03) {
        uvm_resource_pool::get()->set(this, override);
    }

    static this_type* get_by_name(const std::string& scope, const std::string& name, bool rpterr = true) {
        uvm_resource_pool* rp = uvm_resource_pool::get();
        uvm_resource_base* rsrc_base = rp->get_by_name(scope, name, get_type(), rpterr);
        if (!rsrc_base)
            return nullptr;

        if (auto* rsrc = dynamic_cast<this_type*>(rsrc_base))
            return rsrc;

        if (rpterr)
            uvm_error("GET_BY_NAME", "Resource with name " + name + " in scope " + scope + " has incorrect type");
        return nullptr;
    }

    static this_type* get_by_type(const std::string& scope = "", uvm_resource_base* type_handle = nullptr) {
        if (!type_handle)
            return nullptr;

        uvm_resource_pool* rp = uvm_resource_pool::get();
        uvm_resource_base* rsrc_base = rp->get_by_type(scope, type_handle);
        if (!rsrc_base)
            return nullptr;

        if (auto* rsrc = dynamic_cast<this_type*>(rsrc_base))
            return rsrc;

        uvm_error("GET_BY_TYPE", "Resource with specified type handle in scope " + scope + " was not located");
        return nullptr;
    }

    T read(uvm_object* accessor = nullptr) {
        record_read_access(accessor);
        return val;
    }

    void write(const T& t, uvm_object* accessor = nullptr) {
        if (is_read_only()) {
            uvm_error("WRITE_ACCESS", "Resource " + get_name() + " is read-only -- cannot modify");
            return;
        }

        if (val == t)
            return;

        record_write_access(accessor);
        val = t;
        modified = true;
    }

    void set_priority(uvm_resource_types::priority_e pri) override {
        uvm_resource_pool::get()->set_priority(this, pri);
    }

    static this_type* get_highest_precedence(uvm_resource_types::rsrc_q_t& q) {
        if (q.empty())
            return nullptr;

        this_type* rsrc = nullptr;
        unsigned int prec = 0;
        for (auto* r : q) {
            if (auto* casted_r = dynamic_cast<this_type*>(r)) {
                if (!rsrc || casted_r->precedence > prec) {
                    rsrc = casted_r;
                    prec = casted_r->precedence;
                }
            }
        }
        return rsrc;
    }

    std::string get_name() const override {
        return name; // Now returns the actual name
    }
};

// Initialize static member
template <typename T>
typename uvm_resource<T>::this_type* uvm_resource<T>::my_type = uvm_resource<T>::get_type();

template <typename T>
std::unique_ptr<m_uvm_resource_converter<T>> uvm_resource<T>::m_r2s = nullptr;

#endif // UVM_RESOURCE_H
