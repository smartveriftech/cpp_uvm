//
//------------------------------------------------------------------------------
// Copyright 2007-2011 Cadence Design Systems, Inc.
// Copyright 2007-2011 Mentor Graphics Corporation
// Copyright 2010 Synopsys, Inc.
// Copyright 2013 Cisco Systems, Inc.
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

#ifndef UVM_PORT_BASE_H
#define UVM_PORT_BASE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <cassert>

#include "base/uvm_component.h"
#include "base/uvm_object.h"
#include "base/uvm_phase.h"
#include "base/uvm_globals.h"
#include "base/uvm_domain.h"

const int UVM_UNBOUNDED_CONNECTIONS = -1;
const std::string s_connection_error_id = "Connection Error";
const std::string s_connection_warning_id = "Connection Warning";
const std::string s_spaces = "                       ";

class uvm_port_component_base;
typedef std::map<std::string, uvm_port_component_base*> uvm_port_list;

// TITLE: Port Base Classes

//------------------------------------------------------------------------------
// CLASS: uvm_port_component_base
//------------------------------------------------------------------------------
// This class defines an interface for obtaining a port's connectivity lists
// after or during the end_of_elaboration phase.  The sub-class,
// <uvm_port_component #(PORT)>, implements this interface.
//
// The connectivity lists are returned in the form of handles to objects of this
// type. This allowing traversal of any port's fan-out and fan-in network
// through recursive calls to <get_connected_to> and <get_provided_to>. Each
// port's full name and type name can be retrieved using get_full_name and
// get_type_name methods inherited from <uvm_component>.
//------------------------------------------------------------------------------

class uvm_port_component_base : public uvm_component {
public:
    uvm_port_component_base(const std::string& name, uvm_component* parent)
        : uvm_component(name, parent) {}

    // Function: get_connected_to
    //
    // For a port or export type, this function fills ~list~ with all
    // of the ports, exports and implementations that this port is
    // connected to.

    virtual void get_connected_to(uvm_port_list& list) = 0;

    // Function: get_provided_to
    //
    // For an implementation or export type, this function fills ~list~ with all
    // of the ports, exports and implementations that this port is
    // provides its implementation to.

    virtual void get_provided_to(uvm_port_list& list) = 0;
    virtual bool is_port() = 0;
    virtual bool is_export() = 0;
    virtual bool is_imp() = 0;

    virtual void build_phase(uvm_phase* phase) override {
        build();
    }

    virtual void do_task_phase(uvm_phase* phase) {}
};

//------------------------------------------------------------------------------
// CLASS: uvm_port_component #(PORT)
//------------------------------------------------------------------------------
// See description of <uvm_port_component_base> for information about this class
//------------------------------------------------------------------------------

template <typename PORT = uvm_object>
class uvm_port_component : public uvm_port_component_base {
public:
    uvm_port_component(const std::string& name, uvm_component* parent, PORT* port)
        : uvm_port_component_base(name, parent), m_port(port) {
        if (!port) {
            uvm_fatal("Bad usage", "Null handle to port");
        }
    }

    virtual std::string get_type_name() override {
        if (!m_port) return "uvm_port_component";
        return m_port->get_type_name();
    }

    virtual void resolve_bindings() override {
        m_port->resolve_bindings();
    }

    PORT* get_port() {
        return m_port;
    }

    virtual void get_connected_to(uvm_port_list& list) override {
        m_port->get_connected_to(list);
    }

    virtual void get_provided_to(uvm_port_list& list) override {
        m_port->get_provided_to(list);
    }

    virtual bool is_port() override {
        return m_port->is_port();
    }

    virtual bool is_export() override {
        return m_port->is_export();
    }

    virtual bool is_imp() override {
        return m_port->is_imp();
    }

private:
    PORT* m_port;
};

