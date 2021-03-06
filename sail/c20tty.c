/**********************************************************************

	TOPS-20 Display Terminal Hacking

**********************************************************************/

# define IMLAC 4	/* not a standard code */
# define HP 6
# define VT52 15
# define VT100 18

static int ttype;	/* terminal type */
static int dflag;	/* is-display flag */
static int iflag;	/* own variable initialization flag */

int isdisplay ()

	{if (!iflag)	/* own variable initialization */
		{ttype = SYSGYP (0101);
		dflag = (ttype==HP || ttype==VT52 ||
			 ttype==IMLAC || ttype==VT100);
		if (dflag && ttype != IMLAC)
			/* allow ESC to be sent */
			{int buf[2];
			SYSRCOC (0101, buf);
			buf[1] = (buf[1] & ~0600000) | 0400000;
			SYSSCOC (0101, buf);
			/* This hack is because most current programs
			   seem to assume that VT100's are in VT52 mode: */
			if (ttype == VT100)
				{tyo(033);tyo('[');tyo('?');tyo('2');tyo('l');
				/* put it in VT52 mode */
				ttype = VT52; /* henceforth, pretend it's
							a VT52 */
				}
			if (ttype == VT100) /* make sure its in ANSI mode */
				{tyo (033);
				tyo ('<');
				}
			}
		iflag = 1;
		}
	return (dflag);
	}

spctty (c)

	{if (!iflag) isdisplay ();
	if (ttype == IMLAC)
		{int mode;
		mode = _RFMOD (0101);
		_SFMOD (0101, mode & ~0300);
		tyo (0177);
		switch (c) {
		case 'C':	tyo (0220-0176); break;
		case 'T':	tyo (0217-0176);
				tyo (0+1);	/* vertical */
				tyo (0+1);	/* horizontal */
				break;
		case 'L':	tyo (0203-0176); break;
		case 'B':	tyo (0211-0176); break;
		default:	tyo (0203-0176); break;
			}
		_SFMOD (0101, mode);
		return;
		}

	if (ttype == VT100) switch (c) {

		case 'C':	/* clear screen */

				tyo(033);tyo('[');tyo('H');
				tyo(033);tyo('[');tyo('J');
				return;

		case 'T':	/* go to top of screen */

				tyo(033);tyo('[');tyo('H');
				return;

		case 'L':	/* erase line */

				tyo(033);tyo('[');tyo('K');
				return;
	
		default:	return;
				}

	if (ttype == HP || ttype == VT52) switch (c) {

		case 'C':	/* clear screen */

				tyo(033);tyo('H');tyo(033);tyo('J');
				return;

		case 'T':	/* go to top of screen */

				tyo(033);tyo('H');
				return;

		case 'L':	/* erase line */

				tyo(033);tyo('K');
				return;

		default:	return;
				}

	tyo('\r');tyo('\n');
	return;
	}

tyo (c)
	{SYSBOUT (0101, c);
	}

			
utyo (c)
	{SYSBOUT (0101, c);
	}
			
int utyi ()

	{int omode, c;
	omode = _RFMOD ();
	_SFMOD (0100, omode & ~0004000); /* turn echo off */
	c = SYSBIN (0100);
	_SFMOD (0100, omode);
	return (c & 0177);
	}
  