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

// uvm_factory.h

#ifndef UVM_FACTORY_H
#define UVM_FACTORY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "base/uvm_object.h"

class uvm_component;
class uvm_factory_override;

// Instance overrides by requested type lookup
class uvm_factory_queue_class {
public:
    std::vector<uvm_factory_override*> queue;
};

//------------------------------------------------------------------------------
// Title: UVM Factory
//
// This page covers the classes that define the UVM factory facility.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// CLASS: uvm_factory
//
//------------------------------------------------------------------------------
//
// As the name implies, uvm_factory is used to manufacture (create) UVM objects
// and components. Only one instance of the factory is present in a given
// simulation (termed a singleton). Object and component types are registered
// with the factory using lightweight proxies to the actual objects and
// components being created. The <uvm_object_registry #(T,Tname)> and
// <uvm_component_registry #(T,Tname)> class are used to proxy <uvm_objects>
// and <uvm_components>.
//
// The factory provides both name-based and type-based interfaces.
//
// type-based - The type-based interface is far less prone to errors in usage.
//   When errors do occur, they are caught at compile-time.
//
// name-based - The name-based interface is dominated 
//   by string arguments that can be misspelled and provided in the wrong order.
//   Errors in name-based requests might only be caught at the time of the call,
//   if at all. Further, the name-based interface is not portable across
//   simulators when used with parameterized classes.
//
// See <Usage> section for details on configuring and using the factory.
//

class uvm_factory {
public:
    // Constructor
    uvm_factory();

    // Destructor
    ~uvm_factory() = default;

    // Function: get()
    // Get the factory singleton
    //
    static uvm_factory* get();

    // Group: Registering Types

    // Registers the given proxy object, obj, with the factory. The proxy object
    // is a lightweight substitute for the component or object it represents. When
    // the factory needs to create an object of a given type, it calls the proxy's
    // create_object or create_component method to do so.
    //
    // When doing name-based operations, the factory calls the proxy's
    // get_type_name method to match against the requested_type_name argument in
    // subsequent calls to create_component_by_name and create_object_by_name.
    // If the proxy object's get_type_name method returns the empty string,
    // name-based lookup is effectively disabled.
    void register_type(uvm_object_wrapper* obj);

    // Group: Type & Instance Overrides

    // Function: set_inst_override_by_type
    void set_inst_override_by_type(uvm_object_wrapper* original_type, uvm_object_wrapper* override_type, const std::string& full_inst_path);
    // Function: set_inst_override_by_name
    //
    // Configures the factory to create an object of the override's type whenever
    // a request is made to create an object of the original type using a context
    // that matches ~full_inst_path~. The original type is typically a super class
    // of the override type.
    //
    // When overriding by type, the ~original_type~ and ~override_type~ are
    // handles to the types' proxy objects. Preregistration is not required.
    //
    // When overriding by name, the ~original_type_name~ typically refers to a
    // preregistered type in the factory. It may, however, be any arbitrary
    // string. Future calls to any of the create_* methods with the same string
    // and matching instance path will produce the type represented by
    // ~override_type_name~, which must be preregistered with the factory.
    //
    // The ~full_inst_path~ is matched against the contentation of
    // {~parent_inst_path~, ".", ~name~} provided in future create requests. The
    // ~full_inst_path~ may include wildcards (* and ?) such that a single
    // instance override can be applied in multiple contexts. A ~full_inst_path~
    // of "*" is effectively a type override, as it will match all contexts.
    //
    // When the factory processes instance overrides, the instance queue is
    // processed in order of override registrations, and the first override
    // match prevails. Thus, more specific overrides should be registered
    // first, followed by more general overrides.

    void set_inst_override_by_name(const std::string& original_type_name, const std::string& override_type_name, const std::string& full_inst_path);

    // Function: set_type_override_by_type
    void set_type_override_by_type(uvm_object_wrapper* original_type, uvm_object_wrapper* override_type, bool replace = true);
    // Function: set_type_override_by_name
    //
    // Configures the factory to create an object of the override's type whenever
    // a request is made to create an object of the original type, provided no
    // instance override applies. The original type is typically a super class of
    // the override type.
    //
    // When overriding by type, the ~original_type~ and ~override_type~ are
    // handles to the types' proxy objects. Preregistration is not required.
    //
    // When overriding by name, the ~original_type_name~ typically refers to a
    // preregistered type in the factory. It may, however, be any arbitrary
    // string. Future calls to any of the create_* methods with the same string
    // and matching instance path will produce the type represented by
    // ~override_type_name~, which must be preregistered with the factory.
    //
    // When ~replace~ is 1, a previous override on ~original_type_name~ is
    // replaced, otherwise a previous override, if any, remains intact.

