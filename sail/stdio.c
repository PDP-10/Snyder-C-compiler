/* -------------------------------------------- */
/*	stdio.c		(TOPS-20 & WAITS)	*/
/*						*/
/*	reference:				*/
/*	  Kernighan,B.W., D.M.Ritchie,		*/
/*	  "The C Programming Language",		*/
/*	  Prentice-Hall, 1978			*/
/*						*/
/*	stdio (initializations)			*/
/*	fopen, fclose				*/
/*	getc, putc, ungetc, fgets, fputs	*/
/*	sprintf, fprintf, printf		*/
/*	_flushbuf, _fillbuf, __findbuf, _bin	*/
/* -------------------------------------------- */

#define	WAITS		/* **************** WAITS ******************** */

#define	_BUFSIZE  512
#define	_NFILE	   10
#define _NBUFF     16
#define _MAXJFN  0105
#define  PMODE	 0644

#define	_READ	01	/* file opened for reading */
#define	_WRITE	02	/* file opened for writing */
#define	_UNBUF	04	/* file is unbuffered */
#define	_BIGBUF	010	/* big buffer allocated	*/
#define _BINARY 020	/* 8-bit image mode file */
#define	_IMAGE	040	/* 36-bit image mode file */
#define	_FIRST	0100	/* first read of file, for WAITS E files */
#define _BYTE   0200	/* 8-bit files */
#define	_EFILE	0400	/* E editor file */

#define	_EOF	01000	/* EOF occured		   */
#define	_ERR	02000	/* error occured	   */

#define	NULL	0
#define	EOF	(-1)

/* ------------------------------------- */
/*	file structure [ref. p.165]      */
/* ------------------------------------- */

