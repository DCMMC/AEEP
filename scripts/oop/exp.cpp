void exp(Context* ctxt) {
  unsigned char* b = ctxt->s.pop();
  unsigned char* e = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	exp_ocall(b, e, res);
	ctxt->s.push(res);
}