//------------------------------------------------------------------------------
// CLASS: uvm_port_base #(IF)
//------------------------------------------------------------------------------
//
// Transaction-level communication between components is handled via its ports,
// exports, and imps, all of which derive from this class.
//
// The uvm_port_base extends IF, which is the type of the interface implemented
// by derived port, export, or implementation. IF is also a type parameter to
// uvm_port_base.
//
//   IF  - The interface type implemented by the subtype to this base port
//
// The UVM provides a complete set of ports, exports, and imps for the OSCI-
// standard TLM interfaces. They can be found in the ../src/tlm/ directory.
// For the TLM interfaces, the IF parameter is always <uvm_tlm_if_base #(T1,T2)>.
//
// Just before <uvm_component::end_of_elaboration>, an internal
// <uvm_component::resolve_bindings> process occurs, after which each port and
// export holds a list of all imps connected to it via hierarchical connections
// to other ports and exports. In effect, we are collapsing the port's fanout,
// which can span several levels up and down the component hierarchy, into a
// single array held local to the port. Once the list is determined, the port's
// min and max connection settings can be checked and enforced.
//
// uvm_port_base possesses the properties of components in that they have a
// hierarchical instance path and parent. Because SystemVerilog does not support
// multiple inheritance, uvm_port_base can not extend both the interface it
// implements and <uvm_component>. Thus, uvm_port_base contains a local instance
// of uvm_component, to which it delegates such commands as get_name,
// get_full_name, and get_parent.
//
//------------------------------------------------------------------------------

template <typename IF = uvm_void>
class uvm_port_base : public IF {
public:
    typedef uvm_port_base<IF> this_type;

    uvm_port_base(const std::string& name,
                  uvm_component* parent,
                  uvm_port_type_e port_type,
                  int min_size = 0,
                  int max_size = 1)
        : m_port_type(port_type), m_min_size(min_size), m_max_size(max_size), m_resolved(false) {
        m_comp = std::make_shared<uvm_port_component<this_type>>(name, parent, this);
        u_int32_t tmp;
        if (!m_comp->get_config_int("check_connection_relationships", tmp))
            m_comp->set_report_id_action(s_connection_warning_id, UVM_NO_ACTION);
    }

    std::string get_name() {
        return m_comp->get_name();
    }

    std::string get_full_name() {
        return m_comp->get_full_name();
    }

    uvm_component* get_parent() {
        return m_comp->get_parent();
    }

    uvm_port_component_base* get_comp() {
        return m_comp.get();
    }

    virtual std::string get_type_name() {
        switch (m_port_type) {
            case UVM_PORT: return "port";
            case UVM_EXPORT: return "export";
            case UVM_IMPLEMENTATION: return "implementation";
            default: return "unknown";
        }
    }

    int min_size() {
        return m_min_size;
    }

    int max_size() {
        return m_max_size;
    }

    bool is_unbounded() {
        return (m_max_size == UVM_UNBOUNDED_CONNECTIONS);
    }

    bool is_port() {
        return m_port_type == UVM_PORT;
    }

    bool is_export() {
        return m_port_type == UVM_EXPORT;
    }

    bool is_imp() {
        return m_port_type == UVM_IMPLEMENTATION;
    }

    int size() {
        return m_imp_list.size();
    }

    void set_if(int index = 0) {
        m_if = get_if(index);
        if (m_if != nullptr) {
            m_def_index = index;
        }
    }

    int m_get_if_mask() {
        return m_if_mask;
    }

    void set_default_index(int index) {
        m_def_index = index;
    }

