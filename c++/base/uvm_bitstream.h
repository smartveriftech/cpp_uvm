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

#ifndef _UVM_BITSTREAM_H_
#define _UVM_BITSTREAM_H_

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include <memory>
#include <iomanip>
#include <sstream>
#include <cstdint>

// Type definitions for convenience
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint8_t u8;

/**
 * The `uvm_bitstream` class provides a flexible way to manipulate bit vectors.
 * It offers various operations and utilities for handling bit-level data,
 * including assignment, bit manipulation, arithmetic operations, and display utilities.
 * This class is particularly useful in hardware verification environments where bit-level precision is required.
 */
class uvm_bitstream {

    // Static data members
    static u32 mask[32];     // Array of bit masks for bit manipulation
    static u8 count[256];    // Lookup table for counting bits set in a byte

    // Data members for storing bitstream properties and data
    u32 lSize;          // Size of the bitstream in bits
    u32* la = nullptr;  // Pointer to the array holding the bit values
    u32 lWordCount;     // Number of 32-bit words used in the array
    u32 lDisplayMode;   // Display mode (hex, binary, etc.)
    std::string psName; // Name of the bitstream

    // Private helper methods
    /**
     * Converts an unsigned 32-bit integer to a string.
     * @param i The integer to convert.
     * @returns The string representation of the integer.
     */
    std::string itos(u32 i) const;

    /**
     * Converts an unsigned 64-bit integer to a string.
     * @param i The integer to convert.
     * @returns The string representation of the integer.
     */
    std::string itos(u64 i) const;

    /**
     * Converts an unsigned 32-bit integer to a hexadecimal string.
     * @param i The integer to convert.
     * @returns The hexadecimal string representation of the integer.
     */
    std::string itohex(u32 i) const;

    /**
     * Adjusts the size of the bitstream by masking out unused bits.
     * This ensures that only the relevant bits are considered in operations.
     */
    void clip();

    /**
     * Parses a string to set the bitstream value.
     * Supports binary and hexadecimal representations.
     * @param pcValueAssign The string containing the bitstream value.
     */
    void parse_str(const char* pcValueAssign);

protected:
    /**
     * The `uvm_bitproxy` class provides a proxy for accessing and manipulating
     * sub-fields within a `uvm_bitstream`. It allows for operations on specific
     * ranges of bits without affecting the entire bitstream.
     */
    class uvm_bitproxy {

        friend class uvm_bitstream;

    private:
        uvm_bitstream* pbvParent; // Pointer to the parent bitstream
        u32 left;                 // Upper bit index of the sub-field
        u32 right;                // Lower bit index of the sub-field

        /**
         * Sets a sub-field of the parent bitstream using another bitstream.
         * @param b The bitstream to set the sub-field with.
         */
        void set_parent_field(const uvm_bitstream& b);

        /**
         * Retrieves the sub-field from the parent bitstream as a new bitstream.
         * @returns The bitstream representing the sub-field.
         */
        uvm_bitstream get_parent_trans_field() const;

        /**
         * Retrieves the sub-field from the parent bitstream and stores it in the provided bitstream.
         * @param b The bitstream to store the sub-field in.
         */
        void get_parent_trans_field(uvm_bitstream& b) const;

    public:
        /**
         * Constructor for `uvm_bitproxy`.
         * @param pParent Pointer to the parent bitstream.
         * @param l Upper bit index of the sub-field.
         * @param r Lower bit index of the sub-field.
         */
        uvm_bitproxy(uvm_bitstream* pParent, u32 l, u32 r);

        /**
         * Assignment operator to set the sub-field using another bitstream.
         * @param b The bitstream to assign from.
         * @returns Reference to the current `uvm_bitproxy`.
         */
        uvm_bitproxy& operator=(const uvm_bitstream& b);

        /**
         * Assignment operator to set the sub-field using another bit proxy.
         * @param cbp The bit proxy to assign from.
         * @returns Reference to the current `uvm_bitproxy`.
         */
        uvm_bitproxy& operator=(const uvm_bitproxy& cbp);

