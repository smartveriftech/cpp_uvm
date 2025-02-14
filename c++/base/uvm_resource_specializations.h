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

#ifndef UVM_RESOURCE_SPECIALIZATIONS_H
#define UVM_RESOURCE_SPECIALIZATIONS_H

#include <string>
#include <sstream>
#include <typeinfo>
#include <stdexcept>

// Forward declaration
class uvm_resource_base;

#define UVM_FATAL(msg) throw std::runtime_error(msg)

template <typename T>
class uvm_resource;

// Macro to generate get_by_name and get_by_type functions for resource specializations
#define UVM_RESOURCE_GET_FCNS(base_type)                                                              \
public:                                                                                               \
    static this_subtype* get_by_name(const std::string& scope, const std::string& name, bool rpterr = true) { \
        this_subtype* t = nullptr;                                                                    \
        uvm_resource_base* b = uvm_resource<base_type>::get_by_name(scope, name, rpterr);             \
        if (!(t = dynamic_cast<this_subtype*>(b)))                                                    \
            UVM_FATAL("BADCAST: cannot cast resource to resource subtype");                           \
        return t;                                                                                     \
    }                                                                                                 \
                                                                                                      \
    static this_subtype* get_by_type(const std::string& scope = "",                                   \
                                     uvm_resource_base* type_handle = nullptr) {                      \
        this_subtype* t = nullptr;                                                                    \
        uvm_resource_base* b = uvm_resource<base_type>::get_by_type(scope, type_handle);              \
        if (!(t = dynamic_cast<this_subtype*>(b)))                                                    \
            UVM_FATAL("BADCAST: cannot cast resource to resource subtype");                           \
        return t;                                                                                     \
    }

template <typename T>
class uvm_resource : public uvm_resource_base {
public:
    using this_subtype = uvm_resource<T>;

    uvm_resource(const std::string& name, const std::string& scope = "*")
        : name(name), scope(scope) {}

    virtual std::string convert2string() const {
        return typeid(T).name();
    }

    virtual T read() const = 0;

    static uvm_resource_base* get_by_name(const std::string& scope, const std::string& name, bool rpterr = true);
    static uvm_resource_base* get_by_type(const std::string& scope = "", uvm_resource_base* type_handle = nullptr);

protected:
    std::string name;
    std::string scope;
};

class uvm_int_rsrc : public uvm_resource<int> {
public:
    using this_subtype = uvm_int_rsrc;

    uvm_int_rsrc(const std::string& name, const std::string& scope = "*")
        : uvm_resource(name, scope) {}

    std::string convert2string() const override {
        std::ostringstream oss;
        oss << read();
        return oss.str();
    }

    int read() const override {
        // Implement read logic for int resource
        return 0;
    }

    UVM_RESOURCE_GET_FCNS(int)
};

class uvm_string_rsrc : public uvm_resource<std::string> {
public:
    using this_subtype = uvm_string_rsrc;

    uvm_string_rsrc(const std::string& name, const std::string& scope = "*")
        : uvm_resource(name, scope) {}

    std::string convert2string() const override {
        return read();
    }

    std::string read() const override {
        // Implement read logic for string resource
        return "";
    }

    UVM_RESOURCE_GET_FCNS(std::string)
};

class uvm_obj_rsrc : public uvm_resource<uvm_resource_base> {
public:
    using this_subtype = uvm_obj_rsrc;

    uvm_obj_rsrc(const std::string& name, const std::string& scope = "*")
        : uvm_resource(name, scope) {}

    UVM_RESOURCE_GET_FCNS(uvm_resource_base)
};

template <unsigned int N = 1>
class uvm_bit_rsrc : public uvm_resource<std::bitset<N>> {
public:
    using this_subtype = uvm_bit_rsrc<N>;

    uvm_bit_rsrc(const std::string& name, const std::string& scope = "*")
        : uvm_resource<std::bitset<N>>(name, scope) {}

    std::string convert2string() const override {
        return this->read().to_string();
    }

    std::bitset<N> read() const override {
        // Implement read logic for bit resource
        return std::bitset<N>();
    }

    UVM_RESOURCE_GET_FCNS(std::bitset<N>)
};

typedef std::array<uint8_t, N> uint8_array;

template <unsigned int N = 1>
class uvm_byte_rsrc : public uvm_resource<std::array<uint8_t, N>> {
public:
    using this_subtype = uvm_byte_rsrc<N>;

    uvm_byte_rsrc(const std::string& name, const std::string& scope = "*")
        : uvm_resource<std::array<uint8_t, N>>(name, scope) {}

    std::string convert2string() const override {
        std::ostringstream oss;
        for (const auto& byte : this->read()) {
            oss << std::hex << static_cast<int>(byte);
        }
        return oss.str();
    }

    std::array<uint8_t, N> read() const override {
        // Implement read logic for byte resource
        return std::array<uint8_t, N>();
    }

    UVM_RESOURCE_GET_FCNS(uint8_array)
};

#endif // UVM_RESOURCE_SPECIALIZATIONS_H
