//
//------------------------------------------------------------------------------
// Copyright 2007-2011 Cadence Design Systems, Inc.
// Copyright 2007-2011 Mentor Graphics Corporation
// Copyright 2010 Synopsys, Inc.
// Copyright 2013 NVIDIA Corporation
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

#ifndef UVM_OBJECT_H
#define UVM_OBJECT_H

#include <string>
#include <functional>
#include <vector>

#include "base/uvm_misc.h"
#include "base/uvm_globals.h"
#include "macros/uvm_object_defines.h"

//------------------------------------------------------------------------------
//
// CLASS: uvm_object
//
// The uvm_object class is the base class for all UVM data and hierarchical 
// classes. Its primary role is to define a set of methods for such common
// operations as <create>, <copy>, <compare>, <print>, and <record>. Classes
// deriving from uvm_object must implement the pure virtual methods such as 
// <create> and <get_type_name>.
//
//------------------------------------------------------------------------------

class uvm_recorder;
class uvm_object_wrapper;
class uvm_packer;
class uvm_comparer;
class uvm_printer;
class uvm_status_container;
class uvm_report_object;

class uvm_object : public uvm_void  {
public:
    // Function: new
    //
    // Creates a new uvm_object with the given instance ~name~. If ~name~ is not
    // supplied, the object is unnamed.
    uvm_object(const std::string& name = "");
    virtual ~uvm_object() {}

    // Group: Seeding

    // Variable: use_uvm_seeding
    //
    // This bit enables or disables the UVM seeding mechanism. It globally affects
    // the operation of the reseed method. 
    //
    // When enabled, UVM-based objects are seeded based on their type and full
    // hierarchical name rather than allocation order. This improves random
    // stability for objects whose instance names are unique across each type.
    // The <uvm_component> class is an example of a type that has a unique
    // instance name.
    static bool use_uvm_seeding;

    // Function: reseed
    //
    // Calls ~srandom~ on the object to reseed the object using the UVM seeding
    // mechanism, which sets the seed based on type name and instance name instead
    // of based on instance position in a thread. 
    //
    // If the <use_uvm_seeding> static variable is set to 0, then reseed() does
    // not perform any function. 
    virtual void reseed();

    // Group: Identification

    // Function: set_name
    //
    // Sets the instance name of this object, overwriting any previously
    // given name.
    virtual void set_name(const std::string& name);

    // Function: get_name
    //
    // Returns the name of the object, as provided by the ~name~ argument in the
    // <new> constructor or <set_name> method.
    virtual std::string get_name();

    // Function: get_full_name
    //
    // Returns the full hierarchical name of this object. The default
    // implementation is the same as <get_name>, as uvm_objects do not inherently
    // possess hierarchy. 
    //
    // Objects possessing hierarchy, such as <uvm_components>, override the default
    // implementation. Other objects might be associated with component hierarchy
    // but are not themselves components. For example, <uvm_sequence #(REQ,RSP)>
    // classes are typically associated with a <uvm_sequencer #(REQ,RSP)>. In this
    // case, it is useful to override get_full_name to return the sequencer's
    // full name concatenated with the sequence's name. This provides the sequence
    // a full context, which is useful when debugging.
    virtual std::string get_full_name();

    // Function: get_inst_id
    //
    // Returns the object's unique, numeric instance identifier.
    virtual int get_inst_id();

    // Function: get_inst_count
    //
    // Returns the current value of the instance counter, which represents the
    // total number of uvm_object-based objects that have been allocated in
    // simulation. The instance counter is used to form a unique numeric instance
    // identifier.
    static int get_inst_count();

    // Function: get_type
    //
    // Returns the type-proxy (wrapper) for this object. The <uvm_factory>'s
    // type-based override and creation methods take arguments of
    // <uvm_object_wrapper>. This method, if implemented, can be used as convenient
    // means of supplying those arguments.
    //
    // The default implementation of this method produces an error and returns
    // null. To enable use of this method, a user's subtype must implement a
    // version that returns the subtype's wrapper.
    //
    // For example:
    //
    //|  class cmd extends uvm_object;
    //|    typedef uvm_object_registry #(cmd) type_id;
    //|    static function type_id get_type();
    //|      return type_id::get();
    //|    endfunction
    //|  endclass
    //
    // Then, to use:
    //
    //|  factory.set_type_override(cmd::get_type(),subcmd::get_type());
    //
    // This function is implemented by the `uvm_*_utils macros, if employed.
    static uvm_object_wrapper* get_type();

