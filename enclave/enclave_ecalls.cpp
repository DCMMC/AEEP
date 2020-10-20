#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <deque>

#include "opcode.h"
#include "myAes256.hpp"
#include "myKeccak256.hpp"
#include "myBigInt.hpp"

#include "../enclave/ttt.h"

// ==================== enclave headfiles ====================
#include "enclave_t.h"
// ==================== enclave headfiles ====================

// ==================== enclave init setting ====================
#define HEAP_SIZE_BYTES (2 * 1024 * 1024) /* 2 MB */
#define STACK_SIZE_BYTES (24 * 1024)      /* 24 KB */

#define SGX_PAGE_SIZE (4 * 1024) /* 4 KB */

#define TA_UUID /* 4e3c0f91-4f56-452d-92d7-abc4c2c831fa */ {0x4e3c0f91,0x4f56,0x452d,{0x92,0xd7,0xab,0xc4,0xc2,0xc8,0x31,0xfa}}

OE_SET_ENCLAVE_OPTEE(
  TA_UUID,                                  /* UUID */
  HEAP_SIZE_BYTES,                          /* HEAP_SIZE */
  STACK_SIZE_BYTES,                         /* STACK_SIZE */
  TA_FLAG_MULTI_SESSION | TA_FLAG_EXEC_DDR, /* FLAGS */
  "1.0.0",                                  /* VERSION */
  "enclave TA");                      /* DESCRIPTION */

OE_SET_ENCLAVE_SGX(
  1, /* ProductID */
  1, /* SecurityVersion */
#ifdef _DEBUG
  1, /* Debug */
#else
  0, /* Debug */
#endif
  HEAP_SIZE_BYTES / SGX_PAGE_SIZE,  /* NumHeapPages */
  STACK_SIZE_BYTES / SGX_PAGE_SIZE, /* NumStackPages */
  1);                               /* NumTCS */
// ==================== enclave init setting ====================



/* uint8_t* -> vector<uint8_t> */
std::vector<uint8_t> arr2vector(uint8_t* code_arr, size_t code_size) {
  std::vector<uint8_t> vec(code_arr, code_arr + code_size);
  return vec;
}

/* vector<uint8_t> -> uint8_t* */
uint8_t* vector2arr(std::vector<uint8_t> code) {
  uint8_t* code_arr = new uint8_t[code.size()];
  memset(code_arr, 0, sizeof(uint8_t) * code.size());
  memcpy(code_arr, &code[0], sizeof(uint8_t) * code.size());
  return code_arr;
}

