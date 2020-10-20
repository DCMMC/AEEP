void isZero(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	isZero_ocall(x, res);
	ctxt->s.push(res);
}