    // Function: get_object_type
    //
    // Returns the type-proxy (wrapper) for this object. The <uvm_factory>'s
    // type-based override and creation methods take arguments of
    // <uvm_object_wrapper>. This method, if implemented, can be used as convenient
    // means of supplying those arguments. This method is the same as the static
    // <get_type> method, but uses an already allocated object to determine
    // the type-proxy to access (instead of using the static object).
    //
    // The default implementation of this method does a factory lookup of the
    // proxy using the return value from <get_type_name>. If the type returned
    // by <get_type_name> is not registered with the factory, then a null 
    // handle is returned.
    //
    // For example:
    //
    //|  class cmd extends uvm_object;
    //|    typedef uvm_object_registry #(cmd) type_id;
    //|    static function type_id get_type();
    //|      return type_id::get();
    //|    endfunction
    //|    virtual function type_id get_object_type();
    //|      return type_id::get();
    //|    endfunction
    //|  endclass
    //
    // This function is implemented by the `uvm_*_utils macros, if employed.
    virtual uvm_object_wrapper* get_object_type();

    // Function: get_type_name
    //
    // This function returns the type name of the object, which is typically the
    // type identifier enclosed in quotes. It is used for various debugging
    // functions in the library, and it is used by the factory for creating
    // objects.
    //
    // This function must be defined in every derived class. 
    //
    // A typical implementation is as follows:
    //
    //|  class mytype extends uvm_object;
    //|    ...
    //|    const static string type_name = "mytype";
    //|
    //|    virtual function string get_type_name();
    //|      return type_name;
    //|    endfunction
    //
    // We define the ~type_name~ static variable to enable access to the type name
    // without need of an object of the class, i.e., to enable access via the
    // scope operator, ~mytype::type_name~.
    virtual std::string get_type_name();

    // Group: Creation

    // Function: create
    //
    // The create method allocates a new object of the same type as this object
    // and returns it via a base uvm_object handle. Every class deriving from
    // uvm_object, directly or indirectly, must implement the create method.
    //
    // A typical implementation is as follows:
    //
    //|  class mytype extends uvm_object;
    //|    ...
    //|    virtual function uvm_object create(string name="");
    //|      mytype t = new(name);
    //|      return t;
    //|    endfunction 
    virtual uvm_object* create(const std::string& name = "");

    // Function: clone
    //
    // The clone method creates and returns an exact copy of this object.
    // 
    // The default implementation calls <create> followed by <copy>. As clone is
    // virtual, derived classes may override this implementation if desired. 
    virtual uvm_object* clone();

    // Group: Printing

    // Function: print
    // 
    // The print method deep-prints this object's properties in a format and
    // manner governed by the given ~printer~ argument; if the ~printer~ argument
    // is not provided, the global <uvm_default_printer> is used. See 
    // <uvm_printer> for more information on printer output formatting. See also
    // <uvm_line_printer>, <uvm_tree_printer>, and <uvm_table_printer> for details
    // on the pre-defined printer "policies," or formatters, provided by the UVM.
    //
    // The ~print~ method is not virtual and must not be overloaded. To include
    // custom information in the ~print~ and ~sprint~ operations, derived classes
    // The ~print~ method is not virtual and must not be overloaded. To include
    // custom information in the ~print~ and ~sprint~ operations, derived classes
    // must override the <do_print> method and use the provided printer policy
    // class to format the output.
    virtual void print(uvm_printer* printer = nullptr);

    // Function: sprint
    //
    // The ~sprint~ method works just like the <print> method, except the output
    // is returned in a string rather than displayed. 
    //
    // The ~sprint~ method is not virtual and must not be overloaded. To include
    // additional fields in the ~print~ and ~sprint~ operation, derived classes
    // must override the <do_print> method and use the provided printer policy
    // class to format the output. The printer policy will manage all string
    // concatenations and provide the string to ~sprint~ to return to the caller.
    std::string sprint(uvm_printer* printer = nullptr);

