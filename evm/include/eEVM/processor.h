#pragma once

#include "opcode.h"
#include "account.h"
#include "globalstate.h"
#include "trace.h"
#include "transaction.h"
#include "simple/simpleglobalstate.h"
#include "simple/simpleaccount.h"
#include "simple/simplestorage.h"
#include "bigint.h"
#include "util.h"

#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <nlohmann/json.hpp>
#include<ctime>

#include "../../../enclave/ttt.h"



namespace eevm
{
  /**
   * Ethereum bytecode processor.
   */
  class Processor
  {
  private:
    gs_struct gs;
    tx_struct tx;

  public:
	Processor();

    Processor(gs_struct& gs, tx_struct& tx);

	void init(Address sender);

	void save();

	exec_result_struct run(
	  const Address& caller,
	  const Address& callee,
	  const std::vector<uint8_t>& input,
	  const uint256_t& call_value);

	Address deploy(
	  const Address& _caller_address,
	  const Code& _constructor);

	void create_account(
	  const Address& account_address,
	  const uint256_t& account_balance,
	  const Code& account_code,
	  const size_t& account_nonce);

    void printf_gs(bool code=false, bool storage=false);
  };

} // namespace eevm

/* uint256_t -> unsigned char* */
unsigned char* uint2562charx(uint256_t n);

/* unsigned char* -> uint256_t */
uint256_t charx2uint256(unsigned char* n);

/* uint64_t -> unsigned char* */
unsigned char* uint642charx(uint64_t n);

/* unsigned char* -> uint64_t */
uint64_t charx2uint64(unsigned char* n);

/* vector<uint8_t> -> uint8_t* */
uint8_t* vector2arr(std::vector<uint8_t> code);

/* uint8_t* -> vector<uint8_t> */
std::vector<uint8_t> arr2vector(uint8_t* code_arr, size_t code_size);

/* uint8_t* -> char*/
unsigned char* arr2charx(uint8_t* code, size_t code_size);

/* char* -> uint8_t* */
uint8_t* charx2arr(unsigned char* s);

unsigned char* enc_data_store(unsigned char* s);

unsigned char* enc_data_load(unsigned char* s);