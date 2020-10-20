void blockhash(Context* ctxt, gs_struct* _gs) {
  unsigned char* i = ctxt->s.pop();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	blockhash_ocall(i, res);
	ctxt->s.push(res);
}