//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Cadence Design Systems, Inc.
// Copyright 2007-2011 Mentor Graphics Corporation
// Copyright 2010-2011 Synopsys, Inc.
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

#ifndef UVM_CALLBACK_H
#define UVM_CALLBACK_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "base/uvm_object.h"
#include "base/uvm_report_object.h"

// Forward declarations
class uvm_component;
//class uvm_root;

//------------------------------------------------------------------------------
// Class - uvm_callback
//------------------------------------------------------------------------------

class uvm_callback : public uvm_object {
public:
    static uvm_report_object reporter;
    bool m_enabled;

    uvm_callback(const std::string& name = "uvm_callback") : uvm_object(name), m_enabled(true) {}

    bool callback_mode(int on = -1) {
        if (on == 0 || on == 1) {
            std::cerr << "Setting callback mode for " << get_name() << " to " << (on == 1 ? "ENABLED" : "DISABLED") << std::endl;
        } else {
            std::cerr << "Callback mode for " << get_name() << " is " << (m_enabled == 1 ? "ENABLED" : "DISABLED") << std::endl;
        }
        if (on == 0) m_enabled = false;
        if (on == 1) m_enabled = true;
        return m_enabled;
    }

    bool is_enabled() const {
        return m_enabled;
    }

    static std::string type_name;

    virtual std::string get_type_name() {
        return type_name;
    }
};

class uvm_callbacks_base;

//------------------------------------------------------------------------------
// Class - uvm_typeid_base
//------------------------------------------------------------------------------

class uvm_typeid_base {
public:
    static std::string type_name;
    static std::unordered_map<uvm_typeid_base*, uvm_callbacks_base*> typeid_map;
    static std::unordered_map<uvm_callbacks_base*, uvm_typeid_base*> type_map;
};

//------------------------------------------------------------------------------
// Class - uvm_typeid#(T)
//------------------------------------------------------------------------------

template <typename T = uvm_object>
class uvm_typeid : public uvm_typeid_base {
public:
    static uvm_typeid<T>* get() {
        static uvm_typeid<T> instance;
        return &instance;
    }
};

//------------------------------------------------------------------------------
// Class - uvm_callbacks_base
//------------------------------------------------------------------------------

class uvm_callbacks_base : public uvm_object {
public:
    typedef uvm_callbacks_base this_type;

protected:
    static this_type* m_b_inst;
    static std::unordered_map<uvm_object*, std::vector<uvm_callback*>> m_pool;

public:
    static bool m_tracing;

    static this_type* m_initialize() {
        if (m_b_inst == nullptr) {
            m_b_inst = new this_type();
        }
        return m_b_inst;
    }

    static void m_get_q(std::vector<uvm_callback*>& q, uvm_object* obj) {
        if (m_pool.find(obj) == m_pool.end()) {
            q.clear();
            m_pool[obj] = q;
        } else {
            q = m_pool[obj];
        }
    }

    virtual bool m_am_i_a(uvm_object* obj) { return false; }
    virtual bool m_is_for_me(uvm_callback* cb) { return false; }
    virtual bool m_is_registered(uvm_object* obj, uvm_callback* cb) { return false; }
    virtual std::vector<uvm_callback*>* m_get_tw_cb_q(uvm_object* obj) { return nullptr; }
    virtual void m_add_tw_cbs(uvm_callback* cb, bool ordering) {}
    virtual bool m_delete_tw_cbs(uvm_callback* cb) { return false; }

    bool check_registration(uvm_object* obj, uvm_callback* cb) {
        if (m_is_registered(obj, cb)) {
            return true;
        }

        for (auto& type : m_this_type) {
            if (m_b_inst != type && type->m_is_registered(obj, cb)) {
                return true;
            }
        }

        if (obj == nullptr) {
            for (auto& type : m_derived_types) {
                uvm_callbacks_base* dt = uvm_typeid_base::typeid_map[type];
                if (dt != nullptr && dt->check_registration(nullptr, cb)) {
                    return true;
                }
            }
        }

        return false;
    }

    std::vector<this_type*> m_this_type;
    uvm_typeid_base* m_super_type;
    std::vector<uvm_typeid_base*> m_derived_types;
};

//------------------------------------------------------------------------------
// Class - uvm_typed_callbacks#(T)
//------------------------------------------------------------------------------

template <typename T = uvm_object>
class uvm_typed_callbacks : public uvm_callbacks_base {
public:
    typedef uvm_typed_callbacks<T> this_type;
    typedef uvm_callbacks_base super_type;

