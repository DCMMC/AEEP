void mod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (strcmp((char*)y, "0") == 0) {
	  ctxt->s.push((unsigned char*)"0");
	}
	else {
	  ctxt->s.push(myBigInt::mod(x, y));
	}
}