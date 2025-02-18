//
//------------------------------------------------------------------------------
// Copyright 2007-2010 Cadence Design Systems, Inc.
// Copyright 2007-2011 Mentor Graphics Corporation
// Copyright 2010-2013 Synopsys, Inc.
// Copyright 2013      Cisco Systems, Inc.
// Copyright 2013      NVIDIA Corporation
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

#ifndef UVM_PHASE_H
#define UVM_PHASE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include "base/uvm_object.h"
#include "base/uvm_report_object.h"
#include "base/uvm_object_globals.h"

class uvm_component;
class uvm_objection;
class uvm_domain;

//------------------------------------------------------------------------------
// Class: uvm_phase
//------------------------------------------------------------------------------
// This base class defines everything about a phase: behavior, state, and context.
//
// To define behavior, it is extended by UVM or the user to create singleton
// objects which capture the definition of what the phase does and how it does it.
// These are then cloned to produce multiple nodes which are hooked up in a graph
// structure to provide context: which phases follow which, and to hold the state
// of the phase throughout its lifetime.
// UVM provides default extensions of this class for the standard runtime phases.
// VIP Providers can likewise extend this class to define the phase functor for a
// particular component context as required.
//
// *Phase Definition*
//
// Singleton instances of those extensions are provided as package variables.
// These instances define the attributes of the phase (not what state it is in)
// They are then cloned into schedule nodes which point back to one of these
// implementations, and calls it's virtual task or function methods on each
// participating component.
// It is the base class for phase functors, for both predefined and
// user-defined phases. Per-component overrides can use a customized imp.
//
// To create custom phases, do not extend uvm_phase directly: see the
// three predefined extended classes below which encapsulate behavior for
// different phase types: task, bottom-up function and top-down function.
//
// Extend the appropriate one of these to create a uvm_YOURNAME_phase class
// (or YOURPREFIX_NAME_phase class) for each phase, containing the default
// implementation of the new phase, which must be a uvm_component-compatible
// delegate, and which may be a null implementation. Instantiate a singleton
// instance of that class for your code to use when a phase handle is required.
// If your custom phase depends on methods that are not in uvm_component, but
// are within an extended class, then extend the base YOURPREFIX_NAME_phase
// class with parameterized component class context as required, to create a
// specialized functor which calls your extended component class methods.
// This scheme ensures compile-safety for your extended component classes while
// providing homogeneous base types for APIs and underlying data structures.
//
// *Phase Context*
//
// A schedule is a coherent group of one or more phase/state nodes linked
// together by a graph structure, allowing arbitrary linear/parallel
// relationships to be specified, and executed by stepping through them in
// the graph order.
// Each schedule node points to a phase and holds the execution state of that
// phase, and has optional links to other nodes for synchronization.
//
// The main operations are: construct, add phases, and instantiate
// hierarchically within another schedule.
//
// Structure is a DAG (Directed Acyclic Graph). Each instance is a node
// connected to others to form the graph. Hierarchy is overlaid with m_parent.
// Each node in the graph has zero or more successors, and zero or more
// predecessors. No nodes are completely isolated from others. Exactly
// one node has zero predecessors. This is the root node. Also the graph
// is acyclic, meaning for all nodes in the graph, by following the forward
// arrows you will never end up back where you started but you will eventually
// reach a node that has no successors.
//
// *Phase State*
//
// A given phase may appear multiple times in the complete phase graph, due
// to the multiple independent domain feature, and the ability for different
// VIP to customize their own phase schedules perhaps reusing existing phases.
// Each node instance in the graph maintains its own state of execution.
//
// *Phase Handle*
//
// Handles of this type uvm_phase are used frequently in the API, both by
// the user, to access phasing-specific API, and also as a parameter to some
// APIs. In many cases, the singleton package-global phase handles can be
// used (eg. connect_ph, run_ph) in APIs. For those APIs that need to look
// up that phase in the graph, this is done automatically.

class uvm_phase : public uvm_object {
public:
  //`uvm_object_utils(uvm_phase)

  //--------------------
  // Group: Construction
  //--------------------
  
  // Function: new
  //
  // Create a new phase node, with a name and a note of its type
  //   name   - name of this phase
  //   type   - task, topdown func or bottomup func
  //
  uvm_phase(std::string name = "uvm_phase",
            uvm_phase_type phase_type = UVM_PHASE_SCHEDULE,
            uvm_phase* parent = nullptr);

