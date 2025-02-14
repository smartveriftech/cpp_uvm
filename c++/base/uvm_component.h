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

#ifndef UVM_COMPONENT_H
#define UVM_COMPONENT_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>

#include "base/uvm_factory.h"
#include "base/uvm_report_object.h"
#include "base/uvm_pool.h"

class uvm_domain;
class uvm_objection;
class uvm_transaction;
class uvm_recorder;
class uvm_phase;
class uvm_process;
class uvm_process_thread;
class uvm_clock;
class uvm_time;

extern bool uvm_comp_check_phase;

enum uvmm_component_process_state {
    IDLE,                       // either no processes have been added or all processes have finished
    PROCESS_ADDED_NOT_STARTED,  // one or more processes added but not yet started
    ALL_PROCESSES_STARTED       // all processes have started
};

//------------------------------------------------------------------------------
//
// CLASS: uvm_component
//
// The uvm_component class is the root base class for UVM components. In
// addition to the features inherited from <uvm_object> and <uvm_report_object>,
// uvm_component provides the following interfaces:
//
// Hierarchy - provides methods for searching and traversing the component
//     hierarchy.
//
// Phasing - defines a phased test flow that all components follow, with a
//     group of standard phase methods and an API for custom phases and
//     multiple independent phasing domains to mirror DUT behavior e.g. power
//
// Configuration - provides methods for configuring component topology and other
//     parameters ahead of and during component construction.
//
// Reporting - provides a convenience interface to the <uvm_report_handler>. All
//     messages, warnings, and errors are processed through this interface.
//
// Transaction recording - provides methods for recording the transactions
//     produced or consumed by the component to a transaction database (vendor
//     specific). 
//
// Factory - provides a convenience interface to the <uvm_factory>. The factory
//     is used to create new components and other objects based on type-wide and
//     instance-specific configuration.
//
// The uvm_component is automatically seeded during construction using UVM
// seeding, if enabled. All other objects must be manually reseeded, if
// appropriate. See <uvm_object::reseed> for more information.
//
//------------------------------------------------------------------------------
class uvm_component : public uvm_report_object {
public:

    // Function: new
    //
    // Creates a new component with the given leaf instance ~name~ and handle to
    // to its ~parent~.  If the component is a top-level component (i.e. it is
    // created in a static module or interface), ~parent~ should be null.
    //
    // The component will be inserted as a child of the ~parent~ object, if any.
    // If ~parent~ already has a child by the given ~name~, an error is produced.
    //
    // If ~parent~ is null, then the component will become a child of the
    // implicit top-level component, ~uvm_top~.
    //
    // All classes derived from uvm_component must call super.new(name,parent).
    uvm_component(const std::string& name, uvm_component* parent = nullptr);
    virtual ~uvm_component();

    //----------------------------------------------------------------------------
    // Group: Hierarchy Interface
    //----------------------------------------------------------------------------
    //
    // These methods provide user access to information about the component
    // hierarchy, i.e., topology.
    // 
    //----------------------------------------------------------------------------

    // Function: get_parent
    //
    // Returns a handle to this component's parent, or null if it has no parent.
    virtual uvm_component* get_parent();

    // Function: get_full_name
    //
    // Returns the full hierarchical name of this object. The default
    // implementation concatenates the hierarchical name of the parent, if any,
    // with the leaf name of this object, as given by <uvm_object::get_name>. 
    std::string get_full_name() override;

    // Function: get_children
    //
    // This function populates the end of the ~children~ array with the 
    // list of this component's children. 
    //
    //|   uvm_component array[$];
    //|   my_comp.get_children(array);
    //|   foreach(array[i]) 
    //|     do_something(array[i]);
    void get_children(std::vector<uvm_component*>& children);

    uvm_component* get_child(const std::string& name);
    int get_next_child(std::string& name);