    static std::vector<uvm_callback*> m_tw_cb_q;
    static std::string m_typename;

    static this_type* m_initialize() {
        if (m_t_inst == nullptr) {
            super_type::m_initialize();
            m_t_inst = new this_type();
        }
        return m_t_inst;
    }

    virtual bool m_am_i_a(uvm_object* obj) override {
        T* this_type = dynamic_cast<T*>(obj);
        return this_type != nullptr;
    }

    virtual std::vector<uvm_callback*>* m_get_tw_cb_q(uvm_object* obj) override {
        if (m_am_i_a(obj)) {
            for (size_t i = 0; i < m_derived_types.size(); ++i) {
                super_type* dt = uvm_typeid_base::typeid_map[m_derived_types[i]];
                if (dt != nullptr && dt != this) {
                    auto cb_q = dt->m_get_tw_cb_q(obj);
                    if (cb_q != nullptr) {
                        return cb_q;
                    }
                }
            }
            return &m_tw_cb_q;
        } else {
            return nullptr;
        }
    }

    static int m_cb_find(const std::vector<uvm_callback*>& q, uvm_callback* cb) {
        for (size_t i = 0; i < q.size(); ++i) {
            if (q[i] == cb) {
                return i;
            }
        }
        return -1;
    }

    static bool m_cb_find_name(const std::vector<uvm_callback*>& q, const std::string& name, const std::string& where) {
        for (size_t i = 0; i < q.size(); ++i) {
            if (q[i]->get_name() == name) {
                std::cerr << "UVM/CB/NAM/SAM: A callback named \"" << name << "\" is already registered with " << where << std::endl;
                return true;
            }
        }
        return false;
    }

    virtual void m_add_tw_cbs(uvm_callback* cb, bool ordering) override {
        bool warned;
        if (m_cb_find(m_tw_cb_q, cb) == -1) {
            warned = m_cb_find_name(m_tw_cb_q, cb->get_name(), "type");
            if (ordering == true) {
                m_tw_cb_q.push_back(cb);
            } else {
                m_tw_cb_q.insert(m_tw_cb_q.begin(), cb);
            }
        }

        for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
            T* me = dynamic_cast<T*>(it->first);
            if (me != nullptr) {
                auto& q = it->second;
                if (m_cb_find(q, cb) == -1) {
                    if (!warned) {
                        m_cb_find_name(q, cb->get_name(), "object instance " + me->get_full_name());
                    }
                    if (ordering == true) {
                        q.push_back(cb);
                    } else {
                        q.insert(q.begin(), cb);
                    }
                }
            }
        }

