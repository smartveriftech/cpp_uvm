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

#ifndef UVM_CMDLINE_PROCESSOR_H
#define UVM_CMDLINE_PROCESSOR_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>

#include "base/uvm_object_globals.h"

class uvm_cmd_line_verb {
public:
    std::string comp_path;
    std::string id;
    uvm_verbosity verb;
    int exec_time;
};


// Class: uvm_cmdline_processor
//
// This class provides an interface to the command line arguments that 
// were provided for the given simulation.  The class is intended to be
// used as a singleton, but that isn't required.  The generation of the
// data structures which hold the command line argument information 
// happens during construction of the class object.  A global variable 
// called ~uvm_cmdline_proc~ is created at initialization time and may 
// be used to access command line information.
//
// The uvm_cmdline_processor class also provides support for setting various UVM
// variables from the command line such as components' verbosities and configuration
// settings for integral types and strings.  Each of these capablities is described 
// in the Built-in UVM Aware Command Line Arguments section.

class uvm_cmdline_processor {
private:
    static uvm_cmdline_processor* m_inst;

    std::vector<std::string> m_argv;
    std::vector<std::string> m_plus_argv;
    std::vector<std::string> m_uvm_argv;

    uvm_cmdline_processor();

public:
    // Group: Singleton 

    // Function: get_inst
    // Returns the singleton instance of the UVM command line processor.
    static uvm_cmdline_processor* get_inst();

    //API for dpi add sv args
    void add_arg(std::string& arg);
    void add_args(std::vector<std::string>& args); 

    void get_args(std::vector<std::string>& args);
    void get_plusargs(std::vector<std::string>& args);
    void get_uvm_args(std::vector<std::string>& args);
    int get_arg_matches(const std::string& match, std::vector<std::string>& args);
    int get_arg_value(const std::string& match, std::string& value);
    int get_arg_values(const std::string& match, std::vector<std::string>& values);
    std::string get_tool_name();
    std::string get_tool_version();
    bool m_convert_verb(const std::string& verb_str, uvm_verbosity& verb_enum);
};

extern const uvm_cmdline_processor* uvm_cmdline_proc;

// Ensure C linkage for DPI functions
extern "C" {
    extern void uvm_cmdline_processor_add_args();
}

#endif // UVM_CMDLINE_PROCESSOR_H