        /**
         * Sub-field access operator for nested sub-fields.
         * @param l Upper bit index relative to the current sub-field.
         * @param r Lower bit index relative to the current sub-field.
         * @returns A new `uvm_bitproxy` representing the nested sub-field.
         */
        uvm_bitproxy operator()(u32 l, u32 r) { return uvm_bitproxy(pbvParent, l + right, r + right); }

        /**
         * Bitwise NOT operator for the sub-field.
         * @returns The bitwise complement of the sub-field as a bitstream.
         */
        uvm_bitstream operator~() const;

        /**
         * Computes the parity (XOR) of the bits in the sub-field.
         * @returns `true` if the number of set bits is odd, `false` otherwise.
         */
        bool xor_op() const;

        /**
         * Retrieves a 32-bit unsigned integer from the sub-field.
         * @param n Index of the 32-bit word to retrieve (default is 0).
         * @returns The 32-bit unsigned integer value.
         */
        u32 get_u32(u32 n = 0);

        /**
         * @returns The lower bit index of the sub-field.
         */
        u32 get_lower() const { return right; }

        /**
         * @returns The upper bit index of the sub-field.
         */
        u32 get_upper() const { return left; }

        /**
         * Retrieves the sub-field as a string representation.
         * @param isBin If `true`, returns the binary representation; otherwise, returns the default format.
         * @returns The string representation of the sub-field.
         */
        const char* get_signal(bool isBin = false) const;
    };

    friend class uvm_bitproxy;

public:
    // Static members for default settings
    static int defaultRadix;                  // Default radix for numeric conversions
    static int autoStringWidthGeneration;     // Flag for automatic width generation from strings

    // Constructors
    /**
     * Constructor that initializes the bitstream with a value from a string.
     * @param pcValueAssign The string containing the initial value.
     */
    uvm_bitstream(const std::string& pcValueAssign);

    /**
     * Constructor that initializes the bitstream with a value from a C-style string.
     * @param pcValueAssign The string containing the initial value.
     */
    uvm_bitstream(const char* pcValueAssign);

    /**
     * Constructor that initializes the bitstream with a specified number of bytes.
     * @param numBytes Number of bytes to initialize.
     * @param val Pointer to the byte array containing the initial value.
     */
    uvm_bitstream(u32 numBytes, const char* val);

    /**
     * Constructor that initializes the bitstream with a value from a string, specifying size and name.
     * @param pcValueAssign The string containing the initial value.
     * @param lSizeAssign The size of the bitstream in bits.
     * @param psNameAssign The name of the bitstream.
     */
    uvm_bitstream(const char* pcValueAssign, int lSizeAssign, const char* psNameAssign = "??");

    /**
     * Constructor that initializes the bitstream with an integer value, specifying size and name.
     * @param lValueAssign The integer value to initialize with.
     * @param lSizeAssign The size of the bitstream in bits.
     * @param psNameAssign The name of the bitstream.
     */
    uvm_bitstream(int lValueAssign, int lSizeAssign = 32, const char* psNameAssign = "??");

    /**
     * Constructor that initializes the bitstream with an unsigned 32-bit value, specifying size and name.
     * @param lValueAssign The unsigned 32-bit value to initialize with.
     * @param lSizeAssign The size of the bitstream in bits.
     * @param psNameAssign The name of the bitstream.
     */
    uvm_bitstream(u32 lValueAssign, int lSizeAssign = 32, const char* psNameAssign = "??");

    /**
     * Constructor that initializes the bitstream with an unsigned 64-bit value, specifying size and name.
     * @param lValueAssign The unsigned 64-bit value to initialize with.
     * @param lSizeAssign The size of the bitstream in bits.
     * @param psNameAssign The name of the bitstream.
     */
    uvm_bitstream(u64 lValueAssign, int lSizeAssign = 64, const char* psNameAssign = "??");