/* char* -> uint64_t */
uint64_t charx2uint64(unsigned char* n) {
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
unsigned char* uint642charx(uint64_t n) {
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

/* compare str */
int str_compare(unsigned char* a, unsigned char* b) {
  if (strlen((char*)a) > strlen((char*)b)) {
	return 1;
  }
  if (strlen((char*)a) < strlen((char*)b)) {
	return -1;
  }
  return strcmp((char*)a, (char*)b);
}

/* compare str */
int str_compare(unsigned char* a, const char* b) {
  unsigned char* _b = const_cast<unsigned char*>((unsigned char*)b);
  return str_compare(a, _b);
}



struct Stack {
  std::deque<unsigned char*> st;
  size_t MAX_SIZE = 1024;

  unsigned char* pop() {
	if (st.empty()) {
	  throw "Stack out of range";
	}
	unsigned char* val = st.front();
	st.pop_front();
	return val;
  }

  void push(unsigned char* val) {
	if (size() == MAX_SIZE) {
	  throw "Stack mem exceeded";
	}

	try {
	  st.push_front(val);
	}
	catch (std::bad_alloc&) {
	  throw std::runtime_error("bad_alloc while pushing onto stack");
	}
  }

  uint64_t size() {
	return st.size();
  }

  void swap(uint64_t i) {
	if (i >= size()) {
	  throw "Swap out of range";
	}
	std::swap(st[0], st[i]);
  }

  void dup(uint64_t a) {
	if (a >= size()) {
	  throw "Dup out of range";
	}
	st.push_front(st[a]);
  }
};

struct Program {
  std::vector<uint8_t> code;
  std::set<uint64_t> jump_dests;

  Program() {}

  Program(std::vector<uint8_t> c) :
	code(c),
	jump_dests(compute_jump_dests_by_vector(c))
  {}

  Program(uint8_t* account_code, size_t account_code_size) :
	code(compute_code(account_code, account_code_size)),
	jump_dests(compute_jump_dests(account_code, account_code_size))
  {}

  std::vector<uint8_t> compute_code(uint8_t* account_code, size_t account_code_size) {
	return arr2vector(account_code, account_code_size);
  }

  std::set<uint64_t> compute_jump_dests(uint8_t* account_code, size_t account_code_size)
  {
	std::vector<uint8_t> code = arr2vector(account_code, account_code_size);
	std::set<uint64_t> dests;
	for (uint64_t i = 0; i < code.size(); i++)
	{
	  uint8_t op = code[i];
	  if (op >= PUSH1 && op <= PUSH32)
	  {
		uint8_t immediate_bytes = op - static_cast<uint8_t>(PUSH1) + 1;
		i += immediate_bytes;
	  }
	  else if (op == JUMPDEST)
		dests.insert(i);
	}
	return dests;
  }

  std::set<uint64_t> compute_jump_dests_by_vector(std::vector<uint8_t> code)
  {
	std::set<uint64_t> dests;
	for (uint64_t i = 0; i < code.size(); i++)
	{
	  const auto op = code[i];
	  if (op >= PUSH1 && op <= PUSH32)
	  {
		const uint8_t immediate_bytes = op - static_cast<uint8_t>(PUSH1) + 1;
		i += immediate_bytes;
	  }
	  else if (op == JUMPDEST)
		dests.insert(i);
	}
	return dests;
  }
};

struct Context {
  uint64_t pc;
  bool pc_changed = true;

  Stack s;
  std::vector<uint8_t> mem;

  unsigned char* caller;
  account_struct* as;
  std::vector<uint8_t> input;
  unsigned char* call_value;
  Program prog;

  using ReturnHandler = std::function<void(std::vector<uint8_t>)>;
  using HaltHandler = std::function<void()>;
  using ExceptionHandler = std::function<void(unsigned char* _exmsg)>;

  ReturnHandler rh;
  HaltHandler hh;
  ExceptionHandler eh;


  Context() {}

  Context(unsigned char* caller, account_struct* as, std::vector<uint8_t> input, unsigned char* call_value, Program prog, ReturnHandler&& rh, HaltHandler&& hh, ExceptionHandler&& eh) :
	pc(0),
	caller(caller),
	as(as),
	input(input),
	call_value(call_value),
	prog(prog),
	rh(rh),
	hh(hh),
	eh(eh)
  {}

  Context(unsigned char* caller, account_struct* as, uint8_t* input, size_t input_size, unsigned char* call_value, Program prog, ReturnHandler&& rh, HaltHandler&& hh, ExceptionHandler&& eh) :
	pc(0),
	caller(caller),
	as(as),
	input(arr2vector(input, input_size)),
	call_value(call_value),
	prog(prog),
	rh(rh),
	hh(hh),
	eh(eh)
  {}

  void step() {
	if (pc_changed) {
	  pc_changed = false;
	}
	else {
	  pc++;
	}
  }

  uint64_t get_pc() {
	return pc;
  }

  void set_pc(uint64_t pc_) {
	pc = pc_;
	pc_changed = true;
  }
};



/* print char* */
void printf_charx(unsigned char* arr) {
  uint8_t i = 0;
  while (*(arr + i) != 0) {
	printf("%c", *(arr + i));
	i += 1;
  }
  printf("\n");
}

/* print uint8* */
void printf_uint8x(uint8_t* arr, size_t arr_size) {
  for (int i = 0; i < arr_size; i++) {
	printf("%d", *(arr + i));
  }
  printf("\n");
}

/* print stack */
void printf_stack(Stack s) {
  printf("stack size %d\n", s.size());
  for (int i = 0; i < s.st.size(); i++) {
	printf_charx(s.st.at(i));
  }
}

/* print memory */
void printf_mem(std::vector<uint8_t> mem) {
  printf("mem size %d\n", mem.size());
  for (int i = 0; i < mem.size(); i++) {
	printf("%d: %d  ", i, mem[i]);
  }
  printf("\n\n");
}

/* print gs */
void printf_gs(gs_struct* _gs) {
  printf("gs account size: ");
  printf_charx(_gs->accounts_size);
  for (int i = 0; i < charx2uint64((_gs->accounts_size)); i++) {
	printf("account: %d\n", i);
	printf("address: ");
	printf_charx(_gs->accounts[i].address);
	printf("balance: ");
	printf_charx(_gs->accounts[i].account.account_balance);
	printf("code size: ");
	printf_charx(_gs->accounts[i].account.account_code_size);
	
	printf("storage size: ");
	printf_charx(_gs->accounts[i].account.storage_size);
	for (int j = 0; j < charx2uint64(_gs->accounts[i].account.storage_size); j++) {
	  printf("key: ");
	  printf_charx(_gs->accounts[i].account.storage[j].key);
	  printf("value: ");
	  printf_charx(_gs->accounts[i].account.storage[j].value);
	}
  }
}



std::map<char*, uint8_t> ExitReason = {
  {"returned", 0},
  {"halted", 1},
  {"threw", 2}
};

void push_context(
  std::vector<Context>* ctxts,
  Context** ctxt,
  unsigned char* caller,
  account_struct* callee,
  uint8_t* input,
  size_t input_size,
  unsigned char* call_value,
  Program prog,
  std::function<void(std::vector<uint8_t>)>&& rh,
  std::function<void()>&& hh,
  std::function<void(unsigned char* _exmsg)>&& eh
) {
  Context c = Context(caller, callee, input, input_size, call_value, prog, std::move(rh), std::move(hh), std::move(eh));
  ctxts->emplace_back(std::move(c));
  *ctxt = &ctxts->back();
}

void push_context(
  std::vector<Context>* ctxts,
  Context** ctxt,
  unsigned char* caller,
  account_struct* callee,
  std::vector<uint8_t> input,
  unsigned char* call_value,
  Program prog,
  std::function<void(std::vector<uint8_t>)>&& rh,
  std::function<void()>&& hh,
  std::function<void(unsigned char* _exmsg)>&& eh
) {
  Context c = Context(caller, std::move(callee), input, call_value, prog, std::move(rh), std::move(hh), std::move(eh));
  ctxts->emplace_back(std::move(c));
  *ctxt = &ctxts->back();
}

Opcode get_op(uint64_t pc, std::vector<uint8_t> code) {
  return static_cast<Opcode>(code[pc]);
}

void copy_mem_raw(const uint64_t offDst, const uint64_t offSrc, const uint64_t size, std::vector<uint8_t>& dst, const std::vector<uint8_t>& src, const uint8_t pad = 0) {
  if (!size) {
	return;
  }

  auto lastDst = offDst + size;
  if (lastDst < offDst) {
	throw "Integer overflow in copy_mem";
  }
  if (lastDst > 1ull << 25) {
	throw "Memory limit exceeded";
  }

  if (lastDst > dst.size()) {
	dst.resize(lastDst);
  }

  auto lastSrc = offSrc + size;
  auto endSrc = std::min(lastSrc, static_cast<decltype(lastSrc)>(src.size()));
  uint64_t remaining;
  if (endSrc > offSrc) {
	copy(src.begin() + offSrc, src.begin() + endSrc, dst.begin() + offDst);
	remaining = lastSrc - endSrc;
  }
  else {
	remaining = size;
  }

  fill(dst.begin() + lastDst - remaining, dst.begin() + lastDst, pad);
}

void copy_mem(std::vector<uint8_t>& dst, const std::vector<uint8_t>& src, const uint8_t pad, Context* ctxt) {
  auto offDst = charx2uint64(ctxt->s.pop());
  auto offSrc = charx2uint64(ctxt->s.pop());
  auto size = charx2uint64(ctxt->s.pop());

  copy_mem_raw(offDst, offSrc, size, dst, src, pad);
}

void prepare_mem_access(uint64_t offset, uint64_t size, std::vector<uint8_t>* mem) {
  uint64_t end = offset + size;
  if (end < offset) {
	throw "Integer overflow in memory access";
  }
  if (end > 1ull << 25) {
	throw "Memory limit exceeded";
  }

  if (end > mem->size()) {
	mem->resize(end);
  }
}

std::vector<uint8_t> copy_from_mem(uint64_t offset, uint64_t size, std::vector<uint8_t>* mem)
{
  prepare_mem_access(offset, size, mem);
  return { mem->begin() + offset, mem->begin() + offset + size };
}

void a_pay_b(account_struct* a, account_struct* b, unsigned char* value) {
  if (str_compare(a->account_balance, value) < 0) {
	throw "Insufficient funds to pay";
  }
  unsigned char* res = new unsigned char[100];
  memset(res, 0, sizeof(unsigned char) * 100);
  add_ocall(b->account_balance, value, res);
  memcpy(b->account_balance, res, strlen((char*)res) + 1);
  memset(res, 0, sizeof(unsigned char) * 100);
  sub_ocall(a->account_balance, value, res);
  memcpy(a->account_balance, res, strlen((char*)res) + 1);

  unsigned char* tmp1 = uint642charx(charx2uint64(a->account_nonce) + 1);
  memcpy(a->account_nonce, tmp1, strlen((char*)tmp1) + 1);
  unsigned char* tmp2 = uint642charx(charx2uint64(b->account_nonce) + 1);
  memcpy(b->account_nonce, tmp2, strlen((char*)tmp2) + 1);
}

void pop_context(std::vector<Context>* ctxts, Context** ctxt) {
  ctxts->pop_back();
  if (!ctxts->empty()) {
	*ctxt = &ctxts->back();
  }
  else {
	*ctxt = nullptr;
  }
}

unsigned char* pop_addr(unsigned char* n) {
  return myBigInt::and_(n, (unsigned char*)myBigInt::MASK_160);
}

void keccak_256(
  const unsigned char* input,
  unsigned int inputByteLen,
  unsigned char* output)
{
  // Ethereum started using Keccak and called it SHA3 before it was finalised.
  // Standard SHA3-256 (the FIPS accepted version) uses padding 0x06, but
  // Ethereum's "Keccak-256" uses padding 0x01.
  // All other constants are copied from Keccak_HashInitialize_SHA3_256 in
  // KeccakHash.h.
  Keccak_HashInstance hi;
  Keccak_HashInitialize(&hi, 1088, 512, 256, 0x01);
  Keccak_HashUpdate(
	&hi, input, inputByteLen * std::numeric_limits<unsigned char>::digits);
  Keccak_HashFinal(&hi, output);
}



void return_(std::vector<Context>* ctxts, Context** ctxt, exec_result_struct* result) {
  uint64_t offset = charx2uint64((*ctxt)->s.pop());
  uint64_t size = charx2uint64((*ctxt)->s.pop());

  (*ctxt)->rh(copy_from_mem(offset, size, &(*ctxt)->mem));

  pop_context(ctxts, ctxt);
}

void stop(std::vector<Context>* ctxts, Context** ctxt, exec_result_struct* result) {
  auto hh = (*ctxt)->hh;
  pop_context(ctxts, ctxt);
  hh();
}

void revert(std::vector<Context>* ctxts, Context** ctxt, exec_result_struct* result) {
  auto hh = (*ctxt)->hh;
  pop_context(ctxts, ctxt);
  hh();
}

void pop(Context* ctxt) {
  ctxt->s.pop();
}

void swap(Context* ctxt) {
  ctxt->s.swap(get_op(ctxt->get_pc(), ctxt->prog.code) - SWAP1 + 1);
}

void dup(Context* ctxt) {
  ctxt->s.dup(get_op(ctxt->get_pc(), ctxt->prog.code) - DUP1);
}

void log(Context* ctxt, tx_struct* _tx) {
  uint8_t n = get_op(ctxt->get_pc(), ctxt->prog.code) - Opcode::LOG0;
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint64_t size = charx2uint64(ctxt->s.pop());

  std::vector<uint8_t> mem_vector = copy_from_mem(offset, size, &ctxt->mem);
  uint8_t* mem = vector2arr(mem_vector);

  unsigned char** topics = new unsigned char*[n];
  for (int i = 0; i < n; i++) {
	topics[n] = new unsigned char[100];
  }
  for (int i = 0; i < n; i++) {
	*(topics + i) = ctxt->s.pop();
  }
}

void jump(Context* ctxt) {
  uint64_t newPc = charx2uint64(ctxt->s.pop());
  if (ctxt->prog.jump_dests.find(newPc) == ctxt->prog.jump_dests.end()) {
	throw "not a jump destination";
  }
  ctxt->set_pc(newPc);
}

void jumpi(Context* ctxt) {
  uint64_t newPc = charx2uint64(ctxt->s.pop());
  unsigned char* cond = ctxt->s.pop();
  if (str_compare(cond, "0") != 0) {
	if (ctxt->prog.jump_dests.find(newPc) == ctxt->prog.jump_dests.end()) {
	  throw "not a jump destination";
	}
	ctxt->set_pc(newPc);
  }
}

void pc(Context* ctxt) {
  ctxt->s.push(uint642charx(ctxt->get_pc()));
}

void msize(Context* ctxt) {
  ctxt->s.push(uint642charx(((ctxt->mem.size() + 32u - 1) / 32u) * 32));
}

void codesize(Context* ctxt) {
  ctxt->s.push(ctxt->as->account_code_size);
}

void codecopy(Context* ctxt) {
  copy_mem(ctxt->mem, ctxt->prog.code, Opcode::STOP, ctxt);
}

void extcodesize(Context* ctxt, gs_struct* _gs) {
  unsigned char* addr = pop_addr(ctxt->s.pop());

  account_struct* acc = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (str_compare(addr, (_gs->accounts + i)->address) == 0) {
	  acc = &((_gs->accounts + i)->account);
	  break;
	}
  }
  if (!acc) {
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, {}, 0);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

	acc = &_gs->accounts[charx2uint64(_gs->accounts_size) - 1].account;
  }

  ctxt->s.push(acc->account_code_size);
}

