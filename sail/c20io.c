# include "c.defs"
# undefine channel

/*
 *
 *	C20IO - C TOPS-20 I/O Routines (written in C)
 *
 *	Routines:
 *
 *	fd = copen (fname, mode, opt)
 *	setprompt (s)
 *	c = cgetc (fd)
 *	ungetc (c, fd)
 *	i = cgeti (fd)
 *	cputc (c, fd)
 *	cputi (i, fd)
 *	b = ceof (fd)
 *	cflush (fd)
 *	rew (fd)
 *	b = istty (fd)
 *	cclose (fd)
 *	c = getchar ()
 *	gets (s)
 *	putchar (c)
 *	puts (s)
 *	closall ()
 *	b = cisfd (fd)
 *	s = username ()
 *	sleep (nsec)
 *
 *	System-dependent routines:
 *
 *	jfn = cjfn (fd) 
 *	valret (s)
 *
 *	Internal routines:
 *
 *	setup ()
 *	n = parse (s, v)
 *	setio ()
 *	fd = c0open (fname, mode)
 *	errout (s)
 *	p = mkbptr (s)
 *
 *	Variables:
 *
 *	cin - standard input channel
 *	cout - standard output channel
 *	cerr - standard error output channel
 *
 *	cinfn - standard input file name (if redirected)
 *	coutfn - standard output file name (if redirected)
 *	cerrfn - standard errout file name (if redirected)
 *
 *
 */

# rename copen_options "COPTNS"
# rename channel_allocate "CHALLC"
# rename channel_free "CHFREE"

# define EOL 037		/* is this really used as newline on TENEX? */
# define QUOTE 026		/* control-V, for file names */
# define ARGQUOTE '\\'		/* for command args */

# define TTYBUFSIZ 200
# define FILBUFSIZ 0200

	/* file types */

# define FTTY 0		/* interactive terminal */
# define FTEXT 1	/* text file */
# define FBIN 2		/* binary file */
# define FSTR 3		/* string I/O */
# define FBTEXT 4	/* text file (buffered input) - not yet implemented */

	/* file directions */

# define FREAD 0
# define FWRITE 1

typedef struct _channel {
	int jfn;		/* JFN */
	struct _channel *next;	/* next channel in CHANLIST */
	int ftype;		/* file type */
	int direction;		/* I/O direction */
	char *sptr;		/* pointer to next char in string|buffer */
	char *eptr;		/* pointer to end of buffer (char following) */
	int peekc;		/* look-ahead character */
	int eof;		/* end-of-file flag (never reset) */
	char *buf;		/* buffer */
	} *channel;

channel	c0open(), channel_allocate();

channel	cin,			/* standard input unit */
	cout,			/* standard output unit */
	cerr;			/* standard error output unit */

char	*cinfn,		/* standard input file name, if redirected */
	*coutfn,	/* standard output file name, if redirected */
	*cerrfn;	/* standard errout file name, if redirected */

int	cerrno;			/* system OPEN error codes returned here */

static char *tty_prompt;

/**********************************************************************

	COPEN - CIO Open File

	Open a file, given a file name, an optional mode, and an
	optional options string.  The possible modes are

		'r' - read
		'w' - write
		'a' - append

	The default mode is read.  Normally, I/O is character oriented
	and produces text files.  In particular, the lines of a text
	file are assumed (by the user) to be separated by newline
	characters with any conversion to the system format performed
	by the I/O routines.

	If an options string is given and contains the character "b",
	then I/O is integer (word) - oriented and produces image files.

	I/O to and from character strings in core is accomplished by
	including "s" in the options string and supplying a character
	pointer to the string to be read or written into as the first
	argument to COPEN.  Closing a string open for write will
	append a NULL character to the string and return a character
	pointer to that character.

	COPEN returns a CHANNEL, which is a pointer to a control block.
	The external variables CIN, COUT, and CERR contain already-open
	channels for standard input, standard output, and standard
	error output, respectively.

	COPEN returns OPENLOSS in case of error.  The system error code is
	stored in CERRNO.

**********************************************************************/

