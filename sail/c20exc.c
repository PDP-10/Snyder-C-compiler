# include "c.defs"

/**********************************************************************

	EXEC20

**********************************************************************/

int exctime 0;
int exccode 0;

# define _GTJFN SYSGTJFN
# define _RSCAN SYSRSCAN
# define _CLOSF SYSCLOSF

/**********************************************************************

	EXECS - Execute a program with a given command string

	Returns:

		-5	Job valretted something and was not continued.
		-4	Internal fatal error.
		-3	Unable to load program file.
		-2	Unable to create job.
		-1	Unable to open program file.
		0	Job terminated normally.
		other	Job terminated abnormally with said PIRQ

	Sets:

		exctime - job's CPU time in 1/60 sec. units
		exccode - contents of job's loc 1 at termination

**********************************************************************/

#define cons(a,b) ((a<<18)|b)

int execs (pname, args)	char *pname, *args;

	{int j;	/* process handle */
	int f;	/* jfn for program file */
	int rc;	/* return code */
	char jcl[200]; /* to construct JCL */

	f = _GTJFN (0100001000000, mkbptr (pname)); /* access program file */
	if (f >= 0600000) return (-1);

	j = _CFORK (0200000000000, 0);
		/* create process, with my capabilities */
	if (j >= 0600000)
		{_CLOSF (f);
		return (-2);
		}

	rc = _GET (cons (j, f), 0);	/* load program file */
	_CLOSF (f);			/* release program file */
	if (rc)
		{_KFORK (j);
		return (-3);
		}

	sconcat (jcl, 3, pname, " ", args);	/* construct JCL line */
	_RSCAN (mkbptr (jcl));			/* set JCL */
	_SFRKV (j, 0);				/* start job */

	while (TRUE)
		{int sts, code;
		_WFORK (j);
		sts = _RFSTS (j);
		code = (sts >> 18) & 07777;

		if (code == 2) break;
		if (code == 3)
			{int number;
			number = sts & 0777777;
			if (number == 12)
				{int nwork, nused, nperm;
				int usern, dirn, jobn, termn;
				cprint ("Disk quota exceeded.\n");
				_GJINF (&usern, &dirn, &jobn, &termn);
				_GTDAL (dirn, &nwork, &nused, &nperm);
				if (nused == nwork)
					{_DELDF (0, dirn);
					_GTDAL (dirn, &nwork, &nused, &nperm);
					if (nused < nwork)
						{cprint ("%d pages expunged\n",
							nwork - nused);
						goto restart;
						}
					}
				}
			cprint ("Process terminated,");
			cprint (" error number %d.\n", number);
			_KFORK (j);
			return (number);
			}
		cprint ("Process terminated, status %d.\n", code);
		if (code != 0) break;
	restart:
		_RFORK (j); /* unfreeze it if it was frozen */
		_SFORK (j, _RFPC (j));	/* continue it */
		}

	{int junk;
	int acs[16];
	_RUNTM (j, &exctime, &junk);
	exctime = exctime * 60 / 1000;
	_RFACS (j, acs);
	exccode = acs[1];	
	}

	_KFORK (j);
	return (0);
	}

/**********************************************************************

	EXECV - Execute file given a vector of arguments

**********************************************************************/

int execv (prog, argc, argv)
	char *prog, *argv[];

	{char **ap, **ep, buff[300], *p, *s;
	int c;

	p = buff;
	ap = argv;
	ep = argv + argc - 1;

	while (ap <= ep)
		{s = *ap++;
		*p++ = '"';
		while (c = *s++) *p++ = c;
		*p++ = '"';
		*p++ = ' ';
		}

	*p++ = 0;
	return (execs (prog, buff));
	}
  