void extcodecopy(Context* ctxt, gs_struct* _gs) {
  unsigned char* addr = pop_addr(ctxt->s.pop());

  account_struct* acc = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (str_compare(addr, (_gs->accounts + i)->address) == 0) {
	  acc = &((_gs->accounts + i)->account);
	  break;
	}
  }
  if (!acc) {
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, {}, 0);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

	acc = &_gs->accounts[charx2uint64(_gs->accounts_size) - 1].account;
  }

  std::vector<uint8_t> code = arr2vector(acc->account_code, charx2uint64(acc->account_code_size));

  copy_mem(ctxt->mem, code, Opcode::STOP, ctxt);

  memcpy(acc->account_code, vector2arr(code), code.size() + 1);
  memcpy(acc->account_code_size, uint642charx(code.size()), strlen((char*)uint642charx(code.size())) + 1);
}

void sload(Context* ctxt) {
  unsigned char* k = ctxt->s.pop();
  bool flag = false;
  unsigned char* v = new unsigned char[200];
  memset(v, 0, sizeof(unsigned char) * 200);
  for (int i = 0; i < charx2uint64(ctxt->as->storage_size); i++) {
	if (str_compare((ctxt->as->storage + i)->key, k) == 0) {
	  memcpy(v, (ctxt->as->storage + i)->value, strlen((char*)(ctxt->as->storage + i)->value) + 1);
	  flag = true;
	  break;
	}
  }
  if (!flag) {
	v = new unsigned char[200];
	memset(v, 0, sizeof(unsigned char) * 200);
	v[0] = '0';
  }
  ctxt->s.push(v);
}

