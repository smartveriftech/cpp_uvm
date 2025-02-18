//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Cadence Design Systems, Inc. 
// Copyright 2007-2011 Mentor Graphics Corporation
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

#ifndef UVM_POOL_H
#define UVM_POOL_H

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "base/uvm_globals.h"
#include "base/uvm_object.h"
#include "base/uvm_printer.h"

template <typename KEY = int, typename T = uvm_void>
class uvm_pool : public uvm_object {
public:
    static const std::string type_name;

    typedef uvm_pool<KEY, T> this_type;

    static this_type* get_global_pool();
    static T get_global(KEY key);

    uvm_pool(const std::string& name = "");
    virtual ~uvm_pool();

    virtual T get(KEY key) const;
    virtual void add(KEY key, T item);
    virtual int num() const;
    virtual void remove(KEY key);
    virtual bool exists(KEY key) const;
    virtual bool first(KEY& key) const;
    virtual bool last(KEY& key) const;
    virtual bool next(KEY& key) const;
    virtual bool prev(KEY& key) const;

    virtual uvm_object* create(const std::string& name = "");
    virtual std::string get_type_name();
    virtual void do_copy(uvm_object* rhs);
    virtual void do_print(uvm_printer* printer);

    // Add iterator support
    typename std::map<KEY, T>::iterator begin() { return pool.begin(); }
    typename std::map<KEY, T>::iterator end() { return pool.end(); }
    typename std::map<KEY, T>::const_iterator begin() const { return pool.begin(); }
    typename std::map<KEY, T>::const_iterator end() const { return pool.end(); }

protected:
    std::map<KEY, T> pool;

private:
    static this_type* m_global_pool;
};

template <typename KEY, typename T>
const std::string uvm_pool<KEY, T>::type_name = "uvm_pool";

template <typename KEY, typename T>
typename uvm_pool<KEY, T>::this_type* uvm_pool<KEY, T>::m_global_pool = nullptr;

template <typename KEY, typename T>
uvm_pool<KEY, T>::uvm_pool(const std::string& name) : uvm_object(name) {}

template <typename KEY, typename T>
uvm_pool<KEY, T>::~uvm_pool() {}

template <typename KEY, typename T>
typename uvm_pool<KEY, T>::this_type* uvm_pool<KEY, T>::get_global_pool() {
    if (m_global_pool == nullptr) {
        m_global_pool = new this_type("global_pool");
    }
    return m_global_pool;
}

template <typename KEY, typename T>
T uvm_pool<KEY, T>::get_global(KEY key) {
    return get_global_pool()->get(key);
}

template <typename KEY, typename T>
T uvm_pool<KEY, T>::get(KEY key) const {
    if (!pool.count(key)) {
        // Check if T is a pointer type
        if constexpr (std::is_pointer<T>::value) {
            using BaseType = typename std::remove_pointer<T>::type;
            if constexpr (std::is_base_of<uvm_object, BaseType>::value) {
                // For UVM objects, create a new instance
                T new_obj = new BaseType(PSPRINTF("%s", typeid(BaseType).name()));
                const_cast<uvm_pool<KEY, T>*>(this)->pool[key] = new_obj;
                return new_obj;
            } else {
                uvm_warning("POOLGET", PSPRINTF("key %s doesn't exist in the pool (type: %s*)", 
                    typeid(key).name(), typeid(BaseType).name()));
                return nullptr;
            }
        }
        // Check if key is an int
        else if constexpr (std::is_same<T, int>::value) {
            uvm_warning("POOLGET", PSPRINTF("key %d doesn't exist in the pool", key));
            return 0;
        }
        // Check if key is a std::string
        else if constexpr (std::is_same<T, std::string>::value) {
            uvm_warning("POOLGET", PSPRINTF("key %s doesn't exist in the pool", key.c_str()));
            return "";
        }
        // Handle all other types
        else {
            uvm_warning("POOLGET", PSPRINTF("key(type %s) doesn't exist in the pool", typeid(key).name()));
            return T();
        }
    }
    return pool.at(key);
}

template <typename KEY, typename T>
void uvm_pool<KEY, T>::add(KEY key, T item) {
    pool[key] = item;
}

template <typename KEY, typename T>
int uvm_pool<KEY, T>::num() const {
    return pool.size();
}

template <typename KEY, typename T>
void uvm_pool<KEY, T>::remove(KEY key) {
    if (!exists(key)) {
        uvm_warning("POOLDEL", "delete: pool key doesn't exist. Ignoring delete request");
        return;
    }
    pool.erase(key);
}

template <typename KEY, typename T>
bool uvm_pool<KEY, T>::exists(KEY key) const {
    return pool.count(key) > 0;
}

template <typename KEY, typename T>
bool uvm_pool<KEY, T>::first(KEY& key) const {
    if (pool.empty()) return false;
    key = pool.begin()->first;
    return true;
}

template <typename KEY, typename T>
bool uvm_pool<KEY, T>::last(KEY& key) const {
    if (pool.empty()) return false;
    key = pool.rbegin()->first;
    return true;
}

template <typename KEY, typename T>
bool uvm_pool<KEY, T>::next(KEY& key) const {
    auto it = pool.find(key);
    if (it == pool.end() || ++it == pool.end()) return false;
    key = it->first;
    return true;
}

template <typename KEY, typename T>
bool uvm_pool<KEY, T>::prev(KEY& key) const {
    auto it = pool.find(key);
    if (it == pool.begin() || it == pool.end()) return false;
    key = (--it)->first;
    return true;
}

