#include "eEVM/opcode.h"
#include "eEVM/processor.h"
#include "eEVM/simple/simpleglobalstate.h"

#include <cassert>
#include <fmt/format_header_only.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <vector>
#include <map>

#include "../../../enclave/include/myAes256.hpp"



/* uint256_t -> unsigned char* */
unsigned char* __uint2562charx(uint256_t n) {
  unsigned char* res = new unsigned char[100];
  memset(res, 0, sizeof(unsigned char) * 100);
  if (n == 0) {
    res[0] = '0';
    return res;
  }
  std::vector<unsigned char> tmp;
  while (n != 0) {
    tmp.push_back(static_cast<unsigned char>(n % 10 + '0'));
    n /= 10;
  }
  for (int i = tmp.size() - 1, j = 0; i >= 0; i--, j++) {
    res[j] = tmp[i];
  }
  return res;
}


/* char* -> uint256_t */
uint256_t __charx2uint256(unsigned char* n) {
  uint256_t res = 0;
  int end = 0;
  while (*(n + end) != 0) {
    res *= 10;
    res += static_cast<uint8_t>(*(n + end) - '0');
    end += 1;
  }
  return res;
}


/* char* -> uint64_t */
uint64_t __charx2uint64(unsigned char* n) {
  uint64_t res = 0;
  int end = 0;
  while (*(n + end) != 0) {
    res *= 10;
    res += static_cast<uint8_t>(*(n + end) - '0');
    end += 1;
  }
  return res;
}


/* uint64_t -> char* */
unsigned char* __uint642charx(uint64_t n) {
  unsigned char* res = new unsigned char[100];
  memset(res, 0, sizeof(unsigned char) * 100);
  if (n == 0) {
    res[0] = '0';
    return res;
  }
  std::vector<unsigned char> tmp;
  while (n != 0) {
    tmp.push_back(static_cast<unsigned char>(n % 10 + '0'));
    n /= 10;
  }
  for (int i = tmp.size() - 1, j = 0; i >= 0; i--, j++) {
    res[j] = tmp[i];
  }
  return res;
}


/* vector<uint8_t> -> uint8_t* */
uint8_t* __vector2arr(std::vector<uint8_t> code) {
  uint8_t* code_arr = new uint8_t[code.size()];
  memset(code_arr, 0, sizeof(uint8_t) * code.size());
  if (code.size() > 0) {
    memcpy(code_arr, &code[0], sizeof(uint8_t) * code.size());
  }
  return code_arr;
}


/* uint8_t* -> vector<uint8_t> */
std::vector<uint8_t> __arr2vector(uint8_t* code_arr, size_t code_size) {
  std::vector<uint8_t> vec(code_arr, code_arr + code_size);
  return vec;
}


unsigned char* __enc_data_load(unsigned char* s) {
  std::vector<unsigned char> res;
  int i = 0;
  while (s[i] != 0) {
    unsigned char n = (s[i] - '0') * 100 + (s[i + 1] - '0') * 10 + (s[i + 2] - '0');
    res.push_back(std::move(n));
    i += 3;
  }
  unsigned char* _res = new unsigned char[200];
  memset(_res, 0, sizeof(unsigned char) * 200);
  memcpy(_res, &res[0], sizeof(unsigned char) * res.size());
  return _res;
}



std::vector<uint8_t> run_and_check_result(
  const eevm::Address& from,
  const eevm::Address& to,
  const eevm::Code& input,
  const uint256_t& call_value) {

  // processor
  eevm::Processor p;

  // run
  exec_result_struct exec_result;
  try {
	p.init(from);
	exec_result = p.run(from, to, input, call_value);
	if(exec_result.er != 3) p.save();
  }
  catch (std::runtime_error& e) {
	std::cerr << e.what() << std::endl;
  }
  catch (eevm::Exception& e) {
	std::cerr << e.what() << std::endl;
  }
  catch (const char* c) {
    std::cerr << c << std::endl;
  }

  if (exec_result.er != 0) {
    if (exec_result.er == 2) {
      // Rethrow to highlight any exceptions raised in execution
      throw std::runtime_error(fmt::format("Execution threw an error: {}", exec_result.exmsg));
    }
  }

  return __arr2vector(exec_result.output, __charx2uint64(exec_result.output_size));
}


void create_account(
  const eevm::Address& account_address, 
  const uint256_t& account_balance, 
  const eevm::Code& account_code,
  const size_t& account_nonce) {

  // processor
  eevm::Processor p;

  // create
  try {
    p.init(account_address);
    p.create_account(account_address, account_balance, account_code, account_nonce);
    p.save();
  }
  catch (const char* c) {
    // std::cerr << c << std::endl;
  }
  
}


