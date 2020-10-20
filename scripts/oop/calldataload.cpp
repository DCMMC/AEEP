void calldataload(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint64_t sizeInput = ctxt->input.size();

	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	calldataload_ocall(offset, sizeInput, vector2arr(ctxt->input), ctxt->input.size(), res);
	ctxt->s.push(res);
}