channel copen (fname, mode, opt)
	char *fname, *opt;

	{register channel p;
	int append;		/* TRUE if append mode */
	int direction;		/* read or write */
	int ftype;		/* file type */
	int jfn;

	cerrno = 0;
	copen_options (mode, opt, &direction, &ftype, &append);
	if (ftype == FSTR)	/* string I/O */
		{if (append) while (*fname) ++fname;
		}
	else if (fname[0] == 0 && ftype == FTEXT)	/* primary I/O */
		{jfn = 0100;
		if (direction == FWRITE) jfn = 0101;
		}
	else
		{int bs, oflag;
		oflag = 01000000;	/* GTJFN short form */
		if (direction == FWRITE && !append)
			oflag =| 0400000000000;	/* "output" use */
		if (direction == FREAD)
			oflag =| 0100000000000;	/* require old file */
		jfn = SYSGTJFN (oflag, mkbptr (fname));
		if (jfn >= 0600000)
			{cerrno = jfn;
			return (OPENLOSS);
			}
		bs = 7;
		if (ftype == FBIN) bs = 36;
		oflag = 0200000;	/* READ */
		if (direction == FWRITE)
			{if (append) oflag = 020000;
			else oflag = 0100000;
			}
		cerrno = SYSOPENF (jfn, oflag | (bs << 30));
		if (cerrno)
			{SYSRLJFN (jfn);
			return (OPENLOSS);
			}
		}

	p = channel_allocate ();
	if (p == 0) return (OPENLOSS);

	p->jfn = jfn;
	p->ftype = ftype;
	p->direction = direction;
	if (ftype == FSTR) p->sptr = fname;
	if (ftype == FTEXT && istty (p)) p->ftype = FTTY;
	if (p->ftype == FTTY && p->direction == FREAD)
		{p->buf = calloc (TTYBUFSIZ);
		p->sptr = p->buf;
		}
	if (p->ftype == FTEXT && p->direction == FREAD)
		{p->ftype = FBTEXT;
		p->buf = calloc (FILBUFSIZ);
		p->eptr = p->sptr = p->buf+FILBUFSIZ;
		}
	return (p);
	}

/**********************************************************************

	SETPROMPT

**********************************************************************/

setprompt (s)
	char *s;

	{tty_prompt = s;
	return (0);
	}

/**********************************************************************

	CGETC

**********************************************************************/

int cgetc (p)
	channel p;

	{int c, n;

	if ((c = p->peekc) >= 0)
		{p->peekc = -1;
		return (c);
		}
	if (p->eof) return (EOF_VALUE);

	switch (p->ftype) {

	case FBTEXT:	if (p->sptr >= p->eptr)
				{int nc;
				nc=SYSSIN(p->jfn,mkbptr(p->buf),-FILBUFSIZ,0);
				nc = FILBUFSIZ+nc;
				if (nc == 0)
					{p->eof = TRUE;
					return (EOF_VALUE);
					}
				p->eptr = p->buf+nc;
				p->sptr = p->buf;
				}
			c = *p->sptr++;
			if (c == '\r') return (cgetc (p));
			return (c);

	case FTTY:	c = *p->sptr++;
			if (c)
				{if (c == EOL) c = '\n';
				else if (c == '\r') return (cgetc (p));
				else if (c == 032)
					{c = 0;
					p->eof = TRUE;
					}
				return (c);
				}
			if (tty_prompt) SYSPSOUT (mkbptr (tty_prompt));
			p->sptr = p->buf;
			n = rdtty (p->buf, TTYBUFSIZ-1, tty_prompt);
			p->buf[n] = '\n';
			p->buf[n+1] = 0;
			return (cgetc (p));

	case FTEXT:	c = SYSBIN (p->jfn);
			if (c == 0) /* possible end-of-file */
				{if (_GTSTS(p->jfn)&001000000000) /* EOF */
					return (EOF_VALUE);
				}
			/* else if (c == EOL) c = '\n'; */
			else if (c == '\r') return (cgetc (p));
			return (c);

	case FBIN:	return (SYSBIN (p->jfn));
			
	case FSTR:	c = *p->sptr;
			if (c) {++p->sptr; return (c);}
			return (EOF_VALUE);

	default:	return (EOF_VALUE);
		}
	}

/**********************************************************************

	UNGETC

**********************************************************************/

ungetc (c, p)
	channel p;

	{p->peekc = c;	/* CEOF may lose */
	}

/**********************************************************************

	CGETI

**********************************************************************/

int cgeti (p)
	channel p;

	{return (cgetc (p));}

/**********************************************************************

	CPUTC

**********************************************************************/

int cputc (c, p)
	int c;
	channel p;

	{switch (p->ftype) {

	case FTTY:
	case FTEXT:	if (c == '\n') SYSBOUT (p->jfn, '\r');
			/* fall through */
	case FBIN:	SYSBOUT (p->jfn, c); break;
			
	case FSTR:	*p->sptr++ = c; break;
		}

	return (c);
	}