    // Function: get_first_child
    //
    // These methods are used to iterate through this component's children, if
    // any. For example, given a component with an object handle, ~comp~, the
    // following code calls <uvm_object::print> for each child:
    //
    //|    string name;
    //|    uvm_component child;
    //|    if (comp.get_first_child(name))
    //|      do begin
    //|        child = comp.get_child(name);
    //|        child.print();
    //|      end while (comp.get_next_child(name));
    int get_first_child(std::string& name);

    // Function: get_num_children
    //
    // Returns the number of this component's children. 
    int get_num_children();

    // Function: has_child
    //
    // Returns 1 if this component has a child with the given ~name~, 0 otherwise.
    bool has_child(const std::string& name);

    // Function - set_name
    //
    // Renames this component to ~name~ and recalculates all descendants'
    // full names. This is an internal function for now.
    virtual void set_name(const std::string& name) override;

    virtual std::string get_type_name() override;

    // Function: lookup
    //
    // Looks for a component with the given hierarchical ~name~ relative to this
    // component. If the given ~name~ is preceded with a '.' (dot), then the search
    // begins relative to the top level (absolute lookup). The handle of the
    // matching component is returned, else null. The name must not contain
    // wildcards.
    uvm_component* lookup(const std::string& name);

    // Function: get_depth
    //
    // Returns the component's depth from the root level. uvm_top has a
    // depth of 0. The test and any other top level components have a depth
    // of 1, and so on.
    unsigned int get_depth();

    //----------------------------------------------------------------------------
    // Group: Phasing Interface
    //----------------------------------------------------------------------------
    //
    // These methods implement an interface which allows all components to step
    // through a standard schedule of phases, or a customized schedule, and
    // also an API to allow independent phase domains which can jump like state
    // machines to reflect behavior e.g. power domains on the DUT in different
    // portions of the testbench. The phase tasks and functions are the phase
    // name with the _phase suffix. For example, the build phase function is
    // <build_phase>.
    //
    // All processes associated with a task-based phase are killed when the phase
    // ends. See <uvm_phase::execute> for more details.
    //----------------------------------------------------------------------------

    // Function: build_phase
    //
    // The <uvm_build_phase> phase implementation method.
    //
    // Any override should call super.build_phase(phase) to execute the automatic
    // configuration of fields registed in the component by calling 
    // <apply_config_settings>.
    // To turn off automatic configuration for a component, 
    // do not call super.build_phase(phase).
    //
    // This method should never be called directly. 
    virtual void build_phase(uvm_phase* phase);

    // For backward compatibility the base build_phase method calls build.
    virtual void build();

    // Function: connect_phase
    //
    // The <uvm_connect_phase> phase implementation method.
    //
    // This method should never be called directly. 
    virtual void connect_phase(uvm_phase* phase);
 
    // For backward compatibility the base build_phase method calls build.
    virtual void connect();

    // Function: end_of_elaboration_phase
    //
    // The <uvm_end_of_elaboration_phase> phase implementation method.
    //
    // This method should never be called directly.
    virtual void end_of_elaboration_phase(uvm_phase* phase);

    // For backward compatibility the base build_phase method calls build.
    virtual void end_of_elaboration();

    // Function: start_of_simulation_phase
    //
    // The <uvm_start_of_simulation_phase> phase implementation method.
    //
    // This method should never be called directly.
    virtual void start_of_simulation_phase(uvm_phase* phase);

    // For backward compatibility the base build_phase method calls build.
    virtual void start_of_simulation();

    // Task: run_phase
    //
    // The <uvm_run_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // Thn the phase will automatically
    // ends once all objections are dropped using ~phase.drop_objection()~.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // The run_phase task should never be called directly.
    virtual void run_phase(uvm_phase* phase);

    // For backward compatibility the base build_phase method calls build.
    virtual void run();

    // Task: pre_reset_phase
    //
    // The <uvm_pre_reset_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void pre_reset_phase(uvm_phase* phase);

    // Task: reset_phase
    //
    // The <uvm_reset_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void reset_phase(uvm_phase* phase);

    // Task: post_reset_phase
    //
    // The <uvm_post_reset_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void post_reset_phase(uvm_phase* phase);

