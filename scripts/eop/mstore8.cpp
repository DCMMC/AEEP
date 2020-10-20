void mstore8(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint8_t b = 0;

	unsigned char* bb = myBigInt::and_(ctxt->s.pop(), (unsigned char*)"255");
	int end = 0;
	while (*(bb + end) != 0) {
	  b *= 10;
	  b += static_cast<uint8_t>(*(bb + end) - '0');
	  end += 1;
	}

  prepare_mem_access(offset, sizeof(b), &ctxt->mem);
  ctxt->mem[offset] = b;
}