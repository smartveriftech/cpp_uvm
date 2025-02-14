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

#ifndef UVM_TYPES_H
#define UVM_TYPES_H

#include <iostream>
#include <string>
#include <cstdint>
#include <bitset>
#include <vector>
#include <array>

#include "macros/uvm_message_defines.h"

//This bit marks where filtering should occur to remove uvm stuff from a
//scope
const bool uvm_start_uvm_declarations = 1;

//------------------------------------------------------------------------------
//
// Section: Types and Enumerations
//
//------------------------------------------------------------------------------

//------------------------
// Group: Field automation
//------------------------

// Macro: `UVM_MAX_STREAMBITS
//
// Defines the maximum bit vector size for integral types.

#ifndef UVM_MAX_STREAMBITS
#define UVM_MAX_STREAMBITS 4096
#endif

// Macro: `UVM_PACKER_MAX_BYTES
//
// Defines the maximum bytes to allocate for packing an object using
// the <uvm_packer>. Default is <`UVM_MAX_STREAMBITS>, in ~bytes~.
#ifndef UVM_PACKER_MAX_BYTES
#define UVM_PACKER_MAX_BYTES (UVM_MAX_STREAMBITS / 8)
#endif

constexpr int UVM_STREAMBITS = UVM_MAX_STREAMBITS;

// Macro: `UVM_DEFAULT_TIMEOUT
//
// The default timeout for all phases, if not overridden by
// <uvm_root::set_timeout> or <+UVM_TIMEOUT>

//#define UVM_DEFAULT_TIMEOUT 9200s
#define UVM_DEFAULT_TIMEOUT 9200

// Type: uvm_bitstream_t
//
// The bitstream type is used as a argument type for passing integral values
// in such methods as set_int_local, get_int_local, get_config_int, report,
// pack and unpack. 
typedef std::bitset<UVM_STREAMBITS> uvm_bitstream_t;

// Enum: uvm_radix_enum
//
// Specifies the radix to print or record in.
//
// UVM_BIN       - Selects binary (%b) format
// UVM_DEC       - Selects decimal (%d) format
// UVM_UNSIGNED  - Selects unsigned decimal (%u) format
// UVM_OCT       - Selects octal (%o) format
// UVM_HEX       - Selects hexidecimal (%h) format
// UVM_STRING    - Selects string (%s) format
// UVM_TIME      - Selects time (%t) format
// UVM_ENUM      - Selects enumeration value (name) format

// Enumeration for radix formats
enum uvm_radix_enum {
    UVM_BIN       = 0x1000000,
    UVM_DEC       = 0x2000000,
    UVM_UNSIGNED  = 0x3000000,
    UVM_UNFORMAT2 = 0x4000000,
    UVM_UNFORMAT4 = 0x5000000,
    UVM_OCT       = 0x6000000,
    UVM_HEX       = 0x7000000,
    UVM_STRING    = 0x8000000,
    UVM_TIME      = 0x9000000,
    UVM_ENUM      = 0xA000000,
    UVM_REAL      = 0xB000000,
    UVM_REAL_DEC  = 0xC000000,
    UVM_REAL_EXP  = 0xD000000,
    UVM_NORADIX   = 0
};

const uint32_t UVM_RADIX = 0xF000000;

// Function to convert radix to string
inline std::string uvm_radix_to_string(uvm_radix_enum radix) {
    switch (radix) {
        case UVM_BIN: return "b";
        case UVM_DEC: return "d";
        case UVM_UNSIGNED: return "u";
        case UVM_OCT: return "o";
        case UVM_HEX: return "h";
        case UVM_STRING: return "s";
        case UVM_TIME: return "t";
        case UVM_ENUM: return "s";
        case UVM_REAL: return "g";
        case UVM_REAL_DEC: return "f";
        case UVM_REAL_EXP: return "e";
        default: return "x";
    }
}

