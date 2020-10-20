void signextend(Context* ctxt) {
  unsigned char* x = ctxt->s.pop();
  unsigned char* y = ctxt->s.pop();

	if (str_compare(x, "32") >= 0) {
	  ctxt->s.push(y);
	}
	else {
	  unsigned char* idx = myBigInt::add(myBigInt::mul((unsigned char*)"8", myBigInt::and_(x, (unsigned char*)myBigInt::MAX)), (unsigned char*)"7");
	  unsigned char* sign = myBigInt::and_(myBigInt::rightmove(y, idx), (unsigned char*)"1");
	  unsigned char* zero = (unsigned char*)"0";
	  unsigned char* mask = myBigInt::rightmove(myBigInt::not_(zero), myBigInt::sub((unsigned char*)"256", idx));
	  unsigned char* yex = myBigInt::or_(myBigInt::leftmove(str_compare(sign, "0") ? myBigInt::not_(zero) : zero, idx), myBigInt::and_(y, mask));
	  ctxt->s.push(yex);
	}
}