    // Function: do_print
    //
    // The ~do_print~ method is the user-definable hook called by <print> and
    // <sprint> that allows users to customize what gets printed or sprinted 
    // beyond the field information provided by the `uvm_field_* macros,
    // <Utility and Field Macros for Components and Objects>.
    //
    // The ~printer~ argument is the policy object that governs the format and
    // content of the output. To ensure correct <print> and <sprint> operation,
    // and to ensure a consistent output format, the ~printer~ must be used
    // by all <do_print> implementations. That is, instead of using ~$display~ or
    // string concatenations directly, a ~do_print~ implementation must call
    // through the ~printer's~ API to add information to be printed or sprinted.
    //
    // An example implementation of ~do_print~ is as follows:
    //
    //| class mytype extends uvm_object;
    //|   data_obj data;
    //|   int f1;
    //|   virtual function void do_print (uvm_printer printer);
    //|     super.do_print(printer);
    //|     printer.print_int("f1", f1, $bits(f1), DEC);
    //|     printer.print_object("data", data);
    //|   endfunction
    //
    // Then, to print and sprint the object, you could write:
    //
    //| mytype t = new;
    //| t.print();
    //| uvm_info("Received",t.sprint());
    //
    // See <uvm_printer> for information about the printer API.
    virtual void do_print(uvm_printer* printer);

    // Function: convert2string
    //
    // This virtual function is a user-definable hook, called directly by the
    // user, that allows users to provide object information in the form of
    // a string. Unlike <sprint>, there is no requirement to use an <uvm_printer>
    // policy object. As such, the format and content of the output is fully
    // customizable, which may be suitable for applications not requiring the
    // consistent formatting offered by the <print>/<sprint>/<do_print>
    // API.
    //
    // Fields declared in <Utility Macros> macros (`uvm_field_*), if used, will
    // not automatically appear in calls to convert2string.
    //
    // An example implementation of convert2string follows.
    // 
    //| class base extends uvm_object;
    //|   string field = "foo";
    //|   virtual function string convert2string();
    //|     convert2string = {"base_field=",field};
    //|   endfunction
    //| endclass
    //| 
    //| class obj2 extends uvm_object;
    //|   string field = "bar";
    //|   virtual function string convert2string();
    //|     convert2string = {"child_field=",field};
    //|   endfunction
    //| endclass
    //| 
    //| class obj extends base;
    //|   int addr = 'h123;
    //|   int data = 'h456;
    //|   bit write = 1;
    //|   obj2 child = new;
    //|   virtual function string convert2string();
    //|      convert2string = {super.convert2string(),
    //|        $sformatf(" write=%0d addr=%8h data=%8h ",write,addr,data),
    //|        child.convert2string()};
    //|   endfunction
    //| endclass
    //
    // Then, to display an object, you could write:
    //
    //| obj o = new;
    //| uvm_info("BusMaster",{"Sending:\n ",o.convert2string()});
    //
    // The output will look similar to:
    //
    //| UVM_INFO @ 0: reporter [BusMaster] Sending:
    //|    base_field=foo write=1 addr=00000123 data=00000456 child_field=bar
    virtual std::string convert2string();

    // Group: Recording

    // Function: record
    //
    // The record method deep-records this object's properties according to an
    // optional ~recorder~ policy. The method is not virtual and must not be
    // overloaded. To include additional fields in the record operation, derived
    // classes should override the <do_record> method.
    //
    // The optional ~recorder~ argument specifies the recording policy, which
    // governs how recording takes place. If a recorder policy is not provided
    // explicitly, then the global <uvm_default_recorder> policy is used. See
    // uvm_recorder for information.
    //
    // A simulator's recording mechanism is vendor-specific. By providing access
    // via a common interface, the uvm_recorder policy provides vendor-independent
    // access to a simulator's recording capabilities.
    void record(uvm_recorder* recorder = nullptr);

    // Function: do_record
    //
    // The do_record method is the user-definable hook called by the <record>
    // method. A derived class should override this method to include its fields
    // in a record operation.
    //
    // The ~recorder~ argument is policy object for recording this object. A
    // do_record implementation should call the appropriate recorder methods for
    // each of its fields. Vendor-specific recording implementations are
    // encapsulated in the ~recorder~ policy, thereby insulating user-code from
    // vendor-specific behavior. See <uvm_recorder> for more information.
    //
    // A typical implementation is as follows:
    //
    //| class mytype extends uvm_object;
    //|   data_obj data;
    //|   int f1;
    //|   function void do_record (uvm_recorder recorder);
    //|     recorder.record_field_int("f1", f1, $bits(f1), DEC);
    //|     recorder.record_object("data", data);
    //|   endfunction
    virtual void do_record(uvm_recorder* recorder);

