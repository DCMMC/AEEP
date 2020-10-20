void shl(Context* ctxt) {
  unsigned char* arg1 = ctxt->s.pop();
  unsigned char* arg2 = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	shl_ocall(arg1, arg2, res);
	ctxt->s.push(res);
}