    // Task: pre_configure_phase
    //
    // The <uvm_pre_configure_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void pre_configure_phase(uvm_phase* phase);

    // Task: configure_phase
    //
    // The <uvm_configure_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void configure_phase(uvm_phase* phase);

    // Task: post_configure_phase
    //
    // The <uvm_post_configure_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void post_configure_phase(uvm_phase* phase);

    // Task: pre_main_phase
    //
    // The <uvm_pre_main_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void pre_main_phase(uvm_phase* phase);

    // Task: main_phase
    //
    // The <uvm_main_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void main_phase(uvm_phase* phase);

    // Task: post_main_phase
    //
    // The <uvm_post_main_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void post_main_phase(uvm_phase* phase);

    // Task: pre_shutdown_phase
    //
    // The <uvm_pre_shutdown_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void pre_shutdown_phase(uvm_phase* phase);

    // Task: post_shutdown_phase
    //
    // The <uvm_post_shutdown_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void shutdown_phase(uvm_phase* phase);

    // Task: post_shutdown_phase
    //
    // The <uvm_post_shutdown_phase> phase implementation method.
    //
    // This task returning or not does not indicate the end
    // or persistence of this phase.
    // It is necessary to raise an objection
    // using ~phase.raise_objection()~ to cause the phase to persist.
    // Once all components have dropped their respective objection
    // using ~phase.drop_objection()~, or if no components raises an
    // objection, the phase is ended.
    // 
    // Any processes forked by this task continue to run
    // after the task returns,
    // but they will be killed once the phase ends.
    //
    // This method should not be called directly.
    virtual void post_shutdown_phase(uvm_phase* phase);

    // Function: extract_phase
    //
    // The <uvm_extract_phase> phase implementation method.
    //
    // This method should never be called directly.
    virtual void extract_phase(uvm_phase* phase);

    // For backward compatibility the base extract_phase method calls extract.
    virtual void extract();

    // Function: check_phase
    //
    // The <uvm_check_phase> phase implementation method.
    //
    // This method should never be called directly.
    virtual void check_phase(uvm_phase* phase);

    // For backward compatibility the base check_phase method calls check.
    virtual void check();

    // Function: report_phase
    //
    // The <uvm_report_phase> phase implementation method.
    //
    // This method should never be called directly.
    virtual void report_phase(uvm_phase* phase);

    // For backward compatibility the base report_phase method calls report.
    virtual void report();

    // Function: final_phase
    //
    // The <uvm_final_phase> phase implementation method.
    //
    // This method should never be called directly.
    virtual void final_phase(uvm_phase* phase);

    // Function: phase_started
    //
    // Invoked at the start of each phase. The ~phase~ argument specifies
    // the phase being started. Any threads spawned in this callback are
    // not affected when the phase ends.
    virtual void phase_started(uvm_phase* phase);

    // Function: phase_ready_to_end
    //
    // Invoked when all objections to ending the given ~phase~ have been
    // dropped, thus indicating that ~phase~ is ready to end. All this
    // component's processes forked for the given phase will be killed
    // upon return from this method. Components needing to consume delta
    // cycles or advance time to perform a clean exit from the phase
    // may raise the phase's objection. 
    //
    // |phase.raise_objection(this,"Reason");
    //
    // This effectively resets the
    // wait-for-all-objections-dropped loop for ~phase~. It is the
    // responsibility of this component to drop the objection once
    // it is ready for this phase to end (and processes killed).
    virtual void phase_ready_to_end(uvm_phase* phase);

    // Function: phase_ended
    //
    // Invoked at the end of each phase. The ~phase~ argument specifies
    // the phase that is ending.  Any threads spawned in this callback are
    // not affected when the phase ends.
    virtual void phase_ended(uvm_phase* phase);

    //--------------------------------------------------------------------
    // phase / schedule / domain API
    //--------------------------------------------------------------------
    
    // Function: set_domain
    //
    // Apply a phase domain to this component and, if ~hier~ is set, 
    // recursively to all its children. 
    //
    // Calls the virtual <define_domain> method, which derived components can
    // override to augment or replace the domain definition of ita base class.
    void set_domain(uvm_domain* domain, bool hier = true);

