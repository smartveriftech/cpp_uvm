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

#ifndef UVM_GLOBALS_H
#define UVM_GLOBALS_H

#include <string>
#include <regex>
#include <vector>
#include <queue>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <unordered_map>
#include <cstdarg>
#include <thread>
#include <chrono>
#include <atomic>
#include <ctime>
#include <mutex>
#include <condition_variable>

#include "base/uvm_object_globals.h"

#include "dpi/uvm_globals_dpi.h"

// Forward declarations of classes used in the functions
class uvm_object;
class uvm_test_done_objection;

// Title: Globals

//------------------------------------------------------------------------------
//
// Group: Simulation Control
//
//------------------------------------------------------------------------------

// Task: run_test
//
// Convenience function for uvm_top.run_test(). See <uvm_root> for more
// information.
void run_test(const std::string& test_name = "");

#ifndef UVM_NO_DEPRECATED
// Variable- uvm_test_done - DEPRECATED
//
// An instance of the <uvm_test_done_objection> class, this object is
// used by components to coordinate when to end the currently running
// task-based phase. When all participating components have dropped their
// raised objections, an implicit call to <global_stop_request> is issued
// to end the run phase (or any other task-based phase).
extern uvm_test_done_objection* uvm_test_done;

// Method- global_stop_request  - DEPRECATED
//
// Convenience function for uvm_test_done.stop_request(). See 
// <uvm_test_done_objection::stop_request> for more information.

void global_stop_request();

// Method- set_global_timeout  - DEPRECATED
//
// Convenience function for uvm_top.set_timeout(). See 
// <uvm_root::set_timeout> for more information.  The overridable bit 
// controls whether subsequent settings will be honored.

void set_global_timeout(int timeout, bool overridable = true);

// Function- set_global_stop_timeout - DEPRECATED
//
// Convenience function for uvm_test_done.stop_timeout = timeout.
// See <uvm_uvm_test_done::stop_timeout> for more information.
void set_global_stop_timeout(int timeout);

#endif //UVM_NO_DEPRECATED

//----------------------------------------------------------------------------
//
// Group: Reporting
//
//----------------------------------------------------------------------------

// Function: uvm_report_enabled
//
// Returns 1 if the configured verbosity in ~uvm_top~ is greater than 
// ~verbosity~ and the action associated with the given ~severity~ and ~id~
// is not UVM_NO_ACTION, else returns 0.
// 
// See also <uvm_report_object::uvm_report_enabled>.
//
//
// Static methods of an extension of uvm_report_object, e.g. uvm_compoent-based
// objects, can not call ~uvm_report_enabled~ because the call will resolve to
// the <uvm_report_object::uvm_report_enabled>, which is non-static.
// Static methods can not call non-static methods of the same class. 
bool uvm_report_enabled(int verbosity, uvm_severity severity = UVM_INFO, const std::string& id = "");
void uvm_report_info(const std::string& id, const std::string& message, int verbosity = 1, const std::string& filename = "", int line = 0);
void uvm_report_warning(const std::string& id, const std::string& message, int verbosity = 1, const std::string& filename = "", int line = 0);
void uvm_report_error(const std::string& id, const std::string& message, int verbosity = 0, const std::string& filename = "", int line = 0);

// Function: uvm_report_fatal
//
// These methods, defined in package scope, are convenience functions that
// delegate to the corresponding component methods in ~uvm_top~. They can be
// used in module-based code to use the same reporting mechanism as class-based
// components. See <uvm_report_object> for details on the reporting mechanism. 
//
// *Note:* Verbosity is ignored for warnings, errors, and fatals to ensure users
// do not inadvertently filter them out. It remains in the methods for backward
// compatibility.
void uvm_report_fatal(const std::string& id, const std::string& message, int verbosity = -1, const std::string& filename = "", int line = 0);

bool uvm_string_to_severity(const std::string& sev_str, uvm_severity& sev);

bool uvm_string_to_action(const std::string& action_str, uvm_action& action);
  
//------------------------------------------------------------------------------
//
// Group: Configuration
//
//------------------------------------------------------------------------------

// Function: set_config_int
//
// This is the global version of set_config_int in <uvm_component>. This
// function places the configuration setting for an integral field in a
// global override table, which has highest precedence over any
// component-level setting.  See <uvm_component::set_config_int> for
// details on setting configuration.
void set_config_int(const std::string& inst_name, const std::string& field_name, int value);

void set_config_object(const std::string& inst_name, const std::string& field_name, uvm_object* value, bool clone = true);

void set_config_string(const std::string& inst_name, const std::string& field_name, const std::string& value);

std::string uvm_glob_to_re(const std::string& glob);

//----------------------------------------------------------------------------
//
// Group: Miscellaneous
//
//----------------------------------------------------------------------------


// Function: uvm_is_match
//
// Returns 1 if the two strings match, 0 otherwise.
//
// The first string, ~expr~, is a string that may contain '*' and '?'
// characters. A * matches zero or more characters, and ? matches any single
// character. The 2nd argument, ~str~, is the string begin matched against.
// It must not contain any wildcards.
//
//----------------------------------------------------------------------------
bool uvm_is_match(const std::string& expr, const std::string& str);

// Define UVM_LINE_WIDTH if not already defined
#ifndef UVM_LINE_WIDTH
#define UVM_LINE_WIDTH 120
#endif

// Define UVM_NUM_LINES if not already defined
#ifndef UVM_NUM_LINES
#define UVM_NUM_LINES 120
#endif

