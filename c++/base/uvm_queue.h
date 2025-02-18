//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Cadence Design Systems, Inc.
// Copyright 2007-2010 Mentor Graphics Corporation
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

#ifndef UVM_QUEUE_H
#define UVM_QUEUE_H

#include <vector>
#include <string>
#include "base/uvm_object.h"

template <typename T = int>
class uvm_queue : public uvm_object {
public:
    uvm_queue(const std::string& name = "");
    virtual ~uvm_queue() {}

    static uvm_queue<T>* get_global_queue();
    static T get_global(int index);

    virtual T get(int index);
    virtual int size() const;

    virtual void insert(int index, const T& item);
    virtual void remove(int index = -1);

    virtual T pop_front();
    virtual T pop_back();
    virtual void push_front(const T& item);
    virtual void push_back(const T& item);

    virtual uvm_object* create(const std::string& name = "") const;
    virtual std::string get_type_name();
    virtual void do_copy(uvm_object* rhs);

    virtual std::string convert2string() const;

    static const std::string type_name;

protected:
    static uvm_queue<T>* m_global_queue;
    std::vector<T> queue;
};

template <typename T>
const std::string uvm_queue<T>::type_name = "uvm_queue";

template <typename T>
uvm_queue<T>* uvm_queue<T>::m_global_queue = nullptr;

template <typename T>
uvm_queue<T>::uvm_queue(const std::string& name)
    : uvm_object(name) {}

template <typename T>
uvm_queue<T>* uvm_queue<T>::get_global_queue() {
    if (!m_global_queue) {
        m_global_queue = new uvm_queue<T>("global_queue");
    }
    return m_global_queue;
}

template <typename T>
T uvm_queue<T>::get_global(int index) {
    uvm_queue<T>* gqueue = get_global_queue();
    return gqueue->get(index);
}

template <typename T>
T uvm_queue<T>::get(int index) {
    if (index >= size() || index < 0) {
        uvm_warning("QUEUEGET",
            "get: given index out of range for queue of size " + std::to_string(size()) + ". Ignoring get request");
        return T();
    }
    return queue[index];
}

template <typename T>
int uvm_queue<T>::size() const {
    return queue.size();
}

template <typename T>
void uvm_queue<T>::insert(int index, const T& item) {
    if (index >= size() || index < 0) {
        uvm_warning("QUEUEINS",
            "insert: given index out of range for queue of size " + std::to_string(size()) + ". Ignoring insert request");
        return;
    }
    queue.insert(queue.begin() + index, item);
}

template <typename T>
void uvm_queue<T>::remove(int index) {
    if (index >= size() || index < -1) {
        uvm_warning("QUEUEDEL",
            "delete: given index out of range for queue of size " + std::to_string(size()) + ". Ignoring delete request");
        return;
    }
    if (index == -1) {
        queue.clear();
    } else {
        queue.erase(queue.begin() + index);
    }
}

template <typename T>
T uvm_queue<T>::pop_front() {
    if (queue.empty()) {
        return T();
    }
    T item = queue.front();
    queue.erase(queue.begin());
    return item;
}

template <typename T>
T uvm_queue<T>::pop_back() {
    if (queue.empty()) {
        return T();
    }
    T item = queue.back();
    queue.pop_back();
    return item;
}

template <typename T>
void uvm_queue<T>::push_front(const T& item) {
    queue.insert(queue.begin(), item);
}

template <typename T>
void uvm_queue<T>::push_back(const T& item) {
    queue.push_back(item);
}

template <typename T>
uvm_object* uvm_queue<T>::create(const std::string& name) const {
    return new uvm_queue<T>(name);
}

template <typename T>
std::string uvm_queue<T>::get_type_name() {
    return type_name;
}

template <typename T>
void uvm_queue<T>::do_copy(uvm_object* rhs) {
    const uvm_queue<T>* p = dynamic_cast<const uvm_queue<T>*>(rhs);
    if (!p) {
        return;
    }
    queue = p->queue;
}

template <typename T>
std::string uvm_queue<T>::convert2string() const {
    std::string result = "{";
    for (size_t i = 0; i < queue.size(); ++i) {
        result += std::to_string(queue[i]);
        if (i != queue.size() - 1) {
            result += ", ";
        }
    }
    result += "}";
    return result;
}

#endif // UVM_QUEUE_H