void sstore(Context* ctxt) {
  unsigned char* k = ctxt->s.pop();
  unsigned char* v = ctxt->s.pop();
  if (str_compare(v, "0") == 0) {
	std::vector<storage_struct> vec(ctxt->as->storage, ctxt->as->storage + charx2uint64(ctxt->as->storage_size));
	std::vector<storage_struct> res;
	for (int i = 0; i < vec.size(); i++) {
	  if (str_compare(vec[i].key, k) != 0) {
		res.push_back({ vec[i].key, vec[i].value });
	  }
	}
	for (int i = 0; i < res.size(); i++) {
	  memcpy(ctxt->as->storage[i].key, res[i].key, strlen((char*)res[i].key) + 1);
	  memcpy(ctxt->as->storage[i].value, res[i].value, strlen((char*)res[i].value) + 1);
	}
	memcpy(ctxt->as->storage_size, uint642charx(res.size()), strlen((char*)uint642charx(res.size())) + 1);
  }
  else {
	std::vector<storage_struct> res(ctxt->as->storage, ctxt->as->storage + charx2uint64(ctxt->as->storage_size));
	bool flag = true;
	for (int i = 0; i < res.size(); i++) {
	  if (strcmp((char*)res[i].key, (char*)k) == 0) {
		res[i] = { k, v };
		flag = false;
		break;
	  }
	}
	if (flag) {
	  res.push_back({ k,v });
	}
	for (int i = 0; i < res.size(); i++) {
	  memcpy(ctxt->as->storage[i].key, res[i].key, strlen((char*)res[i].key) + 1);
	  memcpy(ctxt->as->storage[i].value, res[i].value, strlen((char*)res[i].value) + 1);
	}
	memcpy(ctxt->as->storage_size, uint642charx(res.size()), strlen((char*)uint642charx(res.size())) + 1);
  }
}

