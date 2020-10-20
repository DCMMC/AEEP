void addmod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();
  unsigned char* m = ctxt->s.pop();

	if (strcmp((char*)m, "0") == 0) {
	  ctxt->s.push((unsigned char*)"0");
	}
	else {
	  unsigned char* n = myBigInt::mod(myBigInt::add(x, y), m);
	  ctxt->s.push(myBigInt::get_lo(n, 256));
	}
}