    /**
     * Constructor that initializes the bitstream with an array of unsigned 64-bit values.
     * @param lValueAssign Pointer to the array of unsigned 64-bit values.
     * @param lSizeAssign The size of the bitstream in bits.
     * @param psNameAssign The name of the bitstream.
     */
    uvm_bitstream(const u64* lValueAssign, int lSizeAssign = 64, const char* psNameAssign = "??");

    /**
     * Copy constructor that initializes the bitstream from another bitstream.
     * @param cbv The bitstream to copy from.
     */
    uvm_bitstream(const uvm_bitstream& cbv);

    /**
     * Constructor that initializes the bitstream from a bit proxy.
     * @param cbp The bit proxy to initialize from.
     */
    uvm_bitstream(const uvm_bitproxy& cbp);

    /**
     * Default constructor that initializes an empty bitstream.
     */
    uvm_bitstream();

    // Destructor
    /**
     * Destructor that releases allocated resources.
     */
    virtual ~uvm_bitstream();

    // Initialization methods
    /**
     * Initializes the bitstream with a specified name and size.
     * @param psNameAssign The name to assign to the bitstream.
     * @param lSizeAssign The size of the bitstream in bits.
     */
    void init(const char* psNameAssign, u32 lSizeAssign);

    /**
     * Initializes the bitstream with a specified size.
     * @param lSizeAssign The size of the bitstream in bits.
     */
    void init(u32 lSizeAssign);

    /**
     * Clears a specific bit in the bitstream (sets it to 0).
     * @param lBit The bit index to clear.
     * @returns `true` if successful, `false` otherwise.
     */
    virtual bool clear_bit(u32 lBit);

    // Copy method
    /**
     * Copies the value from another bitstream into this bitstream.
     * @param cbvRef The bitstream to copy from.
     */
    void copy(const uvm_bitstream& cbvRef);

    // Assignment operators
    /**
     * Assignment operator to set the bitstream with an integer value.
     * @param lVal The integer value to assign.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator=(int lVal);

    /**
     * Assignment operator to set the bitstream with an unsigned 32-bit value.
     * @param lVal The unsigned 32-bit value to assign.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator=(u32 lVal);

    /**
     * Assignment operator to set the bitstream with an unsigned 64-bit value.
     * @param lVal The unsigned 64-bit value to assign.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator=(u64 lVal);

    /**
     * Assignment operator to set the bitstream with a string value.
     * @param psNew The string containing the value to assign.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator=(const char* psNew);

    /**
     * Assignment operator to set the bitstream with another bitstream.
     * @param cbvRef The bitstream to assign from.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator=(const uvm_bitstream& cbvRef);

    // Shift operators
    /**
     * Right shift operator.
     * Shifts the bits of the bitstream to the right by the specified amount.
     * @param lShiftRight The number of bits to shift.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator>>=(u32 lShiftRight);

    /**
     * Left shift operator.
     * Shifts the bits of the bitstream to the left by the specified amount.
     * @param lShiftLeft The number of bits to shift.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator<<=(u32 lShiftLeft);

    // Compound assignment operators
    /**
     * Compound addition operator.
     * Adds another bitstream to this bitstream.
     * @param cbv The bitstream to add.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator+=(const uvm_bitstream& cbv);

    /**
     * Compound subtraction operator.
     * Subtracts another bitstream from this bitstream.
     * @param cbv The bitstream to subtract.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator-=(const uvm_bitstream& cbv);

    /**
     * Compound XOR operator.
     * Performs a bitwise XOR with another bitstream.
     * @param cbv The bitstream to XOR with.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator^=(const uvm_bitstream& cbv);

    /**
     * Compound AND operator.
     * Performs a bitwise AND with another bitstream.
     * @param cbv The bitstream to AND with.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator&=(const uvm_bitstream& cbv);

    /**
     * Compound OR operator.
     * Performs a bitwise OR with another bitstream.
     * @param cbv The bitstream to OR with.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator|=(const uvm_bitstream& cbv);

    // Increment and decrement operators
    /**
     * Prefix increment operator.
     * Increments the bitstream by 1.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator++();

    /**
     * Prefix decrement operator.
     * Decrements the bitstream by 1.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& operator--();

    /**
     * Postfix increment operator.
     * Increments the bitstream by 1.
     * @returns A copy of the bitstream before incrementing.
     */
    uvm_bitstream operator++(int);

