//#include "eEVM/opcode.h"
//#include "eEVM/processor.h"
//#include "eEVM/simple/simpleglobalstate.h"
//
//#include <cassert>
//#include <fmt/format_header_only.h>
//#include <fstream>
//#include <iostream>
//#include <nlohmann/json.hpp>
//#include <random>
//#include <sstream>
//#include <vector>
//
//#include "../../../enclave/ttt.h "
//
//
//
///* uint256_t -> unsigned char* */
//unsigned char* __uint2562charx(uint256_t n) {
//  unsigned char* res = new unsigned char[100];
//  memset(res, 0, sizeof(unsigned char) * 100);
//  if (n == 0) {
//    res[0] = '0';
//    return res;
//  }
//  std::vector<unsigned char> tmp;
//  while (n != 0) {
//    tmp.push_back(static_cast<unsigned char>(n % 10 + '0'));
//    n /= 10;
//  }
//  for (int i = tmp.size() - 1, j = 0; i >= 0; i--, j++) {
//    res[j] = tmp[i];
//  }
//  return res;
//}
//
///* char* -> uint256_t */
//uint256_t __charx2uint256(unsigned char* n) {
//  uint256_t res = 0;
//  int end = 0;
//  while (*(n + end) != 0) {
//    res *= 10;
//    res += static_cast<uint8_t>(*(n + end) - '0');
//    end += 1;
//  }
//  return res;
//}
//
///* char* -> uint64_t */
//uint64_t __charx2uint64(unsigned char* n) {
//  uint64_t res = 0;
//  int end = 0;
//  while (*(n + end) != 0) {
//    res *= 10;
//    res += static_cast<uint8_t>(*(n + end) - '0');
//    end += 1;
//  }
//  return res;
//}
//
///* uint64_t -> char* */
//unsigned char* __uint642charx(uint64_t n) {
//  unsigned char* res = new unsigned char[100];
//  memset(res, 0, sizeof(unsigned char) * 100);
//  if (n == 0) {
//    res[0] = '0';
//    return res;
//  }
//  std::vector<unsigned char> tmp;
//  while (n != 0) {
//    tmp.push_back(static_cast<unsigned char>(n % 10 + '0'));
//    n /= 10;
//  }
//  for (int i = tmp.size() - 1, j = 0; i >= 0; i--, j++) {
//    res[j] = tmp[i];
//  }
//  return res;
//}
//
///* vector<uint8_t> -> uint8_t* */
//uint8_t* __vector2arr(std::vector<uint8_t> code) {
//  uint8_t* code_arr = new uint8_t[code.size()];
//  memset(code_arr, 0, sizeof(uint8_t) * code.size());
//  if (code.size() > 0) {
//    memcpy(code_arr, &code[0], sizeof(uint8_t) * code.size());
//  }
//  return code_arr;
//}
//
///* uint8_t* -> vector<uint8_t> */
//std::vector<uint8_t> __arr2vector(uint8_t* code_arr, size_t code_size) {
//  std::vector<uint8_t> vec(code_arr, code_arr + code_size);
//  return vec;
//}
//
//
//
//// Modify code to append ABI-encoding of arg, suitable for passing to contract
//// execution
//void append_argument(std::vector<uint8_t>& code, const uint256_t& arg)
//{
//  // To ABI encode a function call with a uint256_t (or Address) argument,
//  // simply append the big-endian byte representation to the code (function
//  // selector, or bin). ABI-encoding for more complicated types is more
//  // complicated, so not shown in this sample.
//  const auto pre_size = code.size();
//  code.resize(pre_size + 32u);
//  eevm::to_big_endian(arg, code.data() + pre_size);
//}
//
//// solidity store every parameter in 256-bit (32)
//void append_argument2(std::vector<uint8_t>& code, const std::string& arg){
//  // const auto pre_size = code.size();
//  code.resize(code.size() + 31u);
//  code.push_back(static_cast<uint8_t>(64));
//  code.resize(code.size() + 31u);
//  code.push_back(static_cast<uint8_t>(3));
//  for (auto i=0; i< arg.size(); i++){
//    code.push_back(static_cast<uint8_t>(arg[i]));
//  }
//  code.resize(code.size() + 32u - arg.size());
//}
//
//// solidity store every parameter in 256-bit (32)
//void append_argument3(std::vector<uint8_t>& code, const std::string& arg){
//  // const auto pre_size = code.size();
//  code.resize(code.size() + 31u);
//  code.push_back(static_cast<uint8_t>(32));
//  code.resize(code.size() + 31u);
//  code.push_back(static_cast<uint8_t>(6));
//  for (auto i=0; i< arg.size(); i++){
//    code.push_back(static_cast<uint8_t>(arg[i]));
//  }
//  code.resize(code.size() + 32u - arg.size());
//}
//
//uint256_t get_random_address(size_t bytes)
//{
//  std::vector<uint8_t> raw(bytes);
//  std::generate(raw.begin(), raw.end(), []() { return rand(); });
//  return eevm::from_big_endian(raw.data(), raw.size());
//}
//
//void create_account(const eevm::Address& account_address, const uint256_t& account_balance, const eevm::Code& account_code) {
//  // processor
//  eevm::Processor p;
//
//  // create
//  p.init(account_address);
//  p.create_account(account_address, account_balance, account_code);
//  p.save();
//}
//
//// Run input as an EVM transaction, check the result and return the output
//std::vector<uint8_t> run_and_check_result(const eevm::Address& from, const eevm::Address& to, const eevm::Code& input){
//
//  // processor
//  eevm::Processor p;
//
//  // run
//  p.init(from);
//  const auto exec_result = p.run(from, to, input, 0u);
//  p.save();
//
//  if (exec_result.er != 0){
//    if (exec_result.er == 1){
//      // Rethrow to highlight any exceptions raised in execution
//      throw std::runtime_error(fmt::format("Execution threw an error: {}", exec_result.exmsg));
//    }
//    throw std::runtime_error("Deployment did not return");
//  }
//
//  return __arr2vector(exec_result.output, __charx2uint64(exec_result.output_size));
//}
//
//eevm::Address deploy_contract(const eevm::Address& sender, const nlohmann::json& contract_definition, const uint256_t& _name, const std::string& _name2) {
//
//  // binary constructor
//  auto contract_constructor = eevm::to_bytes(contract_definition["bin"]);
//
//  // The constructor takes a single argument - append it
//  append_argument(contract_constructor, _name);
//  append_argument2(contract_constructor, _name2);
//
//  // processor
//  eevm::Processor p;
//
//  // deploy
//  p.init(sender);
//  eevm::Address contract_address = p.deploy(sender, contract_constructor);
//  p.save();
//
//  return contract_address;
//}
//
//bool hello_set_name(const eevm::Address& sender, const eevm::Address& contract_address, const nlohmann::json& contract_definition, const uint256_t& _name){
//
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["setName(uint256)"]);
//
//  append_argument(function_call, _name);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  // Output should be a bool in a 32-byte vector.
//  if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
//  {
//    throw std::runtime_error("Unexpected output from call to transfer");
//  }
//  const bool success = output[31] == 1;
//
//  return success;
//}
//
//bool hello_set_name2(const eevm::Address& sender, const eevm::Address& contract_address, const nlohmann::json& contract_definition, const std::string& _name){
//
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["setName2(string)"]);
//
//  append_argument3(function_call, _name);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  // Output should be a bool in a 32-byte vector.
//  if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
//  {
//    throw std::runtime_error("Unexpected output from call to transfer");
//  }
//  const bool success = output[31] == 1;
//
//  return success;
//}
//
//uint256_t hello_get_name(const eevm::Address& sender, const eevm::Address& contract_address, const nlohmann::json& contract_definition){
//  
//  const auto function_call = eevm::to_bytes(contract_definition["hashes"]["getName()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//std::string hello_get_name2(const eevm::Address& sender, const eevm::Address& contract_address, const nlohmann::json& contract_definition){
//  
//  const auto function_call = eevm::to_bytes(contract_definition["hashes"]["getName2()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  std::string result = "";
//  for(auto i=output.size()-32; i<output.size(); i++){
//    if((int)output[i] == 0){
//        break;
//      }
//    result.append(1, static_cast<char>(output[i]));
//  }
//
//  return result;
//}
//
//
//
//int main(int argc, char** argv) {
//
//  srand(time(nullptr));
//
//  // contract
//  // solc --evm-version homestead --combined-json bin,hashes --pretty-json --optimize ERC20.sol > ERC20_combined.json
//  const std::string contract_path = "e:/Computer/workspace/evm_with_oe/evm/samples/m_contract/hello_combined.json";
//  std::ifstream contract_fstream(contract_path);
//  if (!contract_fstream) {
//    throw std::runtime_error(
//      fmt::format("Unable to open contract definition file {}", contract_path));
//  }
//  std::stringstream contract_buffer;
//  contract_buffer << contract_fstream.rdbuf();
//  const auto contracts_json = nlohmann::json::parse(contract_buffer.str());
//  const auto all_contracts = contracts_json["contracts"];
//  const auto hello_definition = all_contracts["hello.sol:Hello"];
//
//  // sender address 
//  //const eevm::Address sender = get_random_address(20);
//  const eevm::Address sender = uint256_t(12345);
//  create_account(sender, uint256_t(0), {});
//
//  // deploy contract
//  auto hello_address = deploy_contract(sender, hello_definition, (uint256_t)100, "zhq");
//  std::cout << "hello contract address: " << eevm::to_checksum_address(hello_address) << std::endl;
//
//  // get name(contract function)
//  auto name = hello_get_name(sender, hello_address, hello_definition);
//  auto name2 = hello_get_name2(sender, hello_address, hello_definition);
//  std::cout << "hello contract get_name(): " << name <<std::endl;
//  std::cout << "hello contract get_name2(): " << name2 <<std::endl;
//
//  // set name(contract function)
//  auto success = hello_set_name(sender, hello_address, hello_definition, (uint256_t)200);
//  auto success2 = hello_set_name2(sender, hello_address, hello_definition, "zhqzhq");
//  std::cout << "hello contract set_name(): " << (success ? "succeeded" : "failed") <<std::endl;
//  std::cout << "hello contract set_name2(): " << (success2 ? "succeeded" : "failed") <<std::endl;
//
//  // get name(contract function)
//  auto rename = hello_get_name(sender, hello_address, hello_definition);
//  auto rename2 = hello_get_name2(sender, hello_address, hello_definition);
//  std::cout << "hello contract get_name(): " << rename <<std::endl;
//  std::cout << "hello contract get_name2(): " << rename2 <<std::endl;
//
//  system("pause");
//  return 0;
//}