void address(Context* ctxt) {
  ctxt->s.push(ctxt->as->account_address);
}

void balance(Context* ctxt, gs_struct* _gs) {
  unsigned char* addr = pop_addr(ctxt->s.pop());

  account_struct* acc = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (str_compare(addr, (_gs->accounts + i)->address) == 0) {
	  acc = &((_gs->accounts + i)->account);
	  break;
	}
  }

  if (!acc) {
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, {}, 0);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

	acc = &_gs->accounts[charx2uint64(_gs->accounts_size) - 1].account;
  }

  ctxt->s.push(acc->account_balance);
}

void origin(Context* ctxt, tx_struct* _tx) {
  ctxt->s.push(_tx->origin);
}

void caller(Context* ctxt) {
  ctxt->s.push(ctxt->caller);
}

void callvalue(Context* ctxt) {
  ctxt->s.push(ctxt->call_value);
}

void calldatasize(Context* ctxt) {
  ctxt->s.push(uint642charx(ctxt->input.size()));
}

void calldatacopy(Context* ctxt) {
  copy_mem(ctxt->mem, ctxt->input, 0, ctxt);
}

void selfdestruct(std::vector<Context>* ctxts, Context** ctxt, exec_result_struct* result, gs_struct* _gs, tx_struct* _tx) {
  unsigned char* addr = pop_addr((*ctxt)->s.pop());

  account_struct* recipient = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (str_compare(addr, (_gs->accounts + i)->address) == 0) {
	  recipient = &((_gs->accounts + i)->account);
	  break;
	}
  }
  if (!recipient) {
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, {}, 0);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

	recipient = &_gs->accounts[charx2uint64(_gs->accounts_size) - 1].account;
  }

  a_pay_b((*ctxt)->as, recipient, (*ctxt)->as->account_balance);

  memcpy(*(_tx->selfdestruct_list + charx2uint64(_tx->selfdestruct_list_size)), (*ctxt)->as->account_address, strlen((char*)(*ctxt)->as->account_address) + 1);
  memcpy(_tx->selfdestruct_list_size, uint642charx(charx2uint64(_tx->selfdestruct_list_size) + 1), strlen((char*)uint642charx(charx2uint64(_tx->selfdestruct_list_size) + 1)) + 1);

  stop(ctxts, ctxt, result);
}