    void set_type_override_by_name(const std::string& original_type_name, const std::string& override_type_name, bool replace = true);


    // Group: Creation

    // Function: create_object_by_type
    uvm_object* create_object_by_type(uvm_object_wrapper* requested_type, const std::string& parent_inst_path = "", const std::string& name = "");

    // Function: create_component_by_type
    uvm_component* create_component_by_type(uvm_object_wrapper* requested_type, const std::string& parent_inst_path = "", const std::string& name = "", uvm_component* parent = nullptr);
    // Function: create_component_by_name
    //
    // Creates and returns a component or object of the requested type, which may
    // be specified by type or by name. A requested component must be derived
    // from the <uvm_component> base class, and a requested object must be derived
    // from the <uvm_object> base class.
    //
    // When requesting by type, the ~requested_type~ is a handle to the type's
    // proxy object. Preregistration is not required.
    //
    // When requesting by name, the ~request_type_name~ is a string representing
    // the requested type, which must have been registered with the factory with
    // that name prior to the request. If the factory does not recognize the
    // ~requested_type_name~, an error is produced and a null handle returned.
    //
    // If the optional ~parent_inst_path~ is provided, then the concatenation,
    // {~parent_inst_path~, ".",~name~}, forms an instance path (context) that
    // is used to search for an instance override. The ~parent_inst_path~ is
    // typically obtained by calling the <uvm_component::get_full_name> on the
    // parent.
    //
    // If no instance override is found, the factory then searches for a type
    // override.
    //
    // Once the final override is found, an instance of that component or object
    // is returned in place of the requested type. New components will have the
    // given ~name~ and ~parent~. New objects will have the given ~name~, if
    // provided.
    //
    // Override searches are recursively applied, with instance overrides taking
    // precedence over type overrides. If ~foo~ overrides ~bar~, and ~xyz~
    // overrides ~foo~, then a request for ~bar~ will produce ~xyz~. Recursive
    // loops will result in an error, in which case the type returned will be
    // that which formed the loop. Using the previous example, if ~bar~
    // overrides ~xyz~, then ~bar~ is returned after the error is issued.

    // Function: create_object_by_name
    uvm_object* create_object_by_name(const std::string& requested_type_name, const std::string& parent_inst_path = "", const std::string& name = "");
    uvm_component* create_component_by_name(const std::string& requested_type_name, const std::string& parent_inst_path = "", const std::string& name = "", uvm_component* parent = nullptr);

    // Group: Debug

    // Function: debug_create_by_type
    void debug_create_by_type(uvm_object_wrapper* requested_type, const std::string& parent_inst_path = "", const std::string& name = "");
    // Function: debug_create_by_name
    //
    // These methods perform the same search algorithm as the create_* methods,
    // but they do not create new objects. Instead, they provide detailed
    // information about what type of object it would return, listing each
    // override that was applied to arrive at the result. Interpretation of the
    // arguments are exactly as with the create_* methods.

    void debug_create_by_name(const std::string& requested_type_name, const std::string& parent_inst_path = "", const std::string& name = "");

    // Function: find_override_by_type
    uvm_object_wrapper* find_override_by_type(uvm_object_wrapper* requested_type, const std::string& full_inst_path);

    // Function: find_override_by_name
    //
    // These methods return the proxy to the object that would be created given
    // the arguments. The ~full_inst_path~ is typically derived from the parent's
    // instance path and the leaf name of the object to be created, i.e.
    // { parent.get_full_name(), ".", name }.

    uvm_object_wrapper* find_override_by_name(const std::string& requested_type_name, const std::string& full_inst_path);

    uvm_object_wrapper* find_by_name(const std::string& type_name);

    // Function: print
    //
    // Prints the state of the uvm_factory, including registered types, instance
    // overrides, and type overrides.
    //
    // When ~all_types~ is 0, only type and instance overrides are displayed. When
    // ~all_types~ is 1 (default), all registered user-defined types are printed as
    // well, provided they have names associated with them. When ~all_types~ is 2,
    // the UVM types (prefixed with uvm_) are included in the list of registered
    void print(int all_types = 1);

  //----------------------------------------------------------------------------
  // PRIVATE MEMBERS

protected:
    void m_debug_create(const std::string& requested_type_name, uvm_object_wrapper* requested_type, const std::string& parent_inst_path, const std::string& name);
    void m_debug_display(const std::string& requested_type_name, uvm_object_wrapper* result, const std::string& full_inst_path);
    
private:
    static uvm_factory* m_inst;