inline std::string uvm_radix_name(uvm_radix_enum radix) {
    switch (radix) {
        case UVM_BIN: return "UVM_BIN";
        case UVM_DEC: return "UVM_DEC";
        case UVM_UNSIGNED: return "UVM_UNSIGNED";
        case UVM_OCT: return "UVM_OCT";
        case UVM_HEX: return "UVM_HEX";
        case UVM_STRING: return "UVM_STRING";
        case UVM_TIME: return "UVM_TIME";
        case UVM_ENUM: return "UVM_ENUM";
        case UVM_REAL: return "UVM_REAL";
        case UVM_REAL_DEC: return "UVM_REAL_DEC";
        case UVM_REAL_EXP: return "UVM_REAL_EXP";
        default: return "UVM_NORADIX";
    }
}

// Enum: uvm_recursion_policy_enum
//
// Specifies the policy for copying objects.
//
// UVM_DEEP      - Objects are deep copied (object must implement copy method)
// UVM_SHALLOW   - Objects are shallow copied using default SV copy.
// UVM_REFERENCE - Only object handles are copied.

enum uvm_recursion_policy_enum {
    UVM_DEFAULT_POLICY = 0,
    UVM_DEEP = 0x400,
    UVM_SHALLOW = 0x800,
    UVM_REFERENCE = 0x1000
};

// Enum: uvm_active_passive_enum
//
// Convenience value to define whether a component, usually an agent,
// is in "active" mode or "passive" mode.

enum uvm_active_passive_enum {
    UVM_PASSIVE = 0,
    UVM_ACTIVE = 1
};

// Parameter: `uvm_field_* macro flags
//
// Defines what operations a given field should be involved in.
// Bitwise OR all that apply.
//
// UVM_DEFAULT   - All field operations turned on
// UVM_COPY      - Field will participate in <uvm_object::copy>
// UVM_COMPARE   - Field will participate in <uvm_object::compare>
// UVM_PRINT     - Field will participate in <uvm_object::print>
// UVM_RECORD    - Field will participate in <uvm_object::record>
// UVM_PACK      - Field will participate in <uvm_object::pack>
//
// UVM_NOCOPY    - Field will not participate in <uvm_object::copy>
// UVM_NOCOMPARE - Field will not participate in <uvm_object::compare>
// UVM_NOPRINT   - Field will not participate in <uvm_object::print>
// UVM_NORECORD  - Field will not participate in <uvm_object::record>
// UVM_NOPACK    - Field will not participate in <uvm_object::pack>
//
// UVM_DEEP      - Object field will be deep copied
// UVM_SHALLOW   - Object field will be shallow copied
// UVM_REFERENCE - Object field will copied by reference
//
// UVM_READONLY  - Object field will NOT be automatically configured.

// Flags for field operations
constexpr int UVM_MACRO_NUMFLAGS = 17;

//A=ABSTRACT Y=PHYSICAL
//F=REFERENCE, S=SHALLOW, D=DEEP
//K=PACK, R=RECORD, P=PRINT, M=COMPARE, C=COPY
//--------------------------- AYFSD K R P M C
constexpr int UVM_DEFAULT = 0b000010101010101;
constexpr int UVM_ALL_ON = 0b000000101010101;
constexpr int UVM_FLAGS_ON = 0b000000101010101;
constexpr int UVM_FLAGS_OFF = 0;

//Values are or'ed into a 32 bit value
//and externally
constexpr int UVM_COPY = (1 << 0);
constexpr int UVM_NOCOPY = (1 << 1);
constexpr int UVM_COMPARE = (1 << 2);
constexpr int UVM_NOCOMPARE = (1 << 3);
constexpr int UVM_PRINT = (1 << 4);
constexpr int UVM_NOPRINT = (1 << 5);
constexpr int UVM_RECORD = (1 << 6);
constexpr int UVM_NORECORD = (1 << 7);
constexpr int UVM_PACK = (1 << 8);
constexpr int UVM_NOPACK = (1 << 9);
//constexpr int UVM_DEEP         = (1<<10);
//constexpr int UVM_SHALLOW      = (1<<11);
//constexpr int UVM_REFERENCE    = (1<<12);
constexpr int UVM_PHYSICAL = (1 << 13);
constexpr int UVM_ABSTRACT = (1 << 14);
constexpr int UVM_READONLY = (1 << 15);
constexpr int UVM_NODEFPRINT = (1 << 16);

