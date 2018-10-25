# include "c.defs"
# define QUOTE 22	/* control-V */

/**********************************************************************

	EXPAND ARGUMENT VECTOR CONTAINING FILE NAME PATTERNS
	TOPS-20 Version

**********************************************************************/

static char **next;
static char *bufp;

int exparg (argc, argv, outv, buffer)
	char *argv[], *outv[], buffer[];

	{int i, expfile();
	char *s;

	bufp = buffer;
	next = outv;
	i = 0;
	while (i<argc)
		{s = argv[i++];
		if (expmagic (s)) fdmap (s, expfile);
		else *next++ = s;
		}
	return (next-outv);
	}

int expmagic (s)	/* does it contain magic pattern chars? */
	char *s;

	{int c, flag;
	flag = FALSE;
	while (c = *s++) switch (c) {
		case '%':
		case '*':	flag = TRUE; continue;
		case QUOTE:	if (*s) ++s; continue;
		}
	return (flag);
	}

expfile (s)
	char *s;

	{*next++ = bufp;
	while (*bufp++ = *s++);
	}
