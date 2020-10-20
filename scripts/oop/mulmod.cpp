void mulmod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();
  unsigned char* m = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	mulmod_ocall(x, y, m, res);
	ctxt->s.push(res);
}