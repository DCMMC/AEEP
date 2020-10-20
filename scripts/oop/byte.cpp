void byte(Context* ctxt) {
  unsigned char* idx = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	byte_ocall(idx, ctxt->s.pop(), res);
	ctxt->s.push(res);
}