void exp(Context* ctxt) {
  unsigned char* b = ctxt->s.pop();
  unsigned char* e = ctxt->s.pop();

	ctxt->s.push(myBigInt::exp(b, e));
}