    // Function: get_domain
    //
    // Return handle to the phase domain set on this component
    uvm_domain* get_domain() { return nullptr; };

    // Function: define_domain
    //
    // Builds custom phase schedules into the provided ~domain~ handle.
    //
    // This method is called by <set_domain>, which integrators use to specify
    // this component belongs in a domain apart from the default 'uvm' domain.
    //
    // Custom component base classes requiring a custom phasing schedule can
    // augment or replace the domain definition they inherit by overriding
    // <defined_domain>. To augment, overrides would call super.define_domain().
    // To replace, overrides would not call super.define_domain().
    // 
    // The default implementation adds a copy of the ~uvm~ phasing schedule to
    // the given ~domain~, if one doesn't already exist, and only if the domain
    // is currently empty.
    //
    // Calling <set_domain>
    // with the default ~uvm~ domain (see <uvm_domain::get_uvm_domain>) on
    // a component with no ~define_domain~ override effectively reverts the
    // that component to using the default ~uvm~ domain. This may be useful
    // if a branch of the testbench hierarchy defines a custom domain, but
    // some child sub-branch should remain in the default ~uvm~ domain,
    // call <set_domain> with a new domain instance handle with ~hier~ set.
    // Then, in the sub-branch, call <set_domain> with the default ~uvm~ domain handle,
    // obtained via <uvm_domain::get_uvm_domain()>.
    //
    // Alternatively, the integrator may define the graph in a new domain externally,
    // then call <set_domain> to apply it to a component.
    protected:
    virtual void define_domain(uvm_domain* domain);

    public:
    // Function: set_phase_imp
    //
    // Override the default implementation for a phase on this component (tree) with a
    // custom one, which must be created as a singleton object extending the default
    // one and implementing required behavior in exec and traverse methods
    //
    // The ~hier~ specifies whether to apply the custom functor to the whole tree or
    // just this component.
    void set_phase_imp(uvm_phase* phase, uvm_phase* imp, bool hier = true);

    // Task: suspend
    //
    // Suspend this component.
    //
    // This method must be implemented by the user to suspend the
    // component according to the protocol and functionality it implements.
    // A suspended component can be subsequently resumed using <resume()>. 
    virtual void suspend();

    // Task: resume
    //
    // Resume this component.
    //
    // This method must be implemented by the user to resume a component
    // that was previously suspended using <suspend()>.
    // Some component may start in the suspended state and
    // may need to be explicitly resumed.
    virtual void resume();

    // Function: resolve_bindings
    //
    // Processes all port, export, and imp connections. Checks whether each port's
    // min and max connection requirements are met.
    //
    // It is called just before the end_of_elaboration phase.
    //
    // Users should not call directly.
    virtual void resolve_bindings (){};

    virtual void do_resolve_bindings ();

    //----------------------------------------------------------------------------
    // Group: Factory Interface
    //----------------------------------------------------------------------------
    //
    // The factory interface provides convenient access to a portion of UVM's
    // <uvm_factory> interface. For creating new objects and components, the
    // preferred method of accessing the factory is via the object or component
    // wrapper (see <uvm_component_registry #(T,Tname)> and
    // <uvm_object_registry #(T,Tname)>). The wrapper also provides functions
    // for setting type and instance overrides.
    //
    //----------------------------------------------------------------------------

    // Function: create_component
    //
    // A convenience function for <uvm_factory::create_component_by_name>,
    // this method calls upon the factory to create a new child component
    // whose type corresponds to the preregistered type name, ~requested_type_name~,
    // and instance name, ~name~. This method is equivalent to:
    //
    //|  factory.create_component_by_name(requested_type_name,
    //|                                   get_full_name(), name, this);
    //
    // If the factory determines that a type or instance override exists, the type
    // of the component created may be different than the requested type. See
    // <set_type_override> and <set_inst_override>. See also <uvm_factory> for
    // details on factory operation.
    uvm_component* create_component(const std::string& requested_type_name, const std::string& name);

