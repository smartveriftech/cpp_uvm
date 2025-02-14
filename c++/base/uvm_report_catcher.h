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

#ifndef UVM_REPORT_CATCHER_H
#define UVM_REPORT_CATCHER_H

#include <string>
#include "base/uvm_report_object.h"
#include "base/uvm_callback.h"
#include "base/uvm_report_server.h"

// Forward declaration
class uvm_report_catcher;

typedef uvm_callbacks<uvm_report_object, uvm_report_catcher> uvm_report_cb;
typedef uvm_callback_iter<uvm_report_object, uvm_report_catcher> uvm_report_cb_iter;

struct sev_id_struct {
  bool sev_specified;
  bool id_specified;
  uvm_severity sev;
  std::string id;
  bool is_on;
};

//------------------------------------------------------------------------------
//
// CLASS: uvm_report_catcher
//
// The uvm_report_catcher is used to catch messages issued by the uvm report
// server. Catchers are
// uvm_callbacks#(<uvm_report_object>,uvm_report_catcher) objects,
// so all factilities in the <uvm_callback> and <uvm_callbacks#(T,CB)>
// classes are available for registering catchers and controlling catcher
// state.
// The uvm_callbacks#(<uvm_report_object>,uvm_report_catcher) class is
// aliased to ~uvm_report_cb~ to make it easier to use.
// Multiple report catchers can be 
// registered with a report object. The catchers can be registered as default 
// catchers which catch all reports on all <uvm_report_object> reporters,
// or catchers can be attached to specific report objects (i.e. components). 
//
// User extensions of <uvm_report_catcher> must implement the <catch> method in 
// which the action to be taken on catching the report is specified. The catch 
// method can return ~CAUGHT~, in which case further processing of the report is 
// immediately stopped, or return ~THROW~ in which case the (possibly modified) report 
// is passed on to other registered catchers. The catchers are processed in the order 
// in which they are registered.
//
// On catching a report, the <catch> method can modify the severity, id, action,
// verbosity or the report string itself before the report is finally issued by
// the report server. The report can be immediately issued from within the catcher 
// class by calling the <issue> method.
//
// The catcher maintains a count of all reports with FATAL,ERROR or WARNING severity
// and a count of all reports with FATAL, ERROR or WARNING severity whose severity
// was lowered. These statistics are reported in the summary of the <uvm_report_server>.
//
// This example shows the basic concept of creating a report catching
// callback and attaching it to all messages that get emitted:
//
//| class my_error_demoter extends uvm_report_catcher;
//|   function new(string name="my_error_demoter");
//|     super.new(name);
//|   endfunction
//|   //This example demotes "MY_ID" errors to an info message
//|   function action_e catch();
//|     if(get_severity() == UVM_ERROR && get_id() == "MY_ID")
//|       set_severity(UVM_INFO);
//|     return THROW;
//|   endfunction
//| endclass
//|
//| my_error_demoter demoter = new;
//| initial begin
//|  // Catchers are callbacks on report objects (components are report 
//|  // objects, so catchers can be attached to components).
//|
//|  // To affect all reporters, use null for the object
//|  uvm_report_cb::add(null, demoter); 
//|
//|  // To affect some specific object use the specific reporter
//|  uvm_report_cb::add(mytest.myenv.myagent.mydriver, demoter);
//|
//|  // To affect some set of components using the component name
//|  uvm_report_cb::add_by_name("*.*driver", demoter);
//| end
//
//
//------------------------------------------------------------------------------

class uvm_report_catcher : public uvm_callback {
public:
    enum action_e { UNKNOWN_ACTION, THROW, CAUGHT };

    // Function: new
    //
    // Create a new report catcher. The name argument is optional, but
    // should generally be provided to aid in debugging.
    uvm_report_catcher(const std::string& name = "uvm_report_catcher");

    virtual action_e handle() = 0; // Renamed from catch to handle

    // Group: Debug
       
    // Function: get_report_catcher
    //
    // Returns the first report catcher that has ~name~. 
    static uvm_report_catcher* get_report_catcher(const std::string& name);

    // Function: print_catcher
    //
    // Prints information about all of the report catchers that are 
    // registered. For finer grained detail, the <uvm_callbacks #(T,CB)::display>
    // method can be used by calling uvm_report_cb::display(<uvm_report_object>).
    static void print_catcher(UVM_FILE file = 0);
    static void debug_report_catcher(int what = 0);

    // Function: issue
    // Immediately issues the message which is currently being processed. This
    // is useful if the message is being ~CAUGHT~ but should still be emitted.
    //
    // Issuing a message will update the report_server stats, possibly multiple 
    // times if the message is not ~CAUGHT~.
    void issue();

    // Function: summarize_report_catcher
    //
    // This function is called automatically by <uvm_report_server::summarize()>.
    // It prints the statistics for the active catchers.
    static void summarize_report_catcher(UVM_FILE file);

    //process_all_report_catchers
    //method called by report_server.report to process catchers
    //
    static int process_all_report_catchers(uvm_report_server* server, uvm_report_object* client, uvm_severity& severity, const std::string& name, std::string& id, std::string& message, int& verbosity_level, uvm_action& action, const std::string& filename, int line);

//protected:

