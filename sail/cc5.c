/* cc5.c -- Parser   (C) 1981  K. Chen */

#define	    sc extern
#include    "cc.h"


/* --------------------------------------- */
/*	type-specifier  Ref[1] A.18.2      */
/* --------------------------------------- */

struct TY *typespec()
{
  int n;
  struct TY *strspec(), *t;

  if (csymbol != NULL) {
      switch (csymbol->skey) {
      case TYPESPEC:
           n = token;
           nextoken();
           return &types[n-INT];
      case SUSPEC:
	   return strspec();
      default:
	   if (token == IDENT && csymbol->sclass == STYPEDEF) {
	       t = csymbol->sptr;
	       nextoken();
	       return t;
	   ~
      ~
  ~
  return deftype;
~


/* ------------------------------------ */
/*	declarator  Ref[1]  A.18.2      */
/*					*/
/*	tag = 0    normal		*/
/*	    = 1	   structure tag	*/
/*	    = 2	   func arguments	*/
/* ------------------------------------ */

struct SYMBOL *declarator(s, tag)
struct SYMBOL *s;
{
  struct SYMBOL *decl0();
  
  scalar = s->sptr;
  return decl0(s, tag);
~


/* ------------------------------------- */
/*	process external definition      */
/* ------------------------------------- */

extdef()
{
  struct SYMBOL *s, tempsym, base, *fn;
  struct TY     *t;
  int    sctype, narg;

  scope = SEXTERN;
  sctype = XTERN;	/* default storage class */

  switch (token) {
  case EXTERN: 
       sctype = SEXTERN;
       nextoken();
       break;
  case STATIC:
       sctype = SSTATIC;
       nextoken();
       break;
  case TYPEDEF:
       sctype = STYPEDEF;
       nextoken();
  ~

  t = typespec();
  if (token == SCOLON) {
      nextoken();
      return;
  ~
  tempsym.sname[0] = '\0';
  tempsym.sclass = sctype;
  tempsym.svalue = 0;
  tempsym.sptr = t;
  copysym(&base,&tempsym);

  if ((s = declarator(&tempsym,0)) != NULL) {
      fn = s;
      t = s->sptr;
      if (t->ttype == FNDEF) {	/* parsed "fnname (" */
          narg = 1;
          scope = SARG;
	  while (1) {  /* parameter-list */
	    if (token != IDENT) break;
	    symcpy(base.sname, csymbol->sname);
	    if (csymbol->sclass == SUNDEF) freesym(csymbol);
	    s = creatloc(base.sname);
	    s->sclass = SARG;
	    s->svalue = narg++;
	    s->sptr = &types[INT-INT];
	    nextoken();
	    if (token != COMMA) break;
	    nextoken();
	  ~
	  expect(RPAREN);
          funcdef(fn);
          return;
      ~
      if (t->ttype == FUNCTION) {  /* parsed "fnname ()" */
	  if (token != COMMA && token != SCOLON && token != ASGN) {
              funcdef(fn);
              return;
	  ~
      ~
      if (!eof) datadef(s, &base);
  ~
~

struct SYMBOL *decl0(s, tag)
struct SYMBOL *s;
{
  struct TY t, *u;

  while (token == MPLY) {
      nextoken();
      decl1(s, tag);
      t.ttype = PTR;
      t.tsize = PTRSIZE;
      addtype(s, gettype(&t));
      if (token != MPLY) return s;
  ~
  decl1(s, tag);
  return s;
~

/* -------------------------- */
/*	level 1 declarator    */
/* -------------------------- */

decl1(s, tag)
struct SYMBOL *s;
{
  struct TY     t, *ty;
  struct SYMBOL *ssym, *try, *findsym();
  id     saved;

  switch (token) {
  case IDENT:
       if (s->sname[0]) {
	   error(SYNTAX);
	   return NULL;
       ~
       if (tag == 1) {
	   symcpy(s->sname, "#");
	   symapp(s->sname, csymbol->sname);
	   freesym(csymbol);
       ~
       else symcpy(s->sname, csymbol->sname);
       nextoken();
       break;
  case LPAREN:
       nextoken();
       decl0(s, tag);
       expect(RPAREN);
       break;
  default:
       return;
  ~

  switch (token) {
  case LPAREN:
       nextoken();
       t.tsize = PTRSIZE;
       t.tptr = s->sptr;
       if (token == RPAREN) {
	   nextoken();
	   t.ttype = FUNCTION;
	   addtype(s, gettype(&t));
	   return;
       ~
       if (token == IDENT) {
	   t.ttype = FNDEF;
	   addtype(s, gettype(&t));
	   return;
       ~
       error(SYNTAX);
       break;
  case LBRACK:
       decl2(s, tag);
  ~
~

/* ---------------------- */
/*	declarator[]      */
/* ---------------------- */

decl2(s, tag)
struct SYMBOL *s;
{
  struct TY t;
  nextoken();
  t.ttype = (tag == 2) ? PTR : ARRAY;
  t.tptr = s->sptr;
  t.tsize = (token == RBRACK) ? 0 : pconst();
  addtype(s, gettype(&t));
  expect(RBRACK);
  if (token == LBRACK) decl2(s);
~


/* -------------------------------------------------- */
/*	struct-or-union specifier  Ref[1] A.18.2      */
/* -------------------------------------------------- */

struct TY *strspec()
{
  struct SYMBOL s, *tagname, *findsym();
  struct TY     p, *pt;
  int    typ, etyp, hastag;
  char   *sym, symarray[16];

  typ = token;

  nextoken();
  if (token == IDENT) {
      hastag = 1;
      symcpy(s.sname, "%");
      symapp(s.sname, csymbol->sname);
      symcpy(symarray, csymbol->sname);
      sym = symarray;
      freesym(csymbol);
      nextoken();
  ~
  else {
      hastag = 0;
      symcpy(s.sname, "%%%");
      sym = "Untagged struct";
  ~

  if (hastag) {
      if (token == LBRACE) {			   /* struct FOO { */
          if ((tagname=findsym(s.sname)) == NULL) {
              tagname = creatsym(s.sname);	   /* FOO undefined */
          ~
          else {				   /* FOO defined */
	      if (tagname->sclass == SUNDEF)       /* fwrd. ref.  */
	          tagname->sclass = STAG;
	      else {
		  etyp = (typ == STRUCT) ? EDSTRUC : EDUNION;
	          error(etyp, sym);
	      ~
	  ~ 
	  p.tsize = sdecllist(typ);		   
      ~
      else {					   /* struct FOO */
	  if ((tagname=findsym(s.sname)) == NULL) {
              tagname = creatsym(s.sname);         /* FOO undefined */
	      tagname->sclass = SUNDEF;	           /* mark as fwrd. ref. */
	      tagname->svalue = 0;
	      tagname->sptr = NULL;
	      return &types[STRUCT-INT];
	  ~
          else return tagname->sptr;	   	   /* FOO defined */
      ~
  ~
  else {					   /* struct */
      tagname = creatsym(s.sname);
      if (token == LBRACE) 			   /* struct { */
	  p.tsize = sdecllist(typ);
      else {					   /* struct no-{ */
          expect(LBRACE);
	  recover(SCOLON);
      ~
  ~
  p.ttype = STRUCT;
  p.tptr = NULL;
  pt = gettype(&p);

  tagname->sclass = STAG;
  tagname->svalue = 0;
  tagname->sptr = pt;
  return pt;
~


/* ------------------------------------------ */
/*	struct-decl-list  Ref.[1]  A.8.5      */
/* ------------------------------------------ */

sdecllist(t)
{
  int offset, size, maxsize;

  maxsize = offset = 0;
  expect(LBRACE);
  while (token != RBRACE) {
      if (eof) earlyend();
      size = sdeclaration(offset);
      if (t == UNION) {
	  offset = 0;
          if (size > maxsize) maxsize = size;
      ~
      else 
	  maxsize = (offset += size);
  ~
  nextoken();
  return maxsize;
~

/* -------------------------------------------- */
/*	struct-declaration  Ref.[1]  A.8.5      */
/* -------------------------------------------- */

sdeclaration(offset)
{
  struct TY     *t;
  struct SYMBOL base, tempsym;
  int  size, s;

  size = offset;
  t = typespec();
  base.sname[0] = '\0';
  base.sclass = SMEMBER;
  base.svalue = 0;
  base.sptr = t;

  while (1) {
      copysym(&tempsym, &base);
      s = declstr(&tempsym, offset);
      offset += s;
      if (token != COMMA) break;
  ~
  expect(SCOLON);
  return offset-size;
~

/* ----------------------------------------- */
/*      struct-declarator  Ref.[1]  A.8.5    */
/* ----------------------------------------- */

declstr(s, offset)
struct SYMBOL *s;
{
  struct SYMBOL *t, *u, *findsymbol();

  t = declarator(s, 1);
  u = findsymbol(t->sname);

  if (u == NULL) {	/* struct member not seen before */
      t->sclass = SMEMBER;
      t->svalue = offset;
      u = creatsym(t->sname);
      copysym(u, t);
  ~
  else { /* struct member seen before */
      if (offset != u->svalue) error(EDMEM);
  ~
  return tsize(u->sptr);
~


/* -------------------------------------------- */
/*	abstract-declarator  Ref[1]  A.8.7      */
/* -------------------------------------------- */

struct SYMBOL *absdecl(s)
struct SYMBOL *s;
{
  struct TY  tt;

  if (s == NULL) return NULL;
  switch (token) {
  case LPAREN:
       nextoken();
       s = absdecl(s);
       if (!expect(RPAREN)) {
           error(SYNTAX);
           return s;
       ~
       break;
  case MPLY:
       while (token == MPLY) {
           nextoken();
           tt.ttype = PTR;
           tt.tsize = PTRSIZE;
           tt.tptr = s->sptr;
           s->sptr = gettype(&tt);
       ~
       s = absdecl(s);
       break;
  default:
       return s;
  ~

  /* parse declarator() or declarator[] */

  switch (token) {
  case LPAREN:
       nextoken();
       tt.tsize = PTRSIZE;
       tt.tptr = s->sptr;
       tt.ttype = FUNCTION;
       s->sptr = gettype(&tt);
       expect(RPAREN);
       break;
  case LBRACK:
       nextoken();
       tt.ttype = ARRAY;
       tt.tptr = s->sptr;
       tt.tsize = (token == RBRACK) ? 0 : pconst();
       s->sptr = gettype(&tt);
       expect(RBRACK);
       break;
  ~
  return s;
~


/* ----------------------------------- */
/*	type-name  Ref.[1]  A.8.7      */
/* ----------------------------------- */

struct TY *typename()
{
  struct SYMBOL s, *t;

  s.sname[0] = '\0';
  s.sclass = s.svalue = 0;
  s.sptr = typespec();
  t = absdecl(&s);
  return t->sptr;
~


/* -------------------------------------------- */
/*	function-definition  Ref[1] A.18.4      */
/* -------------------------------------------- */

funcdef(s)
struct SYMBOL *s;
{
  int    n, mtype;
  char   ident, *c;
  char   cc[16];
  struct TY   *t, ftype;
  struct NODE *nnode, *header, *stmtlist(), *u;
  struct SYMBOL *findsym(), *s1, *s2, stemp;

  /* check for multiple decl */

  t = s->sptr;
  ftype.ttype = FUNCTION;
  ftype.tsize = tsize(t);
  ftype.tptr = t->tptr;
  s->sptr = gettype(&ftype);
  if ((s1 = findsym(c = s->sname)) == NULL) {
      fprintf(stderr, "Cannot find symbol %s.\n", c);
      exit(1);
  ~
  if (s->sclass == XTERN) {
      s->sclass = SEXTERN;
      s->svalue = 1;
  ~
  copysym(s1, s);

  header = defnode(N1, 0);
  header->left = defnode(N2, IDENT, s1->sptr, 0, s1);
  nnode = defnode(N2, FUNCTION, NULL, 0, header);

  symcpy(cc,c);
  cc[_IDSIZE-1] = 0;
  
  fprintf(stderr, "[%s]\n", cc);

  if (token != LBRACE) { /* type-decl-list */
      while(1) {
          if (token == REGISTER) nextoken();	/* register == NOP */
	  t = typespec();
	  while (1) {
	      stemp.sname[0] = '\0';
	      stemp.sptr = t;
	      s1 = declarator(&stemp, 2);
	      if ((s2 = findsym(s1->sname)) == NULL || s2->sclass != SARG)
		  error(ENOTARG);
	      else
		  s2->sptr = s1->sptr;
              if (token != COMMA) break;
	      nextoken();
	  ~
	  if (!expect(SCOLON)) recover(SCOLON);
	  if (token == LBRACE) break;
      ~
  ~
  maxinit = maxauto = 0;
  mtype = maxtype;
  scope = SAUTO;	/* function-statement */
  nextoken();
  decllist();
  u = nnode->right = stmtlist();
  while (u->left != NULL) u = u->left;
  u->left = defnode(N3, STATEMENT, NULL, 0, NULL, defnode(N1, RETURN));
  gencode(nnode);
  expect(RBRACE);
  maxtype = mtype;	/* flush local types */
  ridauto(MAXSYM-1);
~


/* ----------------------------------------- */
/*	sc-type, type-specifier, symbol      */
/*	      Ref[1] A.18.4		     */
/* ----------------------------------------- */

datadef(s, base)
struct SYMBOL *s, *base;
{
  int    more;
  struct SYMBOL t;
  struct NODE *izer(), *z;

  z = NULL;
  while (1) {
      z = defnode(N3, DATA, NULL, 0, z, izer(s));
      if (token != COMMA) break;
      nextoken();
      copysym(&t, base);
      s = declarator(&t, 0);
  ~
  expect(SCOLON);
  gencode(z);
~

/* ----------------------- */
/*	initializer        */
/*      Ref.[1] A.8.6      */
/* ----------------------- */

struct NODE *izer(s)
struct SYMBOL *s;
{
  struct TY  *ty, tt;
  struct SYMBOL *t, *findsym();
  struct NODE *izer0(), *izer1(), *y, *z;
  int    n, m;
  char   *c;

  z = NULL;
  if (token == ASGN) {
      nextoken();
      ty=s->sptr;
      n = tsize(ty);
      if (token == LBRACE) {
	  z = izer1(ty);
	  if (n == 0) {
	      tt.ttype = ty->ttype;
	      tt.tsize = z->nflag;
	      tt.tptr = ty->tptr;
	      s->sptr = gettype(&tt);
	  ~
      ~
      else {
	  z = izer0(ty);
	  if (n == 0) {
	      tt.ttype = ty->ttype;
	      tt.tsize = z->nflag;
	      tt.tptr = ty->tptr;
	      s->sptr = gettype(&tt);
	  ~
      ~
  ~

  t = findsym(c=s->sname);

  if (t->sclass == SUNDEF || 
     (scope == SAUTO && (t->sclass == SEXTERN || t->sclass == XTERN))) {
      switch (s->sclass) {
      case SAUTO:
           ty = s->sptr;
	   while (ty->ttype == PTR) {
	       ty = ty->tptr;
	   ~
	   if (ty->ttype == FUNCTION) {
	       if (t->sclass != SEXTERN) {
	           freesym(t);
	           creatsym(c);	  /* create an entry in global table        */
	           s->svalue = 0; /* mark as a declaration (not definition) */
	           s->sclass = SEXTERN;
	       ~
	       s->svalue = t->svalue;
	       s->sclass = t->sclass;
	       break;
	   ~
           if (t->sclass == SUNDEF) freesym(t);
	   t = creatloc(c);
	   s->svalue = maxauto;
	   maxauto += tsize(s->sptr);
	   break;
      case XTERN:
           s->svalue = 1;
	   s->sclass = SEXTERN;
      ~
      copysym(t, s);
  ~
  else {
      ty = s->sptr;
      while (ty->ttype == PTR) ty = ty->tptr;
      if (ty->ttype != FUNCTION) {
          error(EDSYMB, c);
      ~
  ~
  y = defnode(N2, IDENT, t->sptr, 0, t);
  return defnode(N3, IZ, NULL, 0, y, z);
~

struct NODE *izer0(t)
struct TY *t;
{
  struct NODE *y, *z, *root;
  struct TY *u;
  char   *c;
  int    op, m, size;

  z = expression();
  op = z->nop;
  size = tsize(t);

  if (op != ICONST && op != SCONST) {
      error(ECONST);
  ~
  else {
      if (chararray(t)) {
          if (op == SCONST) {
   	      c = z->nsconst;
	      m = 0;
	      root = NULL;
	      u = &types[CHAR-INT];
	      do {
	          m++;
                  y = defnode(N2, ICONST, u, 0, NULL);
	          y->niconst = *c;
	          if (root == NULL) 
                      z = root = defnode(N3, IZLIST, NULL, 0, NULL, y);
	          else
                      z = z->left = defnode(N3, IZLIST, NULL, 0, NULL, y);
	      ~ while (*c++);
	      m = (size) ? size - (m+3)/4 : 0;
          ~
      ~
      else {
	  m = (size) ? (size - 1) : 0;
	  root = (m <= 0) ? z : (z = defnode(N3, IZLIST, NULL, 0, NULL, z));
      ~
  ~
  if (m > 0) {
      y = defnode(N1, BLOCK);
      y->niconst = m;
      z->left = defnode(N3, IZLIST, NULL, 0, NULL, y);
  ~
  else if (m < 0) error(ESIZE);
  return root;
~

struct NODE *izer1(t)
struct TY *t;
{
  int size, m;
  struct NODE *x, *y, *z, *izlist();

  nextoken();
  size = tsize(t);
  z = izlist(t);
  if (token == COMMA) nextoken();
  expect(RBRACE);

  y = z;
  while (y->left != NULL) y = y->left;

  m = z->nflag;
  if (chararray(t)) m = (m+3)/4;
  m = size - m;
  if (m > 0) {
      x = defnode(N1, BLOCK);
      x->niconst = m;
      y->left = defnode(N3, IZLIST, NULL, 0, NULL, x);
      z->nflag = size;
  ~
  else if (size && m < 0) {
      error(ESIZE);
  ~
  return z;
~

struct NODE *izlist(t)
struct TY *t;
{
  int n, size, p;
  struct TY *ty;
  struct NODE *l, *m, *root;

  n = 0;
  m = root = NULL;
  size = tsize(ty=t->tptr);
  p = (charpointer(ty)) ? ty : NULL;
  while (1) {
      l = (token == LBRACE) ? izer1(p) : izer0(p);
      if (root == NULL)
          m = root = defnode(N3, IZLIST, NULL, 0, NULL, l);
      else {
          m = m->left = defnode(N3, IZLIST, NULL, 0, NULL, l);
      ~
      n++;
      if (token != COMMA) break;
      nextoken();
  ~
  root->nflag = n;	/* size carried in node.nflag */
  return root;
~


/* ----------------------------------------- */
/*	parse for a constant expression      */
/* ----------------------------------------- */

pconst()
{
  struct NODE *e;

  e = expression();
  if (e->nop != ICONST) {
      error(ECONST);
      return 0;
  ~
  return e->niconst;
~

/* ----------------------------------------- */
/*	declaration-list  Ref.[1] A.18.3     */
/* ----------------------------------------- */

decllist()
{
  int n;
  while (csymbol != NULL) {
      if ((n = csymbol->sclass) == SRW) {
          n = csymbol->skey;
          if (n != TYPESPEC && n != SCSPEC && n != SUSPEC) return;
      ~
      else {
	  if (n != STYPEDEF) return;
      ~
      decln();
  ~
~


/* -------------------------------------- */
/*	declaration  Ref.[1]  A.18.2      */
/* -------------------------------------- */

decln()
{
  int n;
  struct SYMBOL s;

  n = 0;
  switch (token) {
  case AUTO:
       n = scope;
       break;
  case STATIC:
       n = SSTATIC;
       break;
  case EXTERN:
       n = SEXTERN;
       break;
  case REGISTER:
       /* n = SREGISTER; */
       n = scope;
       break;
  case TYPEDEF:
       n = STYPEDEF;
       break;
  default:
       s.sclass = scope;
  ~
  if (n) {
      nextoken();
      s.sclass = n;
  ~
  s.sptr = typespec();
  s.sname[0] = 0;
  idecllist(&s);
  expect(SCOLON);
~


/* ----------------------------------------------- */
/*	init-declarator-list  Ref.[1]  A.18.2      */
/* ----------------------------------------------- */

idecllist(base)
struct SYMBOL *base;
{
  struct SYMBOL t, *s;
  struct NODE   *y,*z;
  int    savemax;
  
  z = NULL;
  while (1) {
      copysym(&t, base);
      s = declarator(&t, 0);
      y = izer(s);
      if (scope == SAUTO) {
          z = defnode(N3, DATA, NULL, 0, z, y);
      ~
      if (token != COMMA) break;
      nextoken();
  ~
  savemax = maxnode;
  gencode(z);
  maxnode = savemax;
~


/* ---------------------------------------- */
/*	statement-list  Ref.[1] A.18.3      */
/* ---------------------------------------- */

struct NODE *stmtlist()
{
  struct NODE *n, *statement(), *m, *root;

  n = NULL;
  while (1) {
      m = defnode(N3, STATEMENT, NULL, 0, NULL, statement());
      n = (n == NULL) ? (root = m) : (n->left = m);
      if (token == RBRACE) return root;
  ~
~
   