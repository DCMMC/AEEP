//#include "eEVM/processor.h"
//
//
//std::vector<double> time_res;
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
//void append_argument3(std::vector<uint8_t>& code, const std::vector<std::string> arg)
//{
//  // To ABI encode a function call with a uint256_t (or Address) argument,
//  // simply append the big-endian byte representation to the code (function
//  // selector, or bin). ABI-encoding for more complicated types is more
//  // complicated, so not shown in this sample.
//  
//  code.resize(code.size() + 31u);
//  code.push_back(static_cast<uint8_t>(32));
//  code.resize(code.size() + 31u);
//  code.push_back(static_cast<uint8_t>(2));
//
//  for (int i = 0; i < arg.size(); i++) {
//    auto tmp = eevm::to_bytes(arg[i]);
//    for (int j = 0; j < tmp.size(); j++) {
//      code.push_back(tmp[j]);
//    }
//  }
//}
//
//uint256_t get_random_address(size_t bytes)
//{
//  std::vector<uint8_t> raw(bytes);
//  std::generate(raw.begin(), raw.end(), []() { return rand(); });
//  return eevm::from_big_endian(raw.data(), raw.size());
//}
//
//// Run input as an EVM transaction, check the result and return the output
//std::vector<uint8_t> run_and_check_result(const eevm::Address& from, const eevm::Address& to, const eevm::Code& input) {
//
//  // processor
//  eevm::Processor p;
//
//  // run
//  long start_time = clock();
//  p.init(from);
//  const auto exec_result = p.run(from, to, input, 0u);
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
//  const std::vector<std::string> proposal) {
//
//  // binary constructor
//  auto contract_constructor = eevm::to_bytes(contract_definition["bin"]);
//
//  // The constructor takes a single argument - append it
//  append_argument3(contract_constructor, proposal);
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
//void create_account(const eevm::Address& account_address, const uint256_t& account_balance, const eevm::Code& account_code) {
//  // processor
//  eevm::Processor p;
//
//  // create
//  p.init(account_address);
//  p.create_account(account_address, account_balance, account_code, 0);
//  p.save();
//}
//
//bool giveRightToVote(
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address& sender,
//  const eevm::Address& voter) {
//  
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["giveRightToVote(address)"]);
//
//  append_argument(function_call, voter);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
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
//bool delegate(
//  const nlohmann::json& contract_definition, 
//  const eevm::Address& contract_address,
//  const eevm::Address& sender,
//  const eevm::Address& to) {
//
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["delegate(address)"]);
//
//  append_argument(function_call, to);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
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
//bool vote(
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address& sender,
//  const uint256_t& proposal) {
//
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["vote(uint256)"]);
//
//  append_argument(function_call, proposal);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
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
//std::string winnerName(
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address& sender) {
//  
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["winnerName()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  std::string res = "";
//  for (int i = 0; i < output.size(); i++) {
//    res += output[i];
//  }
//
//  return res;
//}
//
//uint256_t get_chairperson(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_chairperson()"]);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//uint256_t get_voter_weight(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const eevm::Address& voter_address)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_voter_weight(address)"]);
//
//  append_argument(function_call, voter_address);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//bool get_voter_voted(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const eevm::Address& voter_address)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_voter_voted(address)"]);
//
//  append_argument(function_call, voter_address);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return output[31] == 1;
//}
//
//uint256_t get_voter_delegate(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const eevm::Address& voter_address)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_voter_delegate(address)"]);
//
//  append_argument(function_call, voter_address);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//uint256_t get_voter_vote(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const eevm::Address& voter_address)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_voter_vote(address)"]);
//
//  append_argument(function_call, voter_address);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//std::string get_proposal_name(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const uint256_t& proposal)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_proposal_name(uint256)"]);
//
//  append_argument(function_call, proposal);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  std::string res = "";
//  for (int i = 0; i < output.size(); i++) {
//    res += output[i];
//  }
//
//  return res;
//}
//
//uint256_t get_proposal_votecount(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const uint256_t& proposal)
//{
//  auto function_call = eevm::to_bytes(contract_definition["hashes"]["get_proposal_voteCount(uint256)"]);
//
//  append_argument(function_call, proposal);
//
//  const auto output = run_and_check_result(sender, contract_address, function_call);
//
//  return eevm::from_big_endian(output.data(), output.size());
//}
//
//void print_vote_state(
//  const std::string& heading,
//  const nlohmann::json& contract_definition,
//  const eevm::Address& contract_address,
//  const eevm::Address chairperson,
//  const eevm::Address account_b,
//  const eevm::Address account_c,
//  const std::vector<std::string> proposal)
//{
//  //std::cout << heading << std::endl;
//
//  eevm::Address _chairperson = get_chairperson(contract_definition, contract_address, chairperson);
//  //std::cout << "chairperson: " << eevm::to_checksum_address(_chairperson) << std::endl;
//
//  uint256_t _weight = get_voter_weight(contract_definition, contract_address, chairperson, chairperson);
//  bool _voted = get_voter_voted(contract_definition, contract_address, chairperson, chairperson);
//  uint256_t _delegate = get_voter_delegate(contract_definition, contract_address, chairperson, chairperson);
//  uint256_t _vote = get_voter_vote(contract_definition, contract_address, chairperson, chairperson);
//  //std::cout << "account vote: " << eevm::to_checksum_address(chairperson) << std::endl;
//  //std::cout << "  weight: " << _weight << std::endl;
//  //std::cout << "  voted: " << _voted << std::endl;
//  //std::cout << "  delegate: " << eevm::to_checksum_address(_delegate) << std::endl;
//  //std::cout << "  vote: " << _vote << std::endl;
//
//  _weight = get_voter_weight(contract_definition, contract_address, chairperson, account_b);
//  _voted = get_voter_voted(contract_definition, contract_address, chairperson, account_b);
//  _delegate = get_voter_delegate(contract_definition, contract_address, chairperson, account_b);
//  _vote = get_voter_vote(contract_definition, contract_address, chairperson, account_b);
//  //std::cout << "account vote: " << eevm::to_checksum_address(account_b) << std::endl;
//  //std::cout << "  weight: " << _weight << std::endl;
//  //std::cout << "  voted: " << _voted << std::endl;
//  //std::cout << "  delegate: " << eevm::to_checksum_address(_delegate) << std::endl;
//  //std::cout << "  vote: " << _vote << std::endl;
//
//  _weight = get_voter_weight(contract_definition, contract_address, chairperson, account_c);
//  _voted = get_voter_voted(contract_definition, contract_address, chairperson, account_c);
//  _delegate = get_voter_delegate(contract_definition, contract_address, chairperson, account_c);
//  _vote = get_voter_vote(contract_definition, contract_address, chairperson, account_c);
//  //std::cout << "account vote: " << eevm::to_checksum_address(account_c) << std::endl;
//  //std::cout << "  weight: " << _weight << std::endl;
//  //std::cout << "  voted: " << _voted << std::endl;
//  //std::cout << "  delegate: " << eevm::to_checksum_address(_delegate) << std::endl;
//  //std::cout << "  vote: " << _vote << std::endl;
//
//  for (int i = 0; i < proposal.size(); i++) {
//    std::string _name = get_proposal_name(contract_definition, contract_address, chairperson, uint256_t(i));
//    uint256_t _votecount = get_proposal_votecount(contract_definition, contract_address, chairperson, uint256_t(i));
//    //std::cout << "proposal: " << _name << std::endl;
//    //std::cout << "  vote count: " << _votecount << std::endl;
//  }
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
//  const std::string contract_path = "e:/Computer/workspace/evm_with_oe/evm/samples/vote/vote_combined.json";
//  std::ifstream contract_fstream(contract_path);
//  if (!contract_fstream) {
//    throw std::runtime_error(
//      fmt::format("Unable to open contract definition file {}", contract_path));
//  }
//  std::stringstream contract_buffer;
//  contract_buffer << contract_fstream.rdbuf();
//  const auto contracts_json = nlohmann::json::parse(contract_buffer.str());
//  const auto all_contracts = contracts_json["contracts"];
//  const auto vote_definition = all_contracts["vote.sol:Ballot"];
//
//
//
//  // account chair person
//  // const eevm::Address chairperson = get_random_address(20);
//  const eevm::Address chairperson = uint256_t(12345);
//  create_account(chairperson, uint256_t(0), {});
//
//  // account b
//  // const eevm::Address account_b = get_random_address(20);
//  const eevm::Address account_b = uint256_t(67890);
//  create_account(account_b, uint256_t(0), {});
//
//  // account c
//  // const eevm::Address account_c = get_random_address(20);
//  const eevm::Address account_c = uint256_t(60978);
//  create_account(account_c, uint256_t(0), {});
//
//  // proposal  16½øÖÆ
//  const std::vector<std::string> proposal = { "3030303030303030303030303030303030303030303030303030303030303031","3030303030303030303030303030303030303030303030303030303030303041" };
//
//  //std::cout << "chairperson: " << eevm::to_checksum_address(chairperson) << std::endl;
//  //std::cout << "account_b: " << eevm::to_checksum_address(account_b) << std::endl;
//  //std::cout << "account_c: " << eevm::to_checksum_address(account_c) << std::endl;
//  for (int i = 0; i < proposal.size(); i++) {
//    auto tmp = eevm::to_bytes(proposal[i]);
//    std::string p = "";
//    for (int i = 0; i < tmp.size(); i++) {
//      p += tmp[i];
//    }
//    //std::cout << "proposal " << i << " :" << p << std::endl;
//  }
//
//
//
//  // deploy
//  auto contract_address = deploy_contract(vote_definition, chairperson, proposal);
//  print_vote_state("-- Initial state --", vote_definition, contract_address, chairperson, account_b, account_c, proposal);
//
//  // giveRightToVote
//  bool success_1 = giveRightToVote(vote_definition, contract_address, chairperson, account_b);
//  bool success_2 = giveRightToVote(vote_definition, contract_address, chairperson, account_c);
//  //std::cout << "chairperson give right to account_b: " << (success_1 ? " succeeded" : " failed") << std::endl;
//  //std::cout << "chairperson give right to account_c: " << (success_2 ? " succeeded" : " failed") << std::endl;
//  print_vote_state("-- After giveRightToVote --", vote_definition, contract_address, chairperson, account_b, account_c, proposal);
//
//  // delegate
//  bool success_3 = delegate(vote_definition, contract_address, account_b, account_c);
//  //std::cout << "account_b delegate account_c: " << (success_3 ? " succeeded" : " failed") << std::endl;
//  print_vote_state("-- After delegate --", vote_definition, contract_address, chairperson, account_b, account_c, proposal);
//
//  // vote
//  bool success_4 = vote(vote_definition, contract_address, chairperson, uint256_t(0));
//  bool success_5 = vote(vote_definition, contract_address, account_c, uint256_t(1));
//  //std::cout << "chairperson vote 0: " << (success_4 ? " succeeded" : " failed") << std::endl;
//  //std::cout << "account_c vote 1: " << (success_5 ? " succeeded" : " failed") << std::endl;
//  print_vote_state("-- After vote --", vote_definition, contract_address, chairperson, account_b, account_c, proposal);
//
//  // winnerName
//  std::string winner = winnerName(vote_definition, contract_address, chairperson);
//  //std::cout << "winner: " << winner << std::endl;
//
//
//  return 0;
//}