void shr(Context* ctxt) {
  unsigned char* arg1 = ctxt->s.pop();
  unsigned char* arg2 = ctxt->s.pop();

	if (str_compare(arg1, "256") >= 0) {
	  ctxt->s.push((unsigned char*)"0");
	  return;
	}
	unsigned char* val = myBigInt::rightmove(arg2, arg1);
	ctxt->s.push(val);
}