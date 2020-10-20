void mload(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  prepare_mem_access(offset, 32u, &ctxt->mem);
  
	unsigned char* res = new unsigned char[100];
	memset(res, 0, sizeof(unsigned char) * 100);
	from_big_endian_ocall(ctxt->mem.data() + offset, 32u, ctxt->mem.size() - offset, res);
	ctxt->s.push(res);
} 