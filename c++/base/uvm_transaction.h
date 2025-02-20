//
//------------------------------------------------------------------------------
// Copyright 2007-2011 Cadence Design Systems, Inc. 
// Copyright 2007-2011 Mentor Graphics Corporation
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

#ifndef UVM_TRANSACTION_H
#define UVM_TRANSACTION_H

#include <set> // For storing unfreed transaction IDs
#include <string>
#include <atomic> // Added for std::atomic
#include "base/uvm_event.h"
#include "base/uvm_component.h"
#include "base/uvm_object.h"
#include "base/uvm_recorder.h"
#include "base/uvm_pool.h"

class uvm_event;

//------------------------------------------------------------------------------
//
// CLASS: uvm_transaction
//
// The uvm_transaction class is the root base class for UVM transactions.
// Inheriting all the methods of uvm_object, uvm_transaction adds a timing and
// recording interface.
//
// This class provides timestamp properties, notification events, and transaction
// recording support. 
//
// Use of this class as a base for user-defined transactions
// is deprecated. Its subtype, <uvm_sequence_item>, shall be used as the
// base class for all user-defined transaction types. 
// 
// The intended use of this API is via a <uvm_driver> to call <uvm_component::accept_tr>,
// <uvm_component::begin_tr>, and <uvm_component::end_tr> during the course of
// sequence item execution. These methods in the component base class will
// call into the corresponding methods in this class to set the corresponding
// timestamps (accept_time, begin_time, and end_tr), trigger the
// corresponding event (<begin_event> and <end_event>, and, if enabled,
// record the transaction contents to a vendor-specific transaction database.
//
// Note that start_item/finish_item (or `uvm_do* macro) executed from a
// <uvm_sequence #(REQ,RSP)> will automatically trigger
// the begin_event and end_events via calls to begin_tr and end_tr. While
// convenient, it is generally the responsibility of drivers to mark a
// transaction's progress during execution.  To allow the driver to control
// sequence item timestamps, events, and recording, you must add
// +define+UVM_DISABLE_AUTO_ITEM_RECORDING when compiling the UVM package. 
// Alternatively, users may use the transaction's event pool, <events>,
// to define custom events for the driver to trigger and the sequences to wait on. Any
// in-between events such as marking the begining of the address and data
// phases of transaction execution could be implemented via the
// <events> pool.
// 
// In pipelined protocols, the driver may release a sequence (return from
// finish_item() or it's `uvm_do macro) before the item has been completed.
// If the driver uses the begin_tr/end_tr API in uvm_component, the sequence can
// wait on the item's <end_event> to block until the item was fully executed,
// as in the following example.
//
//| task uvm_execute(item, ...);
//|     // can use the `uvm_do macros as well
//|     start_item(item);
//|     item.randomize();
//|     finish_item(item);
//|     item.end_event.wait_on();
//|     // get_response(rsp, item.get_transaction_id()); //if needed
//| endtask
//|
//| 
//| task uvm_tree_printer::emit() {
//|     std::string s = knobs->prefix;
//|     std::string space = "                                                                                                   ";
//|     std::string user_format;
//| 
//|     std::string linefeed = (newline == "" || newline == " ") ? newline : (newline + knobs->prefix);
//| 
//|     // Header
//|     if (knobs->header) {
//|         user_format = format_header();
//|         if (!user_format.empty()) {
//|             s += user_format + linefeed;
//|         }
//|     }
//| 
//|     for (auto row_it = m_rows.begin(); row_it != m_rows.end(); ++row_it) {
//|         uvm_printer_row_info& row = *row_it;
//|         user_format = format_row(row);
//|         if (user_format.empty()) {
//|             std::string indent_str = space.substr(0, row.level * knobs->indent);
//| 
//|             // Name (id)
//|             if (knobs->identifier) {
//|                 s += indent_str + row.name;
//|                 if (!row.name.empty() && row.name != "...") {
//|                     s += ": ";
//|                 }
//|             }
//| 
//|             // Type Name
//|             if (row.val[0] == '@') { // is an object w/ knobs->reference on
//|                 s += "(" + row.type_name + row.val + ") ";
//|             } else if (knobs->type_name && !row.type_name.empty() && row.type_name != "-" && row.type_name != "...") {
//|                 s += "(" + row.type_name + ") ";
//|             }
//| 
//|             // Size
//|             if (knobs->size && !row.size.empty() && row.size != "-") {
//|                 s += "(" + row.size + ") ";
//|             }
//| 
//|             auto next_row = std::next(row_it);
//|             if (next_row != m_rows.end() && next_row->level > row.level) {
//|                 s += "{" + linefeed;
//|                 continue;
//|             }
//| 
//|             // Value (unconditional)
//|             s += row.val + " " + linefeed;
//| 
//|             // Scope handling...
//|             if (next_row == m_rows.end() || next_row->level < row.level) {
//|                 for (int l = row.level - 1; next_row == m_rows.end() ? l >= 0 : l >= next_row->level; --l) {
//|                     std::string indent_str = space.substr(0, l * knobs->indent);
//|                     s += indent_str + "}" + linefeed;
//|                 }
//|             }
//|         } else {
//|             s += user_format + linefeed;
//|         }
//|     }
//| 
//|     // Footer
//|     if (knobs->footer) {
//|         user_format = format_footer();
//|         if (!user_format.empty()) {
//|             s += user_format + linefeed;
//|         }
//|     }
//| 
//|     if (newline == "" || newline == " ") {
//|         s += "\n";
//|     }
//| 
//|     m_rows.clear();
//|     return s;
//| }
//| 
//| // Implementation of uvm_line_printer methods
//| 
//| uvm_line_printer::uvm_line_printer() {
//|     knobs = new uvm_printer_knobs();
//|     newline = " ";
//|     knobs->indent = 0;
//| }
//| 
//------------------------------------------------------------------------------