        for (size_t i = 0; i < m_derived_types.size(); ++i) {
            auto cb_pair = uvm_typeid_base::typeid_map[m_derived_types[i]];
            if (cb_pair != this) {
                cb_pair->m_add_tw_cbs(cb, ordering);
            }
        }
    }

    virtual bool m_delete_tw_cbs(uvm_callback* cb) override {
        bool found = false;
        auto pos = m_cb_find(m_tw_cb_q, cb);
        if (pos != -1) {
            m_tw_cb_q.erase(m_tw_cb_q.begin() + pos);
            found = true;
        }

        for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
            auto& q = it->second;
            pos = m_cb_find(q, cb);
            if (pos != -1) {
                q.erase(q.begin() + pos);
                found = true;
            }
        }

        for (size_t i = 0; i < m_derived_types.size(); ++i) {
            auto cb_pair = uvm_typeid_base::typeid_map[m_derived_types[i]];
            if (cb_pair != this) {
                found |= cb_pair->m_delete_tw_cbs(cb);
            }
        }
        return found;
    }

    static void display(T* obj = nullptr) {
        T* me = nullptr;
        auto& ib = m_t_inst;
        std::vector<std::string> cbq;
        std::vector<std::string> inst_q;
        std::vector<std::string> mode_q;
        std::string blanks = "                             ";
        uvm_object* bobj = obj;

        std::vector<uvm_callback*> q;
        std::string tname, str;

        int max_cb_name = 0, max_inst_name = 0;

        m_tracing = false; // don't allow tracing during display

        if (!m_typename.empty()) {
            tname = m_typename;
        } else if (obj != nullptr) {
            tname = obj->get_type_name();
        } else {
            tname = "*";
        }

        q = m_tw_cb_q;
        for (size_t i = 0; i < q.size(); ++i) {
            auto cb = q[i];
            cbq.push_back(cb->get_name());
            inst_q.push_back("(*)");
            mode_q.push_back(cb->is_enabled() ? "ON" : "OFF");

            str = cb->get_name();
            max_cb_name = std::max(max_cb_name, static_cast<int>(str.size()));
            str = "(*)";
            max_inst_name = std::max(max_inst_name, static_cast<int>(str.size()));
        }

        if (obj == nullptr) {
            for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
                me = dynamic_cast<T*>(it->first);
                if (me != nullptr) {
                    break;
                }
            }
            if (me != nullptr || !m_tw_cb_q.empty()) {
                std::cout << "Registered callbacks for all instances of " << tname << std::endl;
                std::cout << "---------------------------------------------------------------" << std::endl;
            }
            if (me != nullptr) {
                for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
                    me = dynamic_cast<T*>(it->first);
                    if (me != nullptr) {
                        q = it->second;
                        if (q.empty()) {
                            q.clear();
                            m_pool[it->first] = q;
                        }
                        for (size_t i = 0; i < q.size(); ++i) {
                            auto cb = q[i];
                            cbq.push_back(cb->get_name());
                            inst_q.push_back(it->first->get_full_name());
                            mode_q.push_back(cb->is_enabled() ? "ON" : "OFF");

                            str = cb->get_name();
                            max_cb_name = std::max(max_cb_name, static_cast<int>(str.size()));
                            str = it->first->get_full_name();
                            max_inst_name = std::max(max_inst_name, static_cast<int>(str.size()));
                        }
                    }
                }
            } else {
                std::cout << "No callbacks registered for any instances of type " << tname << std::endl;
            }
        } else {
            if (m_pool.find(bobj) != m_pool.end() || !m_tw_cb_q.empty()) {
                std::cout << "Registered callbacks for instance " << obj->get_full_name() << " of " << tname << std::endl;
                std::cout << "---------------------------------------------------------------" << std::endl;
            }
            if (m_pool.find(bobj) != m_pool.end()) {
                q = m_pool[bobj];
                if (q.empty()) {
                    q.clear();
                    m_pool[bobj] = q;
                }
                for (size_t i = 0; i < q.size(); ++i) {
                    auto cb = q[i];
                    cbq.push_back(cb->get_name());
                    inst_q.push_back(bobj->get_full_name());
                    mode_q.push_back(cb->is_enabled() ? "ON" : "OFF");

                    str = cb->get_name();
                    max_cb_name = std::max(max_cb_name, static_cast<int>(str.size()));
                    str = bobj->get_full_name();
                    max_inst_name = std::max(max_inst_name, static_cast<int>(str.size()));
                }
            }
        }

        if (cbq.empty()) {
            if (obj == nullptr) {
                str = "*";
            } else {
                str = obj->get_full_name();
            }
            std::cout << "No callbacks registered for instance " << str << " of type " << tname << std::endl;
        }

        for (size_t i = 0; i < cbq.size(); ++i) {
            std::cout << cbq[i] << "  " << blanks.substr(0, max_cb_name - cbq[i].size() - 1) << " on " << inst_q[i] << "  " << blanks.substr(0, max_inst_name - inst_q[i].size() - 1) << mode_q[i] << std::endl;
        }

        m_tracing = true; // allow tracing to be resumed
    }

private:
    static this_type* m_t_inst;
};

template <typename T>
std::vector<uvm_callback*> uvm_typed_callbacks<T>::m_tw_cb_q;

template <typename T>
std::string uvm_typed_callbacks<T>::m_typename;

template <typename T>
typename uvm_typed_callbacks<T>::this_type* uvm_typed_callbacks<T>::m_t_inst = nullptr;

//------------------------------------------------------------------------------
// Class - uvm_callbacks #(T, CB)
//------------------------------------------------------------------------------

template <typename T = uvm_object, typename CB = uvm_callback>
class uvm_callbacks : public uvm_typed_callbacks<T> {
public:
    typedef uvm_typed_callbacks<T> super_type;
    typedef uvm_callbacks<T, CB> this_type;

    static std::string m_cb_typename;

    static this_type* get() {
        static this_type instance;
        return &instance;
    }

    static bool m_register_pair(const std::string& tname = "", const std::string& cbname = "") {
        this_type* inst = get();

        if (!tname.empty()) {
            super_type::m_typename = tname;
        }
        if (!cbname.empty()) {
            m_cb_typename = cbname;
        }

        inst->m_registered = true;
        return true;
    }

