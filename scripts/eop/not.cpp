void not_(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();

	ctxt->s.push(myBigInt::not_(x));
}