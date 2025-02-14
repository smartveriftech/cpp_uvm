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

#ifndef UVM_PRINTER_H
#define UVM_PRINTER_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip> // Included for stream manipulators

#include "base/uvm_globals.h"
#include "base/uvm_object_globals.h"
#include "base/uvm_misc.h"

class uvm_object;
class uvm_scope_stack;

const int UVM_STDOUT = 1;  // Writes to standard out and logfile

// Struct for printer row info
struct uvm_printer_row_info {
    int level;
    std::string name;
    std::string type_name;
    std::string size;
    std::string val;
};

// Class for printer knobs
class uvm_printer_knobs {
public:
    bool header = true;
    bool footer = true;
    bool full_name = false;
    bool identifier = true;
    bool type_name = true;
    bool size = true;
    int depth = -1;
    bool reference = true;
    u_int32_t begin_elements = 5;
    u_int32_t end_elements = 5;
    std::string prefix = "";
    u_int32_t indent = 2;
    bool show_root = false;
    u_int32_t mcd = UVM_STDOUT;
    std::string separator = "{}";
    bool show_radix = true;
    uvm_radix_enum default_radix = UVM_HEX; // Set default radix to HEX
    std::string dec_radix = "'d";
    std::string bin_radix = "'b";
    std::string oct_radix = "'o";
    std::string unsigned_radix = "'d";
    std::string hex_radix = "'h";
    std::string MDC = "UVM_STDOUT";

    std::string get_radix_str(uvm_radix_enum radix) const;
};

// Base uvm_printer class
class uvm_printer {
public:
    uvm_printer_knobs* knobs;
    std::vector<uvm_printer_row_info> m_rows;

    uvm_printer();

    virtual ~uvm_printer() = default;
    
    // Virtual print methods
    virtual void print_int(const std::string& name, uint64_t value, u_int32_t size = 32, uvm_radix_enum radix = UVM_NORADIX, const std::string& scope_separator = ".", const std::string& type_name = "");
    virtual void print_object(const std::string& name, uvm_object* value, const std::string& scope_separator = ".");
    virtual void print_object_header(const std::string& name, uvm_object* value, const std::string& scope_separator = ".");

    virtual void print_string(const std::string& name, const std::string& value, const std::string& scope_separator = ".");
    virtual void print_time(const std::string& name, time_t value, const std::string& scope_separator = ".");
    virtual void print_real(const std::string& name, double value, const std::string& scope_separator = ".");
    virtual void print_generic(const std::string& name, const std::string& type_name, u_int32_t size, const std::string& value, const std::string& scope_separator = ".");
    
    // Formatting methods
    virtual std::string emit();
    virtual std::string format_row(const uvm_printer_row_info& row);
    virtual std::string format_header();
    virtual std::string format_footer();
    virtual std::string adjust_name(const std::string& id, const std::string& scope_separator = ".");
    
    // Array printing methods
    virtual void print_array_header(const std::string& name, u_int32_t size, const std::string& arraytype = "array", const std::string& scope_separator = ".");
    virtual void print_array_range(int min, int max);
    virtual void print_array_footer(u_int32_t size = 0);
    
    // Utility methods
    virtual bool istop();
    virtual std::string index_string(u_int32_t index, const std::string& name);

//protected:
    std::vector<int> m_array_stack;
    uvm_scope_stack* m_scope;
    std::string m_string;
};

// Subclass: uvm_table_printer
class uvm_table_printer : public uvm_printer {
public:
    uvm_table_printer();
    virtual std::string emit() override;

private:
    u_int32_t m_max_name;
    u_int32_t m_max_type;
    u_int32_t m_max_size;
    u_int32_t m_max_value;

    void calculate_max_widths();
};

// Subclass: uvm_tree_printer
class uvm_tree_printer : public uvm_printer {
public:
    uvm_tree_printer();
    virtual std::string emit() override;
    std::string newline;
};

// Subclass: uvm_line_printer
class uvm_line_printer : public uvm_tree_printer {
public:
    uvm_line_printer();
};

#endif // UVM_PRINTER_H