    static void set_type_override_by_type(uvm_object_wrapper* original_type, uvm_object_wrapper* override_type, bool replace = true);
    static void set_type_override(const std::string& original_type_name, const std::string& override_type_name, bool replace = true);
    static void set_inst_override_by_type(const std::string& relative_inst_path, uvm_object_wrapper* original_type, uvm_object_wrapper* override_type);
    static void set_inst_override(const std::string& relative_inst_path, const std::string& original_type_name, const std::string& override_type_name);
    uvm_object* create_object(const std::string& requested_type_name, const std::string& name = "");
    void print_override_info(const std::string& requested_type_name, const std::string& name = "");

    // Reporting Interface
    void set_report_id_verbosity_hier(const std::string& id, int verbosity);
    void set_report_severity_id_verbosity_hier(uvm_severity severity, const std::string& id, int verbosity);
    void set_report_severity_action_hier(uvm_severity severity, uvm_action action);
    void set_report_id_action_hier(const std::string& id, uvm_action action);
    void set_report_severity_id_action_hier(uvm_severity severity, const std::string& id, uvm_action action);
    void set_report_default_file_hier(UVM_FILE file);
    void set_report_severity_file_hier(uvm_severity severity, UVM_FILE file);
    void set_report_id_file_hier(const std::string& id, UVM_FILE file);
    void set_report_severity_id_file_hier(uvm_severity severity, const std::string& id, UVM_FILE file);
    void set_report_verbosity_level_hier(int verbosity);

    //----------------------------------------------------------------------------
    // Group: Configuration Interface
    //----------------------------------------------------------------------------
    //
    // Components can be designed to be user-configurable in terms of its
    // topology (the type and number of children it has), mode of operation, and
    // run-time parameters (knobs). The configuration interface accommodates
    // this common need, allowing component composition and state to be modified
    // without having to derive new classes or new class hierarchies for
    // every configuration scenario. 
    //
    //----------------------------------------------------------------------------
    // Used for caching config settings
    static bool m_config_set;

    std::string massage_scope(const std::string& scope);

    void set_config_int(const std::string& inst_name, const std::string& field_name, uvm_bitstream_t value);
    void set_config_string(const std::string& inst_name, const std::string& field_name, const std::string& value);

    // Function: set_config_object
    //
    // Calling set_config_* causes configuration settings to be created and
    // placed in a table internal to this component. There are similar global
    // methods that store settings in a global table. Each setting stores the
    // supplied ~inst_name~, ~field_name~, and ~value~ for later use by descendent
    // components during their construction. (The global table applies to
    // all components and takes precedence over the component tables.)
    //
    // When a descendant component calls a get_config_* method, the ~inst_name~
    // and ~field_name~ provided in the get call are matched against all the
    // configuration settings stored in the global table and then in each
    // component in the parent hierarchy, top-down. Upon the first match, the
    // value stored in the configuration setting is returned. Thus, precedence is
    // global, following by the top-level component, and so on down to the
    // descendent component's parent.
    //
    // These methods work in conjunction with the get_config_* methods to
    // provide a configuration setting mechanism for integral, string, and
    // uvm_object-based types. Settings of other types, such as virtual interfaces
    // and arrays, can be indirectly supported by defining a class that contains
    // them.
    //
    // Both ~inst_name~ and ~field_name~ may contain wildcards.
    //
    // - For set_config_int, ~value~ is an integral value that can be anything
    //   from 1 bit to 4096 bits.
    //
    // - For set_config_string, ~value~ is a string.
    //
    // - For set_config_object, ~value~ must be an <uvm_object>-based object or
    //   null.  Its clone argument specifies whether the object should be cloned.
    //   If set, the object is cloned both going into the table (during the set)
    //   and coming out of the table (during the get), so that multiple components
    //   matched to the same setting (by way of wildcards) do not end up sharing
    //   the same object.
    //
    //
    // See <get_config_int>, <get_config_string>, and <get_config_object> for
    // information on getting the configurations set by these methods.