    // Group: Copying

    // Function: copy
    //
    // The copy makes this object a copy of the specified object.
    //
    // The copy method is not virtual and should not be overloaded in derived
    // classes. To copy the fields of a derived class, that class should override
    // the <do_copy> method.
    void copy(uvm_object* rhs);

    // Function: do_copy
    //
    // The do_copy method is the user-definable hook called by the copy method.
    // A derived class should override this method to include its fields in a copy
    // operation.
    //
    // A typical implementation is as follows:
    //
    //|  class mytype extends uvm_object;
    //|    ...
    //|    int f1;
    //|    function void do_copy (uvm_object rhs);
    //|      mytype rhs_;
    //|      super.do_copy(rhs);
    //|      $cast(rhs_,rhs);
    //|      field_1 = rhs_.field_1;
    //|    endfunction
    //
    // The implementation must call ~super.do_copy~, and it must $cast the rhs
    // argument to the derived type before copying. 
    virtual void do_copy(uvm_object* rhs);

    // Group: Comparing

    // Function: compare
    //
    // Deep compares members of this data object with those of the object provided
    // in the ~rhs~ (right-hand side) argument, returning 1 on a match, 0 otherwise.
    //
    // The compare method is not virtual and should not be overloaded in derived
    // classes. To compare the fields of a derived class, that class should
    // override the <do_compare> method.
    //
    // The optional ~comparer~ argument specifies the comparison policy. It allows
    // you to control some aspects of the comparison operation. It also stores the
    // results of the comparison, such as field-by-field miscompare
    // results of the comparison, such as field-by-field miscompare information
    // and the total number of miscompares. If a compare policy is not provided,
    // then the global ~uvm_default_comparer~ policy is used. See <uvm_comparer> 
    // for more information.
    bool compare(uvm_object* rhs, uvm_comparer* comparer = nullptr);

    // Function: do_compare
    //
    // The do_compare method is the user-definable hook called by the <compare>
    // method. A derived class should override this method to include its
    // fields in a compare operation. It should return 1 if the comparison succeeds, 0
    // otherwise.
    //
    // A typical implementation is as follows:
    //
    //|  class mytype extends uvm_object;
    //|    ...
    //|    int f1;
    //|    virtual function bit do_compare (uvm_object rhs, uvm_comparer comparer);
    //|      mytype rhs_;
    //|      do_compare = super.do_compare(rhs, comparer);
    //|      $cast(rhs_, rhs);
    //|      do_compare &= comparer.compare_field_int("f1", f1, rhs_.f1);
    //|    endfunction
    //
    // A derived class implementation must call ~super.do_compare()~ to ensure its
    // base class' properties, if any, are included in the comparison. Also, the
    // rhs argument is provided as a generic uvm_object. Thus, you must ~$cast~ it
    // to the type of this object before comparing. 
    //
    // The actual comparison should be implemented using the uvm_comparer object
    // rather than direct field-by-field comparison. This enables users of your
    // class to customize how comparisons are performed and how much miscompare
    // information is collected. See uvm_comparer for more details.
    virtual bool do_compare(uvm_object* rhs, uvm_comparer* comparer);

    // Group: Packing

    // Function: pack
    //
    // The pack methods bitwise-concatenate this object's properties into an array
    // of bits, bytes, or ints. The methods are not virtual and must not be
    // overloaded. To include additional fields in the pack operation, derived
    // classes should override the <do_pack> method.
    //
    // The optional ~packer~ argument specifies the packing policy, which governs
    // the packing operation. If a packer policy is not provided, the global
    // <uvm_default_packer> policy is used. See <uvm_packer> for more information.
    //
    // The return value is the total number of bits packed into the given array.
    // Use the array's built-in ~size~ method to get the number of bytes or ints
    // consumed during the packing process.
    int pack(std::vector<bool>& bitstream, uvm_packer* packer = nullptr);
    int pack_bytes(std::vector<uint8_t>& bytestream, uvm_packer* packer = nullptr);
    int pack_ints(std::vector<uint32_t>& intstream, uvm_packer* packer = nullptr);
    virtual void do_pack(uvm_packer* packer);

