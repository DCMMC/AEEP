void mstore(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  unsigned char* word = ctxt->s.pop();
  prepare_mem_access(offset, 32u, &ctxt->mem);

	to_big_endian_ocall(word, ctxt->mem.data() + offset, ctxt->mem.size() - offset);
}