eevm::Address deploy_contract(
  const std::string& contract_bin,
  const eevm::Address& sender,
  const size_t& nonce) {

  // create
  create_account(sender, 0, {}, nonce);

  // binary constructor
  auto contract_constructor = eevm::to_bytes(contract_bin.substr(2, contract_bin.size() - 2));  // 转换成10进制

  // processor
  eevm::Processor p;

  // deploy
  eevm::Address contract_address = 0;
  try {
    p.init(sender);
    contract_address = p.deploy(sender, contract_constructor);
    p.save();
  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }
  catch (eevm::Exception& e) {
    std::cerr << e.what() << std::endl;
  }
  catch (const char* c) {
    std::cerr << c << std::endl;
  }
  
  return contract_address;
}

void call_function(
  const eevm::Address& contract_address,
  const std::string& contract_bin,
  const eevm::Address& sender) {

  // create
  create_account(sender, 0, {}, 0);

  auto function_call = eevm::to_bytes(contract_bin.substr(2, contract_bin.size() - 2));

  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);

  // return eevm::from_big_endian(output.data(), output.size());

  //if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
  //{
  //  throw std::runtime_error("Unexpected output from call to transfer");
  //}

  //const bool success = output[31] == 1;

  //return success;
}


int main(int argc, char** argv) {

  srand(time(nullptr));


  // ERC20, 8 3
  // Vote, 74, 13
  // OpenAuction, 24 4
  // ERC20, Change Your Life (AAA), 0xD938137E6d96c72E4a6085412aDa2daD78ff89c4, 20, 40
  // ERC20, Uquid Coin (UQC), 0xD01DB73E047855Efb414e6202098C4Be4Cd2423B, 10, 20
  // ERC721, LORDLESS:Bounty (Bounty), 0xB9250c9581e4594b7c6914897823aD18D6B78e96, 12, 26
  // ERC721, gego.dego, 0x27b4bC90fBE56f02Ef50f2E2f79D7813Aa8941A7, 12, 11
  // DAPP Game, WhiteBetting, 0x07859fEc7b937c695F1f807255CfC4A13c7b7138, 11, 30
  // DAPP Game, CocosToken, 0x0C6f5F7D555E7518f6841a79436BD2b1Eef03381, 12, 16
  // DAPP Finance, RiverMount, 0x5AB55ec290BeacAE98f54c3eB70860460B167C3C, 13, 23
  // DAPP Finance, ChangeNOW, 0xE9A95d175a5f4C9369f3b74222402eB1b837693b, 11, 22
  // DAPP Marketplace, Multicoin, 0xF6117cC92d7247F605F11d4c942F0feda3399CB5, 10, 20
  // DAPP Marketplace, PIXBYTOKEN, 0xe8Bad244f2a9881d3D7e826C5Ec23E5391f3166C, 10, 17
  // DAPP Platform, HUNTToken, 0x9AAb071B4129B083B01cB5A0Cb513Ce7ecA26fa5, 10, 20
  // DAPP Platform, BSYToken, 0x7A647637E75d34A7798Ba06BD1aDF5c084bb5dd5, 10, 23
  // DAPP Service, IrbisNetwork, 0x94534cAEbD1887BE3D57179728A670a3436bb443, 10, 20
  // DAPP Service, P2PGlobalNetwork, 0xcecede5A20645EAc6ca2032eeEb1063572D63c29, 10, 20
  // DAPP Other, CloudbitToken, 0xFd3305E1c7cB5D269fb6CeD8eB8240255a50E7a4, 10, 20
  // DAPP Other, ORYX, 0x5C0Bc243Fb13632c4D247F4f0bC27f2f58982C39, 10, 20
  std::string name = "ORYX";
  std::string tx_path = "../../../example/" + name + "_tx.csv";
  std::string storage_path = "../../../example/" + name + "_storage.csv";

  size_t deployer_nonce = -1;
  eevm::Address deployer_address = 0;
  std::string contract_bin = "";
  std::vector<std::string> transactions;
  std::map<std::string, eevm::Address> senders;
  std::map<std::string, std::string> bins;
  std::map<std::string, std::map<std::string, std::string>> storages;


  // transactions
  std::ifstream fp(tx_path);
  if (fp.fail()) { std::cout << "tx.csv not found" << std::endl; }
  std::string line, data, data2, data3;
  while (getline(fp, line)) {
    std::istringstream readstr(line);

    if (deployer_nonce == -1) {
      getline(readstr, data, ',');
      getline(readstr, data2, ',');
      getline(readstr, data3, ',');
      for (int i = 0; i < data2.size(); i++) {
        deployer_nonce *= 10;
        deployer_nonce += data2[i] - '0';
      }
    }
    else if (deployer_address == 0) {
      getline(readstr, data, ',');
      getline(readstr, data2, ',');
      getline(readstr, data3, ',');
      deployer_address = eevm::to_uint256(data2);
      contract_bin = data3;
    }
    else {
      getline(readstr, data, ',');
      getline(readstr, data2, ',');
      getline(readstr, data3, ',');
      transactions.push_back(data);
      senders[data] = eevm::to_uint256(data2);
      bins[data] = data3;
    }
  }


  // storage
  std::ifstream fp2(storage_path);
  if (fp2.fail()) { std::cout << "storage.csv not found" << std::endl; }
  while (getline(fp2, line)) {
    std::istringstream readstr(line);

    getline(readstr, data, ',');
    getline(readstr, data2, ',');
    getline(readstr, data3, ',');
    for (int i = 2; i < data2.size(); i++) {
      if (data2[i] != '0') {
        data2 = "0x" + data2.substr(i, data2.size() - i);
        break;
      }
      if (i == data2.size() - 1) {
        data2 = "0x0";
      }
    }
    for (int i = 2; i < data3.size(); i++) {
      if (data3[i] != '0') {
        data3 = "0x" + data3.substr(i, data3.size() - i);
        break;
      }
      if (i == data3.size() - 1) {
        data3 = "0x0";
      }
    }
    storages[data][data2] = data3;
  }



  int st_size = 0;
  eevm::Address contract_address = deploy_contract(contract_bin, deployer_address, deployer_nonce);
  for (int tt = 0; tt < transactions.size(); tt++) {
    std::cout << "transaction: " << tt << " " << transactions[tt] << std::endl;
    call_function(contract_address, bins[transactions[tt]], senders[transactions[tt]]);

    std::ifstream gs_json_file("globalstate.json");
    if (gs_json_file) {
      nlohmann::json gs_json_tmp;
      gs_json_file >> gs_json_tmp;

      for (int i = 0; i < gs_json_tmp["accounts"].size(); i++) {
        std::string __address_str = gs_json_tmp["accounts"][i]["address"].get<std::string>();
        eevm::Address __address = __charx2uint256((unsigned char*)__address_str.c_str());

        if (__address == contract_address) {
          std::map<std::string, std::string> evm_storage;

          int j = 0;
          for (auto& it : gs_json_tmp["accounts"][i]["storage"].items()) {
            std::string _key_str = gs_json_tmp["accounts"][i]["storage"][j]["key"].get<std::string>();
            std::string _value_str = gs_json_tmp["accounts"][i]["storage"][j]["value"].get<std::string>();
            unsigned char* _key = __enc_data_load((unsigned char*)_key_str.c_str());
            unsigned char* _value = __enc_data_load((unsigned char*)_value_str.c_str());
            j += 1;

            unsigned char* dec_key = new unsigned char[200];
            memset(dec_key, 0, sizeof(unsigned char) * 200);
            unsigned char* dec_value = new unsigned char[200];
            memset(dec_value, 0, sizeof(unsigned char) * 200);
            aes256_dec(_key, dec_key);
            aes256_dec(_value, dec_value);

            evm_storage[eevm::to_hex_string(__charx2uint256(dec_key))] = eevm::to_hex_string(__charx2uint256(dec_value));
          }

          for (std::map<std::string, std::string>::iterator it = storages[transactions[tt]].begin(); it != storages[transactions[tt]].end(); it++) {
            std::cout << "  key: " << it->first << std::endl;
            st_size += 1;
            std::string evm_str = evm_storage.find(eevm::to_hex_string(eevm::to_uint256(it->first))) != evm_storage.end()? evm_storage[eevm::to_hex_string(eevm::to_uint256(it->first))]: "0x0";
            if (it->second != evm_str) {
              std::cout << "    etherscan: " << it->second << std::endl;
              std::cout << "    evm:       " << evm_str << std::endl << std::endl;
            }
          }

          break;
        }
      }
    }
  }


  std::cout << "Done, check " << transactions.size() << " transactions." << std::endl;
  std::cout << "Done, check " << st_size << " storages." << std::endl;


  return 0;
}