void create(std::vector<Context>* ctxts, Context* ctxt, gs_struct* _gs) {
  unsigned char* contractValue = ctxt->s.pop();
  const uint64_t offset = charx2uint64(ctxt->s.pop());
  const uint64_t size = charx2uint64(ctxt->s.pop());
  std::vector<uint8_t> initCode = copy_from_mem(offset, size, &ctxt->mem);

  unsigned char* newAddress = new unsigned char[100];
  memset(newAddress, 0, sizeof(unsigned char) * 100);
  generate_address_ocall(ctxt->as->account_address, charx2uint64(ctxt->as->account_nonce), newAddress);

  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, newAddress, strlen((char*)newAddress) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, newAddress, strlen((char*)newAddress) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, contractValue, strlen((char*)contractValue) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, {}, 0);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
  memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
  memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

  account_struct* newAcc = &_gs->accounts[charx2uint64(_gs->accounts_size) - 1].account;

  a_pay_b(ctxt->as, newAcc, contractValue);

  Context* parentContext = ctxt;
  auto rh = [newAcc, parentContext](std::vector<uint8_t> output) {
	memcpy(newAcc->account_code, &output[0], sizeof(uint8_t) * output.size());
	memcpy(newAcc->account_code_size, uint642charx(output.size()), strlen((char*)uint642charx(output.size())) + 1);
	parentContext->s.push(newAcc->account_address);
  };
  auto hh = [parentContext]() { parentContext->s.push((unsigned char*)"0"); };
  auto eh = [parentContext](unsigned char* _exmsg) { parentContext->s.push((unsigned char*)"0"); };

  push_context(ctxts, &ctxt, ctxt->as->account_address, newAcc, {}, 0, (unsigned char*)"0", Program(initCode), rh, hh, eh);
}

