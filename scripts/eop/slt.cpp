void slt(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (str_compare(x, y) == 0) {
	  ctxt->s.push((unsigned char*)"0");
	}
	else {
	  int signX = str_compare(myBigInt::rightmove(x, 255), "0") ? -1 : 1;
	  int signY = str_compare(myBigInt::rightmove(y, 255), "0") ? -1 : 1;
	  if (signX != signY) {
		if (signX == -1) {
		  ctxt->s.push((unsigned char*)"1");
		}
		else {
		  ctxt->s.push((unsigned char*)"0");
		}
	  }
	  else {
		if (str_compare(x, y) < 0) {
		  ctxt->s.push((unsigned char*)"1");
		}
		else {
		  ctxt->s.push((unsigned char*)"0");
		}
	  }
	}
}