//Extra values that are used for extra methods
constexpr int UVM_MACRO_EXTRAS = (1 << UVM_MACRO_NUMFLAGS);
constexpr int UVM_FLAGS = UVM_MACRO_EXTRAS + 1;
constexpr int UVM_UNPACK = UVM_MACRO_EXTRAS + 2;
constexpr int UVM_CHECK_FIELDS = UVM_MACRO_EXTRAS + 3;
constexpr int UVM_END_DATA_EXTRA = UVM_MACRO_EXTRAS + 4;

//Get and set methods (in uvm_object). Used by the set/get* functions
//to tell the object what operation to perform on the fields.
constexpr int UVM_START_FUNCS = UVM_END_DATA_EXTRA + 1;
constexpr int UVM_SET = UVM_START_FUNCS + 1;
constexpr int UVM_SETINT = UVM_SET;
constexpr int UVM_SETOBJ = UVM_START_FUNCS + 2;
constexpr int UVM_SETSTR = UVM_START_FUNCS + 3;
constexpr int UVM_END_FUNCS = UVM_SETSTR;

//Global string variables
const std::string uvm_aa_string_key = "";

//-----------------
// Group: Reporting
//-----------------

// Enum: uvm_severity
//
// Defines all possible values for report severity.
//
//   UVM_INFO    - Informative messsage.
//   UVM_WARNING - Indicates a potential problem.
//   UVM_ERROR   - Indicates a real problem. Simulation continues subject
//                 to the configured message action.
//   UVM_FATAL   - Indicates a problem from which simulation can not
//       
// Define severity levels

enum uvm_severity {
    UVM_INFO,
    UVM_WARNING,
    UVM_ERROR,
    UVM_FATAL
};

inline const char* uvm_severity_name(uvm_severity severity) {
    switch (severity) {
        case UVM_INFO: return "UVM_INFO";
        case UVM_WARNING: return "UVM_WARNING";
        case UVM_ERROR: return "UVM_ERROR";
        case UVM_FATAL: return "UVM_FATAL";
        default: return "UNKNOWN_SEVERITY";
    }
}

// Enum: uvm_action
//
// Defines all possible values for report actions. Each report is configured
// to execute one or more actions, determined by the bitwise OR of any or all
// of the following enumeration constants.
//
//   UVM_NO_ACTION - No action is taken
//   UVM_DISPLAY   - Sends the report to the standard output
//   UVM_LOG       - Sends the report to the file(s) for this (severity,id) pair
//   UVM_COUNT     - Counts the number of reports with the COUNT attribute.
//                   When this value reaches max_quit_count, the simulation terminates
//   UVM_EXIT      - Terminates the simulation immediately.
//   UVM_CALL_HOOK - Callback the report hook methods 
//   UVM_STOP      - Causes ~$stop~ to be executed, putting the simulation into
//                   interactive mode.

// Define report actions
enum uvm_action {
    UVM_NO_ACTION = 0,
    UVM_DISPLAY   = 1 << 0,
    UVM_LOG       = 1 << 1,
    UVM_COUNT     = 1 << 2,
    UVM_EXIT      = 1 << 3,
    UVM_CALL_HOOK = 1 << 4,
    UVM_STOP      = 1 << 5
};

