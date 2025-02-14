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

#ifndef UVM_RESOURCE_DB_H
#define UVM_RESOURCE_DB_H

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <memory>

#include "base/uvm_resource.h"
#include "base/uvm_globals.h"
#include "base/uvm_cmdline_processor.h"

//----------------------------------------------------------------------
 // Title: UVM Resource Database
 //
 // Topic: Intro
 //
 // The <uvm_resource_db> class provides a convenience interface for
 // the resources facility.  In many cases basic operations such as
 // creating and setting a resource or getting a resource could take
 // multiple lines of code using the interfaces in <uvm_resource_base> or
 // <uvm_resource#(T)>.  The convenience layer in <uvm_resource_db>
 // reduces many of those operations to a single line of code.
 //
 // If the run-time ~+UVM_RESOURCE_DB_TRACE~ command line option is
 // specified, all resource DB accesses (read and write) are displayed.
 //----------------------------------------------------------------------

// Forward declaration of template class
class uvm_object;
class uvm_resource_db_options;

template <typename T = uvm_object>
class uvm_resource_db {
public:
    using rsrc_t = uvm_resource<T>;

    // All of the functions in this class are static, so there is no need
    // to instantiate this class ever.  To make sure that the constructor
    // is never called it's good practice to make it local or at least
    // protected. However, IUS doesn't support protected constructors so
    // we'll just use the default constructor instead.  If support for
    // protected constructors ever becomes available then this comment can
    // be deleted and the protected constructor uncommented.

    //  protected function new();
    //  endfunction

    // function: get_by_type
    //
    // Get a resource by type.  The type is specified in the db
    // class parameter so the only argument to this function is the
    // ~scope~.

    static rsrc_t* get_by_type(const std::string& scope);

    // function: get_by_name
    //
    // Imports a resource by ~name~.  The first argument is the ~name~ of the
    // resource to be retrieved and the second argument is the current
    // ~scope~. The ~rpterr~ flag indicates whether or not to generate
    // a warning if no matching resource is found.
    static rsrc_t* get_by_name(const std::string& scope, const std::string& name, bool rpterr = true);

    // function: set_default
    //
    // add a new item into the resources database.  The item will not be
    // written to so it will have its default value. The resource is
    // created using ~name~ and ~scope~ as the lookup parameters.
    static rsrc_t* set_default(const std::string& scope, const std::string& name);

    // function: set
    //
    // Create a new resource, write a ~val~ to it, and set it into the
    // database using ~name~ and ~scope~ as the lookup parameters. The
    // ~accessor~ is used for auditing.
    static void set(const std::string& scope, const std::string& name, T val, uvm_object* accessor = nullptr);

    // function: set_anonymous
    //
    // Create a new resource, write a ~val~ to it, and set it into the
    // database.  The resource has no name and therefore will not be
    // entered into the name map. But it does have a ~scope~ for lookup
    // purposes. The ~accessor~ is used for auditing.
    static void set_anonymous(const std::string& scope, T val, uvm_object* accessor = nullptr);

    // function set_override
    //
    // Create a new resource, write ~val~ to it, and set it into the
    // database.  Set it at the beginning of the queue in the type map and
    // the name map so that it will be (currently) the highest priority
    // resource with the specified name and type.
    static void set_override(const std::string& scope, const std::string& name, T val, uvm_object* accessor = nullptr);

    // function set_override_type
    //
    // Create a new resource, write ~val~ to it, and set it into the
    // database.  Set it at the beginning of the queue in the type map so
    // that it will be (currently) the highest priority resource with the
    // specified type. It will be normal priority (i.e. at the end of the
    // queue) in the name map.
    static void set_override_type(const std::string& scope, const std::string& name, T val, uvm_object* accessor = nullptr);

    // function set_override_name
    //
    // Create a new resource, write ~val~ to it, and set it into the
    // database.  Set it at the beginning of the queue in the name map so
    // that it will be (currently) the highest priority resource with the
    // specified name. It will be normal priority (i.e. at the end of the
    // queue) in the type map.
    static void set_override_name(const std::string& scope, const std::string& name, T val, uvm_object* accessor = nullptr);

