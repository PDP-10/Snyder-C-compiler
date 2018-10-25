#include <stdio.h>

/* **************************************************************** */
/*	High-level Runtime Support for C (TOPS-20 & WAITS)	    */
/*					  			    */
/*	          k. chen  Aug, 1981	  			    */
/*								    */
/*   - ch[file_descriptor] is a JFN (TOPS20) or channel # (WAITS)   */
/*		except	0 -> TTY input (unredirected)		    */
/*			1 -> TTY output	(unredirected)		    */
/*								    */
/*   - on WAITS, assume command line is of the form:		    */
/*								    */
/*	.RUN FOO ; arg1 arg2 arg3 ...				    */
/*								    */
/*   - for both WAITS and TOPS-20, arguments are separated by       */
/*	spaces, tabs or semicolons.				    */
/* **************************************************************** */

#define	WAITS			/* *************** WAITS ****************** */

#define MAXFILE	  16		/* maximum opened files */
#define	UNDEF	  -1

#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
#define	GJSHT	0000001
#define	GJOLD	0100000
#define	GJFOU	0400000
#define OFWR	0100000
#define	OFRD	0200000
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */
struct	bufhead {
	int	system;
	char	*user;
	int	count;
~;

struct	filespec {
	int	name;
	int	extension;
	int	date;
	int	PPN;
~;
#endif			/* ****************** WAITS ********************* */

#define term(x)   (x == ';')
#define blank(x)  (x == ' ' || x == '\t')

typedef char    string[32];

static	int	ch[MAXFILE],
		ieof;

#ifdef	WAITS		/* ****************** WAITS ********************* */
static	struct	bufhead	buffers[MAXFILE];
#endif			/* ****************** WAITS ********************* */

runtm(n)
{
  int    argc,done;
  char   argbuf[256],*argv[32],*p;
  string in,out,other;
  
  argc = 0;
  in[0] = out[0] = '\0';
  if (n > 0) {

      /* get command line into string */

      p = argbuf;
      while (--n) *p++ = _getty();
      _getty();

#ifdef	WAITS		/* ****************** WAITS ********************* */
      p--;
#endif			/* ****************** WAITS ********************* */

      *p++ = '\0';

      /* scan string, make up *argv[] */

      p = argbuf;
      done = 0;
      while (!done) {
	  if blank(*p) while blank(*p) p++;
          if term(*p) {
  	      p++;
	      if blank(*p) while blank(*p) p++;
	  ~
	  switch (*p) {
	  case '\0':
	  case '\r':
	  case '\n':
	       done = 1;
	       break;
	  case '<':
	       p++;
	       if (in[0]) abort("%stdin redirected more than once.");
	       p = getstr(p,in);
	       break;
	  case '>':
	       p++;
	       if (out[0]) abort("%stdout redirected more than once.");
	       p = getstr(p,out);
	       break;
	  default:
	       argv[argc++] = p;
	       p = getstr(p,other);
	       if (!*p) {
	           done = 1;
		   break;
	       ~
	       *p++ = 0;

#ifdef	WAITS		/* ****************** WAITS ********************* */
		if (argc == 1 && (caseall(argv[argc-1],"RUN")
				  || caseall(argv[argc-1],"RU")
				  || caseall(argv[argc-1],"R"))) argc--;
#endif			/* ****************** WAITS ********************* */
          ~
      ~
  ~
  setup(in,out);	/* init I/O */
  stdio();
  main(argc,argv);	/* call user program */
  sexit();
  exit(0);
~

/* ---------------------- */
/*	set up files      */
/* ---------------------- */

static setup(in,out)
char *in,*out;
{
  int i;
  for (i = 3; i < MAXFILE; i++) ch[i] = UNDEF;
  ch[0] = (*in)  ? creat(in,0)  : 0;
  ch[1] = (*out) ? creat(out,0) : 1;
  ch[2] = 1;
  ieof = 0;
~

/* -------------------------------------------- */
/*	close all files and exit to monitor     */
/* -------------------------------------------- */

exit(n)
{
  int i;
  for (i=0; i < MAXFILE; i++) {
      if (ch[i] != UNDEF) close(i);
  ~
  _exit(n);
~

/* -------------------------------- */
/*	get a string from tty:      */
/* -------------------------------- */

static char *getstr(p,s)
char *p,*s;
{
  while (!term(*p) && !blank(*p) && *p != '\0') *s++ = *p++;
  *s = '\0';
  return p;
~

/* --------------------------------------- */
/*	output error message and exit      */
/* --------------------------------------- */

static abort(s)
char *s;
{
  while (*s) _putty(*s++);
  _putty('\n');
  _exit(0);
~

/* --------------- */
/*	write      */
/* --------------- */

write(f,buf,n)
char *buf;
{
  int    chan,i;
  char	 *c;
  struct bufhead *p;

  if ((chan=ch[f]) < 1 || n < 0) return -1;

  if (chan == 1) {  /* tty output */
      for (i = n; i-- ; ) _putty(*buf++);
      return n;
  ~
#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
  return (_write(chan,buf,n))? n : -1;
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  p = &buffers[chan];
  c = p->user;
  i = p->count;
  while (n--) {
      *++c = *buf++;
      if (--i <= 0) {
          if (!_out(chan)) return -1;
          c = p->user;
          i = p->count;
      ~
  ~
  p->user = c;
  p->count = i;
  return n;

#endif			/* ****************** WAITS ********************* */
~

/* -------------------- */
/*	byte write      */
/* -------------------- */

bwrite(f,buf,n)
char *buf;
{
  int    chan,i,save;
  char	 *c;
  struct bufhead *p;

  if ((chan=ch[f]) < 1 || n < 0) return -1;

  if (chan == 1) {  /* tty output */
      for (i = n; i-- ; ) _putty(*buf++);
      return n;
  ~
#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
  return (_write(chan,buf,n))? n : -1;
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  p = &buffers[chan];
  save = p->user;
  save++;
  c = (save & 0777777) | (0341000<<18);
  c--;
  i = p->count*4;
  while (n--) {
      *++c = *buf++;
      if (--i <= 0) {
          if (!_out(chan)) return -1;
	  save = p->user;
          c = (save & 0777777) | (0341000<<18);
          i = p->count*4;
      ~
  ~
  chan = c;
  p->user = (chan & 0777777) | (save & (0777777<<18));
  p->count = i/4;
  return n;

#endif			/* ****************** WAITS ********************* */
~

/* -------------------------- */
/*	image mode write      */
/* -------------------------- */

iwrite(f,buf,n)
char *buf;
{
  int    chan,i,save;
  char	 *c;
  struct bufhead *p;

  if ((chan=ch[f]) < 1 || n < 0) return -1;

  if (chan == 1) {  /* tty output */
      for (i = n; i-- ; ) _putty(*buf++);
      return n;
  ~
#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
  return (_write(chan,buf,n))? n : -1;
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  p = &buffers[chan];
  c = p->user;
  i = p->count;
  while (n--) {
      *++c = *buf++;
      if (--i <= 0) {
          if (!_out(chan)) return -1;
      ~
  ~
  p->count = i;
  p->user = c;
  return n;

#endif			/* ****************** WAITS ********************* */
~

/* -------------- */
/*	read      */
/* -------------- */

read(f,buf,n)
char *buf;
{
  int    chan,i,k,m;
  char	 *c;
  struct bufhead *p;
  
  if ((chan=ch[f]) == 1 || n < 0) return -1;

  if (!chan) {	/* tty input */
      if (ieof) return 0;
      for (i = 0; n-- ; i++) {
          *buf = _getln();
	  if (!*buf++) {
	      ieof = 1;
	      break;
	  ~
      ~
      return i;
  ~
#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
  return _read(chan,buf,n);
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  p = &buffers[chan];
  c = p->user;
  i = p->count;
  m = 0;
  k = n;
  while (n--) {
      if (i <= 0) {
          if (!_in(chan)) {
	      p->count = 0;
	      return m;
	  ~
          c = p->user;
          i = p->count;
      ~
      *buf++ = *++c;
      i--;
      m++;
  ~
  p->user = c;
  p->count = i;
  return k;

#endif			/* ****************** WAITS ********************* */
~

/* ------------------- */
/*	byte read      */
/* ------------------- */

bread(f,buf,n)
char *buf;
{
  int    chan,i,k,m, save, *addr;
  char	 *c;
  struct bufhead *p;
  
  if ((chan=ch[f]) == 1 || n < 0) return -1;

  if (!chan) {	/* tty input */
      if (ieof) return 0;
      for (i = 0; n-- ; i++) {
          *buf = _getty();
	  if (!*buf++) {
	      ieof = 1;
	      break;
	  ~
      ~
      return i;
  ~
#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
  return _read(chan,buf,n);
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  p = &buffers[chan];
  save = p->user;
  save++;
  c = (save & 0777777) | (0341000<<18);
  c--;
  i = p->count*4;
  m = 0;
  k = n;
  while (n--) {
      if (i <= 0) {
          if (!_in(chan)) {
	      p->count = 0;
	      return m;
	  ~
	  save = p->user;
	  save++;
          c = (save & 0777777) | (0341000<<18);
	  c--;
          i = p->count*4;
      ~
      *buf++ = *++c;
      i--;
      m++;
  ~
  chan = c;
  p->user = (chan & 0777777) | (save & (0777777<<18));
  p->count = i/4;
  return k;

#endif			/* ****************** WAITS ********************* */
~

/* ------------------------- */
/*	image mode read      */
/* ------------------------- */

iread(f,buf,n)
char *buf;
{
  int    chan,i,k,m, save, *addr;
  char	 *c;
  struct bufhead *p;
  
  if ((chan=ch[f]) == 1 || n < 0) return -1;

  if (!chan) {	/* tty input */
      if (ieof) return 0;
      for (i = 0; n-- ; i++) {
          *buf = _getty();
	  if (!*buf++) {
	      ieof = 1;
	      break;
	  ~
      ~
      return i;
  ~
#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */
  return _read(chan,buf,n);
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  p = &buffers[chan];
  i = p->count;
  m = 0;
  k = n;
  while (n--) {
      if (i <= 0) {
          if (!_in(chan)) {
	      p->count = 0;
	      return m;
	  ~
          i = p->count;
      ~
      *buf++ = *++p->user;
      i--;
      m++;
  ~
  p->count = i;
  return k;

#endif			/* ****************** WAITS ********************* */
~

/* ------------------- */
/*	open file      */
/* ------------------- */

open(name,mode)		/* 7-bit ASCII files */
char *name;
{
  _ofile(name,mode,7);
~

bopen(name,mode)	/* 8-bit byte open */
char *name;
{
  _ofile(name,mode,8);
~

iopen(name,mode)	/* 36-bit byte open */
char *name;
{
  _ofile(name,mode,36);
~

static _ofile(name,mode,omode)
char *name;
{
  int    channel,f,buf,device,disk;
  struct filespec fs;

#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */

  
  if ((mode = getmode(mode)) == -1) return -1;
  do {
    channel = _getchannel(name, GJOLD|GJSHT);
    if (channel < 0) return -1;
  ~ while (channel < 2);
  
  switch (omode) {
  case 7:
       if (_open(channel,mode)) break;
       return -1;
  case 8:
       if (_bopen(channel,mode)) break;
       return -1;
  case 36:
       if (_iopen(channel,mode)) break;
       return -1;
  ~
  if (f = getfd()) {
      ch[f] = channel;
      return f;
  ~
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  if (*name == ':') {
      name++;
      device = sixbit(name);
      disk = 0;
  ~    
  else {
      disk = device = sixbit("DSK");
  ~
  if (convert(name,&fs) && (f = getfd())) {
      buf = getbuf(f);
      switch (omode) {
      case 7:
           if (_open(f,buf,device)) break;
	   return -1; 
      case 8:
           if (_bopen(f,buf,device)) break;
	   return -1;
      case 36:
           if (_iopen(f,buf,device)) break;
	   return -1;
      ~
      if (disk) {
          if (!_lookup(f,&fs)) return -1;
          _in(f);
      ~
      return ch[f] = f;
  ~
#endif			/* ****************** WAITS ********************* */

  return -1;
~

/* -------------------------------- */
/*	create file      	    */
/*	ignore protection mode      */
/* -------------------------------- */

creat(name,mode)
char *name;
{
  _cfile(name,mode,7);
~

bcreat(name,mode)
char *name;
{
  _cfile(name,mode,8);
~

icreat(name,mode)
char *name;
{
  _cfile(name,mode,8);
~

static _cfile(name,mode,omode)
char *name;
{
  int    channel,f,buf,device,disk;
  struct filespec fs;

#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */

  do {
    channel = _getchannel(name, GJFOU|GJSHT);
    if (channel < 0) return -1;
  ~ while (channel < 2);
  mode = getmode(2);
  
  switch (omode) {
  case 7:
       if (_open(channel,mode) break;
       return -1;
  case 8:
       if (_bopen(channel,mode)) break;
       return -1;
  case 36:
       if (_iopen(channel,mode)) break;
       return -1;
  ~
  if (f = getfd()) {
      ch[f] = channel;
      return f;
  ~
#endif			/* ****************** TOPS-20 ******************* */

#ifdef	WAITS		/* ****************** WAITS ********************* */

  if (*name == ':') {
      name++;
      device = sixbit(name);
      disk = 0;
  ~
  else {
      disk = device = sixbit("DSK");
  ~
  if (convert(name,&fs) && (f = getfd())) {
      buf = getbuf(f);
      switch (omode) {
      case 7:
           if (_open(f,buf<<18,device)) break;
	   return -1; 
      case 8:
           if (_bopen(f,buf<<18,device)) break;
	   return -1;
      case 36:
           if (_iopen(f,buf<<18,device)) break;
	   return -1;
      ~
      if (disk) {
          if (!_enter(f,&fs)) return -1;
      ~
      _out(f);
      return ch[f] = f;
  ~
#endif			/* ****************** WAITS ********************* */

  return -1;
~


#ifdef	TOPS20		/* ****************** TOPS-20 ******************* */

static getmode(n)
{
  switch (n) {
  case 0:
       return OFRD;
  case 1:
       return OFWR;
  case 2:
       return OFRD|OFWR;
  default:
       return -1;
  ~
~
#endif			/* ****************** TOPS-20 ******************* */

/* -------------------- */
/*	close file      */
/* -------------------- */

close(f)
{
  int chnl;

  chnl = ch[f];
  if (chnl == UNDEF) abort("File never opened.");
  if (chnl < 2) return;
  _close(chnl);
  ch[f] = UNDEF;
~

/* ---------------------------------------- */
/*	find an unused file descriptor      */
/* ---------------------------------------- */

getfd()
{
  int n;
  n = 3;
  while (n < MAXFILE && ch[n] != UNDEF) n++;
  return (n < MAXFILE) ? n : 0;
~


#ifdef	WAITS		/* ****************** WAITS ********************* */

/* ---------------------------------------------- */
/*	compare string with uppercase string      */
/* ---------------------------------------------- */

static caseall(s,t)
char *s,*t;
{
  while (up(*s) == *t++) {
      if (!*s++) return 1;
  ~
  return 0;
~

static up(c)
{
  if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
  return c;
~

/* --------------------------------------------------- */
/*	obtain a sixbit representation for string      */
/* --------------------------------------------------- */

static sixbit(s)
char *s;
{
  int n,w,t;
  w = 0;
  n = 30;
  while (*s && n >= 0) {
      t = *s++;
      if (t & 0100) t = t | 040; else t = t & 0137;
      w += (t&077) << n;
      n -= 6;
  ~
  return w;
~

/* ----------------------------------------------------------------------- */
/*	convert a filename string to internal filespec representation      */
/* ----------------------------------------------------------------------- */

static convert(s,t)
char *s; struct filespec *t;
{
  char u[16],*v;
  int  state;

  state = 1;
  t->name = t->extension = t->date = t->PPN = 0;
  while (state) {
      v = u;
      while (!fend(*s)) *v++ = *s++;
      *v = 0;
      switch (state) {
      case 1:
           t->name = sixbit(u);
           switch (*s) {
	   case 0:
	        state = 0;
		break;
	   case '.':
	        state = 2;
		break;
	   case '[':
	        state = 3;
		break;
	   default:
	        return 0;
	   ~
	   s++;
	   break;
      case 2:
           t->extension = sixbit(u) & 0777777000000;
	   switch (*s) {
	   case 0:
	        state = 0;
		break;
	   case '[':
	        state = 3;
		break;
	   default:
	        return 0;
	   ~
	   s++;
	   break;
      case 3:
           t->PPN = rightjust(sixbit(u));
	   switch (*s) {
	   case ',':
	        state = 4;
		break;
	   default:
	        return 0;
	   ~
	   s++;
	   break;
      case 4:
           t->PPN += (rightjust(sixbit(u)) >> 18);
	   state = 0;
	   break;
      default:
           return 0;
      ~
  ~
  return 1;
~

static fend(c)
{
  if (c == 0 || c == '.' || c == '[' || c == ',' || c == ']') return 1;
  return 0;
~

static rightjust(n)
{
  if (n = n & 0777777000000) {
      while (!(n & 077000000)) n = n >> 6;
  ~
  return n;
~

/* -------------------------------------------------------- */
/*	create an integer that contains buffer address      */
/* -------------------------------------------------------- */

static getbuf(f)
{
  return &buffers[f];
~

#endif			/* ****************** WAITS ********************* */

/* ---------------------------------------------- */
/*	convert string of decimal to integer      */
/* ---------------------------------------------- */

atoi(s)		/* convert s to integer */
char s[];
{
  int i,n;
  n = 0;
  for (i=0; s[i] >= '0' && s[i] <= '9'; ++i)
      n = 10*n + s[i] - '0';
  return n;
~

/* ------------------------------------------------------------ */
/*	returns false if file fp has no characters waiting      */
/*	if fp is NULL, keyboard is assumed			*/
/* ------------------------------------------------------------ */

ttyskp(fp)
FILE *fp;
{
  if (fp == NULL) return tskp(-1);
  return tskp(ch[fp->_fd]);
~
   