    // Group: Unpacking

    // Function: unpack
    //
    // The unpack methods extract property values from an array of bits, bytes, or
    // ints. The method of unpacking ~must~ exactly correspond to the method of
    // packing. This is assured if (a) the same ~packer~ policy is used to pack
    // and unpack, and (b) the order of unpacking is the same as the order of
    // packing used to create the input array.
    //
    // The unpack methods are fixed (non-virtual) entry points that are directly
    // callable by the user. To include additional fields in the <unpack>
    // operation, derived classes should override the <do_unpack> method.
    //
    // The optional ~packer~ argument specifies the packing policy, which governs
    // both the pack and unpack operation. If a packer policy is not provided,
    // then the global ~uvm_default_packer~ policy is used. See uvm_packer for
    // more information.
    //
    // The return value is the actual number of bits unpacked from the given array.
    int unpack(const std::vector<bool>& bitstream, uvm_packer* packer = nullptr);
    int unpack_bytes(const std::vector<uint8_t>& bytestream, uvm_packer* packer = nullptr);
    int unpack_ints(const std::vector<uint32_t>& intstream, uvm_packer* packer = nullptr);
    virtual void do_unpack(uvm_packer* packer);

    // Group: Configuration

    // Function: set_int_local
    //
    // These methods provide write access to integral, string, and 
    // uvm_object-based properties indexed by a ~field_name~ string. The object
    // designer chooses which, if any, properties will be accessible, and overrides
    // the appropriate methods depending on the properties' types. For objects,
    // the optional ~clone~ argument specifies whether to clone the ~value~
    // argument before assignment.
    //
    // The global <uvm_is_match> function is used to match the field names, so
    // ~field_name~ may contain wildcards.
    //
    // An example implementation of all three methods is as follows.
    //
    //| class mytype extends uvm_object;
    //|
    //|   local int myint;
    //|   local byte mybyte;
    //|   local shortint myshort; // no access
    //|   local string mystring;
    //|   local obj_type myobj;
    //| 
    //|   // provide access to integral properties
    //|   function void set_int_local(string field_name, uvm_bitstream_t value);
    //|     if (uvm_is_match(field_name, "myint"))
    //|       myint = value;
    //|     else if (uvm_is_match(field_name, "mybyte"))
    //|       mybyte = value;
    //|   endfunction
    //| 
    //|   // provide access to string properties
    //|   function void set_string_local(string field_name, string value);
    //|     if (uvm_is_match(field_name, "mystring"))
    //|       mystring = value;
    //|   endfunction
    //| 
    //|   // provide access to sub-objects
    //|   function void set_object_local(string field_name, uvm_object value,
    //|                                  bit clone=1);
    //|     if (uvm_is_match(field_name, "myobj")) begin
    //|       if (value != null) begin
    //|         obj_type tmp;
    //|         // if provided value is not correct type, produce error
    //|         if (!$cast(tmp, value))
    //|           /* error */
    //|         else begin
    //|           if (clone)
    //|             $cast(myobj, tmp.clone());
    //|           else
    //|             myobj = tmp;
    //|         end
    //|       end
    //|       else
    //|         myobj = null; // value is null, so simply assign null to myobj
    //|     end
    //|   endfunction
    //|   ...
    //
    // Although the object designer implements these methods to provide outside
    // access to one or more properties, they are intended for internal use (e.g.,
    // for command-line debugging and auto-configuration) and should not be called
    // directly by the user.
    virtual void set_int_local(const std::string& field_name, uvm_bitstream_t value, bool recurse = true);
    virtual void set_string_local(const std::string& field_name, const std::string& value, bool recurse = true);
    virtual void set_object_local(const std::string& field_name, uvm_object* value, bool clone = true, bool recurse = true);

//protected:
    // Internal Methods and Properties
    // Do not use directly
    void m_pack(uvm_packer* packer);
    void m_unpack_pre(uvm_packer* packer);
    void m_unpack_post(uvm_packer* packer);

    static uvm_status_container __m_uvm_status_container;

//private:
    std::string m_leaf_name;
    int m_inst_id;
    static int m_inst_count;

    virtual uvm_report_object* m_get_report_object();
    virtual void __m_uvm_field_automation(uvm_object* tmp_data, int what, const std::string& str);
};

#endif // UVM_OBJECT_H