/**********************************************************************

	CPUTI

**********************************************************************/

int cputi (i, p)
	channel p;

	{return (cputc (i, p));}

/**********************************************************************

	CEOF

**********************************************************************/

int ceof (p)
	channel p;

	{int sts;

	if (p->eof) return (TRUE);
	switch (p->ftype) {
	case FBTEXT:	return (FALSE);
	case FTTY:
	case FTEXT:
	case FBIN:	sts = _GTSTS (p->jfn) & 0001000000000;
			break;
			
	case FSTR:	sts = (*p->sptr == 0);
			break;

	default:	sts = TRUE;
			}
	if (sts) p->eof = TRUE;
	return (sts);
	}

/**********************************************************************

	CFLUSH

**********************************************************************/

cflush (p)
	channel p;

	{return (0);}

/**********************************************************************

	REW

**********************************************************************/

rew (p)
	channel p;

	{if (p->ftype != FSTR) SYSSFPTR (p->jfn, 0);
	if (p->ftype == FBTEXT) p->sptr = p->eptr;
	return (0);
	}

/**********************************************************************

	ISTTY

**********************************************************************/

int istty (p)
	channel p;

	{int vals[3], dvt;

	if (p->ftype == FSTR) return (FALSE);
	if (p->ftype == FTTY) return (TRUE);
	SYSDVCHR (p->jfn, vals);
	dvt = (vals[1] >> 18) & 0777;
	return (dvt == 012 || dvt == 013);
	}

/**********************************************************************

	CCLOSE

**********************************************************************/

cclose (p)
	channel p;

	{if (p->ftype == FSTR)
		{char *s;
		s = p->sptr;
		*s = 0;
		channel_free (p);
		return (s);
		}
	SYSCLOSF (p->jfn);	/* close file and releae jfn */
	channel_free (p);
	return (0);
	}

/**********************************************************************

	GETCHAR - Read a character from the standard input unit

**********************************************************************/

getchar () {return (cgetc (cin));}

/**********************************************************************

	GETS - Read a string from the standard input unit

**********************************************************************/

gets (p)
	char *p;

	{int c;

	while ((c = cgetc (cin)) != '\n' && c>0) *p++ = c;
	*p = 0;
	}

/**********************************************************************

	PUTCHAR - Output a character to the standard output unit

**********************************************************************/

putchar (c)
	int c;

	{return (cputc (c, cout));}

/**********************************************************************

	PUTS - Output a string to the standard output unit

**********************************************************************/

puts (s)
	char *s;

	{int c;

	while (c = *s++) cputc (c, cout);
	cputc ('\n', cout);
	}

/**********************************************************************

	RDTTY - C interface to SYSRDTTY

**********************************************************************/

int rdtty (buf, n, prompt)
	char *buf, *prompt;

	{register char *p;
	if (prompt) prompt = mkbptr (prompt);
	SYSRDTTY (mkbptr (buf), n, prompt);
	p = buf;
	n = 0;
	while (TRUE)
		{register char c;
		c = *p & 0177;
		*p++ = c;
		if (c==EOL || c=='\r' || c=='\n' || c==0) break;
		++n;
		}
	p[-1] = 0;
	return (n);
	}

/**********************************************************************

	CJFN - Return JFN of file.

**********************************************************************/

int cjfn (p)
	channel p;

	{return (p->jfn);}

# define COMSIZE 250	/* size of command buffer */
# define MAXARG 40	/* maximum number of command arguments */

# rename combuf "COMBUF"
# rename argv "ARGV"
# rename argc "ARGC"
# rename setup "$SETUP"
# rename setio "$SETIO"
# rename parse "$PARSE"
# rename errout "ERROUT"

char combuf[COMSIZE];	/* command buffer */
char *argv[MAXARG];	/* command arguments */
int argc;		/* number of command arguments */

/**********************************************************************

	SETUP - Initialization Routine

**********************************************************************/

