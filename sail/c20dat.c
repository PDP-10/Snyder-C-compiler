/**********************************************************************

	C20DAT - TOPS-20 date and time hacking

**********************************************************************/

# define _GTAD SYSGAD
# define _RTAD SYSRTAD
# define _IDTIM SYSIDTIM
# define _ODCNV SYSODCNV

struct _cal {int year, month, day, hour, minute, second;};
# define cal struct _cal

t2cal (tdate, cd)	cal *cd;

	{int vec[3], udate;
	_ODCNV (tdate, 0, vec);
	cd->year = vec[0] >> 18;
	cd->month = (vec[0] & 0777777) + 1;
	cd->day = (vec[1] >> 18) + 1;
	udate = vec[2] & 0777777;
	cd->second = udate%60; udate =/ 60;
	cd->minute = udate%60; udate =/ 60;
	cd->hour = udate%24;
	}

int cal2t (cd) cal *cd;

	{char buf[100];
	int f;
	f = copen (buf, 'w', "s");
	cprint (f, "%d/%d/%d %d:%d:%d", cd->month, cd->day, cd->year,
		cd->hour, cd->minute, cd->second);
	cclose (f);
	return (_IDTIM (mkbptr (buf), 0));
	}

now (cp)
	cal *cp;

	{int i;
	i = _GTAD ();
	t2cal (i, cp);
	}

int etime () /* return elapsed time in 1/60th second units */

	{int rt, ct;
	_RUNTM (-5, &rt, &ct);
	return (ct * 60 / 1000);
	}

/**********************************************************************

	CPUTM

**********************************************************************/

int cputm ()

	{int rt, ct;
	_RUNTM (0400000, &rt, &ct);
	return (rt * 60 / 1000);
	}

/**********************************************************************

	GFILDATE - Get file creation/modification date

**********************************************************************/

gfildate (f, cp)
	int f;
	cal *cp;

	{int q;
	_RTAD (cjfn (f), &q, 1);
	t2cal (q, cp);
	}

   