    void set_config_object(const std::string& inst_name, const std::string& field_name, uvm_object* value, bool clone = true);
    bool get_config_int(const std::string& field_name, u_int32_t& value);
    bool get_config_string(const std::string& field_name, std::string& value);
    bool get_config_object(const std::string& field_name, uvm_object*& value, bool clone = true);
    void check_config_usage(bool recurse = true);
    void apply_config_settings(bool verbose = false);
    void print_config_settings(const std::string& field = "", uvm_component* comp = nullptr, bool recurse = false);
    void print_config(bool recurse = false, bool audit = false);
    void print_config_with_audit(bool recurse = false);

    //----------------------------------------------------------------------------
    // Group: Objection Interface
    //----------------------------------------------------------------------------
    //
    // These methods provide object level hooks into the <uvm_objection> 
    // mechanism.
    // 
    //----------------------------------------------------------------------------

    // Function: raised
    //
    // The ~raised~ callback is called when this or a descendant of this component
    // instance raises the specfied ~objection~. The ~source_obj~ is the object
    // that originally raised the objection. 
    // The ~description~ is optionally provided by the ~source_obj~ to give a
    // reason for raising the objection. The ~count~ indicates the number of
    // objections raised by the ~source_obj~.
    virtual void raised(uvm_objection* objection, uvm_object* source_obj, const std::string& description, int count){};

    // Function: dropped
    //
    // The ~dropped~ callback is called when this or a descendant of this component
    // instance drops the specfied ~objection~. The ~source_obj~ is the object
    // that originally dropped the objection. 
    // The ~description~ is optionally provided by the ~source_obj~ to give a
    // reason for dropping the objection. The ~count~ indicates the number of
    // objections dropped by the the ~source_obj~.
    virtual void dropped(uvm_objection* objection, uvm_object* source_obj, const std::string& description, int count){};

    // Task: all_dropped
    //
    // The ~all_droppped~ callback is called when all objections have been 
    // dropped by this component and all its descendants.  The ~source_obj~ is the
    // object that dropped the last objection.
    // The ~description~ is optionally provided by the ~source_obj~ to give a
    // reason for raising the objection. The ~count~ indicates the number of
    // objections dropped by the the ~source_obj~.
    virtual void all_dropped(uvm_objection* objection, uvm_object* source_obj, const std::string& description, int count){};

    // Function: pre_abort
    //
    // This callback is executed when the message system is executing a
    // <UVM_EXIT> action. The exit action causes an immediate termination of
    // the simulation, but the pre_abort callback hook gives components an 
    // opportunity to provide additional information to the user before
    // the termination happens. For example, a test may want to executed
    // the report function of a particular component even when an error
    // condition has happened to force a premature termination you would
    // write a function like:
    //
    //| function void mycomponent::pre_abort();
    //|   report();
    //| endfunction
    //
    // The pre_abort() callback hooks are called in a bottom-up fashion.
    virtual void pre_abort();

    //----------------------------------------------------------------------------
    // Group: Recording Interface
    //----------------------------------------------------------------------------
    // These methods comprise the component-based transaction recording
    // interface. The methods can be used to record the transactions that
    // this component "sees", i.e. produces or consumes.
    //
    // The API and implementation are subject to change once a vendor-independent
    // use-model is determined.
    //----------------------------------------------------------------------------