  // Function: get_phase_type
  //
  // Returns the phase type as defined by <uvm_phase_type>
  //
  uvm_phase_type get_phase_type();


  //-------------
  // Group: State
  //-------------

  // Function: get_state
  //
  // Accessor to return current state of this phase
  //
  uvm_phase_state get_state();

  // Function: get_run_count
  //
  // Accessor to return the integer number of times this phase has executed
  //
  int get_run_count();

  // Function: find_by_name
  //
  // Locate a phase node with the specified ~name~ and return its handle.
  // With ~stay_in_scope~ set, searches only within this phase's schedule or
  // domain.
  //
  uvm_phase* find_by_name(const std::string& name, bool stay_in_scope = true);

  // Function: find
  //
  // Locate the phase node with the specified ~phase~ IMP and return its handle.
  // With ~stay_in_scope~ set, searches only within this phase's schedule or
  // domain.
  //
  uvm_phase* find(uvm_phase* phase, bool stay_in_scope = true);

  // Function: is
  //
  // returns 1 if the containing uvm_phase refers to the same phase
  // as the phase argument, 0 otherwise
  //
  bool is(const uvm_phase* phase);

  // Function: is_before
  //
  // Returns 1 if the containing uvm_phase refers to a phase that is earlier
  // than the phase argument, 0 otherwise
  //
  bool is_before(const uvm_phase* phase);

  // Function: is_after
  //
  // returns 1 if the containing uvm_phase refers to a phase that is later
  // than the phase argument, 0 otherwise
  //
  bool is_after(const uvm_phase* phase);

  //-----------------
  // Group: Callbacks
  //-----------------

  // Function: exec_func
  //
  // Implements the functor/delegate functionality for a function phase type
  //   comp  - the component to execute the functionality upon
  //   phase - the phase schedule that originated this phase call
  //
  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  // Function: exec_task
  //
  // Implements the functor/delegate functionality for a task phase type
  //   comp  - the component to execute the functionality upon
  //   phase - the phase schedule that originated this phase call
  //
  virtual void exec_task(uvm_component* comp, uvm_phase* phase);


  //----------------
  // Group: Schedule
  //----------------

  // Function: add
  //
  // Build up a schedule structure inserting phase by phase, specifying linkage
  //
  // Phases can be added anywhere, in series or parallel with existing nodes
  //
  //   phase        - handle of singleton derived imp containing actual functor.
  //                  by default the new phase is appended to the schedule
  //   with_phase   - specify to add the new phase in parallel with this one
  //   after_phase  - specify to add the new phase as successor to this one
  //   before_phase - specify to add the new phase as predecessor to this one
  //
  void add(uvm_phase* phase,
           uvm_phase* with_phase = nullptr,
           uvm_phase* after_phase = nullptr,
           uvm_phase* before_phase = nullptr);

  // Function: get_parent
  //
  // Returns the parent schedule node, if any, for hierarchical graph traversal
  //
  uvm_phase* get_parent();

  // Function: get_full_name
  //
  // Returns the full path from the enclosing domain down to this node.
  // The singleton IMP phases have no hierarchy.
  //
  virtual std::string get_full_name();

  // Function: get_schedule
  //
  // Returns the topmost parent schedule node, if any, for hierarchical graph traversal
  //
  uvm_phase* get_schedule(bool hier = false);

  // Function: get_schedule_name
  //
  // Returns the schedule name associated with this phase node
  //
  std::string get_schedule_name(bool hier = false);

  // Function: get_domain
  //
  // Returns the enclosing domain
  //
  uvm_domain* get_domain();

  // Function: get_imp
  //
   // Returns the phase implementation for this this node.
  // Returns null if this phase type is not a UVM_PHASE_LEAF_NODE. 
  uvm_phase* get_imp();

  // Function: get_domain_name
  //
  // Returns the domain name associated with this phase node
  //
  std::string get_domain_name();

  //-----------------------
  // Group: Synchronization
  //-----------------------

  // Function: get_objection
  //
  // Return the <uvm_objection> that gates the termination of the phase.
  //
  uvm_objection* get_objection();

  // Function: raise_objection
  //
  // Raise an objection to ending this phase
  // Provides components with greater control over the phase flow for
  // processes which are not implicit objectors to the phase.
  //
  virtual void raise_objection(uvm_object* obj, const std::string& description = "", int count = 1);

