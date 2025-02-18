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

#ifndef UVM_PACKER_H
#define UVM_PACKER_H

#include <string>
#include <vector>
#include <bitset>
#include <iostream>
#include <stdexcept>
#include <cstdint>

#include "base/uvm_object.h"

// Assuming the necessary definitions for these types are provided elsewhere
typedef std::vector<bool> uvm_pack_bitstream_t;
typedef double real;
typedef unsigned long long uvm_time_t;

class uvm_packer {
public:
    //----------------//
    // Group: Packing //
    //----------------//
  
    // Function: pack_field
    //
    // Packs an integral value (less than or equal to 4096 bits) into the
    // packed array. ~size~ is the number of bits of ~value~ to pack.
    virtual void pack_field(uvm_bitstream_t value, u_int32_t size);

    // Function: pack_field_int
    //
    // Packs the integral value (less than or equal to 64 bits) into the
    // pack array.  The ~size~ is the number of bits to pack, usually obtained by
    // ~$bits~. This optimized version of <pack_field> is useful for sizes up
    // to 64 bits.
    virtual void pack_field_int(u_int64_t value, u_int32_t size);

    // Function: pack_string
    //
    // Packs a string value into the pack array. 
    //
    // When the metadata flag is set, the packed string is terminated by a null
    // character to mark the end of the string.
    //
    // This is useful for mixed language communication where unpacking may occur
    // outside of SystemVerilog UVM.
    virtual void pack_string(const std::string& value);

    // Function: pack_time
    //
    // Packs a time ~value~ as 64 bits into the pack array.
    virtual void pack_time(uvm_time_t value); 

    // Function: pack_real
    //
    // Packs a real ~value~ as 64 bits into the pack array. 
    //
    // The real ~value~ is converted to a 64-bit scalar value using the function
    // $real2bits before it is packed into the array.
    virtual void pack_real(real value);

    // Function: pack_object
    //
    // Packs an object value into the pack array. 
    //
    // A 4-bit header is inserted ahead of the string to indicate the number of
    // bits that was packed. If a null object was packed, then this header will
    // be 0. 
    //
    // This is useful for mixed-language communication where unpacking may occur
    // outside of SystemVerilog UVM.
    virtual void pack_object(uvm_object* value);

    //------------------//
    // Group: Unpacking //
    //------------------//
  
    // Function: is_null
    //
    // This method is used during unpack operations to peek at the next 4-bit
    // chunk of the pack data and determine if it is 0.
    //
    // If the next four bits are all 0, then the return value is a 1; otherwise
    // it is 0. 
    //
    // This is useful when unpacking objects, to decide whether a new object
    // needs to be allocated or not.
    virtual bool is_null();

    // Function: unpack_field_int
    //
    // Unpacks bits from the pack array and returns the bit-stream that was
    // unpacked. 
    //
    // ~size~ is the number of bits to unpack; the maximum is 64 bits. 
    // This is a more efficient variant than unpack_field when unpacking into
    // smaller vectors.
    virtual unsigned long long unpack_field_int(u_int32_t size);

    // Function: unpack_field
    //
    // Unpacks bits from the pack array and returns the bit-stream that was
    // unpacked. ~size~ is the number of bits to unpack; the maximum is 4096 bits.
    virtual uvm_bitstream_t unpack_field(u_int32_t size);

    // Function: unpack_string
    //
    // Unpacks a string. 
    //
    // num_chars bytes are unpacked into a string. If num_chars is -1 then
    // unpacking stops on at the first null character that is encountered.
    virtual std::string unpack_string(int num_chars = -1);

    // Function: unpack_time
    //
    // Unpacks the next 64 bits of the pack array and places them into a
    // time variable.
    virtual uvm_time_t unpack_time(); 

    // Function: unpack_real
    //
    // Unpacks the next 64 bits of the pack array and places them into a
    // real variable. 
    //
    // The 64 bits of packed data are converted to a real using the $bits2real
    // system function.
    virtual real unpack_real();

    // Function: unpack_object
    //
    // Unpacks an object and stores the result into ~value~. 
    //
    // ~value~ must be an allocated object that has enough space for the data
    // being unpacked. The first four bits of packed data are used to determine
    // if a null object was packed into the array. 
    //
    // The <is_null> function can be used to peek at the next four bits in
    // the pack array before calling this method.
    virtual void unpack_object(uvm_object* value);
    virtual void unpack_object_ext(uvm_object*& value);

