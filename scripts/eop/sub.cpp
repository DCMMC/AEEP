void sub(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	ctxt->s.push(myBigInt::sub(x, y));
}