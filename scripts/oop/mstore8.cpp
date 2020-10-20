void mstore8(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint8_t b = 0;

  mstore8_ocall(&b, ctxt->s.pop());

  prepare_mem_access(offset, sizeof(b), &ctxt->mem);
  ctxt->mem[offset] = b;
}