    std::unordered_map<std::string, uvm_object_wrapper*> m_type_names;
    std::unordered_map<uvm_object_wrapper*, uvm_factory_queue_class*> m_inst_override_queues;
    std::unordered_map<std::string, uvm_factory_queue_class*> m_inst_override_name_queues;
    std::vector<uvm_factory_override*> m_type_overrides;
    std::vector<uvm_factory_override*> m_wildcard_inst_overrides;
    std::vector<uvm_factory_override*> m_override_info;
    std::unordered_map<uvm_object_wrapper*, bool> m_types;
    std::unordered_map<std::string, bool> m_lookup_strs;

    bool m_debug_pass;

    bool m_has_wildcard(const std::string& nm);
    bool check_inst_override_exists(uvm_object_wrapper*original_type, uvm_object_wrapper* override_type, const std::string& full_inst_path);
};


//------------------------------------------------------------------------------
//
// Group: Usage
//
// Using the factory involves three basic operations
//
// 1 - Registering objects and components types with the factory
// 2 - Designing components to use the factory to create objects or components
// 3 - Configuring the factory with type and instance overrides, both within and
//     outside components
//
// We'll briefly cover each of these steps here. More reference information can
// be found at <Utility Macros>, <uvm_component_registry #(T,Tname)>,
// <uvm_object_registry #(T,Tname)>, <uvm_component>.
//
// 1 -- Registering objects and component types with the factory:
//
// When defining <uvm_object> and <uvm_component>-based classes, simply invoke
// the appropriate macro. Use of macros are required to ensure portability
// across different vendors' simulators.
//
// Objects that are not parameterized are declared as
//
//|  class packet extends uvm_object;
//|    `uvm_object_utils(packet)
//|  endclass
//|
//|  class packetD extends packet;
//|    `uvm_object_utils(packetD)
//|  endclass
//
// Objects that are parameterized are declared as
//
//|  class packet #(type T=int, int WIDTH=32) extends uvm_object;
//|    `uvm_object_param_utils(packet #(T,WIDTH))
//|   endclass
//
// Components that are not parameterized are declared as
//
//|  class comp extends uvm_component;
//|    `uvm_component_utils(comp)
//|  endclass
//
// Components that are parameterized are declared as
//
//|  class comp #(type T=int, int WIDTH=32) extends uvm_component;
//|    `uvm_component_param_utils(comp #(T,WIDTH))
//|  endclass
//
// The `uvm_*_utils macros for simple, non-parameterized classes will register
// the type with the factory and define the get_type, get_type_name, and create
// virtual methods inherited from <uvm_object>. It will also define a static
// type_name variable in the class, which will allow you to determine the type
// without having to allocate an instance. 
//
// The `uvm_*_param_utils macros for parameterized classes differ from
// `uvm_*_utils classes in the following ways:
//
// - The get_type_name method and static type_name variable are not defined. You
//   will need to implement these manually.
//
// - A type name is not associated with the type when registeriing with the
//   factory, so the factory's *_by_name operations will not work with
//   parameterized classes.
//
// - The factory's <print>, <debug_create_by_type>, and <debug_create_by_name>
//   methods, which depend on type names to convey information, will list
//   parameterized types as <unknown>.
//
// It is worth noting that environments that exclusively use the type-based
// factory methods (*_by_type) do not require type registration. The factory's
// type-based methods will register the types involved "on the fly," when first
// used. However, registering with the `uvm_*_utils macros enables name-based
// factory usage and implements some useful utility functions.
//
//
// 2 -- Designing components that defer creation to the factory:
//
// Having registered your objects and components with the factory, you can now
// make requests for new objects and components via the factory. Using the factory
// instead of allocating them directly (via new) allows different objects to be
// substituted for the original without modifying the requesting class. The
// following code defines a driver class that is parameterized.
//
//|  class driverB #(type T=uvm_object) extends uvm_driver;
//|
//|    // parameterized classes must use the _param_utils version
//|    `uvm_component_param_utils(driverB #(T))
//|
//|    // our packet type; this can be overridden via the factory
//|    T pkt;
//|
//|    // standard component constructor
//|    function new(string name, uvm_component parent=null);
//|      super.new(name,parent);
//|    endfunction
//|
//|    // get_type_name not implemented by macro for parameterized classes
//|    const static string type_name = {"driverB #(",T::type_name,")"};
//|    virtual function string get_type_name();
//|      return type_name;
//|    endfunction
//|
//|    // using the factory allows pkt overrides from outside the class
//|    virtual function void build_phase(uvm_phase phase);
//|      pkt = packet::type_id::create("pkt",this);
//|    endfunction
//|
//|    // print the packet so we can confirm its type when printing
//|    virtual function void do_print(uvm_printer printer);
//|      printer.print_object("pkt",pkt);
//|    endfunction
//|
//|  endclass
//
// For purposes of illustrating type and instance overrides, we define two
// subtypes of the ~driverB~ class. The subtypes are also parameterized, so
// we must again provide an implementation for <uvm_object::get_type_name>,
// which we recommend writing in terms of a static string constant.
//
//|  class driverD1 #(type T=uvm_object) extends driverB #(T);
//|
//|    `uvm_component_param_utils(driverD1 #(T))
//|
//|    function new(string name, uvm_component parent=null);
//|      super.new(name,parent);
//|    endfunction
//|
//|    const static string type_name = {"driverD1 #(",T::type_name,")"};
//|    virtual function string get_type_name();
//|      ...return type_name;
//|    endfunction
//|
//|  endclass
//|
//|  class driverD2 #(type T=uvm_object) extends driverB #(T);
//|
//|    `uvm_component_param_utils(driverD2 #(T))
//|
//|    function new(string name, uvm_component parent=null);
//|      super.new(name,parent);
//|    endfunction
//|
//|    const static string type_name = {"driverD2 #(",T::type_name,")"};
//|    virtual function string get_type_name();
//|      return type_name;
//|    endfunction
//|
//|  endclass
//|
//|  // typedef some specializations for convenience
//|  typedef driverB  #(packet) B_driver;   // the base driver
//|  typedef driverD1 #(packet) D1_driver;  // a derived driver
//|  typedef driverD2 #(packet) D2_driver;  // another derived driver
//
// Next, we'll define a agent component, which requires a utils macro for
// non-parameterized types. Before creating the drivers using the factory, we
// override ~driver0~'s packet type to be ~packetD~.
//
//|  class agent extends uvm_agent;
//|
//|    `uvm_component_utils(agent)
//|    ...
//|    B_driver driver0;
//|    B_driver driver1;
//|
//|    function new(string name, uvm_component parent=null);
//|      super.new(name,parent);
//|    endfunction
//|
//|    virtual function void build_phase(uvm_phase phase);
//|
//|      // override the packet type for driver0 and below
//|      packet::type_id::set_inst_override(packetD::get_type(),"driver0.*");
//|
//|      // create using the factory; actual driver types may be different
//|      driver0 = B_driver::type_id::create("driver0",this);
//|      driver1 = B_driver::type_id::create("driver1",this);
//|
//|    endfunction
//|
//|  endclass
//
// Finally we define an environment class, also not parameterized. Its build
// method shows three methods for setting an instance override on a grandchild
// component with relative path name, ~agent1.driver1~, all equivalent.
//
//|  class env extends uvm_env;
//|
//|    `uvm_component_utils(env)
//|
//|    agent agent0;
//|    agent agent1;
//|
//|    function new(string name, uvm_component parent=null);
//|      super.new(name,parent);
//|    endfunction
//|
//|    virtual function void build_phase(uvm_phase phase);
//|
//|      // three methods to set an instance override for agent1.driver1
//|      // - via component convenience method...
//|      set_inst_override_by_type("agent1.driver1",
//|                                B_driver::get_type(),
//|                                D2_driver::get_type());
//|
//|      // - via the component's proxy (same approach as create)...
//|      B_driver::type_id::set_inst_override(D2_driver::get_type(),
//|                                           "agent1.driver1",this);
//|
//|      // - via a direct call to a factory method...
//|      factory.set_inst_override_by_type(B_driver::get_type(),
//|                                        D2_driver::get_type(),
//|                                        {get_full_name(),".agent1.driver1"});
//|
//|      // create agents using the factory; actual agent types may be different
//|      agent0 = agent::type_id::create("agent0",this);
//|      agent1 = agent::type_id::create("agent1",this);
//|
//|    endfunction
//|
//|    // at end_of_elaboration, print topology and factory state to verify
//|    virtual function void end_of_elaboration_phase(uvm_phase phase);
//|      uvm_top.print_topology();
//|    endfunction
//|
//|    virtual task run_phase(uvm_phase phase);
//|      #100 global_stop_request();
//|    endfunction
//|
//|  endclass
//   
//
// 3 -- Configuring the factory with type and instance overrides:
//
// In the previous step, we demonstrated setting instance overrides and creating
// components using the factory within component classes. Here, we will
// demonstrate setting overrides from outside components, as when initializing
// the environment prior to running the test.
//
//|  module top;
//|
//|    env env0;
//|
//|    initial begin
//|
//|      // Being registered first, the following overrides take precedence
//|      // over any overrides made within env0's construction & build.
//|
//|      // Replace all base drivers with derived drivers...
//|      B_driver::type_id::set_type_override(D_driver::get_type());
//|
//|      // ...except for agent0.driver0, whose type remains a base driver.
//|      //     (Both methods below have the equivalent result.)
//|
//|      // - via the component's proxy (preferred)
//|      B_driver::type_id::set_inst_override(B_driver::get_type(),
//|                                           "env0.agent0.driver0");
//|
//|      // - via a direct call to a factory method
//|      factory.set_inst_override_by_type(B_driver::get_type(),
//|                                        B_driver::get_type(),
//|                                    {get_full_name(),"env0.agent0.driver0"});
//|
//|      // now, create the environment; our factory configuration will
//|      // govern what topology gets created
//|      env0 = new("env0");
//|
//|      // run the test (will execute build phase)
//|      run_test();
//|
//|    end
//|
//|  endmodule
//
// When the above example is run, the resulting topology (displayed via a call to
// <uvm_root::print_topology> in env's <uvm_component::end_of_elaboration_phase> method)
// is similar to the following:
//
//| # UVM_INFO @ 0 [RNTST] Running test ...
//| # UVM_INFO @ 0 [UVMTOP] UVM testbench topology:
//| # ----------------------------------------------------------------------
//| # Name                     Type                Size                Value
//| # ----------------------------------------------------------------------
//| # env0                     env                 -                  env0@2
//| #   agent0                 agent               -                agent0@4
//| #     driver0              driverB #(packet)   -               driver0@8
//| #       pkt                packet              -                  pkt@21
//| #     driver1              driverD #(packet)   -              driver1@14
//| #       pkt                packet              -                  pkt@23
//| #   agent1                 agent               -                agent1@6
//| #     driver0              driverD #(packet)   -              driver0@24
//| #       pkt                packet              -                  pkt@37
//| #     driver1              driverD2 #(packet)  -              driver1@30
//| #       pkt                packet              -                  pkt@39
//| # ----------------------------------------------------------------------
// 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// CLASS: uvm_object_wrapper
//
// The uvm_object_wrapper provides an abstract interface for creating object and
// component proxies. Instances of these lightweight proxies, representing every
// <uvm_object>-based and <uvm_component>-based object available in the test
// environment, are registered with the <uvm_factory>. When the factory is
// called upon to create an object or component, it finds and delegates the
// request to the appropriate proxy.
//
//------------------------------------------------------------------------------

