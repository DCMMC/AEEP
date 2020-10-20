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