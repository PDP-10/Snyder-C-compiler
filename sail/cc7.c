/* cc7.c -- Symbol table   (C) 1981  K. Chen */

#define	    sc extern
#include    "cc.h"

/* ------------------------------------------------ */
/*	symbol table related routines	   	    */
/*						    */
/*	symbol table implemented as a heap	    */
/*	of symbol table entries. Global symbols     */
/*	grow upwards. Local symbols grow from  	    */
/*	high memory towards low.		    */
/*						    */
/*	Symbols (including reserved words and	    */
/*	macros) are hashed through a hash 	    */
/*	table.					    */
/*						    */
/*	Symbol types have a table and hash	    */
/*	table of its own.			    */
/* ------------------------------------------------ */


/* ------------------------- */
/*	initializations      */
/* ------------------------- */

initsym()
{
  int i;
  struct RW     *r;
  struct SYMBOL *s, *creatsym();

  /* initialize symbol, type and hash tables */

  maxlabel = minsym = maxsym = 0;
  minloc = MAXSYM-1;

  for (i = 0 ; i < MAXHSH ; i++) htable[i] = NULL;
  for (i = 0 ; i < TYPES ; i++)  ttable[i] = NULL;

  for (i = 0 ; i < 10 ; i++) {
      types[i].ttype = i+INT;
      types[i].tsize = 0;
      types[i].tptr = NULL;
      ~

  maxtype = 9;
  types[LABEL-INT].ttype = LABEL;
  types[LABEL-INT].tsize = 0;

  deflabel = &types[LABEL-INT];
  deftype =  &types[INT-INT];
  chartype = &types[CHAR-INT];
  scalar = &types[8];

  types[i=INT-INT].tsize = INTSIZE;
  types[CHAR-INT].tsize = CHARSIZE;
  types[LONG-INT].tsize = LONGSIZE;
  types[SHORT-INT].tsize = SHORTSIZE;
  types[FLOAT-INT].tsize = FLOATSIZE;
  types[DOUBLE-INT].tsize = DBLESIZE;
  types[UNSIGNED-INT].tsize = UNSIZE;

  /* enter reserved words in symbol table */

  i = 0;
  r = rw;	
  while (r->n != NULL) {
      s = creatsym(r->n);
      s->sclass = SRW;
      s->stoken = r->t;
      s->skey = r->k;
      r = &rw[++i];
  ~
  minsym = maxsym;
~


/* ------------------------------------------ */
/*	find pointer to the symbol table      */
/* ------------------------------------------ */

struct SYMBOL *findsym (s)
char *s;
{
  int h;
  struct SYMBOL *sym, *symret;

  h = hash(s);
  symret = NULL;
  while ((sym=htable[h]) != NULL) {
      if (symcmp(sym->sname, s)) symret = sym;
      h = (h+1) & (MAXHSH-1);
  ~
  return symret;
~

/* -------------------------------------- */
/*	free a symbol table location      */
/* -------------------------------------- */

freesym(s)
struct SYMBOL *s;
{
  int h, k, w;
  struct SYMBOL *t;

  t = &symbol[maxsym-1];
  h = w = hash(s->sname);
  while (1) {
      k = (h+1) & (MAXHSH-1);
      if (k == w) break;
      if (htable[h] == s) {
	  htable[h] = (htable[k] == NULL) ? NULL : symbol;
	  if (s == t) --maxsym;
	  return;
      ~  
      h = k;
  ~
~

/* -------------------------------------- */
/*	create an entry for a symbol      */
/* -------------------------------------- */

struct SYMBOL *creatsym (s)
char *s;
{
  struct SYMBOL *t;
  int    n;

  n = findhash(s);
  t = htable[n] = &symbol[maxsym++];
  symcpy(t->sname, s);
  t->sptr = t->svalue = NULL;
  t->sclass = SUNDEF;
  return t;
~


/* -------------------------------------------- */
/*	create an entry for a local symbol      */
/* -------------------------------------------- */

struct SYMBOL *creatloc (s)
char *s;
{
  struct SYMBOL *t;
  int    n;

  n = findhash(s);
  t = htable[n] = &symbol[minloc];
  minloc--;
  symcpy(t->sname, s);
  t->sptr = t->svalue = NULL;
  t->sclass = SUNDEF;
  return t;
~


/* ------------------------------------ */
/*	copy symbol representation      */
/* ------------------------------------ */

copysym(s,t)
struct SYMBOL *s, *t;
{
  symcpy(s->sname, t->sname);
  s->sclass = t->sclass;
  s->svalue = t->svalue;
  s->sptr = t->sptr;
~


/* ------------------------------------- */
/*	find unused hash table slot      */
/* ------------------------------------- */

findhash(s)
char *s;
{
  int h;
  h = hash(s);
  while (htable[h] != NULL) {   /* find empty hash slot */
      h = (h+1) & (MAXHSH-1);
  ~
  return h;
~

/* -------------------------------------- */
/*	compare symbols for equality      */
/* -------------------------------------- */

symcmp(s,t)
char *s, *t;
{
  int i;

  i = 1;
  while (*s == *t++) {
      if (!(*s++) || i++ >= _IDSIZE) return 1;
  ~
  return 0;
~

/* ---------------------- */
/*	copy symbols      */
/* ---------------------- */

symcpy(s, t)
char *s, *t;
{
  int i;

  i = 1;
  while (*s++ = *t++) {
      if (i++ >= _IDSIZE) return;
  ~
~


/* ------------------------ */
/*	append symbols      */
/* ------------------------ */

symapp(s, t)
char *s, *t;
{
  int i;

  i = 1;
  while (*s) {
      s++;
      i++;
  ~
  while (*s++ = *t++) {
      if (i++ >= _IDSIZE) return;
  ~
~


/* ------------------------------------ */
/*	compute hash (MAXHSH = 2^N)     */
/* ------------------------------------ */

hash (s)
char *s;
{
  int i,j, count;
  
  i = 0;
  count = 1;
  while ((j = *s++) && count++ <= _IDSIZE) i = ((i+i) + j);
  return (i & (MAXHSH-1));
~



/* --------------------------------------------------------- */
/*	make a copy of a type node, return pointer to it     */
/* --------------------------------------------------------- */

struct TY *maketype(s)
struct TY *s;
{
  struct TY *t;

  t = &types[maxtype++];
  ttable[thash(s)] = t;
  t->ttype = s->ttype;
  t->tsize = s->tsize;
  t->tptr = s->tptr;
  return t;
~

/* ------------------------------------------------------ */
/*	return pointer to type, create if neccessary      */
/* ------------------------------------------------------ */

struct TY *gettype(s)
struct TY *s;
{
  int h, h0;
  struct TY *t;

  h = h0 = thash(s);
  while ((t = ttable[h]) != NULL) {
      if (t->ttype==s->ttype && 
	  t->tsize==s->tsize &&
	  t->tptr==s->tptr)  return t;

      if ((h = (h+1) & (TYPES-1)) == h0) {
	  error (ETYPE);
	  exit(1);
      ~
  ~
  return maketype(s);
~


/* --------------------------------------------------- */
/*	create a new type based on the pattern of      */
/*      an existing one, copying all pointers up to    */
/*      where they differ.			       */
/* --------------------------------------------------- */
 
addtype(s, t)
struct SYMBOL *s;
struct TY     *t;
{
  struct TY *u, *v, *w;

  u = s->sptr;		/* simple add */
  if (u <= scalar || u->ttype == STRUCT) {
      s->sptr = t;
      t->tptr = u;
      return;
  ~
  w = s->sptr = maketype(u);
  while (u != NULL) {
     if ((v=u->tptr) <= scalar || v->ttype == STRUCT) break;
     w = w->tptr = maketype(v);
     u = v;
  ~
  w->tptr = t;
  t->tptr = v;
~


/* --------------------------------- */
/*	extend an existing type      */
/* --------------------------------- */

extendtype(s, t)
struct SYMBOL *s;
struct TY     *t;
{
  struct TY *u, *v;

  u = s->sptr;
  if (u <= scalar || u->ttype == STRUCT) {
      s->sptr = t;
      t->tptr = u;
      return;
  ~
  while (u != NULL) {
     if ((v=u->tptr) <= scalar || v->ttype == STRUCT) break;
     u = v;
  ~
  u->tptr = t;
  t->tptr = v;
~


/* ------------------- */
/*	type hash      */
/* ------------------- */

thash(s)
struct TY *s;
{
  int h;
  h = (s->tptr == NULL) ? 0 : s->tptr - types;
  return (s->ttype + s->tsize + h) & (TYPES-1);
~

/* ------------------------------------------ */
/*	flush auto variable in some range     */
/* ------------------------------------------ */

ridauto(max)
{
  int n,h;
  struct SYMBOL *s;

  while (minloc++ < max) {
      s = &symbol[minloc];
      n = h = hash (s->sname);
      while (htable[h] != s) {
	  h = (h+1) & (MAXHSH-1);
	  if (h == n) {
	      fprintf(stderr, "Compiler error #1.\n");
	      exit(1);
	  ~
      ~
      htable[h] = (((h+1) & (MAXHSH-1)) == NULL) ? NULL : symbol;
  ~
  minloc = max;
~

/* ------------------------------ */
/*	find underlying type      */
/* ------------------------------ */

stype(t)
struct TY *t;
{
  struct TY *u, *v;

  u = t;
  while ((v=u->tptr) != NULL) u = v;
  return u->ttype;  
~

/* -------------------------------------------- */
/*	return if type is an array of char      */
/* -------------------------------------------- */

chararray(t)
struct TY *t;
{
  if (t->ttype != ARRAY) return 0;
  while (1) {
      t = t->tptr;
      if (t->ttype != ARRAY) return (t->ttype == CHAR);
  ~
~


/* --------------------------------------------- */
/*	return if type is a pointer of char      */
/* --------------------------------------------- */

charpointer(t)
struct TY *t;
{
  if (t->ttype != PTR && t->ttype != ARRAY) return 0;
  while (1) {
      t = t->tptr;
      if (t->ttype != ARRAY) return (t->ttype == CHAR);
  ~
~   