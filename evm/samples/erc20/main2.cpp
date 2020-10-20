#include "eEVM/processor.h"



using Addresses = std::vector<eevm::Address>;

void append_argument(std::vector<uint8_t>& code, const uint256_t& arg)
{
  // To ABI encode a function call with a uint256_t (or Address) argument,
  // simply append the big-endian byte representation to the code (function
  // selector, or bin). ABI-encoding for more complicated types is more
  // complicated, so not shown in this sample.
  const auto pre_size = code.size();
  code.resize(pre_size + 32u);
  eevm::to_big_endian(arg, code.data() + pre_size);
}

uint256_t get_random_address(size_t bytes)
{
  std::vector<uint8_t> raw(bytes);
  std::generate(raw.begin(), raw.end(), []() { return rand(); });
  return eevm::from_big_endian(raw.data(), raw.size());
}

std::vector<uint8_t> run_and_check_result(const eevm::Address& from, const eevm::Address& to, const eevm::Code& input) {

  // processor
  eevm::Processor p;

  // run
  long start_time = clock();
  p.init(from);
  const auto exec_result = p.run(from, to, input, 0u);
  p.save();
  long end_time = clock();
  if (CAL_EEVM_TIME) printf("eevm time: %f\n", ((double)end_time - start_time) / 1000);

  if (exec_result.er != 0) {
    if (exec_result.er == 1) {
      // Rethrow to highlight any exceptions raised in execution
      throw std::runtime_error(fmt::format("Execution threw an error: {}", exec_result.exmsg));
    }
    throw std::runtime_error("Deployment did not return");
  }

  return arr2vector(exec_result.output, charx2uint64(exec_result.output_size));
}

eevm::Address deploy_contract(
  const nlohmann::json& contract_definition,
  const eevm::Address& sender,
  const uint256_t total_supply) {

  // binary constructor
  auto contract_constructor = eevm::to_bytes(contract_definition["bin"]);

  // The constructor takes a single argument - append it
  append_argument(contract_constructor, total_supply);

  // processor
  eevm::Processor p;

  // deploy
  p.init(sender);
  eevm::Address contract_address = p.deploy(sender, contract_constructor);
  p.save();

  return contract_address;
}

void create_account(const eevm::Address& account_address, const uint256_t& account_balance, const eevm::Code& account_code) {
  // processor
  eevm::Processor p;

  // create
  p.init(account_address);
  p.create_account(account_address, account_balance, account_code, 0);
  p.save();
}

bool transfer(
  const nlohmann::json& contract_definition,
  const eevm::Address& contract_address,
  const eevm::Address& source_address,
  const eevm::Address& target_address,
  const uint256_t& amount)
{
  // To transfer tokens, the caller must be the intended source address
  auto function_call = eevm::to_bytes(contract_definition["hashes"]["transfer(address,uint256)"]);

  append_argument(function_call, target_address);
  append_argument(function_call, amount);

  //std::cout << fmt::format(
  //  "Transferring {} from {} to {}",
  //  amount,
  //  eevm::to_checksum_address(source_address),
  //  eevm::to_checksum_address(target_address))
  //  << std::endl;

  const auto output = run_and_check_result(source_address, contract_address, function_call);

  // Output should be a bool in a 32-byte vector.
  if (output.size() != 32 || (output[31] != 0 && output[31] != 1))
  {
    throw std::runtime_error("Unexpected output from call to transfer");
  }

  const bool success = output[31] == 1;
  //std::cout << (success ? " (succeeded)" : " (failed)") << std::endl;

  return success;
}

uint256_t get_total_supply(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender)
{
  const auto function_call = eevm::to_bytes(contract_definition["hashes"]["totalSupply()"]);

  const auto output = run_and_check_result(sender, contract_address, function_call);

  return eevm::from_big_endian(output.data(), output.size());
}

uint256_t get_balance(const nlohmann::json& contract_definition, const eevm::Address& contract_address, const eevm::Address& sender, const eevm::Address& target_address)
{
  auto function_call = eevm::to_bytes(contract_definition["hashes"]["balanceOf(address)"]);

  append_argument(function_call, target_address);

  const auto output = run_and_check_result(sender, contract_address, function_call);

  return eevm::from_big_endian(output.data(), output.size());
}

void print_erc20_state(
  const std::string& heading,
  const nlohmann::json& contract_definition,
  const eevm::Address& contract_address,
  const Addresses& users,
  const eevm::Address& owner)
{
  const auto total_supply = get_total_supply(contract_definition, contract_address, owner);

  using Balances = std::vector<std::pair<eevm::Address, uint256_t>>;
  Balances balances;

  for (const auto& user : users)
  {
    balances.emplace_back(std::make_pair(user, get_balance(contract_definition, contract_address, owner, user)));
  }

  //std::cout << heading << std::endl;
  //std::cout << fmt::format("Total supply of tokens is: {}", total_supply) << std::endl;
  //std::cout << "User balances: " << std::endl;
  for (const auto& pair : balances)
  {
    //std::cout << fmt::format(" {} owned by {}", pair.second, eevm::to_checksum_address(pair.first));
    if (pair.first == owner)
    {
      //std::cout << " (original contract creator)";
    }
    //std::cout << std::endl;
  }
  //std::cout << std::string(heading.size(), '-') << std::endl << std::endl;
}



int main(int argc, char** argv) {

  srand(time(nullptr));

  // contract
  const std::string contract_path = "e:/Computer/workspace/evm_with_oe/evm/samples/erc20/ERC20_combined.json";
  std::ifstream contract_fstream(contract_path);
  if (!contract_fstream) {
    throw std::runtime_error(
      fmt::format("Unable to open contract definition file {}", contract_path));
  }
  std::stringstream contract_buffer;
  contract_buffer << contract_fstream.rdbuf();
  const auto contracts_json = nlohmann::json::parse(contract_buffer.str());
  const auto all_contracts = contracts_json["contracts"];
  const auto erc20_definition = all_contracts["ERC20.sol:ERC20Token"];



  // total supply
  const uint256_t total_supply = 1000;
  Addresses users;

  // owner
  //const eevm::Address owner = get_random_address(20);
  const eevm::Address owner = uint256_t(12345);
  create_account(owner, uint256_t(0), {});
  users.push_back(owner);

  // alice
  //const eevm::Address alice = get_random_address(20);
  const eevm::Address alice = uint256_t(67890);
  create_account(alice, uint256_t(0), {});
  users.push_back(alice);



  // deploy
  auto contract_address = deploy_contract(erc20_definition, owner, total_supply);
  print_erc20_state("-- Initial state --", erc20_definition, contract_address, users, owner);

  // transfer
  const auto first_transfer_amount = total_supply / 3;
  const auto success = transfer(erc20_definition, contract_address, owner, alice, first_transfer_amount);
  if (!success) {
    throw std::runtime_error("Expected transfer to succeed, but it failed");
  }
  const auto failure = transfer(erc20_definition, contract_address, alice, owner, first_transfer_amount + 1);
  if (failure) {
    throw std::runtime_error("Expected transfer to fail, but it succeeded");
  }
  print_erc20_state("-- After one transaction --", erc20_definition, contract_address, users, owner);


  system("pause");
  return 0;
}