    void connect(this_type* provider) {
        //if (end_of_elaboration_ph->get_state() == UVM_PHASE_EXECUTING ||
        //    end_of_elaboration_ph->get_state() == UVM_PHASE_DONE) {
        //    m_comp->uvm_report_warning("Late Connection",
        //        "Attempt to connect " + get_full_name() + " (of type " + get_type_name() + ") at or after end_of_elaboration phase. Ignoring.");
        //    return;
        //}

        if (provider == nullptr) {
            m_comp->uvm_report_error(s_connection_error_id, "Cannot connect to null port handle");
            return;
        }

        if (provider == this) {
            m_comp->uvm_report_error(s_connection_error_id, "Cannot connect a port instance to itself");
            return;
        }

        if ((provider->m_if_mask & m_if_mask) != m_if_mask) {
            m_comp->uvm_report_error(s_connection_error_id,
                provider->get_full_name() + " (of type " + provider->get_type_name() +
                ") does not provide the complete interface required of this port (type " + get_type_name() + ")");
            return;
        }

        if (is_imp()) {
            m_comp->uvm_report_error(s_connection_error_id,
                "Cannot call an imp port's connect method. An imp is connected only to the component passed in its constructor. (You attempted to bind this imp to " + provider->get_full_name() + ")");
            return;
        }

        if (is_export() && provider->is_port()) {
            m_comp->uvm_report_error(s_connection_error_id,
                "Cannot connect exports to ports. Try calling port.connect(export) instead. (You attempted to bind this export to " + provider->get_full_name() + ").");
            return;
        }

        if (!m_check_relationship(provider)) {
            return;
        }

        m_provided_by[provider->get_full_name()] = provider;
        provider->m_provided_to[get_full_name()] = this;
    }

    void debug_connected_to(int level = 0, int max_level = -1) {
        int sz = 0, num = 0, curr_num = 0;
        static std::string indent, save;
        this_type* port = nullptr;

        if (level < 0) level = 0;
        if (level == 0) {
            save = "";
            indent = "  ";
        }

        if (max_level != -1 && level >= max_level) return;

        num = m_provided_by.size();

        if (m_provided_by.size() != 0) {
            for (auto& entry : m_provided_by) {
                curr_num++;
                port = entry.second;
                save += indent + "  | \n";
                save += indent + "  |_" + entry.first + " (" + port->get_type_name() + ")\n";
                indent = (num > 1 && curr_num != num) ? indent + "  | " : indent + "    ";
                port->debug_connected_to(level + 1, max_level);
                indent = indent.substr(0, indent.size() - 4);
            }
        }

        if (level == 0) {
            if (!save.empty()) {
                save = "This port's fanout network:\n\n  " + get_full_name() + " (" + get_type_name() + ")\n" + save + "\n";
            }
            if (m_imp_list.empty()) {
                if (end_of_elaboration_ph->get_state() == UVM_PHASE_EXECUTING ||
                    end_of_elaboration_ph->get_state() == UVM_PHASE_DONE) {
                    save += "  Connected implementations: none\n";
                } else {
                    save += "  Connected implementations: not resolved until end-of-elab\n";
                }
            } else {
                save += "  Resolved implementation list:\n";
                for (auto& entry : m_imp_list) {
                    port = entry.second;
                    std::string sz_str = std::to_string(sz);
                    save += indent + sz_str + ": " + entry.first + " (" + port->get_type_name() + ")\n";
                    sz++;
                }
            }
            m_comp->uvm_report_info("debug_connected_to", save);
        }
    }

    void debug_provided_to(int level = 0, int max_level = -1) {
        std::string nm;
        int num = 0, curr_num = 0;
        this_type* port = nullptr;
        static std::string indent, save;

        if (level < 0) level = 0;
        if (level == 0) {
            save = "";
            indent = "  ";
        }

        if (max_level != -1 && level > max_level) return;

        num = m_provided_to.size();

        if (num != 0) {
            for (auto& entry : m_provided_to) {
                curr_num++;
                port = entry.second;
                save += indent + "  | \n";
                save += indent + "  |_" + entry.first + " (" + port->get_type_name() + ")\n";
                indent = (num > 1 && curr_num != num) ? indent + "  | " : indent + "    ";
                port->debug_provided_to(level + 1, max_level);
                indent = indent.substr(0, indent.size() - 4);
            }
        }

        if (level == 0) {
            if (!save.empty()) {
                save = "This port's fanin network:\n\n  " + get_full_name() + " (" + get_type_name() + ")\n" + save + "\n";
            }
            if (m_provided_to.empty()) {
                save += indent + "This port has not been bound\n";
            }
            m_comp->uvm_report_info("debug_provided_to", save);
        }
    }

    void get_connected_to(uvm_port_list& list) {
        list.clear();
        for (auto& entry : m_provided_by) {
            list[entry.first] = entry.second->get_comp();
        }
    }