    /**
     * Postfix decrement operator.
     * Decrements the bitstream by 1.
     * @returns A copy of the bitstream before decrementing.
     */
    uvm_bitstream operator--(int);

    // Bitwise NOT operator
    /**
     * Bitwise NOT operator.
     * Performs a bitwise complement of the bitstream.
     * @returns A new bitstream with the complemented bits.
     */
    uvm_bitstream operator~() const;

    // Bit access operator
    /**
     * Bit access operator.
     * Retrieves the value of a specific bit.
     * @param lBit The bit index to access.
     * @returns The value of the bit (0 or 1).
     */
    u32 operator[](u32 lBit) const;

    /**
     * Bit access operator.
     * Retrieves the value of a specific bit.
     * @param lBit The bit index to access.
     * @returns The value of the bit (0 or 1).
     */
    u32 operator[](int lBit) const;

    // Sub-field access operators
    /**
     * Sub-field access operator (const version).
     * Retrieves a sub-field of the bitstream as a new bitstream.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @returns A new bitstream representing the sub-field.
     */
    const uvm_bitstream operator()(u32 lThisUpper, u32 lThisLower) const;

    /**
     * Sub-field access operator.
     * Provides a proxy for accessing and modifying a sub-field of the bitstream.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @returns A `uvm_bitproxy` for the sub-field.
     */
    uvm_bitproxy& operator()(u32 lThisUpper, u32 lThisLower);

    /**
     * Single-bit access operator (const version).
     * Retrieves a single bit as a new bitstream.
     * @param lThisUpper The bit index to access.
     * @returns A new bitstream representing the bit.
     */
    const uvm_bitstream operator()(u32 lThisUpper) const;

    /**
     * Single-bit access operator.
     * Provides a proxy for accessing and modifying a single bit.
     * @param lThisUpper The bit index to access.
     * @returns A `uvm_bitproxy` for the bit.
     */
    uvm_bitproxy operator()(u32 lThisUpper);

    /**
     * Conversion operator to a C-style string.
     * Provides a string representation of the bitstream.
     * @returns The string representation of the bitstream.
     */
    operator const char*() const;

    // Get methods
    /**
     * Retrieves a 32-bit unsigned integer from the bitstream.
     * @param lSelect Index of the 32-bit word to retrieve (default is 0).
     * @param shift Number of bits to shift (default is 0).
     * @returns The 32-bit unsigned integer value.
     */
    u32 get_u32(u32 lSelect = 0, u32 shift = 0) const;

    /**
     * Retrieves a 64-bit unsigned integer from the bitstream.
     * @param lSelect Index of the 32-bit word to start from (default is 0).
     * @param shift Number of bits to shift (default is 0).
     * @returns The 64-bit unsigned integer value.
     */
    u64 get_u64(u32 lSelect = 0, u32 shift = 0) const;

    /**
     * @returns The size of the bitstream in bits.
     */
    u32 get_size() const;

    /**
     * @returns The number of 32-bit words used in the bitstream.
     */
    u32 get_word_size() const;

    /**
     * @returns A pointer to the array of 32-bit words containing the bitstream data.
     */
    const u32* get_words_ptr() const;

    /**
     * Retrieves a sub-field of the bitstream as a new bitstream.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @returns A new bitstream representing the sub-field.
     */
    uvm_bitstream get_field(u32 lThisUpper, u32 lThisLower) const;

    /**
     * Retrieves a sub-field of the bitstream as a 32-bit unsigned integer.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @returns The 32-bit unsigned integer value.
     */
    u32 get_field_u32(u32 lThisUpper, u32 lThisLower) const;

    /**
     * Retrieves a sub-field of the bitstream as a 64-bit unsigned integer.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @returns The 64-bit unsigned integer value.
     */
    u64 get_field_u64(u32 lThisUpper, u32 lThisLower) const;

