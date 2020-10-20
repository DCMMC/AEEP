void eq(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (str_compare(x, y) == 0) {
	  ctxt->s.push((unsigned char*)"1");
	}
	else {
	  ctxt->s.push((unsigned char*)"0");
	}
}