    void get_provided_to(uvm_port_list& list) {
        list.clear();
        for (auto& entry : m_provided_to) {
            list[entry.first] = entry.second->get_comp();
        }
    }

    void resolve_bindings() {
        if (m_resolved) return;

        if (is_imp()) {
            m_imp_list[get_full_name()] = this;
        } else {
            for (auto& entry : m_provided_by) {
                this_type* port = entry.second;
                port->resolve_bindings();
                m_add_list(port);
            }
        }

        m_resolved = true;

        if (size() < min_size()) {
            m_comp->uvm_report_error(s_connection_error_id,
                "connection count of " + std::to_string(size()) + " does not meet required minimum of " + std::to_string(min_size()));
        }

        if (max_size() != UVM_UNBOUNDED_CONNECTIONS && size() > max_size()) {
            m_comp->uvm_report_error(s_connection_error_id,
                "connection count of " + std::to_string(size()) + " exceeds maximum of " + std::to_string(max_size()));
        }

        if (size()) set_if(0);
    }

    uvm_port_base<IF>* get_if(int index = 0) {
        if (size() == 0) {
            m_comp->uvm_report_warning("get_if", "Port size is zero; cannot get interface at any index");
            return nullptr;
        }
        if (index < 0 || index >= size()) {
            m_comp->uvm_report_warning(s_connection_error_id, "Index " + std::to_string(index) + " out of range [0," + std::to_string(size() - 1) + "]");
            return nullptr;
        }
        for (auto& entry : m_imp_list) {
            if (index == 0) return entry.second;
            index--;
        }
        return nullptr;
    }

//private:
    bool m_check_relationship(this_type* provider) {
        std::string s;
        this_type* from = this;
        uvm_component* from_parent = get_parent();
        uvm_component* to_parent = provider->get_parent();
        uvm_component* from_gparent = (from_parent != nullptr) ? from_parent->get_parent() : nullptr;
        uvm_component* to_gparent = (to_parent != nullptr) ? to_parent->get_parent() : nullptr;

        if (get_type_name() == "uvm_analysis_port") return true;

        if (from->is_port() && provider->is_port() && from_gparent != to_parent) {
            s = provider->get_full_name() + " (of type " + provider->get_type_name() +
                ") is not up one level of hierarchy from this port. A port-to-port connection takes the form child_component.child_port.connect(parent_port)";
            m_comp->uvm_report_warning(s_connection_warning_id, s);
            return false;
        }

        if (from->is_port() && (provider->is_export() || provider->is_imp()) && from_gparent != to_gparent) {
            s = provider->get_full_name() + " (of type " + provider->get_type_name() +
                ") is not at the same level of hierarchy as this port. A port-to-export connection takes the form component1.port.connect(component2.export)";
            m_comp->uvm_report_warning(s_connection_warning_id, s);
            return false;
        }

        if (from->is_export() && (provider->is_export() || provider->is_imp()) && from_parent != to_gparent) {
            s = provider->get_full_name() + " (of type " + provider->get_type_name() +
                ") is not down one level of hierarchy from this export. An export-to-export or export-to-imp connection takes the form parent_export.connect(child_component.child_export)";
            m_comp->uvm_report_warning(s_connection_warning_id, s);
            return false;
        }

        return true;
    }

    void m_add_list(this_type* provider) {
        for (int i = 0; i < provider->size(); i++) {
            this_type* imp = provider->get_if(i);
            if (m_imp_list.find(imp->get_full_name()) == m_imp_list.end()) {
                m_imp_list[imp->get_full_name()] = imp;
            }
        }
    }

    this_type* m_if = nullptr;
    int m_def_index = 0;
    std::shared_ptr<uvm_port_component<this_type>> m_comp;
    std::map<std::string, this_type*> m_provided_by;
    std::map<std::string, this_type*> m_provided_to;
    uvm_port_type_e m_port_type;
    int m_min_size;
    int m_max_size;
    bool m_resolved;
    std::map<std::string, this_type*> m_imp_list;

protected:
    u_int32_t m_if_mask;

};

#endif // UVM_PORT_BASE_H
