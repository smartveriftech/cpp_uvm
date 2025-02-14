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

#ifndef _UVM_BITMEMORY_H_
#define _UVM_BITMEMORY_H_

#include <stdio.h>
#include <vector>

#include "base/uvm_misc.h"
#include "base/uvm_bitstream.h"

class uvm_bitmemory {

  uvm_bitstream ** pacbv;
  std::string psMemoryName = "uvm_bitmemory";

  u_int32_t lWidth;
  u_int32_t lDepth;
  u_int32_t lBitCnt;

  public:

  uvm_bitmemory(u_int32_t lSetWidth, u_int32_t lSetDepth, const char * psName = "undef",int iUseName=0, const char *a =0);
  uvm_bitmemory(const std::vector <uvm_bitstream> & pkt, const char * psName = "undef",int iUseName=0);
  void init(u_int32_t lSetWidth, u_int32_t lSetDepth, const char * psName = "undef",int iUseName=0, const char *a=0);

  uvm_bitmemory();

  ~uvm_bitmemory();

  void resize(u_int32_t newWidth);

  u_int32_t getWidth() const;

  u_int32_t getDepth() const;

  u_int32_t get_bitCnt() const;

  void set_bitCnt(u_int32_t newBitCnt);

  uvm_bitmemory(const uvm_bitmemory & bm);

  uvm_bitstream & operator[] (u_int32_t lAddr) const;

  uvm_bitstream & operator[] (uvm_bitstream cbv) const;

  uvm_bitmemory & operator = (const uvm_bitmemory & bm);

  bool operator == (const uvm_bitmemory & cbm) const;
  void copy(unsigned char *, u_int32_t len) const;
  void diffBM(const uvm_bitmemory & cmpuvm_transactionBM);

 };

extern std::ostream & operator << (std::ostream &, const uvm_bitmemory &);

#endif  //_UVM_UVM_UVM_BITMEMORY_H_