const int UVM_LINE_WIDTH_CONST = UVM_LINE_WIDTH;
const int UVM_NUM_LINES_CONST = UVM_NUM_LINES;

const int UVM_SMALL_STRING_CONST = UVM_LINE_WIDTH_CONST * 8 - 1;
const int UVM_LARGE_STRING_CONST = UVM_LINE_WIDTH_CONST * UVM_NUM_LINES_CONST * 8 - 1;

//----------------------------------------------------------------------------
//
// Function: uvm_string_to_bits
//
// Converts an input string to its bit-vector equivalent. Max bit-vector
// length is approximately 14000 characters.
//----------------------------------------------------------------------------
std::string uvm_string_to_bits(const std::string& str);   //FIXME

//----------------------------------------------------------------------------
//
// Function: uvm_bits_to_string
//
// Converts an input bit-vector to its string equivalent. Max bit-vector
// length is approximately 14000 characters.
//----------------------------------------------------------------------------
std::string uvm_bits_to_string(const std::string& bits);  //FIXME

//----------------------------------------------------------------------------
//
// Task: uvm_wait_for_nba_region
//
// Callers of this task will not return until the NBA region, thus allowing
// other processes any number of delta cycles (#0) to settle out before
// continuing. See <uvm_sequencer_base::wait_for_sequences> for example usage.
//
//----------------------------------------------------------------------------
void uvm_wait_for_nba_region();

//----------------------------------------------------------------------------
//
// Function: uvm_split_string
//
// Returns a queue of strings, ~values~, that is the result of the ~str~ split
// based on the ~sep~.  For example:
//
//| uvm_split_string("1,on,false", ",", splits);
//
// Results in the 'splits' queue containing the three elements: 1, on and 
// false.
//----------------------------------------------------------------------------
void uvm_split_string(const std::string& str, char sep, std::vector<std::string>& values);

///////////////////////////////////////////////////////////////////////////////////////////
void fwrite(const char* filename, const char* format, ...);

/// @brief ///////////////////////
class uvm_print {
public:
    static std::string psprintf(const char* format, ...);
    static void display(const char* format, ...);
    static void fdisplay(int file_id, const char* format, ...);
private:
    static std::string vformat(const char* format, va_list args);

public:
    // Open a file and return its file ID
    static int open_file(const std::string& filename);

    // Close a file given its file ID
    static void close_file(UVM_FILE file_id);

    // Get a pointer to the ofstream object given its file ID
    static std::ofstream& file(UVM_FILE file_id);

private:
    static int next_id;
    static std::unordered_map<int, std::ofstream> file_streams;
};

#define PSPRINTF uvm_print::psprintf
#define DISPLAY uvm_print::display

time_t time();

///////////////////////////////////////////////////////////////////////////////////////////
// C++ class mirror for process in sv
///////////////////////////////////////////////////////////////////////////////////////////
class process {
public:
    enum class State { FINISHED, RUNNING, WAITING, SUSPENDED, KILLED };

    // Constructor
    process();

    // Destructor
    ~process();

    // Static function to return a process instance (singleton pattern for example)
    static process* self();

    // Function to return the current status
    State status();

    // Task to kill the process
    void kill();

    // Task to await the process (for simplicity, just simulate a wait)
    void await();

    // Task to suspend the process
    void suspend();

    // Task to resume the process
    void resume();

protected:
    // The process loop (to be overridden by derived classes)
    virtual void run();

private:
    State currentState;
    std::atomic<bool> running;
    std::thread* processThread;
    std::mutex stateMutex;
};

///////////////////////////////////////////////////////////////////////////////////////////
// C++ class mirror for mailbox in sv
///////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class mailbox {
public:
    // Constructor for bounded/unbounded mailbox
    mailbox(int bound = 0) : bound(bound), running(true) {}

    // Destructor
    ~mailbox() {
        running = false;
    }

    // Method to put data into the mailbox
    void put(const T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [this]() { return queue.size() < bound || bound == 0; });
        queue.push(data);
        cond.notify_all();
    }

    // Method to try to put data into the mailbox
    bool try_put(const T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.size() < bound || bound == 0) {
            queue.push(data);
            cond.notify_all();
            return true;
        } else {
            return false;
        }
    }

    // Method to get data from the mailbox
    void get(T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [this]() { return !queue.empty() || !running; });
        if (!running && queue.empty()) return;
        data = queue.front();
        queue.pop();
        cond.notify_all();
    }

    // Method to try to get data from the mailbox
    bool try_get(T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!queue.empty()) {
            data = queue.front();
            queue.pop();
            cond.notify_all();
            return true;
        } else {
            return false;
        }
    }

    // Method to peek data from the mailbox without removing it
    void peek(T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [this]() { return !queue.empty() || !running; });
        if (!running && queue.empty()) return;
        data = queue.front();
    }

    // Method to try to peek data from the mailbox without removing it
    bool try_peek(T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!queue.empty()) {
            data = queue.front();
            return true;
        } else {
            return false;
        }
    }

    // Method to get the number of entries in the mailbox
    int num() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.size();
    }

private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cond;
    int bound;
    bool running;
};


// Helper function to escape regex special characters
inline std::string uvm_escape_regex(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size() * 2);
    for (char c : str) {
        if (std::strchr(".^$|()[]+*?\\{}", c)) {
            escaped += '\\';
        }
        escaped += c;
    }
    return escaped;
}

#endif // UVM_GLOBALS_H