class uvm_transaction : public uvm_object {
public:
    // Constructor
    // Function: new
    //
    // Creates a new transaction object. The name is the instance name of the
    // transaction. If not supplied, then the object is unnamed.
    uvm_transaction(const std::string& name = "", uvm_component* initiator = nullptr);

    // Function: accept_tr
    //
    // Calling ~accept_tr~ indicates that the transaction item has been received by
    // a consumer component. Typically a <uvm_driver> would call <uvm_component::accept_tr>,
    // which calls this method-- upon return from a get_next_item(), get(), or peek()
    // call on its sequencer port, <uvm_driver::seq_item_port>.
    //
    // With some
    // protocols, the received item may not be started immediately after it is
    // accepted. For example, a bus driver, having accepted a request transaction,
    // may still have to wait for a bus grant before begining to execute
    // the request.
    //
    // This function performs the following actions:
    //
    // - The transaction's internal accept time is set to the current simulation
    //   time, or to accept_time if provided and non-zero. The ~accept_time~ may be
    //   any time, past or future.
    //
    // - The transaction's internal accept event is triggered. Any processes
    //   waiting on the this event will resume in the next delta cycle. 
    //
    // - The <do_accept_tr> method is called to allow for any post-accept
    //   action in derived classes.
    void accept_tr(time_t accept_time = 0);

    // Function: begin_tr
    //
    // This function indicates that the transaction has been started and is not
    // the child of another transaction. Generally, a consumer component begins
    // execution of a transactions it receives. 
    //
    // Typically, a <uvm_driver> would call <uvm_component::begin_tr>, which
    // calls this method, before actual execution of a sequence item transaction.
    // Sequence items received by a driver are always a child of a parent sequence.
    // In this case, begin_tr obtains the parent handle and delegates to <begin_child_tr>.
    //
    // See <accept_tr> for more information on how the
    // begin-time might differ from when the transaction item was received.
    //
    // This function performs the following actions:
    //
    // - The transaction's internal start time is set to the current simulation
    //   time, or to begin_time if provided and non-zero. The begin_time may be
    //   any time, past or future, but should not be less than the accept time.
    //
    // - If recording is enabled, then a new database-transaction is started with
    //   the same begin time as above.
    //
    // - The <do_begin_tr> method is called to allow for any post-begin action in
    //   derived classes.
    //
    // - The transaction's internal begin event is triggered. Any processes
    //   waiting on this event will resume in the next delta cycle. 
    //
    // The return value is a transaction handle, which is valid (non-zero) only if
    // recording is enabled. The meaning of the handle is implementation specific.
    int begin_tr(time_t begin_time = 0);

    // Function: begin_child_tr
    //
    // This function indicates that the transaction has been started as a child of
    // a parent transaction given by ~parent_handle~. Generally, a consumer
    // component calls this method via <uvm_component::begin_child_tr> to indicate
    // the actual start of execution of this transaction.
    //
    // The parent handle is obtained by a previous call to begin_tr or
    // begin_child_tr. If the parent_handle is invalid (=0), then this function
    // behaves the same as <begin_tr>. 
    //
    // This function performs the following actions:
    //
    // - The transaction's internal start time is set to the current simulation
    //   time, or to begin_time if provided and non-zero. The begin_time may be
    //   any time, past or future, but should not be less than the accept time.
    //
    // - If recording is enabled, then a new database-transaction is started with
    //   the same begin time as above. The record method inherited from <uvm_object>
    //   is then called, which records the current property values to this new
    //   transaction. Finally, the newly started transaction is linked to the
    //   parent transaction given by parent_handle.
    //
    // - The <do_begin_tr> method is called to allow for any post-begin
    //   action in derived classes.
    //
    // - The transaction's internal begin event is triggered. Any processes
    //   waiting on this event will resume in the next delta cycle. 
    //
    // The return value is a transaction handle, which is valid (non-zero) only if
    // recording is enabled. The meaning of the handle is implementation specific.
    int begin_child_tr(time_t begin_time = 0, int parent_handle = 0);