    // Function: accept_tr
    //
    // This function marks the acceptance of a transaction, ~tr~, by this
    // component. Specifically, it performs the following actions:
    //
    // - Calls the ~tr~'s <uvm_transaction::accept_tr> method, passing to it the
    //   ~accept_time~ argument.
    //
    // - Calls this component's <do_accept_tr> method to allow for any post-begin
    //   action in derived classes.
    //
    // - Triggers the component's internal accept_tr event. Any processes waiting
    //   on this event will resume in the next delta cycle. 
    void accept_tr(uvm_transaction* tr, time_t accept_time = 0);
    int begin_tr(uvm_transaction* tr, const std::string& stream_name = "main", const std::string& label = "", const std::string& desc = "", time_t begin_time = 0, int parent_handle = 0);
    int begin_child_tr(uvm_transaction* tr, int parent_handle = 0, const std::string& stream_name = "main", const std::string& label = "", const std::string& desc = "", time_t begin_time = 0);
    void end_tr(uvm_transaction* tr, time_t end_time = 0, bool free_handle = true);
    int record_error_tr(const std::string& stream_name = "main", uvm_object* info = nullptr, const std::string& label = "error_tr", const std::string& desc = "", time_t error_time = 0, bool keep_active = false);
    int record_event_tr(const std::string& stream_name = "main", uvm_object* info = nullptr, const std::string& label = "event_tr", const std::string& desc = "", time_t event_time = 0, bool keep_active = false);

//protected:
    int m_begin_tr(uvm_transaction* tr, int parent_handle, bool has_parent, const std::string& stream_name, const std::string& label, const std::string& desc, time_t begin_time);

    // Function: do_accept_tr
    //
    // The <accept_tr> method calls this function to accommodate any user-defined
    // post-accept action. Implementations should call super.do_accept_tr to
    // ensure correct operation.
    virtual void do_accept_tr(uvm_transaction* tr) {}
    virtual void do_begin_tr(uvm_transaction* tr, const std::string& stream_name, int tr_handle) {};
    virtual void do_end_tr(uvm_transaction* tr, int tr_handle) {};


//////////////////////////////////////////////////////////////////////////////////////////////////////
    void m_extract_name(const std::string& name, std::string& leaf, std::string& remainder);

    bool open_file();
    int create_stream(const std::string& name, const std::string& type, const std::string& scope);
    void set_attribute(int txh, const std::string& name, const std::string& value);
    void set_attribute(int txh, const std::string& name, const bool& value, uvm_radix_enum radix, int numbits);
    int check_handle_kind(const std::string& htype, int handle);
    //int begin_tr_internal(uvm_transaction* tr, int parent_handle = 0, bool has_parent = false, const std::string& stream_name = "main", const std::string& label = "", const std::string& desc = "", time_t begin_time = 0);

    bool print_enabled = true;

    int recording_depth;
    std::ofstream file;
    std::string filename = "tr_db.log";
    int tr_handle = 0;
    uvm_radix_enum default_radix = UVM_HEX;
    bool physical = true;
    bool abstract = true;
    bool identifier = true;
    uvm_recursion_policy_enum policy = UVM_DEFAULT_POLICY;

    std::unordered_map<std::string, int> m_stream_handle;
    std::unordered_map<uvm_transaction*, int> m_tr_h;
    std::string m_name;

    static const std::string type_name;

    uvm_recorder* recorder = nullptr;

    std::vector<std::string> m_config_settings;

    //----------------------------------------------------------------------------
    //                     PRIVATE or PSUEDO-PRIVATE members
    //                      *** Do not call directly ***
    //         Implementation and even existence are subject to change. 
    //----------------------------------------------------------------------------
    // Most local methods are prefixed with m_, indicating they are not
    // user-level methods. SystemVerilog does not support friend classes,
    // which forces some otherwise internal methods to be exposed (i.e. not
    // be protected via 'local' keyword). These methods are also prefixed
    // with m_ to indicate they are not intended for public use.
    //
    // Internal methods will not be documented, although their implementa-
    // tions are freely available via the open-source license.
    //----------------------------------------------------------------------------
//protected:
    uvm_domain* m_domain = nullptr; // set_domain stores our domain handle
    /*protected*/ std::unordered_map<uvm_phase*, uvm_phase*> m_phase_imps;  // Functors to override ovm_root defaults

    //TND review protected, provide read-only accessor.
    uvm_phase* m_current_phase = nullptr;
    process*   m_phase_process = nullptr;

    bool m_build_done = false;
    int m_phasing_active = 0;
 
    virtual void set_int_local(const std::string& field_name, uvm_bitstream_t value, bool recurse = true) override;

