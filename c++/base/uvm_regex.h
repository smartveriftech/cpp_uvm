//
//------------------------------------------------------------------------------
// Copyright 2010-2011 Mentor Graphics Corporation
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

#ifndef UVM_REGEX_H
#define UVM_REGEX_H

#include <string>

inline int uvm_re_match(const std::string& re, const std::string& str) {
    size_t e = 0, s = 0;
    size_t es = 0, ss = 0;
    std::string re_copy = re;

    if (re_copy.empty())
        return 0;

    // The ^ used to be used to remove the implicit wildcard, but now we don't
    // use implicit wildcard so this character is just stripped.
    if (re_copy[0] == '^')
        re_copy = re_copy.substr(1);

    // This loop is only needed when the first character of the re may not be a *.
    while (s < str.size() && re_copy[e] != '*') {
        if (re_copy[e] != str[s] && re_copy[e] != '?')
            return 1;
        ++e;
        ++s;
    }

    while (s < str.size()) {
        if (re_copy[e] == '*') {
            ++e;
            if (e == re_copy.size())
                return 0;
            es = e;
            ss = s + 1;
        } else if (re_copy[e] == str[s] || re_copy[e] == '?') {
            ++e;
            ++s;
        } else {
            e = es;
            s = ss++;
        }
    }

    while (re_copy[e] == '*')
        ++e;

    return (e == re_copy.size()) ? 0 : 1;
}

//inline void uvm_dump_re_cache() {
//    // No implementation needed
//}

//inline std::string uvm_glob_to_re(const std::string& glob) {
//    return glob;
//}

#endif // UVM_REGEX_H
