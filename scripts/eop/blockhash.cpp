void blockhash(Context* ctxt, gs_struct* _gs) {
  unsigned char* i = ctxt->s.pop();

	if (str_compare(i, "256") >= 0) {
	  ctxt->s.push((unsigned char*)"0");
	}
	else {
	  //ctxt->s.push(gs.get_block_hash(i % 256));
	  ctxt->s.push((unsigned char*)"0");
	}
}