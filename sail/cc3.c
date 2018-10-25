/* cc3.c -- Preprocessor   (C) 1981  K. Chen */

#define	    sc extern
#include    "cc.h"

/* struct SYMBOL *findsym(); */

/* ---------------------------------------------- */
/*	get next character from input source      */
/* ---------------------------------------------- */

nextc()
{
  int i;

  if (maclevel) {		/* non-zero macro-level */
      if (i = *macptr++) {	/*   => macro expansion */
          if (i < 7) {		/*      => arguments    */
	      mac[maclevel].mptr = macptr;
	      macptr = mac[maclevel++].marg[i-1];
	      return nextc();
	  ~
	  return ch = i;
      ~
      if (--maclevel) {
	  macptr = mac[maclevel].mptr;
      ~
      return ch = nextc();
  ~

  if ((i = getc(in)) == '#' && ch == '\n')
      return preprocess();

  switch (i) {
  case EOF:
      if (level-- > 0) {
          in = inc[level].cptr;
          line = inc[level].cline;
	  page = inc[level].cpage;
          strcpy(input, inc[level].cname);
          ch = '\n';
	  erptr = errlin;
	  *erptr = 0;
	  
          return ch = nextc();
      ~
      eof = 1;
      return ch = 0;
  case '\n':
      line++;
      tline++;
      erptr = errlin;
      *erptr = 0;
      break;
  case 014:
      line = 1;
      page++;
      erptr = errlin;
      *erptr = 0;
      break;
  default:
      *erptr++ = i;
  ~
  return ch = i;
~


/* ----------------------------------------------------- */
/*	push a character back into the input source      */
/* ----------------------------------------------------- */

pushc(c)
{
  if (c == '\n') {
      line--;
      tline--;
  ~
  if (maclevel)
      macptr--;
  else 
      ungetc(c, in);
~


/* ------------------------------- */
/*	service a # statement      */
/* ------------------------------- */

preprocess()
{
  int i;
  char s[_IDSIZE];

  do i = nextc(); while (i == ' ' || i == '\t');

  if (map[i] != IDENT) {
      error(EIDENT);
      flushline(i);   
  ~
  else {
      switch (getstring(s, i)) {
      case 4:
           if (!strcmp(s,"else")) celse();
	   else {
               error(EMACRO, s);
	       flushline(0);
	   ~
	   break;
      case 5:
           if (!strcmp(s,"endif")) cendif(); else
           if (!strcmp(s,"ifdef")) cifdef(0); else
           if (!strcmp(s,"undef")) undefine();
	   else {
               error(EMACRO, s);
	       flushline(0);
	   ~
	   break;
      case 6:
           if (!strcmp(s,"define")) {
	       if (ncond < 0 || nelse < 0) flushline(0); else define(); 
	   ~ else
           if (!strcmp(s,"ifndef")) cifdef(1);
	   else {
               error(EMACRO, s);
	       flushline(0);
	   ~
	   break;
      case 7:
           if (!strcmp(s,"include")) include();
	   else {
               error(EMACRO, s);
	       flushline(0);
	   ~
	   break;
      default:
           error(EMACRO, s);
	   flushline(0);
      ~
  ~
  return ch = '\n';
~


/* ----------------------- */
/*	#define macro      */
/* ----------------------- */

define()
{
  int    i, nargs;
  char   s[16], heap[100], *t, *args[7], *u;
  struct SYMBOL *creatsym(), *sym;

  nextc();
  skipblanks();
  if (map[ch] == IDENT) {
      getstring (s, ch);
      if ((sym=findsym(s)) != NULL && sym->sclass != SMACRO) {
          error (EDSYMB, s);
	  flushline(ch);
	  return;
      ~
      if (sym == NULL) sym = creatsym(s);
      sym->smptr = cpool;
      sym->sclass = SMACRO;
      sym->svalue = 0;
      nextc();

      nargs = 0;
      if (ch == '(') {
	  t = heap;
          while (1) {
	      nextc();
	      skipblanks();
	      if (map[ch] != IDENT) {
	          nargs = 0;
	          error (EARG);
	          break;
	      ~
	      args[nargs] = t;
	      getstring (u=s, ch);
	      while (*t++ = *u++) ;
	      nargs++;
	      nextc();
	      skipblanks();
	      if (ch != ',') break;
          ~
          if (ch != ')') error (EARGEND);
	  else nextc();
      ~
      sym->svalue = nargs;
      skipblanks();
      while (!eof && ch != '\n') {
          if (ch == '\\') {
              nextc();
	      if (ch != '\n') {
	          *cpool++ = '\\';
	          *cpool++ = ch;
	      ~
          ~
  	  else {
	      if (nargs && map[ch] == IDENT) {
		  getstring(s, ch);
		  for (i=0 ; i < nargs ; i++) {
		      if (strcmp(args[i], s) == 0) {
			  *cpool++ = i + 1;
			  break;
		      ~
		  ~
	          if (i >= nargs) {
		      t = s;
		      while (*t) *cpool++ = *t++;
		  ~
	      ~
	      else *cpool++ = ch;
	  ~
	  nextc();
      ~
      *cpool++ = ' ';
      *cpool++ = '\0';
  ~
  else error (EIDENT);
  flushline(ch);
~

/* ---------------------- */
/*	#ifdef macro      */
/* ---------------------- */

cifdef(doit)
{
  int    yes;
  char   s[16];
  struct SYMBOL *sym;

  nextc();
  if (ncond) error(ECONTROL);
  skipblanks();
  if (map[ch] == IDENT) {
      getstring (s, ch);
      yes = ((sym=findsym(s)) == NULL || sym->sclass != SMACRO);
      if (!doit) yes = (yes) ? 0 : 1;
  ~
  else {
      error (EIDENT);
      yes = 0;
  ~
  flushline(ch);
  ncond = 1;  

  if (yes) return;	/* expand */

  ncond = -1;
  while (1) {
      if (ncond == 0 || eof) return;
      nextc();
      flushline(ch);
  ~
~

/* ---------------------- */
/*      #else macro       */
/* ---------------------- */

celse()
{
  if (!ncond) error(ECONTROL);
  nelse = ncond;
  ncond = 0;
  if (ncond == 1) {
      while (1) {
          if (nelse == 0 || eof) return;
          nextc();
          flushline(ch);
      ~
  ~
~

/* ---------------------- */
/*	#endif macro      */
/* ---------------------- */

cendif()
{
  if (!ncond && !nelse) error(ECONTROL);
  ncond = nelse = 0;
~

/* ------------------------ */
/*	#include macro      */
/* ------------------------ */

include()
{
  char f[_FILSIZ];
  FILE *fp, *fopen();
  string s, t;

  skipblanks();
  s = f;
  switch (ch) {
  case '"': 
       nextc();
       while (ch != '"') {
           if (ch == '\n' || eof) {
	       if (eof) earlyend();
               error(EXFILE, f);
   	       return;
           ~
           *s++ = ch;
           nextc();
       ~
       break;
  case '<': 
       t = sprefix;
       while (*t) *s++ = *t++;
       nextc();
       while (ch != '>') {
           if (ch == '\n' || eof) {
	       if (eof) earlyend();
               error(EXFILE, f);
   	       return;
           ~
           *s++ = ch;
           nextc();
       ~
       t = spstfix;
       while (*t) *s++ = *t++;
       break;

  default:
       if (eof) earlyend();
       else {
           error(EXFILE);
	   flushline(0);
	   return;
       ~
  ~
  *s = 0;
  flushline(ch);
  fp = fopen(f, "r");
  if (fp == NULL) {		/* unsuccessful open */
      error(EFILE, f);
      return;
  ~
  strcpy(inc[level].cname, input);	/* save old context */
  inc[level].cptr = in;
  inc[level].cline = line;
  inc[level].cpage = page;

  level++;				/* create new context */
  strcpy(input, f);
  in = fp;
  line = 1;
  page = 1;
~


/* ------------------------- */
/*	#undefine macro      */
/* ------------------------- */

undefine()
{
  char   s[16];
  struct SYMBOL *sym;

  nextc();
  skipblanks();
  if (map[ch] == IDENT) {
      getstring (s, ch);
      if ((sym=findsym(s)) != NULL) {
          if (sym->sclass != SMACRO)
              error (ENOTMAC);
	  else
	      symcpy(sym->sname, "*");
      ~
      else error(EUNDEF,s);
      flushline(0);
      return;
  ~
  error(EIDENT);
  flushline(ch);
~


/* ----------------------------------------------- */
/*	flushes input until the next \n or EOF     */
/* ----------------------------------------------- */

flushline(i)
{
  ch = i;
  while (ch != '\n') nextc();
~


/* --------------------------------------- */
/*	get a string from input source     */
/* --------------------------------------- */

getstring(s, c)
string s;
{
  int m, n;

  n = 0;
  do {
      *s++ = c;
      n++;
      m = map[c = nextc()];
  ~ while (m == IDENT || m == CONST);
  pushc(c);
  *s = 0;
  return n;
~


/* ------------------------------ */
/*	skip blanks and tabs      */
/* ------------------------------ */

skipblanks()
{
  while (ch == ' ' || ch == '\t') nextc();
~


/* ------------------------ */
/*	unexpected eof      */
/* ------------------------ */

earlyend()
{
  error(EEOF);
  exit(1);
~