typedef struct	_iobuf {
		char *_ptr;	/* next character position   */
		int  _cnt;	/* number of characters left */
		char *_base;	/* location of buffer	     */
		int  _flag;	/* file access mode	     */
		int  _fd;	/* file descriptor	     */
	~ FILE;


FILE	_iob[_NFILE] = {
	NULL, 0, NULL, _READ|_UNBUF,  0,	/* stdin  */
	NULL, 0, NULL, _WRITE,        1,	/* stdout */
	NULL, 0, NULL, _WRITE|_UNBUF, 2		/* stderr */
	~;

FILE	*stdin,
	*stdout,
	*stderr;

static struct	{ char   body[_BUFSIZE];
		  int	 use;
		~ _bigbuf[_NBUFF];

static  char	_smallbuf[_MAXJFN];

/* ----------------------------------------------- */
/*	stdio called at runtime initialization     */
/* ----------------------------------------------- */

stdio ()
{
  int i;
  for (i = 0 ; i < _NBUFF ; i++) _bigbuf[i].use = 0;
  for (i = 3 ; i < _NFILE ; i++) _iob[i]._flag = 0;

  stdin =  &_iob[0];
  stdout = &_iob[1]; stdout->_cnt = 0;
  stderr = &_iob[2]; stderr->_cnt = 0;
~  

/* ------------------------------ */
/*	open buffered file        */
/*	  [ref. p.151]		  */
/* ------------------------------ */

FILE *fopen (name, mode)
char *name, *mode;
{
  int  i, m, n, fd, flag;
  FILE *fp;

  m = *mode++;
  n = *mode;
  if (n || 
     (m != 'r' && m != 'w' && m != 'a' && 
      m != 'R' && m != 'W' &&
      m != 'i' && m != 'o')) {
          fputs ("illegal mode ", stderr);
          fputs (mode, stderr);
          fputs (" opening ", stderr);
          fputs (name, stderr);
          putc ('\n', stderr);
          sexit (1);
  ~

  for (i=3 ; i < _NFILE ; i++) 
      if (!(_iob[i]._flag & (_READ|_WRITE))) break;
  if (i >= _NFILE) {
      fputs ("No I/O buffer left.\n", stderr);
      return NULL;
  ~
  fp = &_iob[i];

  switch (m) {
  case 'r':
       fd = open (name, 0);
       flag = _READ | _FIRST;
       break;
  case 'R':
       fd = bopen (name, 0);
       flag = _READ | _BYTE;
       break;
  case 'i':
       fd = iopen (name, 0);
       flag = _READ | _IMAGE;
       break;
  case 'w':
       fd = creat (name, PMODE);
       flag = _WRITE;
       break;
  case 'W':				
       fd = bcreat (name, PMODE);
       flag = _WRITE | _BYTE;
       break;
  case 'o':				
       fd = icreat (name, PMODE);
       flag = _WRITE | _IMAGE;
       break;
  ~

  if (fd < 0) return NULL;

  fp->_fd = fd;
  fp->_cnt = 0;
  fp->_base = NULL;
  fp->_flag = flag;
  return (fp);
~


/* ---------------------------------------- */
/*	open unbuffered file      	    */
/*	  same as fopen but unbuffered      */
/* ---------------------------------------- */

FILE *uopen (name, mode)
char *name, *mode;
{
  int  i, m, n, fd, flag;
  FILE *fp;

  m = *mode++;
  n = *mode;
  if (n || 
     (m != 'r' && m != 'w' && m != 'a' && 
      m != 'R' && m != 'W' &&
      m != 'i' && m != 'o')) {
          fputs ("illegal mode ", stderr);
          fputs (mode, stderr);
          fputs (" opening ", stderr);
          fputs (name, stderr);
          putc ('\n', stderr);
          sexit (1);
  ~

  for (i=3 ; i < _NFILE ; i++) 
      if (!(_iob[i]._flag & (_READ|_WRITE))) break;
  if (i >= _NFILE) {
      fputs ("No I/O buffer left.\n", stderr);
      return NULL;
  ~
  fp = &_iob[i];

  switch (m) {
  case 'r':
       fd = open (name, 0);
       flag = _READ | _FIRST | _UNBUF;
       break;
  case 'R':
       fd = bopen (name, 0);
       flag = _READ | _BYTE | _UNBUF;
       break;
  case 'i':
       fd = iopen (name, 0);
       flag = _READ | _IMAGE | _UNBUF;
       break;
  case 'w':
       fd = creat (name, PMODE);
       flag = _WRITE | _UNBUF;
       break;
  case 'W':				
       fd = bcreat (name, PMODE);
       flag = _WRITE | _BYTE | _UNBUF;
       break;
  case 'o':				
       fd = icreat (name, PMODE);
       flag = _WRITE | _IMAGE | _UNBUF;
       break;
  ~

  if (fd < 0) return NULL;

  fp->_fd = fd;
  fp->_cnt = 0;
  fp->_base = NULL;
  fp->_flag = flag;
  return (fp);
~

/* ---------------------------- */
/*	close buffered file     */
/*	   [ref. p.153]		*/
/* ---------------------------- */

fclose (fp)
FILE *fp;
{
  char *p;
  int  i;

  if (fp->_flag & _WRITE) _flushbuf (0, fp);
  p = fp->_base;
  for (i=0 ; i < _NFILE ; i++) {
      if (_bigbuf[i].body == p) {
          _bigbuf[i].use = 0;
	  break;
      ~
  ~
  close (fp->_fd);
  fp->_flag = 0;
~

/* ------------------------------ */
/*	close files and exit      */
/* ------------------------------ */

sexit (n)
{
  int i;

  _flushbuf (0, stdout);
  _flushbuf (0, stderr);

  for (i=3 ; i < _NFILE ; i++) {
      if (_iob[i]._flag) {
	 fclose (&_iob[i]);
	 _iob[i]._ptr = NULL;
	 _iob[i]._cnt = 0;
	 _iob[i]._base = NULL;
      ~
  ~
~

/* ------------------------------------ */
/*	get character from buffer       */
/*	     [ref. p.166]		*/
/* ------------------------------------ */

getc (fp)
FILE *fp;
{
  int i;

  if (--fp->_cnt >= 0) {
      i = *fp->_ptr++;
      if (i == '\r') {
	  i = getc (fp);
	  if (i != '\n') {
	      ungetc (i, fp);
	      return '\r';
          ~
      ~
#ifdef WAITS
      if (i == 0 && (fp->_flag & _EFILE)) {
          while (!*fp->_ptr && --fp->_cnt >= 0) fp->_ptr++;
	  return getc(fp);
      ~
#endif
      return i;
  ~
  else {
      return _fillbuf (fp);
  ~
~

/* ------------------------------- */
/*	pushback a character       */
/* ------------------------------- */

ungetc (x, fp)
FILE *fp;
{
  fp->_cnt++;
  fp->_ptr--;
  *fp->_ptr = x;
~

/* ------------------------------------ */
/*	output character to buffer      */
/* ------------------------------------ */

putc (x, fp)
FILE *fp;
{
  int i;

  if (x == '\n' && !(fp->_flag&_BYTE)) putc ('\r', fp);

  if (fp->_cnt-- > 0) {
      *fp->_ptr++ = x;
      if ((fp == stdout || fp == stderr) && x == '\n') {
	  _flushbuf (0, fp);
      ~
  ~
  else {
      if ((fp == stdout || fp == stderr) && x == '\n') {
          *fp->_ptr++ = '\n';
          fp->_cnt = 1;
	  _flushbuf (0, fp);
      ~
      else {      
          if (fp->_flag & _UNBUF) {
	      fp->_cnt = 1;
	      _flushbuf (x, fp);
	  
          ~
          else {
              fp->_cnt++;
              _flushbuf (x, fp);
          ~
      ~
  ~
~

/* -------------------------------- */
/*   putc with no \r for compiler   */
/* -------------------------------- */

_putc (x, fp)
FILE *fp;
{
  if (fp->_cnt-- > 0) {
      *fp->_ptr++ = x;
      if ((fp == stdout || fp == stderr) && x == '\n') {
	  _flushbuf (0, fp);
      ~
  ~
  else {
          fp->_cnt++;
          _flushbuf (x, fp);
  ~
~

/* -------------------- */
/*	  get line      */
/*	[ref. p.155]	*/
/* -------------------- */

char *fgets (line, maxline, fp)
FILE *fp; char *line;
{
  int  i, c; 
  char *l;

  l = line;
  for (i=1 ; i < maxline ; i++) {
      if ((c=getc (fp)) == EOF) {
	  line = NULL;
	  break;
      ~
      if ((*l++ = c) == '\n') break;
  ~
  *l = 0;
  return line;
~

/* ----------------------- */
/*	output string      */
/*	[ref. p.155]	   */
/* ----------------------- */

fputs (s, fp)
char *s; 
FILE *fp;
{
  while (*s) putc (*s++, fp);
~

/* ------------------------------------ */
/*	flush buffered file output	*/
/*	     [ref. p.166]		*/
/* ------------------------------------ */

_flushbuf (x, fp)
FILE *fp;
{
  int   n, flag, cnt, unbuf;
  char *s;

  if (!(fp->_flag & _WRITE)) return;

  unbuf = fp->_flag & _UNBUF;
  if (unbuf && !x && !fp->_cnt) return;

  if (fp->_base == NULL) {
     __findbuf (fp);
     fp->_ptr = fp->_base;
     fp->_cnt = _BUFSIZE ;
  ~
  if (unbuf && x) {
      *fp->_base = x;
      write (fp->_fd, fp->_base, 1);
      fp->_ptr = fp->_base;
      fp->_cnt = 0;
      return;
  ~
  if (n = (unbuf) ? 1 : _BUFSIZE - fp->_cnt) {
      if (fp->_flag & _BYTE) {
          bwrite (fp->_fd, fp->_base, n);
      ~
      else if (fp->_flag & _IMAGE) {
          iwrite (fp->_fd, fp->_base, n);
      ~
      else write (fp->_fd, fp->_base, n);

      fp->_ptr = fp->_base;
      fp->_cnt = (unbuf) ? 0 :_BUFSIZE;
  ~
  if (x) {
      *fp->_ptr++ = x;
      fp->_cnt--;
  ~
~

/* ------------------------------------------ */
/*	fill buffer and return character      */
/* ------------------------------------------ */

_fillbuf (fp)
FILE *fp;
{
  int  i, flag, c, cnt;
  char *s;

  flag = fp->_flag;
  if (!(flag & _READ) || (flag & (_EOF|_ERR))) return EOF;

  if (fp->_base == NULL) __findbuf (fp);

  fp->_ptr = fp->_base;
  flag = fp->_flag;

  if ((flag & _BYTE) && !(flag & _UNBUF)) {
      cnt = bread (fp->_fd, fp->_base, _BUFSIZE);
  ~
  else if (flag & _IMAGE) {
      cnt = iread (fp->_fd, fp->_base, (flag & _UNBUF) ? 1 : _BUFSIZE);
  ~
  else {
      cnt = read (fp->_fd, fp->_base, (flag & _UNBUF) ? 1 : _BUFSIZE);
  ~

/* ******************************************************************* */
#ifdef WAITS		/* check for E directory page */

  if (fp->_flag & _FIRST) {
      fp->_flag = fp->_flag & (_FIRST);
      if (cnt >= 9) {
          if (check("COMMENT ",fp->_base)) {
	      s = fp->_base;
	      while (*s != 014) {
	          if (--cnt <= 0) {
		      cnt = read(fp->_fd, fp->_base, _BUFSIZE);
		      if (cnt <= 0) { /* error of some sort */
		          fp->_flag = fp->_flag | _ERR;
			  fp->_cnt = 0;
			  return EOF;
		      ~
		      s = fp->_base;
		  ~
		  else s++;
	      ~
	      fp->_ptr = ++s;
	      fp->_cnt = cnt;
	      c = getc(fp);
	      fp->_flag = fp->_flag | _EFILE;
	      return c;
	  ~
      ~
  ~

#endif
/* ******************************************************************* */

  fp->_cnt = cnt;
  if (fp->_cnt <= 0)  {
      fp->_flag = fp->_flag | (cnt == -1) ? _EOF : _ERR ;
      fp->_cnt = 0;
      return EOF;
  ~
  i = getc(fp);
  return i;
~

#ifdef WAITS		/* ******************** WAITS ******************** */

static check(s,t)
char *s,*t;
{
  while (*s++ == *t) t++ ;
  return (*t == 026);
~

#endif

/* ----------------------------- */
/*	find buffer for I/O      */
/* ----------------------------- */

__findbuf (fp)
FILE *fp;
{
  int flag, i;

  flag = fp->_flag;
  if (flag & _UNBUF) {
      fp->_base = &_smallbuf[fp->_fd];	/* unbuffered */
      return;
  ~
  for (i=3 ; i < _NBUFF ; i++)
      if (_bigbuf[i].use == 0) break;

  if (i < _NBUFF) {
      fp->_flag = flag | _BIGBUF;
      fp->_base = _bigbuf[i].body;
      _bigbuf[i].use = 1;
  ~
  else {
      fp->_flag = flag | _UNBUF;
      fp->_base = &_smallbuf[fp->_fd];	/* unbuffered */
  ~
~

/* ------------------------------------------- */
/*	in-memory conversion [ref. p. 150]     */
/* ------------------------------------------- */

sprintf (str, control, a, b, c, d, e, f, g, h)
char *str, *control;
{
  int sign, value, def, i;

  while (1) {
      switch (*control) {
      case 0: 
	  *str = 0;
   	  return;
      case '%':
	  control++;
	  if (sign=(*control == '-')) control++;
	  value = 0;
   	  while (*control >= '0' && *control <= '9') {
	      value = value*10 + *control++ - '0';
	      ~
	  if (sign) value = -value;
	  def = 1;

	  switch (*control) {
          case 'c':
	       *str++ = a;
	       break;
	  case 'd':
	       __pn (&str, a, value, 10);
	       break;
	  case 'o':
	       __pn (&str, a, value, 8);
	       break;
	  case 's':
	       __ps (&str, a, value);
	       break;
	  case 'x':
	       __pn (&str, a, value, 16);
	       break;
	  default:
	       def = 0;
	  ~

 	  if (def) {
	      sprintf (str, ++control, b, c, d, e, f, g, h);
	      return;
      	  ~

      default:
	  *str++ = *control++;
      ~
  ~
~

/* --------------------------------------- */
/*	formatted output [ref. p. 145]     */
/* --------------------------------------- */

printf (control, a, b, c, d, e, f, g, h)
char *control;
{
  char str[132];
  sprintf (str, control, a, b, c, d, e, f, g, h);
  fputs (str, stdout);
~

/* -------------------------------------------- */
/*	file formatted output [ref. p. 152]     */
/* -------------------------------------------- */

fprintf (fp, control, a, b, c, d, e, f, g, h)
FILE *fp;
char *control;
{
  char str[132];
  sprintf (str, control, a, b, c, d, e, f, g, h);
  fputs (str, fp);
~

/* ---------------------- */
/*	print string      */
/* ---------------------- */

__ps (s, t, n)	/* string */
char **s, *t;
{
  int  i, field;
  char *u;

  field = (n < 0) ? (-n) : n;
  i = field - _ps_(t);
  u = *s;
  if (n < 0) {
      while (*t) *u++ = *t++;
      while (i-- > 0) *u++ = ' ';
  ~
  else {
      while (i-- > 0) *u++ = ' ';
      while (*t) *u++ = *t++;
  ~
  *s = u;
~

_ps_ (s)
char *s;
{
  int n;
  n = 0;
  while (*s++) n++;
  return n;
~

/* ---------------------- */
/*	print number      */
/* ---------------------- */

__pn (s, t, n, base)
char **s;
{
  int  i, field;
  char *u, *v, str[132];

  field = (n < 0) ? (-n) : n;
  v = str;

  switch (base) {
  case 8:
       i = _po_(&v, t);
       break;
  case 10:
       i = _pd_(&v, t);
       break;
  case 16:
       i = _px_(&v, t);
       break;
  ~

  i = field - i;
  *v = '\0';
  u = *s;
  v = str;
  if (n < 0) {
      while (*v) *u++ = *v++;
      while (i-- > 0) *u++ = ' ';
  ~
  else {
      while (i-- > 0) *u++ = ' ';
      while (*v) *u++ = *v++;
  ~
  *s = u;
~

/* ---------------------- */
/*	print decimal     */
/* ---------------------- */

_pd_ (s,t)
char **s;
{
  char *u;
  int i,n;

  if (t < 0) {
      t = -t;
      u = *s;
      *u++ = '-';
      *s = u;
      n = 2;
  ~
  else n = 1;

  if (i=t/10)
      n = _pd_ (s, i) + n;

  u = *s;
  *u++ = t%10 + '0';
  *s = u;
  return n;
~

/* --------------------- */
/*	print octal      */
/* --------------------- */

_po_ (s,t)
char **s;
{
  char *u;
  int i,n;

  if (i=t>>3)	/* assume logical shift */
      n = _po_ (s, i) + 1;
  else
      n = 1;
  u = *s;
  *u++ = (t&07) + '0';
  *s = u;
  return n;
~

/* --------------------------- */
/*	print hexadecimal      */
/* --------------------------- */

_px_ (s,t)
char **s;
{
  char *u;
  int i,n;

  if (i=t>>4)	/* assume logical shift */
      n = _px_ (s, i) + 1;
  else
      n = 1;
  i = (t&0xF);
  u = *s;
  *u++ = (i < 10) ? i + '0' : i + 'A' - 10;
  *s = u;
  return n;
~
   