    // function: read_by_name
    //
    // locate a resource by ~name~ and ~scope~ and read its value. The value 
    // is returned through the output argument ~val~.  The return value is a bit 
    // that indicates whether or not the read was successful. The ~accessor~
    // is used for auditing.
    static bool read_by_name(const std::string& scope, const std::string& name, T& val, uvm_object* accessor = nullptr);

    // function: read_by_type
    //
    // Read a value by type.  The value is returned through the output
    // argument ~val~.  The ~scope~ is used for the lookup. The return
    // value is a bit that indicates whether or not the read is successful.
    // The ~accessor~ is used for auditing.
    static bool read_by_type(const std::string& scope, T& val, uvm_object* accessor = nullptr);

    // function: write_by_name
    //
    // write a ~val~ into the resources database.  First, look up the
    // resource by ~name~ and ~scope~.  If it is not located then add a new 
    // resource to the database and then write its value.
    //
    // Because the ~scope~ is matched to a resource which may be a
    // regular expression, and consequently may target other scopes beyond
    // the ~scope~ argument. Care must be taken with this function. If
    // a <get_by_name> match is found for ~name~ and ~scope~ then ~val~
    // will be written to that matching resource and thus may impact
    // other scopes which also match the resource.
    static bool write_by_name(const std::string& scope, const std::string& name, T val, uvm_object* accessor = nullptr);

    // function: write_by_type
    //
    // write a ~val~ into the resources database.  First, look up the
    // resource by type.  If it is not located then add a new resource to
    // the database and then write its value.
    //
    // Because the ~scope~ is matched to a resource which may be a
    // regular expression, and consequently may target other scopes beyond
    // the ~scope~ argument. Care must be taken with this function. If
    // a <get_by_name> match is found for ~name~ and ~scope~ then ~val~
    // will be written to that matching resource and thus may impact
    // other scopes which also match the resource.
    static bool write_by_type(const std::string& scope, T val, uvm_object* accessor = nullptr);

    // function: dump
    //
    // Dump all the resources in the resource pool. This is useful for
    // debugging purposes.  This function does not use the parameter T, so
    // it will dump the same thing -- the entire database -- no matter the
    // value of the parameter.
    static void dump();

protected:
    // function- show_msg
    // internal helper function to print resource accesses
    static void m_show_msg(const std::string& id, const std::string& rtype, const std::string& action,
                           const std::string& scope, const std::string& name, uvm_object* accessor, rsrc_t* rsrc);
};

//----------------------------------------------------------------------
// Class: uvm_resource_db_options
//
// Provides a namespace for managing options for the
// resources DB facility.  The only thing allowed in this class is static
// local data members and static functions for manipulating and
// retrieving the value of the data members.  The static local data
// members represent options and settings that control the behavior of
// the resources DB facility.
//
// Options include:
//
//  * tracing:  on/off
//
//    The default for tracing is off.
//
//----------------------------------------------------------------------
class uvm_resource_db_options {
public:

    // Function: turn_on_tracing
    //
    // Turn tracing on for the resource database. This causes all
    // reads and writes to the database to display information about
    // the accesses. Tracing is off by default.
    //
    // This method is implicitly called by the ~+UVM_RESOURCE_DB_TRACE~.
    static void turn_on_tracing();

