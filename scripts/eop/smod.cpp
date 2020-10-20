void smod(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (strcmp((char*)y, "0") == 0) {
	  ctxt->s.push((unsigned char*)"0");
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
	  unsigned char* z = myBigInt::mod(x, y);
	  if (signX == -1) {
		z = myBigInt::sub((unsigned char*)"0", z);
	  }
	  ctxt->s.push(z);
	}
}