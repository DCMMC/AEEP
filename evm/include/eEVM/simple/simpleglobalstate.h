// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "eEVM/globalstate.h"
#include "eEVM/simple/simpleaccount.h"
#include "eEVM/simple/simplestorage.h"

namespace eevm
{
  /**
   * Simple std::map-backed implementation of GlobalState
   */
  class SimpleGlobalState : public GlobalState
  {
  public:
    using StateEntry = std::pair<SimpleAccount, SimpleStorage>;

  private:
    Block currentBlock;

    std::map<Address, StateEntry> accounts;

  public:
    SimpleGlobalState() = default;
    explicit SimpleGlobalState(Block b) : currentBlock(std::move(b)) {}

    virtual void remove(const Address& addr) override;

    AccountState get(const Address& addr) override;
    AccountState create(
      const Address& addr, const uint256_t& balance, const Code& code) override;

    bool exists(const Address& addr);
    virtual size_t num_accounts() override;

    virtual const Block& get_current_block() override;
    virtual uint256_t get_block_hash(uint8_t offset) override;

    virtual std::map<Address, std::pair<SimpleAccount, SimpleStorage>> get_all_accounts() override;
    virtual void set_block(Block b) override;
    virtual void set_accounts(std::map<Address, std::pair<SimpleAccount, SimpleStorage>> acc) override;


    /**
     * For tests which require some initial state, allow manual insertion of
     * pre-constructed accounts
     */
    void insert(const StateEntry& e);

    friend void to_json(nlohmann::json&, const SimpleGlobalState&);
    friend void from_json(const nlohmann::json&, SimpleGlobalState&);
    friend bool operator==(const SimpleGlobalState&, const SimpleGlobalState&);
  };

  void to_json(nlohmann::json&, const SimpleGlobalState&);
  void from_json(const nlohmann::json&, SimpleGlobalState&);
  bool operator==(const SimpleGlobalState&, const SimpleGlobalState&);
} // namespace eevm