    // Function: turn_off_tracing
    //
    // Turn tracing off for the resource database.
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

// Implementation of uvm_resource_db template methods

template <typename T>
typename uvm_resource_db<T>::rsrc_t* uvm_resource_db<T>::get_by_type(const std::string& scope) {
    return rsrc_t::get_by_type(scope);
}

template <typename T>
typename uvm_resource_db<T>::rsrc_t* uvm_resource_db<T>::get_by_name(const std::string& scope, const std::string& name, bool rpterr) {
    return rsrc_t::get_by_name(scope, name, rpterr);
}

template <typename T>
typename uvm_resource_db<T>::rsrc_t* uvm_resource_db<T>::set_default(const std::string& scope, const std::string& name) {
    rsrc_t* r = new rsrc_t(name, scope);
    r->set();
    return r;
}

template <typename T>
void uvm_resource_db<T>::set(const std::string& scope, const std::string& name, T val, uvm_object* accessor) {
    rsrc_t* rsrc = new rsrc_t(name, scope);
    rsrc->write(val, accessor);
    rsrc->set();
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/SET", "Resource", "set", scope, name, accessor, rsrc);
}

template <typename T>
void uvm_resource_db<T>::set_anonymous(const std::string& scope, T val, uvm_object* accessor) {
    rsrc_t* rsrc = new rsrc_t("", scope);
    rsrc->write(val, accessor);
    rsrc->set();
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/SETANON", "Resource", "set", scope, "", accessor, rsrc);
}

template <typename T>
void uvm_resource_db<T>::set_override(const std::string& scope, const std::string& name, T val, uvm_object* accessor) {
    rsrc_t* rsrc = new rsrc_t(name, scope);
    rsrc->write(val, accessor);
    rsrc->set_override();
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/SETOVRD", "Resource", "set", scope, name, accessor, rsrc);
}

template <typename T>
void uvm_resource_db<T>::set_override_type(const std::string& scope, const std::string& name, T val, uvm_object* accessor) {
    rsrc_t* rsrc = new rsrc_t(name, scope);
    rsrc->write(val, accessor);
    rsrc->set_override(uvm_resource_types::TYPE_OVERRIDE);
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/SETOVRDTYP", "Resource", "set", scope, name, accessor, rsrc);
}

template <typename T>
void uvm_resource_db<T>::set_override_name(const std::string& scope, const std::string& name, T val, uvm_object* accessor) {
    rsrc_t* rsrc = new rsrc_t(name, scope);
    rsrc->write(val, accessor);
    rsrc->set_override(uvm_resource_types::NAME_OVERRIDE);
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/SETOVRDNAM", "Resource", "set", scope, name, accessor, rsrc);
}

template <typename T>
bool uvm_resource_db<T>::read_by_name(const std::string& scope, const std::string& name, T& val, uvm_object* accessor) {
    rsrc_t* rsrc = get_by_name(scope, name);
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/RDBYNAM", "Resource", "read", scope, name, accessor, rsrc);
    if (!rsrc)
        return false;
    val = rsrc->read(accessor);
    return true;
}

template <typename T>
bool uvm_resource_db<T>::read_by_type(const std::string& scope, T& val, uvm_object* accessor) {
    rsrc_t* rsrc = get_by_type(scope);
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/RDBYTYP", "Resource", "read", scope, "", accessor, rsrc);
    if (!rsrc)
        return false;
    val = rsrc->read(accessor);
    return true;
}

template <typename T>
bool uvm_resource_db<T>::write_by_name(const std::string& scope, const std::string& name, T val, uvm_object* accessor) {
    rsrc_t* rsrc = get_by_name(scope, name);
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/WR", "Resource", "written", scope, name, accessor, rsrc);
    if (!rsrc)
        return false;
    rsrc->write(val, accessor);
    return true;
}

template <typename T>
bool uvm_resource_db<T>::write_by_type(const std::string& scope, T val, uvm_object* accessor) {
    rsrc_t* rsrc = get_by_type(scope);
    if (uvm_resource_db_options::is_tracing())
        m_show_msg("RSRCDB/WRTYP", "Resource", "written", scope, "", accessor, rsrc);
    if (!rsrc)
        return false;
    rsrc->write(val, accessor);
    return true;
}

template <typename T>
void uvm_resource_db<T>::dump() {
    uvm_resource_pool* rp = uvm_resource_pool::get();
    rp->dump();
}

template <typename T>
void uvm_resource_db<T>::m_show_msg(const std::string& id, const std::string& rtype, const std::string& action,
                                    const std::string& scope, const std::string& name, uvm_object* accessor, rsrc_t* rsrc) {

    std::ostringstream oss;
    oss << rtype << " '" << scope << "." << name << "' (type " << typeid(T).name() << ") "
        << action << " by " << (accessor != nullptr ? accessor->get_full_name() : "<unknown>")
        << " = ";

    if (rsrc == nullptr) {
        oss << "null (failed lookup)";
    } else {
        // Correctly use convert2string without invalid casts
        oss << rsrc->convert2string();
    }

    // Temporarily replace uvm_report_info with std::cout to isolate the issue
    uvm_report_info(id, oss.str(), UVM_NONE);
}


#endif // UVM_RESOURCE_DB_H

// Ensure that the `m_uvm_resource_converter<T>` and `uvm_resource<T>` classes
// are correctly implemented as per previous instructions.

// No separate uvm_resource_db.cpp is necessary for template classes.
// All implementations should reside in the header file.