  // Function: drop_objection
  //
  // Drop an objection to ending this phase
  //
  // The drop is expected to be matched with an earlier raise.
  //
  virtual void drop_objection(uvm_object* obj, const std::string& description = "", int count = 1);

  // Functions: sync and unsync
  //
  // Add soft sync relationships between nodes
  //
  // Summary of usage:
  //| my_phase.sync(.target(domain)
  //|              [,.phase(phase)[,.with_phase(phase)]]);
  //| my_phase.unsync(.target(domain)
  //|                [,.phase(phase)[,.with_phase(phase)]]);
  //
  // Components in different schedule domains can be phased independently or in sync
  // with each other. An API is provided to specify synchronization rules between any
  // two domains. Synchronization can be done at any of three levels:
  //
  // - the domain's whole phase schedule can be synchronized
  // - a phase can be specified, to sync that phase with a matching counterpart
  // - or a more detailed arbitrary synchronization between any two phases
  //
  // Each kind of synchronization causes the same underlying data structures to
  // be managed. Like other APIs, we use the parameter dot-notation to set
  // optional parameters.
  //
  // When a domain is synced with another domain, all of the matching phases in
  // the two domains get a 'with' relationship between them. Likewise, if a domain
  // is unsynched, all of the matching phases that have a 'with' relationship have
  // the dependency removed. It is possible to sync two domains and then just
  // remove a single phase from the dependency relationship by unsyncing just
  // the one phase.

  // Function: sync
  //
  // Synchronize two domains, fully or partially
  //
  //   target       - handle of target domain to synchronize this one to
  //   phase        - optional single phase in this domain to synchronize, 
  //                  otherwise sync all
  //   with_phase   - optional different target-domain phase to synchronize with,
  //                  otherwise use ~phase~ in the target domain
  //
  void sync(uvm_domain* target,
            uvm_phase* phase = nullptr,
            uvm_phase* with_phase = nullptr);

  // Function: unsync
  //
  // Remove synchronization between two domains, fully or partially
  //
  //   target       - handle of target domain to remove synchronization from
  //   phase        - optional single phase in this domain to un-synchronize, 
  //                  otherwise unsync all
  //   with_phase   - optional different target-domain phase to un-synchronize with,
  //                  otherwise use ~phase~ in the target domain
  //
  void unsync(uvm_domain* target,
              uvm_phase* phase = nullptr,
              uvm_phase* with_phase = nullptr);

  // Function: wait_for_state
  //
  // Wait until this phase compares with the given ~state~ and ~op~ operand.
  // For <UVM_EQ> and <UVM_NE> operands, several <uvm_phase_states> can be
  // supplied by ORing their enum constants, in which case the caller will
  // wait until the phase state is any of (UVM_EQ) or none of (UVM_NE) the
  // provided states.
  //
  // To wait for the phase to be at the started state or after
  //
  //| wait_for_state(UVM_PHASE_STARTED, UVM_GTE);
  //
  // To wait for the phase to be either started or executing
  //
  //| wait_for_state(UVM_PHASE_STARTED | UVM_PHASE_EXECUTING, UVM_EQ);
  //
  void wait_for_state(uvm_phase_state state, uvm_wait_op op = UVM_EQ);

  //---------------
  // Group: Jumping
  //---------------

  // Force phases to jump forward or backward in a schedule
  //
  // A phasing domain can execute a jump from its current phase to any other.
  // A jump passes phasing control in the current domain from the current phase
  // to a target phase. There are two kinds of jump scope:
  //
  // - local jump to another phase within the current schedule, back- or forwards
  // - global jump of all domains together, either to a point in the master
  //   schedule outwith the current schedule, or by calling jump_all()
  //
  // A jump preserves the existing soft synchronization, so the domain that is
  // ahead of schedule relative to another synchronized domain, as a result of
  // a jump in either domain, will await the domain that is behind schedule.
  //
  // *Note*: A jump out of the local schedule causes other schedules that have
  // the jump node in their schedule to jump as well. In some cases, it is
  // desirable to jump to a local phase in the schedule but to have all
  // schedules that share that phase to jump as well. In that situation, the
  // jump_all static function should be used. This function causes all schedules
  // that share a phase to jump to that phase.

