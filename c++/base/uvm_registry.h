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

#ifndef UVM_REGISTRY_H
#define UVM_REGISTRY_H

#include <string>
#include <iostream>
#include "base/uvm_factory.h"
#include "base/uvm_component.h"
#include "base/uvm_object.h"

//------------------------------------------------------------------------------
//
// CLASS: uvm_component_registry
//
// The uvm_component_registry serves as a lightweight proxy for a component of
// type T. The proxy enables efficient
// registration with the uvm_factory. Without it, registration would
// require an instance of the component itself.
//
//------------------------------------------------------------------------------

template <typename T>
class uvm_component_registry : public uvm_object_wrapper {
public:
    typedef uvm_component_registry<T> this_type;

    // Function: get_type_name
    // Returns the value given by the string parameter.
    virtual std::string get_type_name() override {
        return type_name;
    }

    static std::string type_name;

    static this_type* me;

    // Function: get
    // Returns the singleton instance of this type.
    static this_type* get() {
        if (me == nullptr) {
            me = new this_type();
        }
        return me;
    }

    // Function: create_object
    //
    // Creates a new object with the optional ~name~.
    // An object proxy (e.g., <uvm_object_registry #(T,Tname)>) implements this
    // method to create an object of a specific type, T.
    virtual uvm_object* create_object(const std::string& name = "") override {
        uvm_error("FACTORY", PSPRINTF("Type name %s not support create_object", this_type::type_name.c_str()));
        return nullptr;
    };

    // Function: create_component
    //
    // Creates a new component, passing to its constructor the given ~name~ and
    // ~parent~. A component proxy (e.g. <uvm_component_registry #(T,Tname)>)
    // implements this method to create a component of a specific type, T.
    virtual uvm_component* create_component(const std::string &name = "", uvm_component* parent = nullptr) override {
        //return dynamic_cast<uvm_component*>(this_type::create(name, parent));
        return new T(name, parent);
    };

    // Function: create
    // Returns an instance of the component type, T, represented by this proxy.
    static T* create(const std::string &name, uvm_component* parent, const std::string &contxt = "") {
        uvm_object* obj;
        uvm_factory* f = uvm_factory::get();
        std::string context = contxt.empty() && parent != nullptr ? parent->get_full_name() : contxt;
        obj = f->create_component_by_type(get(), context, name, parent);
        T* result;
        if (!(result = dynamic_cast<T*>(obj))) {
            std::cerr << "Factory did not return a component of type '" << type_name
                      << "'. A component of type '" << (obj == nullptr ? "null" : obj->get_type_name())
                      << "' was returned instead. Name=" << name << " Parent="
                      << (parent == nullptr ? "null" : parent->get_type_name())
                      << " contxt=" << context << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return result;
    }

    // Function: set_type_override
    static void set_type_override(uvm_object_wrapper* override_type, bool replace = true) {
        uvm_factory::get()->set_type_override_by_type(get(), override_type, replace);
    }

    // Function: set_inst_override
    static void set_inst_override(uvm_object_wrapper* override_type, const std::string &inst_path, uvm_component* parent = nullptr) {
        std::string full_inst_path = parent != nullptr ? parent->get_full_name() + "." + inst_path : inst_path;
        uvm_factory::get()->set_inst_override_by_type(get(), override_type, full_inst_path);
    }
};

template <typename T>
std::string uvm_component_registry<T>::type_name = "__UNDEF__";

template <typename T>
uvm_component_registry<T>* uvm_component_registry<T>::me = nullptr;

//------------------------------------------------------------------------------
//
// CLASS: uvm_object_registry
//
// The uvm_object_registry serves as a lightweight proxy for an uvm_object of
// type T, The proxy enables efficient
// registration with the uvm_factory. Without it, registration would
// require an instance of the object itself.
//
//------------------------------------------------------------------------------

template <typename T>
class uvm_object_registry : public uvm_object_wrapper {
public:
    typedef uvm_object_registry<T> this_type;

    // Function: create_object
    // Creates an object of type T and returns it as a handle to an uvm_object.
    virtual uvm_object* create_object(const std::string &name = "") override {
        T* obj = new T();
        if (!name.empty()) {
            obj->set_name(name);
        }
        return obj;
    }

    // Function: create_component
    // Since this is an object registry, this function should return nullptr.
    virtual uvm_component* create_component(const std::string &name = "", uvm_component* parent = nullptr) override {
        return nullptr;
    }

    // Function: get_type_name
    // Returns the value given by the string parameter, Tname.
    virtual std::string get_type_name() override {
        return type_name;
    }

    static std::string type_name;

    static this_type* me;

    // Function: get
    // Returns the singleton instance of this type.
    static this_type* get() {
        if (me == nullptr) {
            me = new this_type();
        }
        return me;
    }

    // Function: get_object_type
    // Returns the type ID of the object
    virtual uvm_object_wrapper* get_object_type() {  // Added implementation
        return const_cast<this_type*>(this);
    }

    // Function: create
    // Returns an instance of the object type, T, represented by this proxy.
    static T* create(const std::string &name = "", uvm_component* parent = nullptr, const std::string &contxt = "") {
        uvm_object* obj;
        uvm_factory* f = uvm_factory::get();
        std::string context = contxt.empty() && parent != nullptr ? parent->get_full_name() : contxt;
        obj = f->create_object_by_type(get(), context, name);
        T* result;
        if (!(result = dynamic_cast<T*>(obj))) {
            std::cerr << "Factory did not return an object of type '" << type_name
                      << "'. A component of type '" << (obj == nullptr ? "null" : obj->get_type_name())
                      << "' was returned instead. Name=" << name << " Parent="
                      << (parent == nullptr ? "null" : parent->get_type_name())
                      << " contxt=" << context << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return result;
    }

    // Function: set_type_override
    static void set_type_override(uvm_object_wrapper* override_type, bool replace = true) {
        uvm_factory::get()->set_type_override_by_type(get(), override_type, replace);
    }

    // Function: set_inst_override
    static void set_inst_override(uvm_object_wrapper* override_type, const std::string &inst_path, uvm_component* parent = nullptr) {
        std::string full_inst_path = parent != nullptr ? parent->get_full_name() + "." + inst_path : inst_path;
        uvm_factory::get()->set_inst_override_by_type(get(), override_type, full_inst_path);
    }
};

template <typename T>
std::string uvm_object_registry<T>::type_name = "__UNDEF__";

template <typename T>
uvm_object_registry<T>* uvm_object_registry<T>::me = nullptr;

#endif // UVM_REGISTRY_H