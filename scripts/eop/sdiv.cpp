void sdiv(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	unsigned char* min = myBigInt::add(myBigInt::div((unsigned char*)myBigInt::MAX, (unsigned char*)"2"), (unsigned char*)"1");
	if (strcmp((char*)y, "0") == 0) {
	  ctxt->s.push((unsigned char*)"0");
	}
	else if (strcmp((char*)x, (char*)min) == 0 && strcmp((char*)y, "-1") == 0) {
	  ctxt->s.push(x);
	}
	else {
	  int signX = str_compare(myBigInt::rightmove(x, 255), "0") ? -1 : 1;
	  int signY = str_compare(myBigInt::rightmove(y, 255), "0") ? -1 : 1;
	  if (signX == -1) {
		x = myBigInt::sub((unsigned char*)"0", x);
	  }
	  if (signY == -1) {
		y = myBigInt::sub((unsigned char*)"0", y);
	  }
	  unsigned char* z = myBigInt::div(x, y);
	  if (signX != signY) {
		z = myBigInt::sub((unsigned char*)"0", z);
	  }
	  ctxt->s.push(z);
	}
}