  // Function: jump
  //
  // Jump to a specified ~phase~. If the destination ~phase~ is within the current 
  // phase schedule, a simple local jump takes place. If the jump-to ~phase~ is
  // outside of the current schedule then the jump affects other schedules which
  // share the phase.
  //
  void jump(uvm_phase* phase);

  // Function: jump_all
  //
  // Make all schedules jump to a specified ~phase~, even if the jump target is local.
  // The jump happens to all phase schedules that contain the jump-to ~phase~,
  // i.e. a global jump. 
  //
  static void jump_all(uvm_phase* phase);

  // Function: get_jump_target
  //
  // Return handle to the target phase of the current jump, or null if no jump
  // is in progress. Valid for use during the phase_ended() callback
  //
  uvm_phase* get_jump_target();

  int max_ready_to_end_iter = 20;

  // Register current thread with this phase
  void register_thread();
  
  // Unregister current thread from this phase
  void unregister_thread();

//protected:
  // Implementation - Construction
  //------------------------------
  uvm_phase_type m_phase_type;
  uvm_phase* m_parent;     // our 'schedule' node [or points 'up' one level]
  uvm_phase* m_imp;        // phase imp to call when we execute this node

  // Implementation - State
  //-----------------------
  uvm_phase_state m_state;
  int m_run_count;         // num times this phase has executed
  std::unique_ptr<std::thread> m_phase_proc;  // Replace process* with std::thread
  int m_num_procs_not_yet_returned;
  uvm_phase* m_find_predecessor(uvm_phase* phase, bool stay_in_scope = true, uvm_phase* orig_phase = nullptr);
  uvm_phase* m_find_successor(uvm_phase* phase, bool stay_in_scope = true, uvm_phase* orig_phase = nullptr);
  uvm_phase* m_find_predecessor_by_name(const std::string& name, bool stay_in_scope = true, uvm_phase* orig_phase = nullptr);
  uvm_phase* m_find_successor_by_name(const std::string& name, bool stay_in_scope = true, uvm_phase* orig_phase = nullptr);
  void m_print_successors();

  // Implementation - Callbacks
  //---------------------------
  // Provide the required component traversal behavior. Called by execute()
  virtual void traverse(uvm_component* comp, uvm_phase* phase, uvm_phase_state state);
  // Provide the required per-component execution flow. Called by traverse()
  virtual void execute(uvm_component* comp, uvm_phase* phase);

  // Implementation - Schedule
  //--------------------------
  std::unordered_map<uvm_phase*, bool> m_predecessors;
  std::unordered_map<uvm_phase*, bool> m_successors;
  uvm_phase* m_end_node;
  uvm_phase* get_begin_node();
  uvm_phase* get_end_node();

  // Implementation - Synchronization
  //---------------------------------
  std::vector<uvm_phase*> m_sync;  // schedule instance to which we are synced
  uvm_objection* phase_done;        // phase done objection
  unsigned int m_ready_to_end_count;

  unsigned int get_ready_to_end_count();

  void m_wait_for_pred();

  // Implementation - Jumping
  //-------------------------
  bool m_jump_bkwd;
  bool m_jump_fwd;
  uvm_phase* m_jump_phase;
  void clear(uvm_phase_state state = UVM_PHASE_DORMANT);
  void clear_successors(uvm_phase_state state = UVM_PHASE_DORMANT, uvm_phase* end_state = nullptr);

  // Implementation - Overall Control
  //---------------------------------
  static mailbox<uvm_phase*> m_phase_hopper;

  static void m_run_phases();
  void execute_phase();
  void m_terminate_phase();
  void m_print_termination_state();
  void kill();
  void kill_successors();

  // TBD add more useful debug
  //---------------------------------
  static bool m_phase_trace;
  static bool m_use_ovm_run_semantic;

  std::string convert2string();

  std::string m_aa2string(const std::unordered_map<uvm_phase*, bool>& aa);

  bool is_domain();

  virtual void m_get_transitive_children(std::vector<uvm_phase*>& phases);

private:
  // Constructor helpers
  void initialize(std::string name, uvm_phase_type phase_type, uvm_phase* parent);

  // Thread management
  std::unordered_map<std::thread::id, std::thread*> m_threads;
  std::mutex m_thread_mutex;
  
  // Free all threads registered to this phase
  void free_threads();
};

#endif // UVM_PHASE_H