    // Group: Change Message State

    // Function: set_severity
    //
    // Change the severity of the message to ~severity~. Any other
    // report catchers will see the modified value.
    void set_severity(uvm_severity severity);

    // Function: set_verbosity
    //
    // Change the verbosity of the message to ~verbosity~. Any other
    // report catchers will see the modified value.
    void set_verbosity(int verbosity);

    // Function: set_id
    //
    // Change the id of the message to ~id~. Any other
    // report catchers will see the modified value.
    void set_id(const std::string& id);

    // Function: set_message
    //
    // Change the text of the message to ~message~. Any other
    // report catchers will see the modified value.
    void set_message(const std::string& message);

    // Function: set_action
    //
    // Change the action of the message to ~action~. Any other
    // report catchers will see the modified value.
    void set_action(uvm_action action);

    // Group: Current Message State

    // Function: get_client
    //
    // Returns the <uvm_report_object> that has generated the message that
    // is currently being processes.
    uvm_report_object* get_client() const;

    // Function: get_severity
    //
    // Returns the <uvm_severity> of the message that is currently being
    // processed. If the severity was modified by a previously executed
    // catcher object (which re-threw the message), then the returned 
    // severity is the modified value.
    uvm_severity get_severity() const;

    // Function: get_context
    //
    // Returns the context (source) of the message that is currently being
    // processed. This is typically the full hierarchical name of the component
    // that issued the message. However, when the message comes via a report
    // handler that is not associated with a component, the context is
    // user-defined.
    std::string get_context() const;

    // Function: get_verbosity
    //
    // Returns the verbosity of the message that is currently being
    // processed. If the verbosity was modified by a previously executed
    // catcher (which re-threw the message), then the returned 
    // verbosity is the modified value.
    int get_verbosity() const;

    // Function: get_id
    //
    // Returns the string id of the message that is currently being
    // processed. If the id was modified by a previously executed
    // catcher (which re-threw the message), then the returned 
    // id is the modified value.
    std::string get_id() const;

    // Function: get_message
    //
    // Returns the string message of the message that is currently being
    // processed. If the message was modified by a previously executed
    // catcher (which re-threw the message), then the returned 
    // message is the modified value.
    std::string get_message() const;

    // Function: get_action
    //
    // Returns the <uvm_action> of the message that is currently being
    // processed. If the action was modified by a previously executed
    // catcher (which re-threw the message), then the returned 
    // action is the modified value.
    uvm_action get_action() const;

    // Function: get_fname
    //
    // Returns the file name of the message.
    std::string get_fname() const;

    // Function: get_line
    //
    // Returns the line number of the message.
    int get_line() const;

    // Group: Callback Interface
    
    // Function: catch
    //
    // This is the method that is called for each registered report catcher.
    // There are no arguments to this function. The <Current Message State>
    // interface methods can be used to access information about the 
    // current message being processed.

    virtual action_e catch_report() = 0;

    // Group: Reporting

    // Function: uvm_report_fatal
    //
    // Issues a fatal message using the current message's report object.
    // This message will bypass any message catching callbacks.
    void uvm_report_fatal(const std::string& id, const std::string& message, int verbosity, const std::string& fname, int line);

    // Function: uvm_report_error
    //
    // Issues a error message using the current message's report object.
    // This message will bypass any message catching callbacks.
    void uvm_report_error(const std::string& id, const std::string& message, int verbosity, const std::string& fname, int line);

    // Function: uvm_report_warning
    //
    // Issues a warning message using the current message's report object.
    // This message will bypass any message catching callbacks.
    void uvm_report_warning(const std::string& id, const std::string& message, int verbosity, const std::string& fname, int line);

    // Function: uvm_report_info
    //
    // Issues a info message using the current message's report object.
    // This message will bypass any message catching callbacks.
    void uvm_report_info(const std::string& id, const std::string& message, int verbosity, const std::string& fname, int line);

private:

    //process_all_report_catchers
    //method called by report_server.report to process catchers
    int process_report_catcher();

    //f_display
    //internal method to check if file is open
    //
    static void f_display(UVM_FILE file, const std::string& str);

    static uvm_severity m_modified_severity;
    static int m_modified_verbosity;
    static std::string m_modified_id;
    static std::string m_modified_message;
    static std::string m_file_name;
    static int m_line_number;
    static uvm_report_object* m_client;
    static uvm_action m_modified_action;
    static bool m_set_action_called;
    static uvm_report_server* m_server;
    static std::string m_name;

    static int m_demoted_fatal;
    static int m_demoted_error;
    static int m_demoted_warning;
    static int m_caught_fatal;
    static int m_caught_error;
    static int m_caught_warning;

    static const int DO_NOT_CATCH = 1;
    static const int DO_NOT_MODIFY = 2;
    static int m_debug_flags;

    static uvm_severity m_orig_severity;
    static uvm_action m_orig_action;
    static std::string m_orig_id;
    static int m_orig_verbosity;
    static std::string m_orig_message;

    static bool do_report;
};

#endif // UVM_REPORT_CATCHER_H