    // Function: end_tr
    //
    // This function indicates that the transaction execution has ended.
    // Generally, a consumer component ends execution of the transactions it
    // receives. 
    //
    // You must have previously called <begin_tr> or <begin_child_tr> for this
    // call to be successful.
    //
    // Typically, a <uvm_driver> would call <uvm_component::end_tr>, which
    // calls this method, upon completion of a sequence item transaction.
    // Sequence items received by a driver are always a child of a parent sequence.
    // In this case, begin_tr obtain the parent handle and delegate to <begin_child_tr>.
    //
    // This function performs the following actions:
    //
    // - The transaction's internal end time is set to the current simulation
    //   time, or to ~end_time~ if provided and non-zero. The ~end_time~ may be any
    //   time, past or future, but should not be less than the begin time.
    //
    // - If recording is enabled and a database-transaction is currently active,
    //   then the record method inherited from uvm_object is called, which records
    //   the final property values. The transaction is then ended. If ~free_handle~
    //   is set, the transaction is released and can no longer be linked to (if
    //   supported by the implementation).
    //
    // - The <do_end_tr> method is called to allow for any post-end
    //   action in derived classes.
    //
    // - The transaction's internal end event is triggered. Any processes waiting
    //   on this event will resume in the next delta cycle. 
    void end_tr(time_t end_time = 0, bool free_handle = true);

    // Function: set_transaction_id
    //
    // Sets this transaction's numeric identifier to id. If not set via this
    // method, the transaction ID defaults to -1. 
    //
    // When using sequences to generate stimulus, the transaction ID is used along
    // with the sequence ID to route responses in sequencers and to correlate
    // responses to requests.
    void set_transaction_id(int id);

    // Function: get_transaction_id
    //
    // Returns this transaction's numeric identifier, which is -1 if not set
    // explicitly by ~set_transaction_id~.
    //
    // When using a <uvm_sequence #(REQ,RSP)> to generate stimulus, the transaction 
    // ID is used along
    // with the sequence ID to route responses in sequencers and to correlate
    // responses to requests.
    int get_transaction_id();

    // Function: set_initiator
    //
    // Sets initiator as the initiator of this transaction. 
    //
    // The initiator can be the component that produces the transaction. It can
    // also be the component that started the transaction. This or any other
    // usage is up to the transaction designer.
    void set_initiator(uvm_component* initiator);

    // Function: get_initiator
    //
    // Returns the component that produced or started the transaction, as set by
    // a previous call to set_initiator.
    uvm_component* get_initiator();

    // Function: get_event_pool
    //
    // Returns the event pool associated with this transaction. 
    //
    // By default, the event pool contains the events: begin, accept, and end.
    // Events can also be added by derivative objects. An event pool is a
    // specialization of an <uvm_pool #(T)>, e.g. a ~uvm_pool#(uvm_event)~.
    uvm_event_pool* get_event_pool();

    // Function: is_active
    //
    // Returns 1 if the transaction has been started but has not yet been ended.
    // Returns 0 if the transaction has not been started.
    bool is_active();

    // Function: get_begin_time
    time_t get_begin_time();

    // Function: get_end_time
    //
    // Returns the time at which this transaction was accepted, begun, or ended, 
    // as by a previous call to <accept_tr>, <begin_tr>, <begin_child_tr>, or <end_tr>.
    time_t get_end_time();

    // Function: get_accept_time
    time_t get_accept_time();

    // Function: disable_recording
    //
    // Turns off recording for the transaction stream. This method does not
    // effect a <uvm_component>'s recording streams.
    void disable_recording();

    // Function: enable_recording
    //
    // Turns on recording to the stream specified by stream, whose interpretation
    // is implementation specific. The optional ~recorder~ argument specifies
    //
    // If transaction recording is on, then a call to record is made when the
    // transaction is started and when it is ended.
    void enable_recording(const std::string& stream, uvm_recorder* recorder = nullptr);

    // Function: is_recording_enabled
    //
    // Returns 1 if recording is currently on, 0 otherwise.
    bool is_recording_enabled();

    // Function: get_tr_handle
    //
    // Returns the handle associated with the transaction, as set by a previous
    // call to <begin_child_tr> or <begin_tr> with transaction recording enabled.
    int get_tr_handle();

    // Function: lock
    //
    // Increments the reference count to indicate ownership.
    void lock();

    // Virtual destructor
    virtual ~uvm_transaction();

protected:
    // Protected member functions
    // Function: do_accept_tr
    //
    // This user-definable callback is called by <accept_tr> just before the accept
    // event is triggered. Implementations should call ~super.do_accept_tr~ to
    // ensure correct operation.
    virtual void do_accept_tr();

