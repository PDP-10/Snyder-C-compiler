/* ------------------------------------------------------------ */
/*	C Compiler	Phase 1					*/
/*	(c) 1981   	k. chen					*/
/*								*/
/*	started: 20-Mar-81					*/
/*	preprocessor completed: 28-Mar-81			*/
/*	lexical scanner completed: 30-Mar-81			*/
/*	symbol & type tables completed: 30-Mar-81		*/
/*	parser mostly completed: 30-Mar-81			*/
/*	full bootstrapped for TOPS-20: 15-Jun-81		*/
/*								*/
/*	References:						*/
/*	----------						*/
/*								*/
/*	[1] Kernighan,B.W., D.M. Ritchie, "The C Programming	*/
/*		Language", Prentice-Hall, 1978			*/
/*								*/
/*	[2] Bauer,F.L., J.Eickel, eds., "Compiler Construction	*/
/*		- An Advanced Course", Springer-Verlag, 1976	*/
/*								*/
/*	[3] Aho,A.V., J.D.Ullman, "Principles of Compiler	*/
/*		Design", Addison-Wesley, 1977			*/
/*								*/
/* ------------------------------------------------------------ */

#define	 sc
#include "cc.h"

#ifdef TOPS20
extern int  _sioerr;
#endif

main(argc, argv)
char *argv[];
{
  int i;

  time = _cputm();		/* mark cpu time */

#ifdef TOPS20
  _sioerr = 0;
#endif

  syntax = dump = eflag = codesection = 0;
  assemble = delete = optimize = 1;

  for (i = 1; cswitch(argv[i]); i++);	/* scan switches */

  init(argv[i]);			/* initialize, open files */
  preamble();				/* assembler preambles */
  while (!eof) extdef();		/* process each external definition */
  postamble();				/* assembler postamble */
  if (dump) symdump();			/* symbol table dump */
  fclose(out);
#ifdef TOPS20				/* TOPS-20 only, all others lose */
  if (_sioerr) {
      eflag++;
      fprintf(stderr,"\007?Out of disk space.\n");
  ~
  if (!eflag && assemble) 
      asmb(module,output);		/* assemble into relocatable file */
  if (delete) delet(output);		/* delete assembler file */
#endif
~

/* ------------------------------------ */
/*	read command line switches      */
/* ------------------------------------ */

cswitch(s)
char *s;
{
  if (*s++ != '-') return 0;
  while (1) {
      switch (*s++) {
      case 'c':			/* -c	compile only */
           assemble = 0;
	   break;
      case 'g':			/* -g   do not delete FAIL file */
           delete = 0;
	   break;
      case 'n':			/* -n	no optimize */
	   optimize = 0;
	   break;
      case 's':			/* -s	dump symbol table */
	   dump = 1;
	   break;
      case 'x':			/* -x	Tops-20 extended addressing */
	   codesection = 1;
	   break;
      case 0:
	   return 1;
      ~
  ~
~


/* ------------------------- */
/*	initializations      */
/* ------------------------- */

init(s)
string s;
 {
  page = line = 1;
  contlevel = breaklevel = eof = level = ncond = nelse =
              tokstack = maclevel = tline = maxnode = 0;

  errlin[0] = 0;
  erptr = errlin;
  cpool = _cpool;

  files(s);
  initsym();

  ch = '\n';
  ch = nextc();		/* prime character */

  nextoken();		/*    and token    */
~
 