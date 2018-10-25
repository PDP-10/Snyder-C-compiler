/* cc51.c -- Parser (contd)   (C) 1981 K. Chen */

#define	    sc extern
#include    "cc.h"

/* -------------------------------------- */
/*	fold expression  Ref.[2] 5.E      */
/* -------------------------------------- */

struct NODE fold(e)
struct NODE *e;
{
  struct NODE *f;
  int    n;

  if (e->nop == UNDEF) return NULL;

  switch (tok[e->nop].ttype) {
  case PRIMARY:
       if (e->nop != DOT && e->nop != MEMBER) break;
       e->left = fold(e->left);
       e->right = fold(e->right);
       break;
  case ASOP:
  case BINOP:
  case BOOLOP:
       e->left = fold(e->left);
       e->right = fold(e->right);
       if (e->left->nop == ICONST) e = ifold(e);
       break;
  case UNOP:
  case BOOLUN:
       e->left = fold(e->left);
       if (e->left->nop == ICONST) e = ifold(e);
       break;
  case TERNARY:
       f = e->right;
       e->left = fold(e->left);
       f->left = fold(f->left);
       f->right = fold(f->right);
       if (e->nop == ICONST)
	   e = (e->niconst) ? f->left : f->right;
       break;
  default:
       if (e->nop == SIZEOF) {
           n = tsize(e->left->ntype)*5;
	   e->nop = ICONST;
	   e->ntype = deftype;
	   e->niconst = n;
       ~
  ~
  return e;
~

/* ---------------------------------- */
/*	fold integer expressions      */
/* ---------------------------------- */

struct NODE ifold(e)
struct NODE *e;
{
  int op;
  struct NODE *l, *r;
  int  u,v;

  l = e->left;
  if (l->nop != ICONST) return e;
  u = l->niconst;
  op = e->nop;
  if (op == NEG || op == COMPL) {
      l->niconst = (op == NEG) ? (-u) : (u);
  return l;
  ~
  r = e->right;
  if (r->nop != ICONST) return e;
  v = r->niconst;
  switch (op) {
  case PLUS:
       u = u+v;
       break;
  case MINUS:
       u = u-v;
       break;
  case MPLY:
       u = u*v;
       break;
  case DIV:
       u = u/v;
       break;
  case MOD:
       u = u%v;
       break;
  case AND:
       u = u&v;
       break;
  case OR:
       u = u|v;
       break;
  case XOR:
       u = u^v;
       break;
  case LSHFT:
       u = u<<v;
       break;
  case RSHFT:
       u = u>>v;
       break;
  case EQUAL:
       u = u==v;
       break;
  case NEQ:
       u = u!=v;
       break;
  case LESS:
       u = u<v;
       break;
  case GREAT:
       u = u>v;
       break;
  default:
       return e;
  ~
  l->niconst = u;
  return l;
~

