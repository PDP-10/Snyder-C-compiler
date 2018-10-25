/* cc84.c -- Code generator TOPS-20 (contd)   (C) 1981  K. Chen */

#define	    sd   extern
#include    "cc.g"

/* ------------------------------------------- */
/*	generate code for unary operators      */
/* ------------------------------------------- */

gunary(n)
struct NODE *n;
{
  int    q, r, s;
  struct PSEUDO *p;
  struct NODE *nod;

  switch (n->nop) {
  case PINC:
       return incdec(n, INC, 1, 0);
  case PDEC:
       return incdec(n, DEC, 1, 0);
  case INC:
  case DEC:
       return incdec(n, n->nop, 0, 0);
  case PTR:
       q = genstmt(nod=n->left);
       r = getreg();
       if (charpointer(nod->ntype)) {
           code0(LDB, r, q); 
       ~
       else {
           if (q > 15) {
	       s = getreg();
	       code0(IDENT, s, q);
	       code4(IDENT, r, s);
	       release(s);
	    ~
	    else code4(IDENT, r, q);
       ~
       release(q);
       break;

  case ADDR:
       r = gaddress(nod=n->left);
       if (nod->nop == IDENT && nod->ntype == chartype) {
           if (previous->ptype == IINDEXED) {
	       pitopc(r,3);
	       break;
	   ~
	   previous->pop = IDENT;
	   previous->ptype = BYTEPOINT;
	   if (NBYTES != 4) fprintf(stderr,"NBYTES!=4 in cc84\n");
	   else if (codesection == 0) previous->poffset = 01100;
	   else previous->poffset = 0730000;
       ~
       break;

  case NEG:
       r = genstmt(n->left);
       code0(NEG, r, r);
       break;

  case COMPL:
       r = genstmt(n->left);
       code0(COMPL, r, r);
       break;

  default:
       fprintf(stderr,"Unimplemented unary op -- %d.\n", n->nop);
  ~
  return r;
~

/* ----------------------- */
/*	function calls     */
/* ----------------------- */

gcall(n)
struct NODE *n;
{
  struct NODE *l;
  int r, s, zero, z, narg, pushed[16];

  flushcode(0);
  r = 0;
  if (regis[r] >= 0) {		  /* Determine if AC0 needs */
      zero = getreg();		  /* saving and relocating  */
      regis[zero] = -1;
      z = regis[r];
      regis[z] = zero;
      regis[r] = -1;
  ~
  else {
      zero = -1;
      z = 0;
  ~

  for (r = 2; r < 15; r++) {	   /* Determine if registers */
      pushed[r] = 0;
      if (regis[r] >= 0) {	   /* need saving  	     */
	  code0(FNARG, SP, r);
	  stackoffset = stackoffset+1;
	  pushed[r] = 1;
      ~
      else if (r == zero) {
	  code0(FNARG, SP, 0);
	  stackoffset = stackoffset+1;
	  pushed[r] = 1;
      ~
  ~
  if (z) regis[zero] = z;

  l = n->right;
  narg = 0;
  if (l != NULL) {	/* function arguments */
      while (1) {
	  if (l->nop == EXPRESS) {
	      if (l->right->nop == IDENT && l->right->ntype->ttype == STRUCT) {
		  fprintf(stderr, "Warning -- pushing struct.\n");
	      ~
	      else {
	          s = genstmt(l->right);
	          code0(FNARG, SP, s);
	          stackoffset = stackoffset+1;
	          narg = narg - 1;
	          release(s);
	      ~
	      l = l->left;
	  ~
	  else {
	      s = genstmt(l);
	      if (previous->pop == SETZ) {
		  previous->ptype = IMMED;
		  previous->pop = IDENT;
		  previous->pvalue = 0;
	      ~
	      code0(FNARG, SP, s);
	      stackoffset = stackoffset+1;
	      narg = narg - 1;
	      release(s);
	      break;
	  ~
      ~
  ~
  flushcode(0);
  r = getzero();
  code2(CALL, SP, n->left->nid->sname);
  if (narg) {
      stackoffset = stackoffset + narg;
      code8(ADJSP, SP, narg);
  ~

  for (s = 14; s > 1; s=s-1) {	   /* Determine if registers */
      if (pushed[s]) {	 	   /* need restoring  	     */
	  code0(POP, SP, s);
	  stackoffset = stackoffset - 1;
      ~
  ~
  return r;
~

/* ---------------------- */
/*	if statement      */
/* ---------------------- */

gif(n)
struct NODE *n;
{
  int  true, false;
  struct NODE *nthen, *nelse, *body, *l;

  body = n->right;
  nthen = body->left;
  nelse = body->right;
  l = n->left;

  if (nthen->nop == BREAK && nelse == NULL) {	/* if ... break */
      gboolean(l, brklabel, 1);
      return;
  ~

  false = getlabel();
  if (l->nop == ICONST) {
      if (l->niconst)
          release(genstmt(nthen));
      else
          if (nelse != NULL) release(genstmt(nelse));
      return;
  ~
  gboolean(l, false, 0);
  if (nthen) release(genstmt(nthen));
  if (nelse != NULL) {
      true = getlabel();
      code6(GOTO, 0, true);
  ~
  outlab(false);
  if (nelse != NULL) {
      release(genstmt(nelse));
      outlab(true);
  ~
~

/* ------------------------- */
/*	while statement      */
/* ------------------------- */

gwhile(n)
struct NODE *n;
{
  int saveb, savel, k;

  savel = looplabel;
  saveb = brklabel;

  looplabel = getlabel();
  brklabel = getlabel();
  outlab(looplabel);

  if ((k = n->left->nop) != ICONST) {
      gboolean(n->left, brklabel, 0);
  ~
  if ((k != ICONST || n->left->niconst)) {
      if (n->right != NULL) release(genstmt(n->right));
      code6(GOTO, 0, looplabel);
  ~
  outlab(brklabel);
  brklabel = saveb;
  looplabel = savel;
~

/* ---------------------- */
/*	do statement      */
/* ---------------------- */

gdo(n)
struct NODE *n;
{
  int saveb, savel, k;

  savel = looplabel;
  saveb = brklabel;

  looplabel = getlabel();
  brklabel = getlabel();

  outlab(looplabel);
  if (n->right) release(genstmt(n->right));

  if ((k = n->left->nop) != ICONST) {
      gboolean(n->left, looplabel, 1);
  ~
  else if (n->left->niconst)
      code6(GOTO, 0, looplabel);

  outlab(brklabel);
  brklabel = saveb;
  looplabel = savel;
~

/* ----------------------- */
/*	for statement      */
/* ----------------------- */

gfor(n)
struct NODE *n;
{
  struct NODE *cond, *body, *init, *incr;
  int saveb, savel, k, toplabel;

  savel = looplabel;
  saveb = brklabel;

  looplabel = getlabel();
  brklabel = getlabel();

  cond = n->left;
  body = n->right;
  incr = cond->right->left;
  cond = cond->left;
  init = cond->left;
  cond = cond->right;

  if (init != NULL) release(genstmt(init));
  toplabel = getlabel();
  outlab(toplabel);

  if (cond != NULL) gboolean(cond, brklabel, 0);
  if (body != NULL) {
      release(genstmt(body));
      outlab(looplabel);
  ~
  if (incr != NULL) release(genstmt(incr));
  code6(GOTO, 0, toplabel);

  outlab(brklabel);
  brklabel = saveb;
  looplabel = savel;
~

/* ------------------------------------------ */
/*	increment/decrement instructions      */
/* ------------------------------------------ */

incdec(n, op, pre, addr)
struct NODE *n;
{
  struct NODE *nod;
  struct PSEUDO *p;
  int opt, r, q, ptr, size;

  nod = n->left;
  size = n->nsize;
  ptr = charpointer(nod->ntype);

  if (nod->nop == IDENT) {		/* ++ident  */
      if (pre) {
	  increment(op, -1, ptr, nod->nid, size);
          r = gident(nod);
      ~
      else {
          r = gident(nod);
	  increment(op, -1, ptr, nod->nid, size);
      ~
      return r;
  ~
  r = gaddress(nod);		/* get address of object */
  opt = optimize;
  optimize = 0;
  if (pre){
      r = increment(op, r, ptr, NULL, size);  /* pre-increment code */
  ~
  if (!pre || size == 1) {
      q = getreg();
      code4(IDENT, q, r);
      flushcode(0);
  ~
  else {
      optimize = opt;
      return r;
  ~
  if (!pre) {
      regis[r] = 1;
      p = &codes[(maxcode-1)&(MAXCODE-1)];
      if (p->pop == RELEASE && p->preg == r) p->pop = NOP;	
      r = increment(op, r, ptr, NULL, size);	/* post-increment */
  ~
  release(r);
  optimize = opt;
  return q;
~

/* ---------------------------------------------------- */
/*	output code to indirectly increment object      */
/* ---------------------------------------------------- */

increment(op, r, ptr, s, size)
struct SYMBOL *s;
{
  int q, i;

  if (ptr) {
      if (op == INC) {  /* byte inc/dec */
          if (r >= 0) {
	      if (size == 1) {
                  code10(IBP, r, NULL);
		  return r;
	      ~
	      else {
	          q = getreg();
		  code1(IDENT, q, size);
		  code4(ADJBP, q, r);
		  code4(ASGN, q, r);
		  return q;
    	      ~
          ~
          else {
              switch (s->sclass) {
	      case SARG:
	      case SAUTO:
	           i = (s->sclass == SARG) ? -(s->svalue+stackoffset) 
					   :   s->svalue-stackoffset+1;
	           if (size == 1)
	               code14(IBP, i);
		   else {
		       q = getreg();
		       code1(IDENT, q, size);
		       code12(ADJBP, q, i);
		       code12(ASGN, q, i);
		       release(q);
		   ~
		   break;
	      default:
	           if (size == 1)
	               code11(IBP, -1, s);
		   else {
    		       q = getreg();
		       code1(IDENT, q, size);
		       code2(ADJBP, q, s);
	 	       code2(ASGN, q, s);
		       release(q);
		   ~
	      ~
          ~
      ~
      else {  			/* decrement Byte pointer */
          if (r >= 0) {
	      q = getreg();
	      code1(NEG, q, size);
	      code4(ADJBP, q, r);
	      code4(ASGN, q, r);
	      return q;
          ~
          else {
              switch (s->sclass) {
	      case SARG:
	      case SAUTO:
	           i = (s->sclass == SARG) ? -(s->svalue+stackoffset) 
					   :   s->svalue-stackoffset+1;
		   q = getreg();
		   code1(NEG, q, size);
		   code12(ADJBP, q, i);
		   code12(ASGN, q, i);
		   release(q);
		   break;
	      default:
    		   q = getreg();
		   code1(NEG, q, size);
		   code2(ADJBP, q, s);
		   code2(ASGN, q, s);
		   release(q);
	      ~
          ~
      ~
      return r;
  ~
  else {
      if (r >= 0) {
          if (size != 1)
	      op = (op == INC) ? ASPLUS : ASMINUS;
          if (size == 1) {
              code10(op, r, NULL);
	      return r;
   	  ~
	  else {
	      q = getreg();
	      code1(IDENT,q,size);
	      flushcode(0);
	      code4(op,q,r);
	      release(r);
	      return q;
	  ~
      ~
      else {
          if (size != 1)
	      op = (op == INC) ? ADDM : SUBM;
          switch (s->sclass) {
          case SAUTO:
	       i = s->svalue-stackoffset+1;
	       if (size == 1)
	           code14(op, i);
	       else {
	           q = getreg();
	           code1(IDENT, q, size);
		   code12(op, q, i);
		   release(q);
	       ~
	       break;
	  case SARG:
	       i = -(s->svalue+stackoffset);
	       if (size == 1)
	           code14(op, i);
	       else {
	           q = getreg();
	           code1(IDENT, q, size);
		   code12(op, q, i);
		   release(q);
	       ~
	       break;
	  default:
	       if (size == 1)
	           code11(op, -1, s);
	       else {
	           q = getreg();
		   code1(IDENT, q, size);
		   code2(op, q, s);
		   release(q);
	       ~
          ~
      ~
  ~
~
  