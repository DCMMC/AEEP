void byte(Context* ctxt) {
  unsigned char* idx = ctxt->s.pop();

	if (str_compare(idx, "32") >= 0) {
	  ctxt->s.push((unsigned char*)"0");
	}
	else {
	  unsigned char* shift = myBigInt::sub((unsigned char*)"248", myBigInt::mul((unsigned char*)"8", myBigInt::and_(idx, (unsigned char*)"255")));
	  unsigned char* mask = myBigInt::leftmove((unsigned char*)"255", shift);
	  ctxt->s.push(myBigInt::rightmove(myBigInt::and_(ctxt->s.pop(), mask), shift));
	}
}