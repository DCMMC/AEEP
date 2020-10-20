void sgt(Context* ctxt) {
  ctxt->s.swap(1);
  slt(ctxt);
}



void dispatch(std::vector<Context>* ctxts, Context** ctxt, gs_struct* _gs, tx_struct* _tx, exec_result_struct* result)
{
  Opcode op = get_op((*ctxt)->get_pc(), (*ctxt)->prog.code);

  switch (op)
  {
  case Opcode::PUSH1:
  case Opcode::PUSH2:
  case Opcode::PUSH3:
  case Opcode::PUSH4:
  case Opcode::PUSH5:
  case Opcode::PUSH6:
  case Opcode::PUSH7:
  case Opcode::PUSH8:
  case Opcode::PUSH9:
  case Opcode::PUSH10:
  case Opcode::PUSH11:
  case Opcode::PUSH12:
  case Opcode::PUSH13:
  case Opcode::PUSH14:
  case Opcode::PUSH15:
  case Opcode::PUSH16:
  case Opcode::PUSH17:
  case Opcode::PUSH18:
  case Opcode::PUSH19:
  case Opcode::PUSH20:
  case Opcode::PUSH21:
  case Opcode::PUSH22:
  case Opcode::PUSH23:
  case Opcode::PUSH24:
  case Opcode::PUSH25:
  case Opcode::PUSH26:
  case Opcode::PUSH27:
  case Opcode::PUSH28:
  case Opcode::PUSH29:
  case Opcode::PUSH30:
  case Opcode::PUSH31:
  case Opcode::PUSH32:
	push(*ctxt);
	break;
  case Opcode::POP:
	pop(*ctxt);
	break;
  case Opcode::SWAP1:
  case Opcode::SWAP2:
  case Opcode::SWAP3:
  case Opcode::SWAP4:
  case Opcode::SWAP5:
  case Opcode::SWAP6:
  case Opcode::SWAP7:
  case Opcode::SWAP8:
  case Opcode::SWAP9:
  case Opcode::SWAP10:
  case Opcode::SWAP11:
  case Opcode::SWAP12:
  case Opcode::SWAP13:
  case Opcode::SWAP14:
  case Opcode::SWAP15:
  case Opcode::SWAP16:
	swap(*ctxt);
	break;
  case Opcode::DUP1:
  case Opcode::DUP2:
  case Opcode::DUP3:
  case Opcode::DUP4:
  case Opcode::DUP5:
  case Opcode::DUP6:
  case Opcode::DUP7:
  case Opcode::DUP8:
  case Opcode::DUP9:
  case Opcode::DUP10:
  case Opcode::DUP11:
  case Opcode::DUP12:
  case Opcode::DUP13:
  case Opcode::DUP14:
  case Opcode::DUP15:
  case Opcode::DUP16:
	dup(*ctxt);
	break;
  case Opcode::LOG0:
  case Opcode::LOG1:
  case Opcode::LOG2:
  case Opcode::LOG3:
  case Opcode::LOG4:
	log(*ctxt, _tx);
	break;
  case Opcode::ADD:
	add(*ctxt);
	break;
  case Opcode::MUL:
	mul(*ctxt);
	break;
  case Opcode::SUB:
	sub(*ctxt);
	break;
  case Opcode::DIV:
	div(*ctxt);
	break;
  case Opcode::SDIV:
	sdiv(*ctxt);
	break;
  case Opcode::MOD:
	mod(*ctxt);
	break;
  case Opcode::SMOD:
	smod(*ctxt);
	break;
  case Opcode::ADDMOD:
	addmod(*ctxt);
	break;
  case Opcode::MULMOD:
	mulmod(*ctxt);
	break;
  case Opcode::EXP:
	exp(*ctxt);
	break;
  case Opcode::SIGNEXTEND:
	signextend(*ctxt);
	break;
  case Opcode::LT:
	lt(*ctxt);
	break;
  case Opcode::GT:
	gt(*ctxt);
	break;
  case Opcode::SLT:
	slt(*ctxt);
	break;
  case Opcode::SGT:
	sgt(*ctxt);
	break;
  case Opcode::EQ:
	eq(*ctxt);
	break;
  case Opcode::ISZERO:
	isZero(*ctxt);
	break;
  case Opcode::AND:
	and_(*ctxt);
	break;
  case Opcode::OR:
	or_(*ctxt);
	break;
  case Opcode::XOR:
	xor_(*ctxt);
	break;
  case Opcode::NOT:
	not_(*ctxt);
	break;
  case Opcode::BYTE:
	byte(*ctxt);
	break;
  case Opcode::SHL:
	shl(*ctxt);
	break;
  case Opcode::SHR:
	shr(*ctxt);
	break;
  case Opcode::JUMP:
	jump(*ctxt);
	break;
  case Opcode::JUMPI:
	jumpi(*ctxt);
	break;
  case Opcode::PC:
	pc(*ctxt);
	break;
  case Opcode::MSIZE:
	msize(*ctxt);
	break;
  case Opcode::MLOAD:
	mload(*ctxt);
	break;
  case Opcode::MSTORE:
	mstore(*ctxt);
	break;
  case Opcode::MSTORE8:
	mstore8(*ctxt);
	break;
  case Opcode::CODESIZE:
	codesize(*ctxt);
	break;
  case Opcode::CODECOPY:
	codecopy(*ctxt);
	break;
  case Opcode::EXTCODESIZE:
	extcodesize(*ctxt, _gs);
	break;
  case Opcode::EXTCODECOPY:
	extcodecopy(*ctxt, _gs);
	break;
  case Opcode::SLOAD:
	sload(*ctxt);
	break;
  case Opcode::SSTORE:
	sstore(*ctxt);
	break;
  case Opcode::ADDRESS:
	address(*ctxt);
	break;
  case Opcode::BALANCE:
	balance(*ctxt, _gs);
	break;
  case Opcode::ORIGIN:
	origin(*ctxt, _tx);
	break;
  case Opcode::CALLER:
	caller(*ctxt);
	break;
  case Opcode::CALLVALUE:
	callvalue(*ctxt);
	break;
  case Opcode::CALLDATALOAD:
	calldataload(*ctxt);
	break;
  case Opcode::CALLDATASIZE:
	calldatasize(*ctxt);
	break;
  case Opcode::CALLDATACOPY:
	calldatacopy(*ctxt);
	break;
  case Opcode::RETURN:
	return_(ctxts, ctxt, result);
	break;
  case Opcode::SELFDESTRUCT:
	selfdestruct(ctxts, ctxt, result, _gs, _tx);
	break;
  case Opcode::CREATE:
	create(ctxts, *ctxt, _gs);
	break;
  case Opcode::CALL:
  case Opcode::CALLCODE:
  case Opcode::DELEGATECALL:
	call(ctxts, *ctxt, _gs);
	break;
  case Opcode::JUMPDEST:
	jumpdest(*ctxt);
	break;
  case Opcode::BLOCKHASH:
	blockhash(*ctxt, _gs);
	break;
  case Opcode::NUMBER:
	number(*ctxt, _gs);
	break;
  case Opcode::GASPRICE:
	gasprice(*ctxt, _tx);
	break;
  case Opcode::COINBASE:
	coinbase(*ctxt, _gs);
	break;
  case Opcode::TIMESTAMP:
	timestamp(*ctxt, _gs);
	break;
  case Opcode::DIFFICULTY:
	difficulty(*ctxt, _gs);
	break;
  case Opcode::GASLIMIT:
	gaslimit(*ctxt, _gs);
	break;
  case Opcode::GAS:
	gas(*ctxt, _tx);
	break;
  case Opcode::SHA3:
	sha3(*ctxt);
	break;
  case Opcode::STOP:
	stop(ctxts, ctxt, result);
	break;
  case Opcode::REVERT:
	revert(ctxts, ctxt, result);
	break;
  default:
	throw "Unknown/unsupported Opcode\n";
  };
}