class uvm_object_wrapper {
public:
    // Function: create_object
    //
    // Creates a new object with the optional ~name~.
    // An object proxy (e.g., <uvm_object_registry #(T,Tname)>) implements this
    // method to create an object of a specific type, T.
    virtual uvm_object* create_object(const std::string& name = "") = 0;

    // Function: create_component
    //
    // Creates a new component, passing to its constructor the given ~name~ and
    // ~parent~. A component proxy (e.g. <uvm_component_registry #(T,Tname)>)
    // implements this method to create a component of a specific type, T.
    virtual uvm_component* create_component(const std::string& name, uvm_component* parent) = 0;

    // Function: get_type_name
    // 
    // Derived classes implement this method to return the type name of the object
    // created by <create_component> or <create_object>. The factory uses this
    // name when matching against the requested type in name-based lookups.
    virtual std::string get_type_name() = 0;
};

//------------------------------------------------------------------------------
//
// CLASS: uvm_factory_override
//
// Internal class.
//------------------------------------------------------------------------------

class uvm_factory_override {
public:
    std::string full_inst_path;
    std::string orig_type_name;
    std::string ovrd_type_name;
    bool selected;
    uvm_object_wrapper* orig_type;
    uvm_object_wrapper* ovrd_type;

    uvm_factory_override(const std::string& full_inst_path = "",
                         const std::string& orig_type_name = "",
                         uvm_object_wrapper* orig_type = nullptr,
                         uvm_object_wrapper* ovrd_type = nullptr);
};

//-----------------------------------------------------------------------------
// our singleton factory; it is statically initialized
//-----------------------------------------------------------------------------
extern const uvm_factory* factory;

#endif // UVM_FACTORY_H