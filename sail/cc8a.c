/* cc8A.c -- Code generator TOPS-20 (contd)   (C) 1981  K. Chen */

#define	    sd   extern
#include    "cc.g"

/* ---------------------------- */
/*	generate real code      */
/* ---------------------------- */

realcode(n)
{
  struct PSEUDO *p;
  char *op, lstr[10], *t;
  int  opr, typ, i, big;

  p = &codes[n&(MAXCODE-1)];

  if ((opr = p->pop) >= 1000) {
      switch (opr) {
      case RELEASE:
           if (p->preg < 20) {
	       if (p->preg >= 16) {
	           i = regis[p->preg];
	           if (i >= 0) {
		       regis[i] = -1;
		       regis[p->preg] = -1;
	           ~
	       ~
	       else regis[p->preg] = -1;
	   ~
	   p->pop = CODED;
	   break;
      case GLABEL:
	   outsym(p->plabel);
	   op = (p->plabel[0] == '$' && FAIL) ? "::\n" : ":\n";
	   outstr(op);
	   break;
      case CLABEL:
	   if (codesection == 0) outstr("\t$");
	   else {
	       outstr("\t");
	       outnum(codesection);
	       outstr(",,$");
	   ~
	   outnum(p->pvalue);
	   outc('\n');
	   break;
      case CVALUE:
           outstr("\t");
	   outnum(p->pvalue);
	   outc('\n');
	   break;
      case CINIT:
           outstr("\tMOVE\t2,$");
	   outnum(p->pvalue);
	   outc('\n');
	   break;
      ~
      return 1;
  ~

  big = 0;
  typ = p->ptype;

  if (typ == RCONST) {
      switch (opr) {
      case ADJSP:
      case LSHFT:
      case RSHFT:
           break;
      default:
           if (p->pvalue > 0777777 || p->pvalue < 0)
               opr = directop(opr);
      ~
  ~
  if (typ == IINDEXED && p->pop == FNARG) {
       if (codesection != 0) outstr("\tXMOVEI");
       else outstr("\tMOVEI");
       outreg(14);
       if (i = p->poffset) outnum(i);
       outindex(p->pindex);
       outstr("\n");
  ~

#ifdef NOADJBP
  if (opr == ADJBP) {
      switch (typ) {
      case REGIS:
           i = p->pr1;
	   break;
      case GLOBAL:
      case INDEXED:
      case BYTEPOINT:
           i = p->preg;
	   break;
      default:
	   printf("\nUnimplemented ADJBP!\n");
	   return 0;
      ~
      outstr("\tMOVE\t16,");
      oreg(i);
      outstr("\n\tASH");
      outreg(i);
      outstr("-2\n\tADD");
      outreg(i);

      switch (typ) {
      case REGIS:
	   oreg(p->pr2);
           break;

      case GLOBAL:
           if (*p->pptr->sname) {
	       outsym(p->pptr->sname); /* not in non-zero sections! */
               outoffset(p);
           ~
           else outnum(p->poffset);
           outindex(p->pindex);
           break;

      case INDEXED:
           if (p->poffset) outnum(p->poffset);
           outindex(p->pindex);
	   break;

      case BYTEPOINT:
           outc('[');
           outnum(p->poffset);
           outstr(",,");
           outsym(p->pptr->sname); /* not in non-zero sections */
           outc(']');
           break;
      ~
      outstr("\n\tANDI\t16,3\n\tSUB");
      outreg(i);
      outstr("$BYTE(16)\n\tSKIPGE\t,");
      oreg(i);
      outstr("\n\tADD");
      outreg(i);
      outstr("[440000,,000001]\n");
      return 0;
  ~
#endif

  if (opr == ADJBP) {
      i = p->preg;
      if (i > 15) i = regis[i];
      if (i == 0) {
          outstr("\tMOVE\t16,0\n");
	  p->preg = 14;
      ~
  ~
  tab();
  if (codesection != 0 &&
     (typ == IINDEXED || typ == GADDR) &&
      opr == IDENT && p->pop != FNARG)outstr("XMOVE");	/* use XMOVEI */
  else outstr(dec20op[opr]);

  switch (typ) {
  case IMMED:
       if (opr == FNARG) {	/* DEC-20 operators with no immediate fields */
           outreg(p->preg);
	   outc('[');
           outnum(p->pvalue);
	   outc(']');
  	   break;
       ~
       if (p->pvalue > 0777777 || p->pvalue < 0)
           big = 1;
       else
           outc('I');

       outreg(p->preg);
       if (big) {
	   outc('[');
	   outnum(p->pvalue);
           outc(']');
       ~
       else outnum(p->pvalue);
       break;

  case RCONST:
       switch (opr) {
       case LSHFT:
       case RSHFT:
       case ADJSP:
            break;
       default:
            big = (p->pvalue > 0777777 || p->pvalue < 0);
       ~
       outreg(p->preg);
       if (big) {
	   outc('[');
	   outnum(p->pvalue);
           outc(']');
       ~
       else outnum(p->pvalue);
       break;

  case BYTEPOINT:
       outreg(p->preg);
       outc('[');
       outnum(p->poffset);
       outstr(",,");
       outsym(p->pptr->sname);
       outc(']');
       break;

  case ISINGLE:
       tab();
       if (p->preg >= 0) {
	   outc('(');
           oreg(p->preg);
	   outc(')');
           break;
       ~
       outc('@');
       outsym(p->pptr->sname);
       outoffset(p);
       break;

  case SINGLE:
       tab();
       if (p->preg >= 0) {
           oreg(p->preg);
           break;
       ~
       if (p->pptr != NULL) {
	   outsym(p->pptr->sname);
	   outoffset(p);
       ~
       else if (p->poffset) outnum(p->poffset);
       outindex(p->pindex);
       break;

  case GADDR:
       if (opr == FNARG) {	/* DEC-20 operators with no immediate fields */
           outreg(p->preg);
	   outc('[');
	   if (codesection != 0) {
	       outnum(codesection);
	       outstr(",,");
	   ~
           outsym(p->pptr->sname);
	   outc(']');
  	   break;
       ~
       outc('I');
       outreg(p->preg);
       if (*p->pptr->sname) {
	   outsym(p->pptr->sname);
           outoffset(p);
       ~
       else outnum(p->poffset);
       break;

  case REGIS:
       outreg(p->pr1);
       oreg(p->pr2);
       break;

  case ONEREG:
       outreg(p->pr1);
       break;

  case IINDEXED:
       if (p->pop == FNARG) {
           outreg(SP);
	   outnum(14);
           break;
       ~
       outc('I');
       outreg(p->preg);
       if (i = p->poffset) outnum(i);
       outindex(p->pindex);
       break;

  case INDEXED:
       outreg(p->preg);
       if (i = p->poffset) outnum(i);
       outindex(p->pindex);
       break;

  case BOTH:
       outc('B');
  case GLOBAL:
  case MINDEXED:
       outreg(p->preg);
       if (p->pptr) {
	   outsym(p->pptr->sname);
           outoffset(p);
       ~
       else outnum(p->poffset);
       outindex(p->pindex);
       break;

  case BLABEL:
       outstr("\t@$");
       outnum(p->poffset);
       outindex(p->pindex);
       break;

  case DLABEL:
       outreg(p->preg);
       outc('$');
       outnum(p->poffset);
       outindex(p->pindex);
       break;

  case INDIRECT:
       outreg(p->pr1);
       outc('@');
       oreg(p->pr2);
       break;

  case MINDIRECT:
       outreg(p->preg);
       outc('@');
       if (*p->pptr->sname) {
	   outsym(p->pptr->sname);
           outoffset(p);
       ~
       else outnum(p->poffset);
       outindex(p->pindex);
       break;

  case LAB:
       if (opr != GOTO) {
           outreg(p->preg);
       ~
       else tab();
       maklabel(lstr, p->pvalue);
       outsym(lstr);
       break;

  case BR:
       outreg(p->preg);
       if (p->pvalue >= 0) {
	   opr = p->pvalue;
	   op = ".+";
       ~
       else {
	   opr = -p->pvalue;
	   op = ".-";
       ~
       outstr(op);
       outnum(opr);
       break;
  ~
  nl();
  return 0;
~

outreg(n)
{
  putc('\t', out);
  oreg(n);
  putc(',', out);
~

oreg(n)
{
  if (n > 15) n = regis[n];
  outnum(n);
~

outindex(n)
{
  if (n) {
      outc('(');
      outnum(n);
      outc(')');
  ~
~

outoffset(p)
struct PSEUDO *p;
{
 int i; 

 if (i = (p->poffset & 0777777)) {
     if (i > 0) outc('+');
     outnum(i);
 ~
~

tab()   { putc('\t', out);~
comma() { putc(',',  out);~
nl()    { putc('\n', out);~