    /**
     * Retrieves the value of a specific bit in the bitstream.
     * @param lBit The bit index to access.
     * @returns The value of the bit (0 or 1).
     */
    bool get_bit(u32 lBit) const;

    /**
     * @returns The most significant bit index that is set to 1.
     * Returns 0xFFFFFFFF if no bits are set.
     */
    u32 get_msb() const;

    /**
     * @returns The least significant bit index that is set to 1.
     * Returns 0xFFFFFFFF if no bits are set.
     */
    u32 get_lsb() const;

    /**
     * Retrieves the bitstream as a vector of 32-bit unsigned integers.
     * @param vec The vector to store the values in.
     */
    void get(std::vector<u32>& vec) const;

    // Set methods
    /**
     * Sets a sub-field of the bitstream using another bitstream.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @param cbv The bitstream to set the sub-field with.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& set_field(u32 lThisUpper, u32 lThisLower, const uvm_bitstream& cbv);

    /**
     * Sets a sub-field of the bitstream using a string value.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @param cp The string containing the value to set.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& set_field(u32 lThisUpper, u32 lThisLower, const char* cp);

    /**
     * Sets a sub-field of the bitstream using an integer value.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @param cp The integer value to set.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& set_field(u32 lThisUpper, u32 lThisLower, int cp);

    /**
     * Sets a sub-field of the bitstream using an unsigned 32-bit value.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @param cp The unsigned 32-bit value to set.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& set_field(u32 lThisUpper, u32 lThisLower, u32 cp);

    /**
     * Sets a sub-field of the bitstream using an unsigned 64-bit value.
     * @param lThisUpper The upper bit index of the sub-field.
     * @param lThisLower The lower bit index of the sub-field.
     * @param cp The unsigned 64-bit value to set.
     * @returns Reference to the current bitstream.
     */
    uvm_bitstream& set_field(u32 lThisUpper, u32 lThisLower, u64 cp);

    /**
     * Sets the bitstream using a vector of 32-bit unsigned integers.
     * @param vec The vector containing the values to set.
     */
    void set(std::vector<u32>& vec);

    /**
     * Sets the bitstream using a vector of 8-bit unsigned integers.
     * @param vec The vector containing the values to set.
     */
    void set(std::vector<u8>& vec);

    /**
     * Sets a specific bit in the bitstream (sets it to 1).
     * @param lBit The bit index to set.
     * @returns `true` if successful, `false` otherwise.
     */
    bool set_bit(u32 lBit);

    /**
     * Sets a 32-bit unsigned integer value at a specific position in the bitstream.
     * @param lSelect Index of the 32-bit word to set.
     * @param val The value to set.
     * @param shift Number of bits to shift (default is 0).
     * @param upper Upper bound for masking (default is 32).
     */
    void set_u_int32_t(u32 lSelect, u32 val, u32 shift = 0, u32 upper = 32);

    // Utility methods
    /**
     * Computes the parity (XOR) of the bits in the bitstream.
     * @returns `true` if the number of set bits is odd, `false` otherwise.
     */
    bool parity() const;

    /**
     * Computes the XOR of all bits in the bitstream.
     * @returns The result of the XOR operation.
     */
    u32 xor_op() const;

    /**
     * Counts the number of set bits in a 32-bit unsigned integer.
     * @param val The value to count bits in.
     * @returns The number of bits set to 1.
     */
    static u32 bit_cnt(u32 val);

    /**
     * Counts the number of set bits in the bitstream.
     * @returns The number of bits set to 1.
     */
    u32 bit_cnt() const;

    /**
     * Clears the bitstream (sets all bits to 0).
     */
    inline void clear() { memset(la, 0, sizeof(*la) * lWordCount); }

    // Display methods
    /**
     * @returns The current display mode of the bitstream.
     */
    u32 get_display_mode() const;

    /**
     * Sets the display mode to hexadecimal.
     */
    void set_display_hex();

    /**
     * Sets the display mode to binary.
     */
    void set_display_binary();

