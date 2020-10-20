//#include "eEVM/processor.h"
//
//
//
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
//uint256_t get_random_address(size_t bytes)
//{
//  std::vector<uint8_t> raw(bytes);
//  std::generate(raw.begin(), raw.end(), []() { return rand(); });
//  return eevm::from_big_endian(raw.data(), raw.size());
//}
//
//std::vector<uint8_t> run_and_check_result(const eevm::Address& from, const eevm::Address& to, const eevm::Code& input, const uint256_t& call_value) {
//
//  // processor
//  eevm::Processor p;
//
//  // run
//  long start_time = clock();
//  p.init(from);
//  const auto exec_result = p.run(from, to, input, call_value);
//  p.save();
//  long end_time = clock();
//  if (CAL_EEVM_TIME) printf("eevm time: %f\n", ((double)end_time - start_time) / 1000);
//
//  if (exec_result.er != 0) {
//    if (exec_result.er == 1) {
//      // Rethrow to highlight any exceptions raised in execution
//      throw std::runtime_error(fmt::format("Execution threw an error: {}", exec_result.exmsg));
//    }
//    throw std::runtime_error("Deployment did not return");
//  }
//
//  return arr2vector(exec_result.output, charx2uint64(exec_result.output_size));
//}
//
//eevm::Address deploy_contract(
//  const nlohmann::json& contract_definition, 
//  const eevm::Address& sender, 
//  const eevm::Address beneficiary) {
//
//  // binary constructor
//  auto contract_constructor = eevm::to_bytes(contract_definition["bin"]);  // 转换成10进制
//
//  // The constructor takes a single argument - append it
//  append_argument(contract_constructor, beneficiary);
//
//  // processor
//  eevm::Processor p;
//
//  // deploy
//  p.init(sender);
//  eevm::Address contract_address = 0;
//  try {
//    contract_address = p.deploy(sender, contract_constructor);
//  }
//  catch (const char* c) {
//    std::cerr << c << std::endl;
//    abort();
//  }
//  p.save();
//
//  return contract_address;
//}
//
//void create_account(const eevm::Address& account_address, const uint256_t& account_balance, const eevm::Code& account_code) {
//  // processor
//  eevm::Processor p;
//
//  // create
//  p.init(account_address);
//  try {
//    p.create_account(account_address, account_balance, account_code, 0);
//  }
//  catch (const char* c) {
//    std::cerr << c << std::endl;
//    abort();
//  }
//  p.save();
//}
//
//bool bid(
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address& sender,
//  const uint256_t& call_value) {
//  
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["bid()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, call_value);
//
//  // Output should be a bool in a 32-byte vector.
//  if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
//  {
//    throw std::runtime_error("Unexpected output from call to transfer");
//  }
//  
//  const bool success = output[31] == 1;
//  
//  return success;
//}
//
//bool withdraw(
//  const nlohmann::json& contract_definition, 
//  const eevm::Address& contract_address,
//  const eevm::Address& sender) {
//
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["withdraw()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);
//
//  // Output should be a bool in a 32-byte vector.
//  if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
//  {
//    throw std::runtime_error("Unexpected output from call to transfer");
//  }
//
//  const bool success = output[31] == 1;
//
//  return success;
//}
//
//bool auctionEnd(
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address& sender) {
//
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["auctionEnd()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);
//
//  // Output should be a bool in a 32-byte vector.
//  if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
//  {
//    throw std::runtime_error("Unexpected output from call to transfer");
//  }
//
//  const bool success = output[31] == 1;
//
//  return success;
//}
//
//uint256_t get_beneficiary(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["beneficiary()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//uint256_t get_highestBidder(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["highestBidder()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//uint256_t get_highestBid(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["highestBid()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//uint256_t get_pendingReturns(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const eevm::Address& _address)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_pendingReturns(address)"]);
//
//  append_argument(function_call, _address);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call, 0u);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//void print_state(
//  const std::string& heading,
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address& beneficiary,
//  const eevm::Address& account_b,
//  const eevm::Address& account_c)
//{
//  //std::cout << heading << std::endl;
//
//  eevm::Address _beneficiary = get_beneficiary(contract_definition, contract_address, beneficiary);
//  //std::cout << "  beneficiary: " << eevm::to_checksum_address(_beneficiary) << std::endl;
//
//  uint256_t _highestBidder = get_highestBidder(contract_definition, contract_address, beneficiary);
//  //std::cout << "  highestBidder: " << eevm::to_checksum_address(_highestBidder) << std::endl;
//
//  uint256_t _highestBid = get_highestBid(contract_definition, contract_address, beneficiary);
//  //std::cout << "  highestBid: " << _highestBid << std::endl;
//
//  uint256_t _b_return = get_pendingReturns(contract_definition, contract_address, beneficiary, account_b);
//  //std::cout << "  b_return: " << _b_return << std::endl;
//
//  uint256_t _c_return = get_pendingReturns(contract_definition, contract_address, beneficiary, account_c);
//  //std::cout << "  c_return: " << _c_return << std::endl;
//
//  //std::cout << "accounts balance: " << std::endl;
//
//  //eevm::Processor p;
//  //p.init(beneficiary);
//  //p.printf_gs();
//  //p.save();
//
//  //std::cout << std::endl;
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
//  const std::string contract_path = "e:/Computer/workspace/evm_with_oe/evm/samples/openauction/openauction_combined.json";
//  std::ifstream contract_fstream(contract_path);
//  if (!contract_fstream) {
//    throw std::runtime_error(
//      fmt::format("Unable to open contract definition file {}", contract_path));
//  }
//  std::stringstream contract_buffer;
//  contract_buffer << contract_fstream.rdbuf();
//  const auto contracts_json = nlohmann::json::parse(contract_buffer.str());
//  const auto all_contracts = contracts_json["contracts"];
//  const auto openauction_definition = all_contracts["openauction.sol:SimpleAuction"];
//
//
//
//  // account beneficiary
//  // const eevm::Address beneficiary = get_random_address(20);
//  const eevm::Address beneficiary = uint256_t(12345);
//  create_account(beneficiary, uint256_t(0), {});
//
//  // account b
//  // const eevm::Address account_b = get_random_address(20);
//  const eevm::Address account_b = uint256_t(67890);
//  create_account(account_b, uint256_t(10000), {});
//
//  // account c
//  // const eevm::Address account_c = get_random_address(20);
//  const eevm::Address account_c = uint256_t(60978);
//  create_account(account_c, uint256_t(10000), {});
//
//  //std::cout << "beneficiary: " << eevm::to_checksum_address(beneficiary) << std::endl;
//  //std::cout << "account_b: " << eevm::to_checksum_address(account_b) << std::endl;
//  //std::cout << "account_c: " << eevm::to_checksum_address(account_c) << std::endl;
//
//
//
//  try {
//    // deploy
//    auto contract_address = deploy_contract(openauction_definition, beneficiary, beneficiary);
//    print_state("-- Initial state --", openauction_definition, contract_address, beneficiary, account_b, account_c);
//
//    // bid
//    auto success_1 = bid(openauction_definition, contract_address, account_b, 1000);  // 钱如何打到智能合约账户上
//    auto success_2 = bid(openauction_definition, contract_address, account_c, 2000);
//    //std::cout << "account_b bid: " << (success_1 ? " succeeded" : " failed") << std::endl;
//    //std::cout << "account_c bid: " << (success_2 ? " succeeded" : " failed") << std::endl;
//    print_state("-- After bid --", openauction_definition, contract_address, beneficiary, account_b, account_c);
//
//    // withdraw
//    auto success_3 = withdraw(openauction_definition, contract_address, account_b);
//    //std::cout << "account_b withdraw: " << (success_3 ? " succeeded" : " failed") << std::endl;
//    print_state("-- After withdraw --", openauction_definition, contract_address, beneficiary, account_b, account_c);
//
//    // auctionEnd
//    auto success_4 = auctionEnd(openauction_definition, contract_address, beneficiary);
//    //std::cout << "beneficiary auctionEnd: " << (success_4 ? " succeeded" : " failed") << std::endl;
//    print_state("-- After auctionEnd --", openauction_definition, contract_address, beneficiary, account_b, account_c);
//
//
//  }
//  catch (std::runtime_error& e) {
//    std::cerr << e.what() << std::endl;
//    abort();
//  }
//  catch (eevm::Exception& e) {
//    std::cerr << e.what() << std::endl;
//    abort();
//  }
//  catch (const char* c) {
//    std::cerr << c << std::endl;
//    abort();
//  }
//
//  return 0;
//}