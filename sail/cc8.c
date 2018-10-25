/* cc8.c -- Code generator TOPS-20   (C) 1981  K. Chen */

#define	    sd
#include    "cc.g"

/* ------------------------------------------------------------ */
/*	    code generation (KL-10, FAIL)	   	        */
/*						   		*/
/*    char is stored as right justified character    		*/
/*    char[] is stored as 9 bit bytes, always word aligned	*/
/*    *char is stored as a byte pointer		   		*/
/*    *int is stored as an address in the right half 		*/
/*								*/
/*	An activation record looks like:			*/
/*								*/
/*		argument n		(low memory)		*/
/*		argument n-1					*/
/*		...						*/
/*		argument 1					*/
/*		return address  	<- SP at entry		*/
/*		local 1						*/
/*		...						*/
/*		local m			<- SP at start of func	*/
/*								*/
/*	Function return is through AC0 (and AC1, if needed)	*/
/*								*/
/*	Calling routine looks like:				*/
/*								*/
/*		PUSH	17,arg_n				*/
/*		PUSH	17,arg_n-1				*/
/*		...						*/
/*		PUSH	17,arg_1				*/
/*		PUSHJ	17,function				*/
/*		ADJSP	17,-n					*/
/*								*/
/*	Register 17 (octal) used for stack manipulation,	*/
/*	Registers 2 through 16 (octal) may be used by callee.	*/
/* ------------------------------------------------------------ */

gencode(n)
struct NODE *n;
{
  struct SYMBOL *s;
  int  scl;

  if (eflag) {
      maxnode = 0;
      return;	/* generate no further code upon error */
  }

  switch (n->nop) {
  case DATA:
       s = n->right->left->nid;
       scl = s->sclass;
       if (scl == SEXTERN && s->svalue == 1) scl = XTERN;
       switch (scl) {
       case XTERN:
       case SSTATIC:
	    gendata(n); 
	    break;
       case SAUTO:
            if (n->right->right) gendata(n);
	    break;
       }
       break;
  case FUNCTION:
       genbody(n);
  }
  maxnode = 0;
  return;
}

/* ----------------------------- */
/*	assembler preambles      */
/* ----------------------------- */

preamble()
{

  outstr("\tTITLE\t");
  outstr(module); 
  nl();
  if (codesection != 0) outstr("\tOPDEF\tXMOVEI [SETMI]\n");
#ifdef USEIBP
  outstr("\tOPDEF\tADJBP [IBP]\n");
#endif
  outstr("\t.LIBRARY ");
  outstr(libprefix);
  if (codesection != 0) outstr("XCLIB");
  else outstr("CLIB");
  outstr(libpstfix);
  nl();
  outstr("\tEXTERN\t.START\n");
}

/* ------------------------------ */
/*	assembler postambles      */
/* ------------------------------ */

postamble()
{
  int t, u, l;

#ifdef NOADJBP
  outstr("$BYTE:\tOCT\t0,110000000000,220000000000,330000000000\n");
#endif

  gdecl();
  outstr("\n\tEND\n");
  t = (_cputm() - time)/10;
  l = tline * 6000 / t;
  u = t%100;
  if (ncond) {
      fprintf(stderr,"Unterminated compiler control line seen.\n\n");
  }
  fprintf(stderr,"Processed %d lines in %d.%d%d seconds (%d lines/min.)\n",
		  tline, t/100, u/10, u%10, l);
}

/* ------------------------------------------ */
/*	external & internal declarations      */
/* ------------------------------------------ */

gdecl()
{
  struct NODE *u;
  char *op;
  int  count;

  gdecl0("\tINTERN\t", 1, 1000);
  gdecl0("\tEXTERN\t", 0, 1000);
}

gdecl0(op, val, count)
char *op;
{
  int n;
  struct SYMBOL *s;

  n = 0;
  while(n < maxsym) {
      s = &symbol[n];
      if ((s->sclass == SEXTERN && s->svalue == val) ||
          (s->sclass == SAUTO   && val == 0)) {

          if (count > 48) {
	      nl();
   	      outstr(op);
	      count = 0;
          }
          if (count) putc(',', out);
          count += (outsym(s->sname)+1);
      }
      n++;
  }
  if (count < 50) nl();
}


/* ---------------------------- */
/*	generate code body      */
/* ---------------------------- */

genbody(n)
struct NODE *n;
{
  struct NODE *s;
  int    size;

  previous = NULL;
  for (mincode = 0; mincode < 32; mincode++) regis[mincode] = -1;
  stackoffset = looplabel = brklabel = maxcode = mincode = maxlit = 0;
  glabel(n->left->left->nid->sname);
  if (maxauto) {
      code8(ADJSP, 017, maxauto);
      stackoffset += maxauto;
  }
  while (maxinit--) {
      size = tsize(isym[maxinit]->sptr);
      if (size == 1) {
          code18(vsym[maxinit]);
      }
      else fprintf(stderr,"Cannot init %s.\n", isym[maxinit]->sname);
  }
  s = n->right;
  while (s->nop == STATEMENT) {
      if (s->right != NULL)
          release(genstmt(s->right));
      s = s->left;
  }
  gend();
}

/* ----------------------------------- */
/*	generate data definitions      */
/* ----------------------------------- */

gendata(n)
struct NODE *n;
{
  struct NODE *nod, *r, *l;
  int    state, count, ncount;