    virtual bool m_is_registered(uvm_object* obj, uvm_callback* cb) override {
        if (this->m_is_for_me(cb) && this->m_am_i_a(obj)) {
            return this->m_registered;
        }
        return false;
    }

    virtual bool m_is_for_me(uvm_callback* cb) override {
        CB* this_cb = dynamic_cast<CB*>(cb);
        return this_cb != nullptr;
    }

    static void add(T* obj, CB* cb, bool ordering = true) {
        std::vector<uvm_callback*>* q;
        std::string nm, tnm;

        this_type* inst = get();

        if (cb == nullptr) {
            if (obj == nullptr) {
                nm = "(*)";
            } else {
                nm = obj->get_full_name();
            }

            if (!inst->super_type::m_typename.empty()) {
                tnm = inst->super_type::m_typename;
            } else if (obj != nullptr) {
                tnm = obj->get_type_name();
            } else {
                tnm = "uvm_object";
            }

            std::cerr << "CBUNREG: Null callback object cannot be registered with object " << nm << " (" << tnm << ")" << std::endl;
            return;
        }

        if (!inst->super_type::check_registration(obj, cb)) {
            if (obj == nullptr) {
                nm = "(*)";
            } else {
                nm = obj->get_full_name();
            }

            if (!inst->super_type::m_typename.empty()) {
                tnm = inst->super_type::m_typename;
            } else if (obj != nullptr) {
                tnm = obj->get_type_name();
            } else {
                tnm = "uvm_object";
            }

            std::cerr << "CBUNREG: Callback " << cb->get_name() << " cannot be registered with object " << nm << " because callback type " << cb->get_type_name() << " is not registered with object type " << tnm << std::endl;
        }

        if (obj == nullptr) {
            if (super_type::m_cb_find(super_type::m_tw_cb_q, cb) != -1) {
                if (!inst->super_type::m_typename.empty()) {
                    tnm = inst->super_type::m_typename;
                } else {
                    tnm = "uvm_object";
                }

                std::cerr << "CBPREG: Callback object " << cb->get_name() << " is already registered with type " << tnm << std::endl;
            } else {
                //super_type::m_add_tw_cbs(cb, ordering); FIXME
            }
        } else {
            q = &super_type::m_pool[obj];

            if (q == nullptr) {
                q = new std::vector<uvm_callback*>;
                //super_type::m_pool[obj] = q;
            }

            if (q->empty()) {
                uvm_report_object* o = dynamic_cast<uvm_report_object*>(obj);

                if (o != nullptr) {
                    auto qr = uvm_callbacks<uvm_report_object, uvm_callback>::get()->m_tw_cb_q;
                    for (size_t i = 0; i < qr.size(); ++i) {
                        q->push_back(qr[i]);
                    }
                }

                for (size_t i = 0; i < super_type::m_tw_cb_q.size(); ++i) {
                    q->push_back(super_type::m_tw_cb_q[i]);
                }
            }

            if (super_type::m_cb_find(*q, cb) != -1) {
                std::cerr << "CBPREG: Callback object " << cb->get_name() << " is already registered with object " << obj->get_full_name() << std::endl;
            } else {
                super_type::m_cb_find_name(*q, cb->get_name(), "object instance " + obj->get_full_name());
                if (ordering == true) {
                    q->push_back(cb);
                } else {
                    q->insert(q->begin(), cb);
                }
            }
        }
    }

    static void delete_cb(T* obj, CB* cb) {
        std::vector<uvm_callback*>* q;
        bool found = false;
        int pos;
        this_type* inst = get();

        if (obj == nullptr) {
            found = super_type::m_delete_tw_cbs(cb);
        } else {
            q = &super_type::m_pool[obj];
            pos = super_type::m_cb_find(*q, cb);
            if (pos != -1) {
                q->erase(q->begin() + pos);
                found = true;
            }
        }
        if (!found) {
            std::string nm = (obj == nullptr) ? "(*)" : obj->get_full_name();
            std::cerr << "CBUNREG: Callback " << cb->get_name() << " cannot be removed from object " << nm << " because it is not currently registered to that object." << std::endl;
        }
    }

    static void add_by_name(const std::string& name, CB* cb, uvm_component* root, bool ordering = true) {
	/*
        std::vector<uvm_component*> cq;
        uvm_root* top = uvm_root::get();
        T* t;

        this_type* inst = get();

        if (cb == nullptr) {
            std::cerr << "CBUNREG: Null callback object cannot be registered with object(s) " << name << std::endl;
            return;
        }

        top->find_all(name, cq, root);
        if (cq.empty()) {
            std::cerr << "CBNOMTC: add_by_name failed to find any components matching the name " << name << ", callback " << cb->get_name() << " will not be registered." << std::endl;
        }

        for (size_t i = 0; i < cq.size(); ++i) {
            if ((t = dynamic_cast<T*>(cq[i])) != nullptr) {
                add(t, cb, ordering);
            }
        }
	*/
    }