    /**
     * Sets the display mode to wide binary.
     */
    void set_display_binary_wide();

    /**
     * Writes the bitstream as a hexadecimal string into a target buffer.
     * @param psTarget The buffer to write the string into.
     */
    void write_str_hex(char* psTarget) const;

    /**
     * Displays the bitstream in hexadecimal format to the specified output stream.
     * @param ios The output stream to write to.
     */
    void display_hex(std::ostream& ios) const;

    /**
     * Displays the bitstream in binary format to the specified output stream.
     * @param ios The output stream to write to.
     */
    void display_binary(std::ostream& ios) const;

    /**
     * Displays the bitstream in wide binary format to the specified output stream.
     * @param ios The output stream to write to.
     */
    void display_binary_wide(std::ostream& ios) const;

    /**
     * Retrieves the bitstream as a binary string.
     * @returns The binary string representation of the bitstream.
     */
    const char* display_binary_char() const;

    /**
     * Converts the bitstream to a string representation.
     * @returns The string representation of the bitstream.
     */
    std::string convert2string() const;

    /**
     * @returns The name of the bitstream.
     */
    virtual std::string get_name() const;

    /**
     * Sets the name of the bitstream.
     * @param name The new name to set.
     */
    virtual void set_name(std::string name);

    /**
     * Prints the bitstream value and name to the standard output.
     */
    void print() const;

    // Conversion and signaling methods
    /**
     * Retrieves the bitstream as a string representation in binary or hexadecimal format.
     * @param isBin If `true`, returns the binary representation.
     * @param isHex If `true`, returns the hexadecimal representation.
     * @returns The string representation of the bitstream.
     */
    const char* get_signal(bool isBin = false, bool isHex = false) const;

    // Friends for non-member operators
    friend bool operator>(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
    friend bool operator<(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
    friend bool operator>=(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
    friend bool operator<=(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
    friend bool operator!=(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
    friend bool operator==(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
    friend uvm_bitstream operator-(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
};

// Non-member operators
bool operator>(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
bool operator>(const uvm_bitstream& cbv1, const char* cbv2);
bool operator>(const uvm_bitstream& cbv1, int cbv2);
bool operator<(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
bool operator<(const uvm_bitstream& cbv1, const char* cbv2);
bool operator<(const uvm_bitstream& cbv1, int cbv2);
bool operator>=(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
bool operator>=(const uvm_bitstream& cbv1, const char* cbv2);
bool operator>=(const uvm_bitstream& cbv1, int cbv2);
bool operator<=(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
bool operator<=(const uvm_bitstream& cbv1, const char* cbv2);
bool operator<=(const uvm_bitstream& cbv1, int cbv2);
bool operator!=(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
bool operator!=(const uvm_bitstream& cbv1, const char* cbv2);
bool operator!=(const uvm_bitstream& cbv1, int cbv2);
bool operator!=(const uvm_bitstream& cbv1, u32 cbv2);
bool operator!=(const uvm_bitstream& cbv1, u64 cbv2);
bool operator==(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
bool operator==(const uvm_bitstream& cbv1, const char* cbv2);
bool operator==(const uvm_bitstream& cbv1, int cbv2);
bool operator==(const uvm_bitstream& cbv1, u32 cbv2);
bool operator==(const uvm_bitstream& cbv1, u64 cbv2);
uvm_bitstream operator-(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
std::ostream& operator<<(std::ostream& ios, const uvm_bitstream& cbv);
uvm_bitstream operator+(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
uvm_bitstream operator+(const uvm_bitstream& cbv1, int cbv2);
uvm_bitstream operator+(const uvm_bitstream& cbv1, u32 cbv2);
uvm_bitstream operator+(const uvm_bitstream& cbv1, u64 cbv2);
uvm_bitstream operator,(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
uvm_bitstream operator&(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
uvm_bitstream operator|(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);
uvm_bitstream operator^(const uvm_bitstream& cbv1, const uvm_bitstream& cbv2);

#endif //_UVM_BITSTREAM_H_