void call(std::vector<Context>* ctxts, Context* ctxt, gs_struct* _gs) {
  const Opcode op = get_op(ctxt->get_pc(), ctxt->prog.code);
  ctxt->s.pop();
  unsigned char* addr = pop_addr(ctxt->s.pop());
  unsigned char* value = const_cast<unsigned char*>(op == DELEGATECALL ? (unsigned char*)"0" : ctxt->s.pop());
  const uint64_t offIn = charx2uint64(ctxt->s.pop());
  const uint64_t sizeIn = charx2uint64(ctxt->s.pop());
  const uint64_t offOut = charx2uint64(ctxt->s.pop());
  const uint64_t sizeOut = charx2uint64(ctxt->s.pop());

  if (str_compare(addr, "1") >= 0 && str_compare(addr, "8") <= 0) {
	throw "Precompiled contracts/native extensions are not implemented";
  }

  account_struct* callee = nullptr;
  for (int i = 0; i < charx2uint64(_gs->accounts_size); i++) {
	if (str_compare(addr, (_gs->accounts + i)->address) == 0) {
	  callee = &(((_gs->accounts + i))->account);
	  break;
	}
  }
  if (!callee) {
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_address, addr, strlen((char*)addr) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_balance, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code, {}, 0);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_code_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.account_nonce, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts[charx2uint64(_gs->accounts_size)].account.storage_size, uint642charx(0), strlen((char*)uint642charx(0)) + 1);
	memcpy(_gs->accounts_size, uint642charx(charx2uint64(_gs->accounts_size) + 1), strlen((char*)uint642charx(charx2uint64(_gs->accounts_size) + 1)) + 1);

	callee = &_gs->accounts[charx2uint64(_gs->accounts_size) - 1].account;
  }

  a_pay_b(ctxt->as, callee, value);

  if (str_compare(callee->account_code_size, "0") == 0) {
	ctxt->s.push((unsigned char*)"1");
	return;
  }
  
  prepare_mem_access(offOut, sizeOut, &ctxt->mem);
  std::vector<uint8_t> input = copy_from_mem(offIn, sizeIn, &ctxt->mem);

  Context* parentContext = ctxt;
  auto rh = [offOut, sizeOut, parentContext](const std::vector<uint8_t>& output) {
	copy_mem_raw(offOut, 0, sizeOut, parentContext->mem, output);
	parentContext->s.push((unsigned char*)"1");
  };
  auto hh = [parentContext]() { parentContext->s.push((unsigned char*)"1"); };
  auto eh = [parentContext](unsigned char* _exmsg) { parentContext->s.push((unsigned char*)"0"); };

  switch (op)
  {
  case Opcode::CALL:
	push_context(ctxts, &ctxt, ctxt->as->account_address, callee, ctxt->input, value, Program(callee->account_code, charx2uint64(callee->account_code_size)), rh, hh, eh);
	break;
  case Opcode::CALLCODE:
	push_context(ctxts, &ctxt, ctxt->as->account_address, ctxt->as, ctxt->input, value, Program(callee->account_code, charx2uint64(callee->account_code_size)), rh, hh, eh);
	break;
  case Opcode::DELEGATECALL:
	push_context(ctxts, &ctxt, ctxt->caller, ctxt->as, ctxt->input, ctxt->call_value, Program(callee->account_code, charx2uint64(callee->account_code_size)), rh, hh, eh);
	break;
  default:
	throw "Unknown call opcode";
  }
}

void jumpdest(Context* ctxt) {}

void number(Context* ctxt, gs_struct* _gs) {
  ctxt->s.push(uint642charx(_gs->currentBlock.number));
}

void gasprice(Context* ctxt, tx_struct* _tx) {
  ctxt->s.push(uint642charx(_tx->gas_price));
}

void coinbase(Context* ctxt, gs_struct* _gs) {
  ctxt->s.push(_gs->currentBlock.coinbase);
}

void timestamp(Context* ctxt, gs_struct* _gs) {
  ctxt->s.push(uint642charx(_gs->currentBlock.timestamp));
}

void difficulty(Context* ctxt, gs_struct* _gs) {
  ctxt->s.push(uint642charx(_gs->currentBlock.difficulty));
}

void gaslimit(Context* ctxt, gs_struct* _gs) {
  ctxt->s.push(uint642charx(_gs->currentBlock.gas_limit));
}

void gas(Context* ctxt, tx_struct* _tx) {
  ctxt->s.push(uint642charx(_tx->gas_limit));
}

void add(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	ctxt->s.push(myBigInt::add(x, y));
}

void addmod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();
  unsigned char* m = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	addmod_ocall(x, y, m, res);
	ctxt->s.push(res);
}

void and_(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	ctxt->s.push(myBigInt::and_(x, y));
}

void blockhash(Context* ctxt, gs_struct* _gs) {
  unsigned char* i = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	blockhash_ocall(i, res);
	ctxt->s.push(res);
}

void byte(Context* ctxt) {
  unsigned char* idx = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	byte_ocall(idx, ctxt->s.pop(), res);
	ctxt->s.push(res);
}

void calldataload(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint64_t sizeInput = ctxt->input.size();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	calldataload_ocall(offset, sizeInput, vector2arr(ctxt->input), ctxt->input.size(), res);
	ctxt->s.push(res);
}