    static void delete_by_name(const std::string& name, CB* cb, uvm_component* root) {
	/*
        std::vector<uvm_component*> cq;
        uvm_root* top = uvm_root::get();
        T* t;

        this_type* inst = get();

        top->find_all(name, cq, root);
        if (cq.empty()) {
            std::cerr << "CBNOMTC: delete_by_name failed to find any components matching the name " << name << ", callback " << cb->get_name() << " will not be unregistered." << std::endl;
        }

        for (size_t i = 0; i < cq.size(); ++i) {
            if ((t = dynamic_cast<T*>(cq[i])) != nullptr) {
                delete_cb(t, cb);
            }
        }
	*/
    }

    static void m_get_q(std::vector<uvm_callback*>& q, T* obj) {
	/* FIXME
        if (super_type::m_pool.find(obj) == super_type::m_pool.end()) {
            q = (obj == nullptr) ? super_type::m_tw_cb_q : super_type::m_get_tw_cb_q(obj);
        } else {
            q = super_type::m_pool[obj];
        }
	*/
    }

    static CB* get_first(int&  itr, T* obj) {
        std::vector<uvm_callback*> q;
        CB* cb = nullptr;

        get();

        m_get_q(q, obj);
        for (itr = 0; itr < q.size(); ++itr) {
            if ((cb = dynamic_cast<CB*>(q[itr])) != nullptr && cb->callback_mode()) {
                return cb;
            }
        }
        return nullptr;
    }

    static CB* get_last(int&  itr, T* obj) {
        std::vector<uvm_callback*> q;
        CB* cb = nullptr;

        get();

        m_get_q(q, obj);
        for (itr = q.size() - 1; itr >= 0; --itr) {
            if ((cb = dynamic_cast<CB*>(q[itr])) != nullptr && cb->callback_mode()) {
                return cb;
            }
        }
        return nullptr;
    }

    static CB* get_next(int&  itr, T* obj) {
        std::vector<uvm_callback*> q;
        CB* cb = nullptr;

        get();

        m_get_q(q, obj);
        for (++itr; itr < q.size(); ++itr) {
            if ((cb = dynamic_cast<CB*>(q[itr])) != nullptr && cb->callback_mode()) {
                return cb;
            }
        }
        return nullptr;
    }

    static CB* get_prev(int&  itr, T* obj) {
        std::vector<uvm_callback*> q;
        CB* cb = nullptr;

        get();

        m_get_q(q, obj);
        for (--itr; itr >= 0; --itr) {
            if ((cb = dynamic_cast<CB*>(q[itr])) != nullptr && cb->callback_mode()) {
                return cb;
            }
        }
        return nullptr;
    }

    static void display(T* obj = nullptr) {
        this_type* inst = get();
        super_type::display(obj);
    }

private:
    static this_type* m_inst;
    bool m_registered = false;
};

template <typename T, typename CB>
std::string uvm_callbacks<T, CB>::m_cb_typename;

template <typename T, typename CB>
typename uvm_callbacks<T, CB>::this_type* uvm_callbacks<T, CB>::m_inst = nullptr;

//------------------------------------------------------------------------------
// Class - uvm_callback_iter #(T, CB)
//------------------------------------------------------------------------------

template <typename T = uvm_object, typename CB = uvm_callback>
class uvm_callback_iter {
public:
    int m_i;
    T* m_obj;
    CB* m_cb;

    uvm_callback_iter(T* obj) : m_i(0), m_obj(obj), m_cb(nullptr) {}

    CB* first() {
        m_cb = uvm_callbacks<T, CB>::get_first(m_i, m_obj);
        return m_cb;
    }

    CB* last() {
        m_cb = uvm_callbacks<T, CB>::get_last(m_i, m_obj);
        return m_cb;
    }

    CB* next() {
        m_cb = uvm_callbacks<T, CB>::get_next(m_i, m_obj);
        return m_cb;
    }

    CB* prev() {
        m_cb = uvm_callbacks<T, CB>::get_prev(m_i, m_obj);
        return m_cb;
    }

    CB* get_cb() const {
        return m_cb;
    }
};

#endif // UVM_CALLBACK_H