  if (n->left != NULL) gendata(n->left);
  r = n->right;
  if (r->nop == IZ) {
      l = r->left;
      if ((r = r->right) == NULL) {
	  if (l->nid->sptr->ttype == FUNCTION) {
	      /* a function declaration, really */
	      l->nid->sclass = SEXTERN;
	      /* l->nid->svalue = 0; */
	      return;
	  }
	  izsym(l->nid);
	  if ((count = tsize(l->nid->sptr)) == 1) {
	      outstr("0");
	      nl();
	  }
	  else {
	      outstr("BLOCK\t");
	      outnum(count);
	      nl();
	  }
      }
      else {
          switch (r->nop) {
	  case IZLIST:
	       izsym(l->nid);
	       giz(r, l->ntype);
	       break;
          case ICONST:
	       izsym(l->nid);
	       outnum(r->niconst);
	       nl();
	       break;
          case SCONST:
	       izsym(l->nid);
	       if (NBYTES != 4) fprintf(stderr,
			"NBYTES!=4 not implemented in code generator\n");
	       else if (codesection == 0) {
		   outstr("POINT\t9,[");
		   asciz(r->nsconst);
		   outstr("],8");
	       }
	       else {
		   outnum(0700000+codesection);
		   outstr(",,[");
		   asciz(r->nsconst);
		   outstr("]");
	       }
	       nl();
	       break;
          default:
		       fprintf(stderr,"Internal inconsistency.\n");
	       exit(0);
          }
      }
  }
}

/* ---------------------------- */
/*	initializer symbol      */
/* ---------------------------- */

izsym(sym)
struct SYMBOL *sym;
{
  char sn[32];

  if (sym->sclass == SAUTO) {
      maklabel(sn,getlabel());
      outsym(sn);
      isym[maxinit] = sym;
      vsym[maxinit++] = maxlabel;
  }
  else outsym(sym->sname);
  outstr(":\t");
}


giz(r, t)
struct NODE *r;
struct TY   *t;
{
  int state, count, ncount, byte;
  char *op;
  struct NODE *nod;

  state = count = 0;
  byte = chararray(t);
  ncount = (byte) ? 4 : 7;
  while (1) {
      nod = r->right;
      switch (nod->nop) {
      case IZLIST:
	   if (state) {
	       nl();
	       tab();
	   }
   	   giz(nod, t->tptr);
   	   state = 0;
	   break;
      case BLOCK:
	   if (state) {
	       nl();
	       tab();
	       state = 0;
	   }
           outstr("BLOCK\t");
	   count = 0;
	   state = 2;
	   outnum(nod->niconst);
	   break;
      case ICONST:
	   if (state >= 2 || count >= ncount || !byte) {
	       nl();
	       tab();
	       state = 0;
	   }
	   if (!state) {
	       if (byte) {
		   if (NBYTES == 5) outstr("BYTE (7)\t");
		   else outstr("BYTE (9)\t");
	       }
	       count = 0;
	   }
	   else {
	       outc(',');
	   }
   	   state = 1;
           outnum(nod->niconst);
	   break;
      case SCONST:
	   if (state != 0) {
	       nl();
	       tab();
	   }
	   if (NBYTES != 4) fprintf(stderr,
			"NBYTES!=4 not implemented in code generator\n");
	   else if (codesection == 0) {
	       outstr("POINT\t9,[");
	       asciz(nod->nsconst);
	       outstr("],8");
	   }
	   else {
	       outnum(0700000+codesection);
	       outstr(",,[");
	       asciz(nod->nsconst);
	       outstr("]");
	   }
	   state = 3;
 	   break;
      }
      count++;
      if ((r = r->left) == NULL) break;
  }
  nl();
}


/* ----------------------------------------- */
/*	output string to assembler file      */
/* ----------------------------------------- */

outstr(s)
char *s;
{
  int n;

  n = 0;
  while (*s) {
      putc(*s++,out);
      n++;
  }
  return n;
}

/* ----------------------------------------- */
/*	output symbol to assembler file      */
/* ----------------------------------------- */

outsym(s)
char *s;
{
  int n;

  n = 0;
  while (*s) {
      putc(((*s == '_') ? '.' : *s), out);
      if (n++ >= 5) break;
      s++;
  }
  return n;
}

/* -------------------------------------------- */
/*	output character to assembler file      */
/* -------------------------------------------- */

outc(n)
{
  putc(n, out);
}

/* ----------------------------- */
/*	tab to second field      */
/* ----------------------------- */

outt(n)
{
  putc(((n < 7) ? '\t' : ' '), out);
}

/* ----------------------------- */
/*	output octal number      */
/* ----------------------------- */

outnum(n)
{
  int k;
  char str[13], *s;

  if (n < 0) {
      n = -n;
      putc('-', out);
  }
  s = str;
  *s = '\0';
  do {
      *++s = (n&07) + '0';
      n = n>>3;
  } while (n);

  while (*s) putc(*s--, out);
}

/* ---------------------- */
/*	asciz string      */
/* ---------------------- */

asciz(s)
char *s;
{
  int k;
  if (NBYTES != 4) fprintf(stderr,
	"NBYTES!=4 not implemented in code generator\n");
  else {
      while (1) {
          outstr("BYTE\t(9) ");
	  if (*s) outnum(*s);
	  else {
	      outnum(0);
	      break;
	  }
	  if (!*s++) break;
	      putc(',', out);
	      outnum(*s);
	  if (!*s++) break;
	      putc(',', out);
	      outnum(*s);
	  if (!*s++) break;
	      putc(',', out);
	      outnum(*s);
	  if (!*s++) break;
	      outstr("\n\t");
      }
      nl();
  }
}
    