// Allow bitwise operations on uvm_action
inline uvm_action operator|(uvm_action lhs, uvm_action rhs) {
    return static_cast<uvm_action>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline uvm_action& operator|=(uvm_action& lhs, uvm_action rhs) {
    lhs = lhs | rhs;
    return lhs;
}

// Define verbosity levels
enum uvm_verbosity {
    UVM_NONE   = 0,
    UVM_LOW    = 100,
    UVM_MEDIUM = 200,
    UVM_HIGH   = 300,
    UVM_FULL   = 400,
    UVM_DEBUG  = 500
};

// Define uvm component mode, sync or async
enum uvm_clock_edge {
    UVM_EDGE_NEG,    // neg edge
    UVM_EDGE_POS     // pos edge
};

// Define uvm component mode, sync or async
enum uvm_clock_mode {
    UVM_CLK_MODE_NEG,    // clock only work at neg edge
    UVM_CLK_MODE_POS,    // clock only work at pos edge
    UVM_CLK_MODE_BOTH    // clock work both at neg and pos edge
};

enum uvm_time_unit { 
    UVM_TU_UNDEF,    // No unit defined
    UVM_TU_1FS,      // 1 femtosecond
    UVM_TU_10FS,     // 10 femtoseconds
    UVM_TU_100FS,    // 100 femtoseconds
    UVM_TU_1PS,      // 1 picosecond
    UVM_TU_10PS,     // 10 picoseconds
    UVM_TU_100PS,    // 100 picoseconds
    UVM_TU_1NS,      // 1 nanosecond
    UVM_TU_10NS,     // 10 nanoseconds
    UVM_TU_100NS,    // 100 nanoseconds
    UVM_TU_1US       // 1000 nanoseconds
};


//----------------------------------------------------------------------------
// Process status enumeration
//----------------------------------------------------------------------------
/**
 * @brief Enumeration for process statuses.
 */
enum uvm_process_status {
    UVM_PS_IDLE = 0,
    UVM_PS_RUNNING
    //UVM_PS_SLEEPING,
    //UVM_PS_HUNG,
    //UVM_PS_KILLED
};

//----------------------------------------------------------------------------
/**
 * @enum uvm_process_type
 * @brief Enumeration for different types of processes.
 */
enum uvm_process_type {
    UVM_PS_SEQUENTIAL,    ///< Processes execute one after another
    UVM_PS_FORK_JOIN     ///< All fork-join processes must complete before proceeding
    //Below two types not support right now.
    //VPI_PS_FORK_JOIN_ANY, ///< Any one fork-join process completing allows proceeding
    //VPI_PS_FORK_JOIN_NONE ///< Fork-join processes execute independently without affecting progression
};

inline
bool operator<(uvm_verbosity lhs, uvm_verbosity rhs) {
    return static_cast<int>(lhs) < static_cast<int>(rhs);
}

inline
bool operator>(uvm_verbosity lhs, uvm_verbosity rhs) {
    return static_cast<int>(lhs) > static_cast<int>(rhs);
}

inline
bool operator<=(uvm_verbosity lhs, uvm_verbosity rhs) {
    return static_cast<int>(lhs) <= static_cast<int>(rhs);
}

inline
bool operator>=(uvm_verbosity lhs, uvm_verbosity rhs) {
    return static_cast<int>(lhs) >= static_cast<int>(rhs);
}

inline
bool operator==(uvm_verbosity lhs, uvm_verbosity rhs) {
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}

inline
bool operator!=(uvm_verbosity lhs, uvm_verbosity rhs) {
    return static_cast<int>(lhs) != static_cast<int>(rhs);
}

using UVM_FILE = int;

// Define port types
enum uvm_port_type_e {
    UVM_PORT,
    UVM_EXPORT,
    UVM_IMPLEMENTATION
};

// Define sequencer arbitration modes
enum uvm_sequencer_arb_mode {
    SEQ_ARB_FIFO,
    SEQ_ARB_WEIGHTED,
    SEQ_ARB_RANDOM,
    SEQ_ARB_STRICT_FIFO,
    SEQ_ARB_STRICT_RANDOM,
    SEQ_ARB_USER
};

using SEQ_ARB_TYPE = uvm_sequencer_arb_mode; // backward compatibility

// Define sequence states
enum uvm_sequence_state {
    CREATED = 1,
    PRE_START = 2,
    PRE_BODY = 4,
    BODY = 8,
    POST_BODY = 16,
    POST_START = 32,
    ENDED = 64,
    STOPPED = 128,
    FINISHED = 256
};

using uvm_sequence_state_enum = uvm_sequence_state; // backward compatibility

// Define sequence library modes
enum uvm_sequence_lib_mode {
    UVM_SEQ_LIB_RAND,
    UVM_SEQ_LIB_RANDC,
    UVM_SEQ_LIB_ITEM,
    UVM_SEQ_LIB_USER
};

// Define phase types
enum uvm_phase_type {
    UVM_PHASE_IMP,
    UVM_PHASE_NODE,
    UVM_PHASE_TERMINAL,
    UVM_PHASE_SCHEDULE,
    UVM_PHASE_DOMAIN,
    UVM_PHASE_GLOBAL
};

inline const char* uvm_phase_type_name(uvm_phase_type phase_type) {
    switch (phase_type) {
        case UVM_PHASE_IMP: return "UVM_PHASE_IMP";
        case UVM_PHASE_NODE: return "UVM_PHASE_NODE";
        case UVM_PHASE_TERMINAL: return "UVM_PHASE_TERMINAL";
        case UVM_PHASE_SCHEDULE: return "UVM_PHASE_SCHEDULE";
        case UVM_PHASE_DOMAIN: return "UVM_PHASE_DOMAIN";
        case UVM_PHASE_GLOBAL: return "UVM_PHASE_GLOBAL";
        default: return "UNKNOWN_PHASE_TYPE";
    }
}

// Define phase states
enum uvm_phase_state {
    UVM_PHASE_DORMANT = 1,
    UVM_PHASE_SCHEDULED = 2,
    UVM_PHASE_SYNCING = 4,
    UVM_PHASE_STARTED = 8,
    UVM_PHASE_EXECUTING = 16,
    UVM_PHASE_READY_TO_END = 32,
    UVM_PHASE_ENDED = 64,
    UVM_PHASE_CLEANUP = 128,
    UVM_PHASE_DONE = 256,
    UVM_PHASE_JUMPING = 512
};

inline const char* uvm_phase_state_name(uvm_phase_state state) {
    switch (state) {
        case UVM_PHASE_DORMANT: return "UVM_PHASE_DORMANT";
        case UVM_PHASE_SCHEDULED: return "UVM_PHASE_SCHEDULED";
        case UVM_PHASE_SYNCING: return "UVM_PHASE_SYNCING";
        case UVM_PHASE_STARTED: return "UVM_PHASE_STARTED";
        case UVM_PHASE_EXECUTING: return "UVM_PHASE_EXECUTING";
        case UVM_PHASE_READY_TO_END: return "UVM_PHASE_READY_TO_END";
        case UVM_PHASE_ENDED: return "UVM_PHASE_ENDED";
        case UVM_PHASE_CLEANUP: return "UVM_PHASE_CLEANUP";
        case UVM_PHASE_DONE: return "UVM_PHASE_DONE";
        case UVM_PHASE_JUMPING: return "UVM_PHASE_JUMPING";
        default: return "UNKNOWN_PHASE_STATE";
    }
}

// Define phase transitions
enum uvm_phase_transition {
    UVM_COMPLETED = 0x01,
    UVM_FORCED_STOP = 0x02,
    UVM_SKIPPED = 0x04,
    UVM_RERUN = 0x08
};

// Define wait operations
enum uvm_wait_op {
    UVM_LT,
    UVM_LTE,
    UVM_NE,
    UVM_EQ,
    UVM_GT,
    UVM_GTE
};

// Define objection events
enum uvm_objection_event {
    UVM_RAISED = 0x01,
    UVM_DROPPED = 0x02,
    UVM_ALL_DROPPED = 0x04
};

#endif // UVM_TYPES_H