    uvm_component* m_parent = nullptr;

    std::unordered_map<std::string, uvm_component*> m_children;
    std::unordered_map<uvm_component*, uvm_component*> m_children_by_handle;

    virtual bool m_add_child(uvm_component* child);
    void m_set_full_name();
    void set_full_name(std::string& new_full_name);

    // overridden to disable
    virtual uvm_object* create(const std::string& name = "") override; 
    virtual uvm_object* clone() override;

    uvm_event_pool* event_pool;

    uvm_verbosity recording_detail = UVM_NONE;

    void do_print(uvm_printer* printer) override;

    // Internal methods for setting up command line messaging stuff
    void m_set_cl_msg_args();
    void m_set_cl_verb();
    void m_set_cl_action();
    void m_set_cl_sev();
    void m_apply_verbosity_settings(uvm_phase* phase);

    // The verbosity settings may have a specific phase to start at. 
    // We will do this work in the phase_started callback. 
    struct m_verbosity_setting {
        std::string comp;
        std::string phase;
        time_t offset;
        uvm_verbosity verbosity;
        std::string id;

        // Define the < operator for sorting
        bool operator<(const m_verbosity_setting& other) const {
            if (verbosity != other.verbosity)
                return verbosity < other.verbosity;
            // Optionally compare other members if verbosity is equal
            if (offset != other.offset)
                return offset < other.offset;
            if (comp != other.comp)
                return comp < other.comp;
            if (phase != other.phase)
                return phase < other.phase;
            return id < other.id;
        }
    };

    std::vector<m_verbosity_setting> m_verbosity_settings;
    static std::vector<m_verbosity_setting> m_time_settings;

    // does the pre abort callback hierarchically
    /*local*/ void m_do_pre_abort();

    // Used to indicate whether phase be synced
    bool phase_synced = false;

    //indicate whether clk_trigger need to be called.
    bool under_clk_ctrl = true;

    // Virtual methods for dpi exchange data
    virtual bool dpi_sync(int reason, int trans_id);

    // Function to run for one clock cycle
    virtual void run_1cyc(uvm_clock_edge edge);

    // Function to run for one procision step
    virtual void run_1precision();

    virtual uvm_time& get_time();

    //------------------------------------------------------------------------------------//
    // Map of thread ID to queue of processes to be executed
    //------------------------------------------------------------------------------------//
    std::vector<std::shared_ptr<uvm_process_thread>> active_thread_q;

    double delay_time = 0;
    uvm_time_unit delay_unit = UVM_TU_UNDEF;

    uvmm_component_process_state proc_state = IDLE;

    int force_thread_id = -1;  //indicate later must use this thread_id;

    // Function to create a process thread, return the index of thread created.
    int create_process_thread();

    // Function to add a process to a thread
    bool add_process_to_thread(std::shared_ptr<uvm_process> proc);

    uvm_clock* clk = nullptr;  // Pointer to the associated vpi_clock, if any

    // Function to link a clock to this component
    virtual void setup_clock(uvm_clock* clk);

    virtual void handle_proc_and_thread();

    // Function to trigger clock and call run_phase
    virtual void clk_trigger(uvm_clock_edge clk_edge);

};

/// Manages UVM components by registering, retrieving, and removing them by ID.
class uvm_component_manager {
public:
    /// Registers a UVM component in the manager by its unique instance ID.
    /// @param comp Pointer to the UVM component to be registered.
    /// @throws std::runtime_error if a component with the same ID already exists.
    static void register_component(uvm_component* comp);

    /// Retrieves a UVM component by its unique instance ID.
    /// @param id The unique integer ID of the component.
    /// @return Pointer to the UVM component if found, or nullptr if not found.
    static uvm_component* get_component(int id);

    /// Removes a UVM component from the manager by its unique instance ID.
    /// @param id The unique integer ID of the component.
    static void remove_component(int id);

private:
    /// A static unordered map to store components indexed by their integer IDs.
    static std::unordered_map<int, uvm_component*> components;
};

#endif // UVM_COMPONENT_H