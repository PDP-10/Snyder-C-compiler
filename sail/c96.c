# include "cc.h"

/*

	C Compiler
	Routines common to phases L, P, C, M, E

	Copyright (c) 1976 by Alan Snyder

	cprint
	stcpy
	slen

*/

/**********************************************************************

	CPRINT - C Formatted Print Routine

	Extendable Format Version:
		Print Routines should expect the following
		arguments (n specified when defined):
			1 to n:	n data arguments
			n+1:	file descriptor
			n+2:	field width (0 if not given)

*/

# define internal static
# ifdef unix
# define internal
# define format_table Xfmt_table
# define format_nargs Xfmt_nargs
extern int prc(), prd(), pro(), prs();
# endif

internal int (*format_table[26]) () {
	/* a */ 0, 0, prc, prd, 0, 0, 0, 0,
	/* i */ 0, 0, 0, 0, 0, 0, pro, 0,
	/* q */ 0, 0, prs, 0, 0, 0, 0, 0,
	/* y */ 0, 0};

internal int format_nargs [26] {
	/* a */ 0, 0, 1, 1, 0, 0, 0, 0,
	/* i */	0, 0, 0, 0, 0, 0, 1, 0,
	/* q */ 0, 0, 1, 0, 0, 0, 0, 0,
	/* y */ 0, 0};

deffmt (c, p, n)	int (*p)();

	{if (c >= 'A' && c <= 'Z') c =+ ('a' - 'A');
	if (c >= 'a' && c <= 'z')
		{if (n >= 0 && n <= 3)
			{format_table [c - 'a'] = p;
			format_nargs [c - 'a'] = n;
			}
		else cprint ("bad nargs to DEFFMT: %d\n", n);
		}
	else cprint ("bad character to DEFFMT: %c\n", c);
	}

cprint (a1,a2,a3,a4,a5,a6,a7,a8)

	{int *adx, c, width;
	char *fmt;
	int fn, (*p)(), n;
	extern int cout;

	if (cisfd (a1))	/* file number */
		{fn = a1;
		fmt = a2;
		adx = &a3;
		}
	else
		{fn = cout;
		fmt = a1;
		adx = &a2;
		}

	while (c= *fmt++)

		{if (c!='%') cputc (c,fn);
		else
			{width = 0;
			while ((c = *fmt)>='0' && c<='9')
				width = (width*10) + (*fmt++ - '0');
			c = *fmt++;
			if (c >= 'A' && c <= 'Z') c =+ ('a' - 'A');
			if (c >= 'a' && c <= 'z')
				{p = format_table [c - 'a'];
				n = format_nargs [c - 'a'];
				if (p)
					{switch (n) {
			case 0:		(*p) (fn, width); break;
			case 1:		(*p) (adx[0], fn, width); break;
			case 2:		(*p) (adx[0], adx[1], fn, width); break;
			case 3:		(*p) (adx[0], adx[1], adx[2], fn, width); break;
						}
					adx =+ n;
					continue;
					}
				cputc (c, fn);
				}
			else cputc (c, fn);
			}
		}
	}

/**********************************************************************

	PRO - Print Octal Integer

**********************************************************************/

pro (i, fn, width)

	{int b[30], *p, a;

	p = b;
	while (a = ((i>>3) & WORDMASK))
		{*p++ = (i&07) + '0';
		i = a;
		}
	*p++ = i+'0';
	if (i) *p++ = '0';
	i = width - (p-b);
	while (--i>=0) cputc (' ', fn);
	while (p > b) cputc (*--p, fn);
	}

/**********************************************************************

	PRD - Print Decimal Integer

**********************************************************************/

prd (i, fn, width)

	{int b[30], *p, a, flag;

	flag = 0;
	p = b;
	if (i < 0) {i = -i; flag = 1;}
	if (i < 0) {stcpy (SMALLEST, b); p = b+slen(b); flag = 0;}
	else
		{while (a = i/10)
			{*p++ = i%10 + '0';
			i = a;
			}
		*p++ = i+'0';
		}
	if (flag) *p++ = '-';
	i = width - (p-b);
	while (--i>=0) cputc (' ', fn);
	while (p > b) cputc (*--p, fn);
	}

/**********************************************************************

	PRS - Print String

**********************************************************************/

prs (s, fn, width)	char *s;

	{int i, c;

	i = (width > 0 ? width - slen (s) : 0);
	while (--i >= 0) cputc (' ', fn);
	while (c = *s++) cputc (c, fn);
	}

/**********************************************************************

	PRC - Print Character

**********************************************************************/

prc (c, fn, width)

	{int i;

	i = width - 1;
	while (--i >= 0) cputc (' ', fn);
	cputc (c, fn);
	}

/**********************************************************************

	STCPY - String Copy

**********************************************************************/

stcpy (s, d)
	char *s, *d;

	{while (*d++ = *s++);}

/**********************************************************************

	SLEN - String Length

**********************************************************************/

int slen (s)
	char *s;

	{int i;

	i = 0;
	while (*s++) ++i;
	return (i);
	}
