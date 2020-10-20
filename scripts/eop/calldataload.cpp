void calldataload(Context* ctxt) {
  uint64_t offset = charx2uint64(ctxt->s.pop());
  uint64_t sizeInput = ctxt->input.size();

	unsigned char* v = (unsigned char*)"0";
	for (uint8_t i = 0; i < 32u; i++)
	{
	  const auto j = offset + i;
	  if (j < sizeInput)
	  {
		v = myBigInt::add(myBigInt::leftmove(v, 8), uint642charx(ctxt->input[j]));  // ????????&??????????????Ա????nt?ʽ??0x01&???õ??????Ķ?????
	  }
	  else
	  {
		v = myBigInt::leftmove(v, 8 * (32u - i));
		break;
	  }
	}
	ctxt->s.push(v);
}