setup ()

	{int n;
	n = SYSRSCAN (0);	/* command line in rescan buffer */
	if (n > 0)
		{int i;
		SYSSIN (0777777, mkbptr(combuf), n, 0);
		i = n-1;
		while (i >= 0)	/* remove trailing CR ane NLs */
			{int c;
			c = combuf[i];
			if (c != '\r' && c != '\n') break;
			--i;
			}
		combuf[i+1] = 0;	/* terminate string */
		}
	if (n==0 || nojcl ())
		/* if run from IDDT or by some EXEC command */
		/* could use a more reliable method! */
		{int prompt;
		combuf[0] = '.';	/* dummy program name */
		combuf[1] = ' ';
		prompt = mkbptr ("Command: ");
		SYSPSOUT (prompt);
		rdtty (combuf+2, COMSIZE-2, prompt);
		}
	argc = parse (combuf, argv);
	setio ();
	}

int nojcl ()

	{char temp[COMSIZE], *p;
	stcpy (combuf, temp);
	p = temp;
	while (*p)
		{if (*p == ' ') {*p = 0; break;}
		*p = lower (*p); ++p;
		}
	if (stcmp (temp, "run")) return (TRUE);
	if (stcmp (temp, "r")) return (TRUE);
	if (stcmp (temp, "iddt")) return (TRUE);
	return (FALSE);
	}

static int append, errappend;

/**********************************************************************

	PARSE - Parse Command Arguments

	given:	in -	the command string
		av -	a pointer to a character pointer array
			where pointers to the args should be placed
	returns:	number of arguments

	Command syntax:

		Arguments beginning with <, >, >>, %, %% do file
			redirection, a la Unix.
		(A < redirection must not have an unmatched '>'
			in the file name.)
		Arguments are separated by spaces.
		Arguments may be surrounded by "s, in which case
			embedded spaces are allowed and embedded
			"s must be doubled.
		^V and \ both prohibit the special interpretation of
			the next character (i.e., space, ", <, >, %)
		A \ is eaten, a ^V is left in the string unless it
			is followed by a - or a ?.

**********************************************************************/

int parse (in, av)
	char *in, *av[];

	{int ac;
	char *out;

	ac = 0;
	out = in;
	append = errappend = FALSE;
	cinfn = coutfn = cerrfn = 0;

	while (TRUE)
		{int quoteflag, firstch, secondch, c;
		char *s;

		quoteflag = FALSE;

		/* find beginning of next arg */

		c = *in++;
		while (c == ' ') c = *in++;
		if (c == 0) break;
		if (c == '"') {quoteflag = TRUE; c = *in++;}
		if (c == 0) break;
		firstch = c;		/* \< should not be special */
		secondch = *in;		/* >\> should not be special */
		av[ac] = s = out;

		/* scan arg */

		while (TRUE)
			{if (c == 0) break;
			if (quoteflag)
				{if (c == '"')
					{c = *in++;
					if (c != '"') break;
					}
				}
			else if (c == ' ') break;
			if (c == QUOTE || c == ARGQUOTE)
				{if (c == QUOTE)
					{c = *in++;
					if (c != '?' && c != '-') *out++ = QUOTE;
					}
				else c = *in++;
				if (c == 0) break;
				}
			*out++ = c;
			c = *in++;
			}

		*out++ = 0;

		/* check for redirection command */

		if (ac == 0) firstch = -1;
		switch (firstch) {
		case '<':	/* if there is a matching '>' then this
				   is not a redirection command */
			{char *p, t;
			int level;
			p = s+1;
			level = 0;
			while (t = *p++)
				{if (t == QUOTE && *p)
					{++p; continue;}
				if (t == '<') ++level;
				if (t == '>')
					{if (level == 0) break; /* unmatched */
					--level;
					}
				}
			if (s[1] && (t != '>')) cinfn = s+1;
			else if (++ac < MAXARG) av[ac] = out;
			}
			break;
		case '>':
			if (secondch == '>')
				{if (s[2]) {coutfn = s+2; append = TRUE;}}
			else {if (s[1]) {coutfn = s+1; append = FALSE;}}
			break;
		case '%':
			if (secondch == '%')
				{if (s[2]) {cerrfn = s+2; errappend=TRUE;}}
			else {if (s[1]) {cerrfn = s+1; errappend = FALSE;}}
			break;
		default:
			/* normal argument */
			if (++ac < MAXARG) av[ac] = out;
			}

		if (c == 0) break;
		}
	return (ac>MAXARG ? MAXARG : ac);
	}

/**********************************************************************

	SETIO - Setup standard I/O

**********************************************************************/

