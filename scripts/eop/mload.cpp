void mload(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  prepare_mem_access(offset, 32u, &ctxt->mem);
  
	const auto start = ctxt->mem.data() + offset;
	ctxt->s.push(myBigInt::from_big_endian(start, 32u));
} 