void isZero(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();

	if (str_compare(x, "0") == 0) {
	  ctxt->s.push((unsigned char*)"1");
	}
	else {
	  ctxt->s.push((unsigned char*)"0");
	}
}