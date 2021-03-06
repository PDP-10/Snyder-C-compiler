/* cc83.c -- Code generator TOPS-20 (contd)   (C) 1981  K. Chen */

#define	    sd   extern
#include    "cc.g"

/* --------------------------------------------- */
/*	generate code for ternary operators      */
/* --------------------------------------------- */

gternary(n)
struct NODE *n;
{
  int false, done, r, q, first, second, opt;

  false = getlabel();
  done = getlabel();
  gboolean(n->left, false, 0);
  n = n->right;
  first = r = genstmt(n->left);
  opt = optimize;
  optimize = 0;
  release (r);
  optimize = opt;
  if (first >= 16) first = regis[first];
  code6(GOTO, 0, done);
  outlab(false);
  second = q = genstmt(n->right);
  if (second >= 16) second = regis[second];
  if (first != second) {
      changereg(r, q);
      if (first) regis[first] = 1;
      release(q);
  ~
  else if (second == 0) release(q);

  outlab(done);
  if (first == 0) {
      flushcode(0);
      r = getzero();
  ~
  return r;
~


/* --------------------------------------------- */
/*	generate short logicals if possible      */
/* --------------------------------------------- */

genshort(n, opr, label, mode)
struct NODE *n;
{
  int op, r;
  struct NODE *l;

  if ((op = tok[n->nop].ttype) == BOOLOP || op == BOOLUN)
     gboolean(n, label, mode);
  else {
     r = genstmt(n);
     code6(opr, r, label);
     release(r);
  ~
~

/* ----------------------------------------- */
/*	jump to false label if not true      */
/*	reverse sense if reverse bit set     */
/* ----------------------------------------- */

gboolean(n, false, reverse)
struct NODE *n;
{
  int r, lab, op, val;
  struct PSEUDO *p;

  switch (n->nop) {
  case NOT:
       r = genstmt(n->left);
       op = (reverse) ? LAND : LOR;
       code6(op, r, false);
       release(r);
       break;
  case LAND:
       op = n->left->nop ;
       if (op == NOT || op == LAND || op == LOR) {
	   gboolean(n->left, false, reverse);
	   gboolean(n->right, false, reverse);
       ~
       else {
	   r = genstmt(n->left);
           op = (reverse) ? LOR : LAND;
           lab = (reverse) ? getlabel() : false;
           code6(LAND, r, lab);
           release(r);

	   genshort(n->right, n->nop, false, reverse);
           if (reverse) outlab(lab);
       ~
       break;
  case LOR:
       lab = (reverse) ? false : getlabel();
       op = (reverse) ? LOR : LAND;
       r = genstmt(n->left);
       code6(LOR, r, lab);
       release(r);
       r = genstmt(n->right);
       code6(op, r, false);
       release(r);
       if (!reverse) outlab(lab);
       break;
  case NEQ:
  case LEQ:
  case GEQ:
  case LESS:
  case GREAT:
  case EQUAL:
       gboolop(n, reverse);
       code6(GOTO, 0, false);
       break;
  default:
       if (n->nop == ICONST) {
	   op = n->niconst;
	   if ((!op && !reverse) || (op && reverse)) {
	       code6(GOTO, 0, false);
	   ~
	   return;
       ~
       r = genstmt(n);
       op = (reverse) ? LOR : LAND;
       code6(op, r, false);
       release(r);
       flushcode(0);
       break;
  ~
~

/* ---------------------------------- */
/*	code for == > < <= >= !=      */
/* ---------------------------------- */

gboolop(n, reverse)
struct NODE *n;
{
  int op, r1, r2;
  struct PSEUDO *p;

  r1 = genstmt(n->left);
  r2 = genstmt(n->right);
  op = n->nop;
  if (reverse) op = revop(op);

  if ((p = previous) != NULL) {
      if (p->ptype == GLOBAL && p->pop == IDENT && p->preg == r2) {
          p->pop = op;
	  p->preg = r1;
      ~
      else {
	  if (p->pop == SETZ && p->ptype == ONEREG && p->preg == r2) {
	      p->ptype = IMMED;
	      p->pop = IDENT;
	      p->pvalue = 0;
          ~
   	  if (p->ptype == IMMED && p->preg == r2) {
	      p->ptype = RCONST;
	      p->pop = immedop(op);
	      p->preg = r1;
	   ~
	   else
	      code0(op, r1, r2);
      ~
      release(r1);
      release(r2);
      return;
   ~
   code0(op, r1, r2);
   release(r2);
   release(r1);
~

/* ----------------------------------- */
/*	reverse boolean operation      */
/* ----------------------------------- */

revop(op)
{
  switch (op) {
  case NEQ:
       return EQUAL;
  case EQUAL:
       return NEQ;
  case LESS:
       return GEQ;
  case GEQ:
       return LESS;
  case LEQ:
       return GREAT;
  case GREAT:
       return LEQ;
  ~
~

/* ------------------------------------------------------ */
/*	return immediate version of boolean operator      */
/* ------------------------------------------------------ */

immedop(op)
{
  switch (op) {
  case NEQ:
       return CAIN;
  case EQUAL:
       return CAIE;
  case LESS:
       return CAIL;
  case GEQ:
       return CAIGE;
  case LEQ:
       return CAILE;
  case GREAT:
       return CAIG;
  ~
~

directop(op)
{
  switch (op) {
  case CAIN:
       return NEQ;
  case CAIE:
       return EQUAL;
  case CAIL:
       return LESS;
  case CAIG:
       return GREAT;
  case CAILE:
       return LEQ;
  case CAIGE:
       return GEQ;
  ~
~


coerce(n)
struct NODE *n;
{
  int r, s, opt;
  switch (n->nc) {
  case NOCOER:
       return genstmt(n->left);
  case PI_PC:
       r = genstmt(n->left);
       pitopc(r,0);
       return r;
  case PC_PI:
       r = genstmt(n->left);
       pctopi(r);
       return r;
  case PC_PC:
       if (NBYTES =! 4) fprintf(stderr,"PC_PC not implemented if NBYTES=!4\n");
       r = genstmt(n->left);
       s = getreg();
       opt = optimize;
       optimize = 0;
       if (codesection == 0) {
	   code0(HRRZ, s, r);
	   code8(LSHFT, s, 2);
	   code8(ROT, r, 3);
	   code1(AND, r, 3);
/* previous 2 could be code1(LDB, r, r+...) */
	   optimize = opt;
	   code0(MINUS, s, r);
       ~
       else {
					/* this isn't very good */
	   code1(LDB, s, 0331100000000 + ((r > 15) ? regis[r] : r) );
	   code1(MINUS, s, 070);
	   code8(TLZ, r, 0770000);
	   code8(LSHFT, r, 2);
	   optimize = opt;
	   code0(PLUS, s, r);
       ~
       release(r);
       return s;
  default:
       fprintf(stderr, "Unknown coercion %d\n", n->nc);
       return 0;
  ~
~

static bytepos[4] = {0331100,0221100,0111100,01100~;
pitopc(r,bnum)
{
  if (NBYTES != 4) fprintf(stderr,"NBYTES!=4 not implemented in coercions\n");
  else if (codesection == 0) code1(HRL,r,bytepos[bnum]);
  else code8(TLO,r,0700000+(bnum<<12));
~

pctopi(r)
{
  if (codesection == 0) code1(HRL,r,0);
  else code8(TLZ,r,0770000);
~
 