void div(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	div_ocall(x, y, res);
	ctxt->s.push(res);
}

void eq(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (str_compare(x, y) == 0) {
	  ctxt->s.push((unsigned char*)"1");
	}
	else {
	  ctxt->s.push((unsigned char*)"0");
	}
}

void exp(Context* ctxt) {
  unsigned char* b = ctxt->s.pop();
  unsigned char* e = ctxt->s.pop();

	ctxt->s.push(myBigInt::exp(b, e));
}

void gt(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (str_compare(x, y) > 0) {
	  ctxt->s.push((unsigned char*)"1");
	}
	else {
	  ctxt->s.push((unsigned char*)"0");
	}
}

void isZero(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();

	if (str_compare(x, "0") == 0) {
	  ctxt->s.push((unsigned char*)"1");
	}
	else {
	  ctxt->s.push((unsigned char*)"0");
	}
}

void lt(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (str_compare(x, y) < 0) {
	  ctxt->s.push((unsigned char*)"1");
	}
	else {
	  ctxt->s.push((unsigned char*)"0");
	}
}

void mload(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  prepare_mem_access(offset, 32u, &ctxt->mem);
  
	const auto start = ctxt->mem.data() + offset;
	ctxt->s.push(myBigInt::from_big_endian(start, 32u));
} 

void mod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	mod_ocall(x, y, res);
	ctxt->s.push(res);
}

void mstore(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  unsigned char* word = ctxt->s.pop();
  prepare_mem_access(offset, 32u, &ctxt->mem);

  myBigInt::to_big_endian(word, ctxt->mem.data() + offset);
}

void mstore8(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint8_t b = 0;

  mstore8_ocall(&b, ctxt->s.pop());

  prepare_mem_access(offset, sizeof(b), &ctxt->mem);
  ctxt->mem[offset] = b;
}

void mul(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	ctxt->s.push(myBigInt::mul(x, y));
}

void mulmod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();
  unsigned char* m = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	mulmod_ocall(x, y, m, res);
	ctxt->s.push(res);
}

void not_(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();

	ctxt->s.push(myBigInt::not_(x));
}

void or_(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	ctxt->s.push(myBigInt::or_(x, y));
}

void push(Context* ctxt) {
  uint8_t bytes = get_op(ctxt->get_pc(), ctxt->prog.code) - PUSH1 + 1;
  uint64_t end = ctxt->get_pc() + bytes;
  if (end < ctxt->get_pc()) {
	throw "Integer overflow in push";
  }
  if (end >= ctxt->prog.code.size()) {
	throw "Push immediate exceeds size of program";
  }

  uint64_t _pc = ctxt->get_pc() + 1;
  unsigned char* imm = new unsigned char[100];
  memset(imm, 0, sizeof(char) * 100);

	imm[0] = '0';
	for (int i = 0; i < bytes; i++) {
	  imm = myBigInt::or_(myBigInt::leftmove(imm, 8), uint642charx(ctxt->prog.code[_pc]));
	  _pc += 1;
	}

  ctxt->s.push(imm);
  ctxt->set_pc(_pc);
}

void sdiv(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	sdiv_ocall(x, y, res);
	ctxt->s.push(res);
}

void sha3(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint64_t size = charx2uint64(ctxt->s.pop());
  prepare_mem_access(offset, size, &ctxt->mem);

  uint8_t h[32];
  keccak_256(ctxt->mem.data() + offset, static_cast<unsigned int>(size), h);
  
	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	sha3_ocall(h, res);
	ctxt->s.push(res);
}

void shl(Context* ctxt) {
  unsigned char* arg1 = ctxt->s.pop();
  unsigned char* arg2 = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	shl_ocall(arg1, arg2, res);
	ctxt->s.push(res);
}

void shr(Context* ctxt) {
  unsigned char* arg1 = ctxt->s.pop();
  unsigned char* arg2 = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	shr_ocall(arg1, arg2, res);
	ctxt->s.push(res);
}

void signextend(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	signextend_ocall(x, y, res);
	ctxt->s.push(res);
}

void slt(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	slt_ocall(x, y, res);
	ctxt->s.push(res);
}

void smod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	smod_ocall(x, y, res);
	ctxt->s.push(res);
}

void sub(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	ctxt->s.push(myBigInt::sub(x, y));
}

void xor_(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	xor_ocall(x, y, res);
	ctxt->s.push(res);
}

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