setio ()

	{int f;
	closall ();
	if (cinfn)	/* input is redirected */
		{f = cin = c0open (cinfn, 'r');
		if (f == OPENLOSS) {cinfn = 0;}
		}
	if (!cinfn) cin = c0open ("", 'r');
	if (coutfn)	/* output is redirected */
		{f = cout = c0open (coutfn, append ? 'a' : 'w');
		if (f == OPENLOSS)
			{SYSPSOUT(mkbptr("Can't open specified output file."));
			coutfn = 0;
			}
		}
	if (!coutfn) cout = c0open ("", 'w');
	if (cerrfn)	/* errout is redirected */
		{f = cerr = c0open (cerrfn, errappend ? 'a' : 'w');
		if (f == OPENLOSS)
			{SYSPSOUT(mkbptr("Can't open specified error file."));
			cerrfn = 0;
			}
		}
	if (!cerrfn) cerr = copen ("", 'w');
	}

/**********************************************************************

	C0OPEN - Open with error message

**********************************************************************/

channel c0open (name, mode)

	{int f;

	f = copen (name, mode, 0);
	if (f == OPENLOSS)
		{errout ("Unable to ");
		if (mode != 'r') errout ("write");
		    else errout ("read");
		errout (" '");
		errout (name);
		errout ("'\r\n");
		}
	return (f);
	}

/**********************************************************************

	ERROUT - Write error message

**********************************************************************/

errout (s)
	char *s;

	{SYSPSOUT (mkbptr (s));
	}

/**********************************************************************

	MKBPTR - Make byte pointer from string

**********************************************************************/

int mkbptr (p)
	int p;

	{p =& 0777777;
	p =| 0444400000000;
	return (p);
	}

/**********************************************************************

	COPEN_OPTIONS - Process mode and options.  Set direction,
		ftype, and append flag.

**********************************************************************/

int copen_options (mode, opt, pdirection, pftype, pappend)
	char *opt;
	int *pdirection, *pftype, *pappend;

	{register int c;

	if (mode<'A' || mode>'z') mode = 'r';
	c = opt;
	if (c<0100 || c>=01000000) opt = "";
	else if (opt[0]<'A' || opt[0]>'z') opt = "";

	*pdirection = FREAD;
	*pappend = FALSE;
	*pftype = FTEXT;
	switch (lower (mode)) {
		case 'a':	*pappend = TRUE; /* fall through */
		case 'w':	*pdirection = FWRITE; break;
		default:	break;
			}
	while (c = *opt++) switch (lower (c)) {
		case 'b':	*pftype = FBIN; break;
		case 's':	*pftype = FSTR; break;
			}
	}

/**********************************************************************

	CHANLIST - List of open channels.

**********************************************************************/

channel chanlist;

channel channel_allocate ()

	{channel p;
	int sz;

	sz = sizeof(*p)/sizeof(p);
	p = salloc (sz);		/* presumed initialized to zero */
	p->next = chanlist;
	chanlist = p;
	p->peekc = -1;
	return (p);
	}

channel_free (p)
	channel p;

	{channel q, *qq;

	qq = &chanlist;
	q = *qq;
	while (q)
		{if (q == p)
			{*qq = p->next;
			if (p->buf) cfree (p->buf);
			sfree (p);
			return;
			}
		qq = &q->next;
		q = *qq;
		}
	}

/**********************************************************************

	CLOSALL

**********************************************************************/

closall ()

	{while (chanlist) cclose (chanlist);
	}

/**********************************************************************

	CISFD

**********************************************************************/

int cisfd (p)
	channel p;

	{channel q;

	q = chanlist;
	while (q)
		{if (q == p) return (TRUE);
		q = q->next;
		}
	return (FALSE);
	}

/**********************************************************************

	USERNAME

**********************************************************************/

char *username ()

	{static char buffer[40];
	int un, p;

	p = &un;
	p =| 0777777000000;
	SYSGJI (-1, p, 2);	/* GETJI - read user number */
	SYSDIRST (mkbptr (buffer), un);
	return (buffer);
	}

/**********************************************************************

	VALRET - return "command string" to superior

**********************************************************************/

valret (s) char *s;

	{SYSRSCAN (mkbptr (s));	/* put string in RSCAN buffer */
	SYSRSCAN (0);	/* attach RSCAN buffer to terminal input */
	_HALTF ();	/* return control to superior */
	}

/**********************************************************************
	
	SLEEP (nsec)

**********************************************************************/

sleep (nsec)

	{SYSDSMS(nsec*1000);}

/**********************************************************************

	STKDMP - a dummy

**********************************************************************/

stkdmp ()

	{;}
  