template <typename KEY, typename T>
uvm_object* uvm_pool<KEY, T>::create(const std::string& name) {
    return new this_type(name);
}

template <typename KEY, typename T>
std::string uvm_pool<KEY, T>::get_type_name() {
    return type_name;
}

template <typename KEY, typename T>
void uvm_pool<KEY, T>::do_copy(uvm_object* rhs) {
    const this_type* p = dynamic_cast<const this_type*>(rhs);
    if (!p) return;
    pool = p->pool;
}

template <typename KEY, typename T>
void uvm_pool<KEY, T>::do_print(uvm_printer* printer) {
    printer->print_array_header("pool", pool.size(), "aa_object_string");
    for (const auto& item : pool) {
        std::ostringstream key_stream;
        key_stream << item.first;
        std::ostringstream value_stream;
        if constexpr (std::is_base_of<uvm_object, T>::value) {
            item.second->do_print(printer);
        } else {
            value_stream << item.second;
            printer->print_generic("[" + key_stream.str() + "]", "", -1, value_stream.str(), "[" );
        }
    }
    printer->print_array_footer();
}

template <typename T>
class uvm_object_string_pool : public uvm_pool<std::string, T> {
public:
    typedef uvm_object_string_pool<T> this_type;

    static this_type* get_global_pool();
    static T get_global(const std::string& key);

    uvm_object_string_pool(const std::string& name = "");
    virtual ~uvm_object_string_pool();

    virtual T get(std::string key) const;
    virtual void remove(std::string key);

    virtual std::string get_type_name();
    virtual void do_print(uvm_printer* printer);

private:
    static this_type* m_global_pool;
};

template <typename T>
typename uvm_object_string_pool<T>::this_type* uvm_object_string_pool<T>::m_global_pool = nullptr;

template <typename T>
uvm_object_string_pool<T>::uvm_object_string_pool(const std::string& name) : uvm_pool<std::string, T>(name) {}

template <typename T>
uvm_object_string_pool<T>::~uvm_object_string_pool() {}

template <typename T>
typename uvm_object_string_pool<T>::this_type* uvm_object_string_pool<T>::get_global_pool() {
    if (m_global_pool == nullptr) {
        m_global_pool = new this_type("global_pool");
    }
    return m_global_pool;
}

template <typename T>
T uvm_object_string_pool<T>::get_global(const std::string& key) {
    return get_global_pool()->get(key);
}

template <typename T>
T uvm_object_string_pool<T>::get(std::string key) const {
    if (!this->pool.count(key)) {
        // Check if T is a pointer type
        if constexpr (std::is_pointer<T>::value) {
            using BaseType = typename std::remove_pointer<T>::type;
            if constexpr (std::is_base_of<uvm_object, BaseType>::value) {
                // For UVM objects, create a new instance
                T new_obj = new BaseType(key);  // Use key as the name for better debugging
                const_cast<uvm_object_string_pool<T>*>(this)->pool[key] = new_obj;
                return new_obj;
            } else {
                uvm_warning("POOLGET", PSPRINTF("key %s doesn't exist in the pool (type: %s*)", 
                    key.c_str(), typeid(BaseType).name()));
                return nullptr;
            }
        }
        // Check if T is an int
        else if constexpr (std::is_same<T, int>::value) {
            uvm_warning("POOLGET", PSPRINTF("key %s doesn't exist in the pool", key.c_str()));
            return 0;
        }
        // Check if T is a string
        else if constexpr (std::is_same<T, std::string>::value) {
            uvm_warning("POOLGET", PSPRINTF("key %s doesn't exist in the pool", key.c_str()));
            return "";
        }
        // Handle all other types
        else {
            uvm_warning("POOLGET", PSPRINTF("key %s doesn't exist in the pool (type: %s)", 
                key.c_str(), typeid(T).name()));
            return T();
        }
    }
    return this->pool.at(key);
}

template <typename T>
void uvm_object_string_pool<T>::remove(std::string key) {
    if (!this->exists(key)) {
        uvm_warning("POOLDEL", "delete: key '" + key + "' doesn't exist");
        return;
    }
    this->pool.erase(key);
}

template <typename T>
std::string uvm_object_string_pool<T>::get_type_name() {
    return "uvm_obj_str_pool";
}

template <typename T>
void uvm_object_string_pool<T>::do_print(uvm_printer* printer) {
    printer->print_array_header("pool", this->pool.size(), "aa_object_string");
    for (const auto& item : this->pool) {
        std::ostringstream key_stream;
        key_stream << item.first;

        std::ostringstream value_stream;
        if constexpr (std::is_base_of<uvm_object, T>::value) {
            // If T is derived from uvm_object and is a pointer
            if constexpr (std::is_pointer<T>::value) {
                item.second->do_print(printer);
            } else {
                // If T is derived from uvm_object and is not a pointer
                item.second.do_print(printer);
            }
        } else {
            // Otherwise, just insert it into the string stream
            value_stream << item.second;
            printer->print_generic("[" + key_stream.str() + "]", "", -1, value_stream.str(), "[" );
        }
    }
    printer->print_array_footer();
}

class uvm_barrier;
class uvm_event;

using uvm_barrier_pool = uvm_object_string_pool<uvm_barrier*>;
using uvm_event_pool = uvm_object_string_pool<uvm_event*>;

#endif // UVM_POOL_H