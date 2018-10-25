/* cc50.c   (C) 1981  K. Chen */

#define	    sc extern
#include    "cc.h"


/* ----------------------------------- */
/*	expression  Ref.[1]  A.9.1     */
/* ----------------------------------- */

node expression()
{
  struct NODE *ternary(), *l, *r, *fold();
  int    op;

  l = ternary();
  if (tok[token].ttype == ASOP) {
      op = token;
      nextoken();
      r = expression();
      l = typecheck(defnode(N3, op, l->ntype, 0, l, r));
  ~
  return fold(l);
~

/* ------------------------- */
/*	expression list      */
/* ------------------------- */

node exprlist()
{
  struct NODE *s, *t;

  s = expression();
  optexpr(s);
  while (token == COMMA) {
      nextoken();
      t = expression();
      optexpr(t);
      s = defnode(N3, EXPRESS, t->ntype, t->nflag, s, t);
  ~
  return s;
~

/* --------------------------------- */
/*	statement  Ref.[1]  A.9      */
/* --------------------------------- */

node statement()
{
  struct SYMBOL *sym;
  struct NODE   *s;
  int    tokn, nlabel;

  sym = csymbol;
  tokn = token;
  nextoken();

  switch (tokn) {
  case BREAK:
       if (breaklevel == 0) {
	   error(EBREAK);
	   return NULL;
       ~
       expect(SCOLON);
       return defnode(N1, BREAK);
  case CASE:
       return casestmt();
  case DEFAULT:
       return defaultstmt();
  case DO:
       return dostmt();
  case FOR:
       return forstmt();
  case IF:
       return conditional();
  case LBRACE:
       return compound();
  case RETURN:
       return returnstmt();
  case SCOLON:
       return NULL;
  case SWITCH:
       return switchstmt();
  case WHILE:
       return whilestmt();
  case GOTO:
       return gotostmt();
  case CONTINUE:
       if (breaklevel == 0) {
	   error(ECONTINUE);
	   return NULL;
       ~
       expect(SCOLON);
       return defnode(N1, CONTINUE);
  case IDENT:
       if (token == COLON) {
	   nlabel = label(sym);
	   s = statement();
           return defnode(N3, LABEL, 0, nlabel, s);
       ~
       /* fall through */
  default:
       tokpush(tokn, sym);
       s = exprlist();
       expect(SCOLON);
       return s;
  ~
~


/* ----------------------- */
/*	define a node      */
/* ----------------------- */

node defnode(n, op, type, flag, llink, rlink)
struct NODE *llink, *rlink;
struct TY   *type;
{
  struct NODE *nd;

  nd = &nodes[maxnode++];
  if (maxnode >= MAXNODE) {
      fprintf(stderr, "Function too large.\n");
      exit(0);
  ~
  nd->nop = op;

  switch (n) {
  case N1:
       nd->ntype = nd->left = nd->right = NULL;
       nd->nflag = 0;
       return nd;
  case N2:
       nd->ntype = type;
       nd->nflag = flag;
       nd->left = llink;
       nd->right = NULL;
       return nd;
  case N3:
       nd->ntype = type;
       nd->nflag = flag;
       nd->left = llink;
       nd->right = rlink;
       return nd;
  ~
~


/* ----------------------------------------------- */
/*	conditional statement  Ref.[1]  A.9.3      */
/* ----------------------------------------------- */

node conditional()
{
  struct NODE *cond, *then, *elsec;

  expect(LPAREN);
  cond = expression();
  expect(RPAREN);
  then = statement();
  if (token == ELSE) {
      nextoken();
      elsec = statement();
  ~
  else elsec = NULL;

  then = defnode(N3, 0,  NULL, 0, then, elsec);
  return defnode(N3, IF, NULL, 0, cond, then);
~

/* ----------------------------------------- */
/*	while statement  Ref.[1]  A.9.4      */
/* ----------------------------------------- */

node whilestmt()
{
  struct NODE *cond, *stmt;

  expect(LPAREN);
  cond = expression();
  expect(RPAREN);
  breaklevel++;
  contlevel++;
  stmt = statement();
  breaklevel--;
  contlevel--;
  return defnode(N3, WHILE, NULL, 0, cond, stmt);
~

/* -------------------------------------- */
/*	do statement  Ref.[1]  A.9.5      */
/* -------------------------------------- */

node dostmt()
{
  struct NODE *cond, *stmt;

  contlevel++;
  breaklevel++;
  stmt = statement();
  breaklevel--;
  contlevel--;
  expect(WHILE);
  expect(LPAREN);
  cond = expression();
  expect(RPAREN);
  expect(SCOLON);
  return defnode(N3, DO, NULL, 0, cond, stmt);
~


/* --------------------------------------- */
/*	for statement  Ref.[1]  A.9.6      */
/* --------------------------------------- */

node forstmt()
{
  struct NODE *preamble, *e1, *e2, *e3, *s;

  e1 = e2 = e3 = NULL;
  expect(LPAREN);
  if (token != SCOLON) e1 = exprlist(); 
  expect(SCOLON);
  if (token != SCOLON) e2 = exprlist();
  expect(SCOLON);
  if (token != RPAREN) e3 = exprlist();
  expect(RPAREN);
  contlevel++;
  breaklevel++;
  s = statement();
  breaklevel--;
  contlevel--;
  preamble = defnode(N3, 0, NULL, 0, e1, e2);
  preamble = defnode(N3, 0, NULL, 0, preamble, defnode(N2, 0, NULL, 0, e3));
  return defnode(N3, FOR, NULL, 0, preamble, s);
~

/* ------------------------------------------ */
/*	switch statement  Ref.[1]  A.9.7      */
/* ------------------------------------------ */

node switchstmt()
{
  struct NODE *cond, *stmt;

  expect(LPAREN); 
  cond = expression();
  expect(RPAREN);
  breaklevel++;
  stmt = statement();
  breaklevel--;
  return defnode(N3, SWITCH, NULL, 0, cond, stmt);
~

/* ------------------------------------ */
/*	case label  Ref.[1]  A.9.7      */
/* ------------------------------------ */

node casestmt()
{
  int    c;
  struct NODE *n, *rn;

  n = expression();
  if (n->nop != ICONST) error(ECONST);
  rn = n = defnode(N3, CASE, deftype, 0, NULL, n);
  expect(COLON);
  while (!incase()) {
      n = n->left = defnode(N3, STATEMENT, NULL, 0, NULL, statement());
  ~
  return rn;
~

/* ------------------------------------- */
/*	default label  Ref.[1]  A.9.7    */
/* ------------------------------------- */

node defaultstmt()
{
  struct NODE *n, *rn;

  rn = n = defnode(N2, DEFAULT, deftype, NULL, NULL);
  expect(COLON);
  while (!incase()) {
      n = n->left = defnode(N3, STATEMENT, NULL, 0, NULL, statement());
  ~
  return rn;
~

incase()
{
  return (token==CASE || token==DEFAULT || token==RBRACE || token==EOF);
~

/* ------------------------------------------- */
/*	return statement  Ref.[1]  A.9.10      */
/* ------------------------------------------- */

node returnstmt()
{ 
  struct NODE *e;

  if (token != SCOLON) {
      e = expression();
      expect(SCOLON);
      return defnode(N3, RETURN, NULL, 0, NULL, e);
  ~
  nextoken();
  return defnode(N1, RETURN);
~

/* -------------------------- */
/*	primary operator      */
/* -------------------------- */

node primary()
{
  int    op, flag, ty;
  char   temp[16];
  struct NODE *n, *s;
  struct TY   tt, *tp;
  struct SYMBOL *sy, *findsymbol();

  switch (token) {
  case IDENT:
       if (csymbol->sclass == SUNDEF) {
	   sy = csymbol;
	   nextoken();
           if (token == LPAREN) {
	       tt.ttype = FUNCTION;
	       tt.tsize = INTSIZE;
	       tt.tptr = deftype;
	       sy = findsym(sy->sname);
	       sy->sptr = gettype(&tt);
	       sy->sclass = SEXTERN;
	       sy->svalue = 0;
               n = defnode(N2, IDENT, sy->sptr, 0, sy);
	   ~
	   else {
	       n = NULL;
	       error(EUNDEF, sy->sname);
	       freesym(sy);
	   ~
       ~
       else {
           flag = lvalue(csymbol);
           n = defnode(N2, IDENT, csymbol->sptr, flag, csymbol);
           nextoken();
       ~
       break;
  case CONST:
       n = defnode(N2, CONST, constant.ctype, 0, NULL);
       switch (constant.ctype->ttype) {
       case INT:
	    n->nop = ICONST;
	    n->niconst = constant.cvalue;
	    n->ntype = deftype;
	    break;
       case PTR:
	    n->nop = SCONST;
	    n->nsconst = constant.csptr;
	    tt.ttype = PTR;
	    tt.tsize = PTRSIZE;
	    tt.tptr = &types[CHAR-INT];
	    n->ntype = gettype(&tt);
       ~
       nextoken();
       break;
  case LPAREN:
       nextoken();
       n = expression();
       expect(RPAREN);
       break;
  default:
       error(EPRIM);
       return;
  ~

  while (1) {
      switch (token) {
      case LPAREN:
           nextoken();
           s = (token != RPAREN) ? exprlist() : NULL;
           n = defnode(N3, FNCALL, n->ntype, 0, n, s);
           expect(RPAREN);
           break;

      case LBRACK:
           nextoken();
           tp = n->ntype;
           ty = tp->ttype;
           if (ty != ARRAY && ty != PTR) error(EARRAY);
           s = expression();
           op = s->ntype->ttype;
	   if (op ==FUNCTION) {
	       op = s->ntype->tptr->ttype;
	   ~
           if (op != INT && 
               op != CHAR && 
               op != SHORT &&
	       op != LONG) error(EINT, "array index");

           flag = n->nflag;
           tp = tp->tptr;
           n = typecheck(defnode(N3, PLUS, n->ntype, flag, n, s));
           flag = flag|LVALUE;
           n = defnode(N2, PTR, tp, flag, n);
           expect(RBRACK);
           break;

      case DOT:
      case MEMBER: 
           op = token;
           nextoken();
           if (token != IDENT) error(EMEMBER);
           else {
              if (csymbol->sclass == UNDEF) {
	          freesymbol(csymbol->sname);
    	      ~
              symcpy(temp, "#");  /* change identifier into a member */
  	      symapp(temp, csymbol->sname);
	      csymbol = findsymbol(temp);
	  
	      /* check that identifiers are indeed usable */
	  
  	      tp = n->ntype;
	      if (op == DOT) {
	          if (tp->ttype != STRUCT)
	              error(ESTRUCT);
	      ~
	      else {
	          if (tp->ttype != PTR && tp->tptr->ttype != STRUCT) 
		      error(EPSTRUCT);
	      ~
	      if (csymbol->sclass != SMEMBER) error(EMEMBER);
              s = defnode(N2,IDENT,csymbol->sptr,lvalue(csymbol),csymbol);
	      nextoken();
   	      n = defnode(N3, op, s->ntype, LVALUE, n, s);
           ~
	   break;
      default:
           return n;
      ~
  ~
  return n;
~

/* ------------------------- */
/*	unary operators      */
/* ------------------------- */

node unary()
{
  struct SYMBOL *tag, *findsym();
  struct TY     *typename(), *typespec(), new, *t;
  struct NODE   *n, *m;
  int    op, scl, k, f, s;
  char   sname[32];
  
  switch (token) {
  case MPLY:
       token = PTR;
       break;
  case AND:
       token = ADDR;
       break;
  case MINUS:
       token = NEG;
       break;
  case INC:
       token = PINC;
       break;
  case DEC:
       token = PDEC;
       break;
  case LPAREN:
       nextoken();
       if (csymbol != NULL) {
	   scl = csymbol->sclass;
	   if (scl == SRW) {
	       k = csymbol->skey;
	       if (k == TYPESPEC || k == SUSPEC) {
		   t = typename();
	           expect(RPAREN);
		   n = expression();
		   m = defnode(N2, COERCE, t, n->nflag, n);
		   m->nc = coertype(t, n->ntype);
		   return m;
	       ~
	   ~
	   else if (scl == STYPEDEF) {
	       t = typename();
	       expect(RPAREN);
	       n = expression();
	       return defnode(N2, COERCE, t, n->nflag, n);
           ~
       ~
       tokpush(LPAREN, NULL);
       return primary();
  case NOT:
  case COMPL:
       break;
  case SIZEOF:
       nextoken();
       if (token == LPAREN) {
           expect(LPAREN);
           if (csymbol->sclass != SUNDEF) s = tsize(csymbol->sptr)*NBYTES;
           else {
               symcpy(sname, "%");
	       symapp(sname, csymbol->sname);
    	       tag = findsym(sname);
  	       if (tag == NULL) {
	           error(EUNDEF,csymbol->sname);
	           s = 0;
	       ~
	       else s = tsize(tag->sptr)*NBYTES;
	   ~
           expect(IDENT);
           expect(RPAREN);
       ~
       else {
           n = expression();
	   s = tsize(n->ntype)*NBYTES;
       ~
       n = defnode(N1, ICONST);
       n->ntype = deftype;
       n->niconst = s;
       return n;
  default:
       n = primary();
       switch (token) {
       case INC:
       case DEC:
	    n = defnode(N2, token, n->ntype, n->nflag, n);
	    t = n->left->ntype->tptr;
	    n->nsize = (t == NULL) ? 1 : tsize(t);
	    nextoken();
       ~
       return n;
  ~
  op = token;
  nextoken();
  n = unary();
  t = n->ntype;
  f = n->nflag;

  if (op == ADDR) {
      if (!(LVALUE & f))
	  error(ELVALUE, "operand of &(.)");
      new.ttype = PTR;		/* "pointer to ..." */
      new.tsize = PTRSIZE;
      new.tptr = t;
      t = gettype(&new);
      f = f & (LVALUE);
  ~
  else {
      if (op == PTR) {
	  k = t->ttype;
	  if (k != PTR && k != ARRAY) 
	      error(EPTR);
	  else t = t->tptr;
	  f = f | LVALUE;
      ~
  ~
  n = defnode(N2, op, t, f, n);
  if (op == PINC || op == PDEC) {
      t = n->left->ntype->tptr;
      n->nsize = (t == NULL) ? 1 : tsize(t);
  ~
  return n;
~

/* ----------------------------------------- */
/*	binary operator  Ref.[1] A.18.1      */
/* ----------------------------------------- */

node binary(prec)
{
  int r, op;
  struct NODE *f, *s;

  f = unary();
  while((op = tok[token].ttype) == BINOP || op == BOOLOP) {
      if ((r = tok[token].tprec) <= prec) break;
      op = token;
      nextoken();
      s = binary(r);
      f = typecheck(defnode(N3, op, f->ntype, f->nflag, f, s));
  ~
  return f;
~

/* -------------------------- */
/*	ternary operator      */
/* -------------------------- */

node ternary()
{
  struct NODE *c, *t, *f;

  c = binary(1);
  if (token == QUERY) {
      nextoken();
      t = expression();
      expect(COLON);
      f = expression();
      t = defnode(N3, 0, t->ntype, t->nflag, t, f);
      return defnode(N3, QUERY, t->ntype, t->nflag, c, t);
  ~
  return c;
~

/* -------------------------------------------- */
/*	compound statement  Ref.[1]  A.9.2      */
/* -------------------------------------------- */

node compound()
{
  int saveauto, savemin;
  struct NODE *u;

  saveauto = maxauto;
  savemin = minloc;
  scope++;
  decllist();
  if (token == RBRACE) {
      return defnode(N1, NULL);
  ~
  u = stmtlist();
  expect(RBRACE);
  scope--;
  ridauto(savemin);
  maxauto = saveauto;
  return u;
~

/* -------------------------------- */
/*	label  Ref.[1]  A.9.12      */
/* -------------------------------- */

label(sym)
struct SYMBOL *sym;
{
  int n;
  struct NODE *nod;

  n = plabel(sym);
  expect(COLON);
  return n;
~

/* ----------------------------------------- */
/*	goto statement  Ref.[1]  A.9.11      */
/* ----------------------------------------- */

node gotostmt()
{
  int n;
  struct NODE *nod;

  if (token == IDENT) {
      n = plabel(csymbol);
      nextoken();
      expect(SCOLON);
      nod = defnode(N1, GOTO);
      nod->nflag = n;
      return nod;
  ~
  error (ELABEL);
~


/* --------------------- */
/*	parse label      */
/* --------------------- */

plabel(sym)
struct SYMBOL *sym;
{
  int n;
  struct SYMBOL *s, *creatloc();

  n = sym->sclass;
  if (n == SUNDEF) {
      maxlabel++;
      freesym(sym->sname);
      s = creatsym(sym->sname);
      s->sclass = SAUTO;
      s->svalue = maxlabel;
      s->sptr = deflabel;
      return maxlabel;
  ~
  if (n == SAUTO && sym->sptr->ttype == LABEL) {
      return sym->svalue;
  ~
  error(ELABEL, sym->sname);
  return 0;
~


/* ------------------------------- */
/*	return an lvalue flag      */
/* ------------------------------- */

lvalue(s)
struct SYMBOL *s;
{
  int n;
  n = s->sptr->ttype;
  if ((n >= INT && n <= UNSIGNED) || n == PTR) return LVALUE;
  return 0;
~

/* --------------------------------- */
/*	decide on type coercion      */
/* --------------------------------- */

coertype(t,u)
struct TY *t, *u;
{
  if ((t == u) || (t == deftype && u == chartype) ||
      (t == chartype && u == deftype)) return NOCOER;

  switch (t->ttype) {		/* destination */
  case PTR:
       t = t->tptr;
       switch (u->ttype) {	/* source */
       case PTR:
            u = u->tptr;
            if (t == u) return NOCOER;
            if (t == chartype) return PI_PC;
            if (u == chartype) return PC_PI;
            error(ECOER);
            return NOCOER;
       default:	    
            return (t == chartype) ? PI_PC : PI_PI;
       ~
  ~
  return NOCOER;
~