void enclave_run(exec_result_struct* result, gs_struct* _gs, tx_struct* _tx, unsigned char* _caller, unsigned char* _callee, uint8_t* _input, size_t _input_size, unsigned char* _call_value) {

  // decode
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	account_struct* acc = &_gs->accounts[i].account;
	for (int j = 0; j < charx2uint64(acc->storage_size); j++) {
	  unsigned char* dec_key = new unsigned char[200];
	  memset(dec_key, 0, sizeof(unsigned char) * 200);
	  unsigned char* dec_value = new unsigned char[200];
	  memset(dec_value, 0, sizeof(unsigned char) * 200);
	  aes256_dec(acc->storage[j].key, dec_key);
	  aes256_dec(acc->storage[j].value, dec_value);
	  memcpy(acc->storage[j].key, dec_key, sizeof(unsigned char) * 200);
	  memcpy(acc->storage[j].value, dec_value, sizeof(unsigned char) * 200);
	}
  }
  
  // find callee
  account_struct* callee = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (strcmp((char*)_gs->accounts[i].address, (char*)_callee) == 0) {
	  callee = &_gs->accounts[i].account;
	}
  }
  
  // function of setting result
  auto rh = [result](std::vector<uint8_t> output_) {
	result->er = ExitReason["returned"];
	memcpy(result->output, &output_[0], sizeof(uint8_t) * output_.size());
	unsigned char* tmp = uint642charx(output_.size());
	memcpy(result->output_size, tmp, strlen((char*)tmp) + 1);
  };
  auto hh = [result]() {
	result->er = ExitReason["halted"];
  };
  auto eh = [result](unsigned char* _exmsg) {
	result->er = ExitReason["threw"];
	memcpy(result->exmsg, _exmsg, strlen((char*)_exmsg) + 1);
  };
  
  // context
  std::vector<Context> ctxts;
  Context* ctxt;
  push_context(&ctxts, &ctxt, _caller, callee, _input, _input_size, _call_value, Program(callee->account_code, charx2uint64(callee->account_code_size)), rh, hh, eh);

  // call value
  account_struct* caller_account = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (strcmp((char*)_gs->accounts[i].address, (char*)_caller) == 0) {
	  caller_account = &_gs->accounts[i].account;
	  break;
	}
  }
  a_pay_b(caller_account, callee, _call_value);

  // opcode
  while (ctxt->get_pc() < ctxt->prog.code.size()) {
	if(PRINT_OPCODE) printf("OPCODE: 0x%x\n", get_op(ctxt->get_pc(), ctxt->prog.code));
	try{
	  dispatch(&ctxts, &ctxt, _gs, _tx, result);
	}
	catch (unsigned char* msg){
	  ctxt->eh(msg);
	  pop_context(&ctxts, &ctxt);
	}

	if (!ctxt) {
	  break;
	}
	ctxt->step();

	if (PRINT_STACK_MEM) printf_stack(ctxt->s);
	if (PRINT_STACK_MEM) printf_mem(ctxt->mem);
  }

  if (ctxt) {
	stop(&ctxts, &ctxt, result);
  }

  // some delete
  gs_accounts_struct* final_accounts = new gs_accounts_struct[100];
  size_t final_accounts_size = 0;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	gs_accounts_struct t = _gs->accounts[i];
	bool flag = true;
	for (int j = 0; j < charx2uint64(_tx->selfdestruct_list_size); j++) {
	  unsigned char* a = _tx->selfdestruct_list[j];
	  if (str_compare(t.address, a) == 0) {
		flag = false;
		break;
	  }
	}
	if (flag) {
	  final_accounts[final_accounts_size] = t;
	  final_accounts_size += 1;
	}
  }

  memcpy(_gs->accounts, final_accounts, sizeof(gs_accounts_struct) * final_accounts_size);
  unsigned char* tmp = uint642charx(final_accounts_size);
  memcpy(_gs->accounts_size, tmp, strlen((char*)tmp) + 1);

  // encode
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	account_struct* acc = &_gs->accounts[i].account;
	for (int j = 0; j < charx2uint64(acc->storage_size); j++) {
	  unsigned char* enc_key = new unsigned char[200];
	  memset(enc_key, 0, sizeof(unsigned char) * 200);
	  unsigned char* enc_value = new unsigned char[200];
	  memset(enc_key, 0, sizeof(unsigned char) * 200);
	  aes256_enc(acc->storage[j].key, enc_key);
	  aes256_enc(acc->storage[j].value, enc_value);
	  memcpy(acc->storage[j].key, enc_key, sizeof(unsigned char) * 200);
	  memcpy(acc->storage[j].value, enc_value, sizeof(unsigned char) * 200);
	}
  }
}