    // Function: do_begin_tr
    //
    // This user-definable callback is called by <begin_tr> and <begin_child_tr> just
    // before the begin event is triggered. Implementations should call
    // ~super.do_begin_tr~ to ensure correct operation.
    virtual void do_begin_tr();

    // Function: do_end_tr
    //
    // This user-definable callback is called by <end_tr> just before the end event
    // is triggered. Implementations should call ~super.do_end_tr~ to ensure correct
    // operation.
    virtual void do_end_tr();

    virtual void do_print(uvm_printer* printer);
    virtual void do_record(uvm_recorder* recorder);
    virtual void do_copy(uvm_object* rhs);

    //----------------------------------------------------------------------------
    //
    // Internal methods properties; do not use directly
    //
    //----------------------------------------------------------------------------
    //Override data control methods for internal properties
private:
    // Private member functions
    int m_begin_tr(time_t begin_time, int parent_handle, bool has_parent);

    // Member variables
    int m_transaction_id;
    time_t begin_time;
    time_t end_time;
    time_t accept_time;
    uvm_component* initiator;
    int stream_handle;
    int tr_handle;
    bool record_enable;
    uvm_recorder* m_recorder;

    // Variable: events
    //
    // The event pool instance for this transaction. This pool is used to track
    // various The <begin_event>
    const uvm_event_pool events;

    // Variable: begin_event
    //
    // A <uvm_event> that is triggered when this transaction's actual execution on the
    // bus begins, typically as a result of a driver calling <uvm_component::begin_tr>. 
    // Processes that wait on this event will block until the transaction has
    // begun. 
    //
    // For more information, see the general discussion for <uvm_transaction>.
    // See <uvm_event> for details on the event API.
    //| virtual task my_sequence::body();
    //|  ...
    //|  start_item(item);
    //|  item.randomize();     } `uvm_do(item)
    //|  finish_item(item);
    //|  // return from finish item does not always mean item is completed
    //|  item.end_event.wait_on();
    //|  ...
    uvm_event* begin_event;

    // Variable: end_event
    //
    // A <uvm_event> that is triggered when this transaction's actual execution on
    // the bus ends, typically as a result of a driver calling <uvm_component::end_tr>. 
    // Processes that wait on this event will block until the transaction has
    // ended. 
    //
    // For more information, see the general discussion for <uvm_transaction>.
    // See <uvm_event> for details on the event API.
    //
    //| virtual task my_sequence::body();
    //|  ...
    //|  start_item(item);
    //|  item.randomize();     } `uvm_do(item)
    //|  finish_item(item);
    //|  // return from finish item does not always mean item is completed
    //|  item.end_event.wait_on();
    //|  ...
    uvm_event* end_event;

    // Add the lock counter member variable
    int lock_count;

    // Declare uvm_transaction_manager as a friend class
    friend class uvm_transaction_manager;
};

// RAII Wrapper for uvm_transaction using std::shared_ptr with a custom deleter and debug messages
class uvm_transaction_manager {

public:
    // Constructor: Locks the transaction by creating a shared_ptr with a custom deleter
    uvm_transaction_manager(uvm_transaction* txn_ptr, const std::string& parent_name, const char* file = __FILE__, int line = __LINE__);

    // Destructor: Automatically handled by std::shared_ptr's deleter

    // Delete copy constructor and copy assignment operator
    uvm_transaction_manager(const uvm_transaction_manager&) = delete;
    uvm_transaction_manager& operator=(const uvm_transaction_manager&) = delete;

    // Allow move constructor and move assignment operator
    uvm_transaction_manager(uvm_transaction_manager&&) noexcept = default;
    uvm_transaction_manager& operator=(uvm_transaction_manager&&) noexcept = default;

    // Accessors
    std::shared_ptr<uvm_transaction> get_shared_ptr() const { return transaction_ptr_; }
    uvm_transaction* get() const { return transaction_ptr_.get(); }

    // Dereference operators for convenience
    uvm_transaction& operator*() const { return *transaction_ptr_; }
    uvm_transaction* operator->() const { return transaction_ptr_.get(); }

    static void report();

    // Function to add an ID to the set of unfreed transactions
    static void add_trans_id(int id);

    // Function to remove an ID from the set of unfreed transactions
    static void remove_trans_id(int id);

    static uvm_verbosity verb;

private:
    // Static set to keep track of unfreed transaction IDs
    static std::set<int> unfreed_transactions;

    std::shared_ptr<uvm_transaction> transaction_ptr_;
    std::string file_;
    int line_;
    std::string parent_name_;

};

typedef std::unique_ptr<uvm_transaction_manager> uvm_trans_man;

#endif // UVM_TRANSACTION_H