    // Function: get_packed_size
    //
    // Returns the number of bits that were packed.
    virtual int get_packed_size(); 

    //------------------//
    // Group: Variables //
    //------------------//

    // Variable: physical
    //
    // This bit provides a filtering mechanism for fields.
    //
    // The <abstract> and physical settings allow an object to distinguish between
    // two different classes of fields. It is up to you, in the
    // <uvm_object::do_pack> and <uvm_object::do_unpack> methods, to test the
    // setting of this field if you want to use it as a filter.
    bool physical = true;

    // Variable: abstract
    //
    // This bit provides a filtering mechanism for fields. 
    //
    // The abstract and physical settings allow an object to distinguish between
    // two different classes of fields. It is up to you, in the
    // <uvm_object::do_pack> and <uvm_object::do_unpack> routines, to test the
    // setting of this field if you want to use it as a filter.
    bool abstract = false;

    // Variable: use_metadata
    //
    // This flag indicates whether to encode metadata when packing dynamic data,
    // or to decode metadata when unpacking.  Implementations of <uvm_object::do_pack>
    // and <uvm_object::do_unpack> should regard this bit when performing their
    // respective operation. When set, metadata should be encoded as follows:
    //
    // - For strings, pack an additional null byte after the string is packed.
    //
    // - For objects, pack 4 bits prior to packing the object itself. Use 4'b0000
    //   to indicate the object being packed is null, otherwise pack 4'b0001 (the
    //   remaining 3 bits are reserved).
    //
    // - For queues, dynamic arrays, and associative arrays, pack 32 bits
    //   indicating the size of the array prior to to packing individual elements.
    bool use_metadata = false;

    // Variable: big_endian
    //
    // This bit determines the order that integral data is packed (using
    // <pack_field>, <pack_field_int>, <pack_time>, or <pack_real>) and how the
    // data is unpacked from the pack array (using <unpack_field>,
    // <unpack_field_int>, <unpack_time>, or <unpack_real>). When the bit is set,
    // data is associated msb to lsb; otherwise, it is associated lsb to msb. 
    //
    // The following code illustrates how data can be associated msb to lsb and
    // lsb to msb:
    //
    //|  class mydata extends uvm_object;
    //|
    //|    logic[15:0] value = 'h1234;
    //|
    //|    function void do_pack (uvm_packer packer);
    //|      packer.pack_field_int(value, 16);
    //|    endfunction
    //|
    //|    function void do_unpack (uvm_packer packer);
    //|      value = packer.unpack_field_int(16);
    //|    endfunction
    //|  endclass
    //|
    //|  mydata d = new;
    //|  bit bits[];
    //|
    //|  initial begin
    //|    d.pack(bits);  // 'b0001001000110100
    //|    uvm_default_packer.big_endian = 0;
    //|    d.pack(bits);  // 'b0010110001001000
    //|  end

    bool big_endian = true;

    // variables and methods primarily for internal use
    static std::vector<bool> bitstream;   // local bits for (un)pack_bytes
    static std::vector<bool> fabitstream; // field automation bits for (un)pack_bytes
    u_int32_t count = 0;                // used to count the number of packed bits
    uvm_scope_stack scope;

    bool reverse_order = false;      // flip the bit order around
    unsigned char byte_size = 8;  // set up bytesize for endianess
    u_int32_t word_size = 16; // set up worksize for endianess
    bool nopack = false;             // only count packable bits

    uvm_recursion_policy_enum policy = UVM_DEFAULT_POLICY;

    uvm_pack_bitstream_t m_bits;
    u_int32_t m_packed_size = 0;

    // Constructor
    uvm_packer() = default;

    // Utility functions
    void index_error(u_int32_t index, const std::string& id, u_int32_t sz);
    bool enough_bits(u_int32_t needed, const std::string& id);
    void reset();

    // Get functions
    uvm_pack_bitstream_t get_packed_bits();
    void set_packed_size();
    bool get_bit(unsigned int index);
    unsigned char get_byte(unsigned int index);
    unsigned int get_int(unsigned int index);
    void get_bits(std::vector<bool>& bits);
    void get_bytes(std::vector<unsigned char>& bytes);
    void get_ints(std::vector<unsigned int>& ints);

    // Put functions
    void put_bits(const std::vector<bool>& bitstream);
    void put_bytes(const std::vector<unsigned char>& bytestream);
    void put_ints(const std::vector<unsigned int>& intstream);
};

#endif // UVM_PACKER_H