void enclave_deploy(gs_struct* _gs, tx_struct* _tx, unsigned char* _caller, uint8_t* _constructor, size_t _constructor_size, unsigned char* contract_address) {

  // get new contract address
  size_t caller_nonce = 0;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (strcmp((char*)_gs->accounts[i].address, (char*)_caller) == 0) {
	  caller_nonce = charx2uint64(_gs->accounts[i].account.account_nonce);
	}
  }
  generate_address_ocall(_caller, caller_nonce, contract_address);

  // if contract address exist or not
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (str_compare(contract_address, (_gs->accounts + i)->address) == 0) {
	  throw "Contract address exist!";
	}
  }

  // create new account
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, contract_address, strlen((char*)contract_address) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, contract_address, strlen((char*)contract_address) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, _constructor, sizeof(uint8_t) * _constructor_size);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(_constructor_size), strlen((char*)uint642charx(_constructor_size)) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
  memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

  // init result
  exec_result_struct _result;
  _result.exmsg = new unsigned char[100];
  _result.output = new uint8_t[7000];
  _result.output_size = new unsigned char[100];

  // find callee
  account_struct* callee = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (strcmp((char*)_gs->accounts[i].address, (char*)contract_address) == 0) {
	  callee = &_gs->accounts[i].account;
	  break;
	}
  }

  // do
  enclave_run(&_result, _gs, _tx, _caller, contract_address, {}, 0, (unsigned char*)"0");

  // set code
  memcpy(callee->account_code, _result.output, sizeof(uint8_t) * charx2uint64(_result.output_size));
  memcpy(callee->account_code_size, _result.output_size, strlen((char*)_result.output_size) + 1);
}