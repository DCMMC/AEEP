void mstore(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  unsigned char* word = ctxt->s.pop();
  prepare_mem_access(offset, 32u, &ctxt->mem);

  myBigInt::to_big_endian(word, ctxt->mem.data() + offset);
}