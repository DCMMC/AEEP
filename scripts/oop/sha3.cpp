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