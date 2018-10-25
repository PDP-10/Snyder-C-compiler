#include "c.defs"

/**********************************************************************

	rename (fn1, fn2)
	delete (fn)

**********************************************************************/

/**********************************************************************

	RENAME (file1, file2)

	Should work even if a file2 already exists.
	Return 0 if no error.

	*TOPS-20 VERSION*

**********************************************************************/

int rename (s1, s2) char *s1, *s2;

	{int jfn1, jfn2, rc;
	jfn1 = SYSGTJFN (0100001000000, mkbptr (s1));	/* old file */
	if (jfn1 >= 0600000) return (jfn1);
	jfn2 = SYSGTJFN (0400001000000, mkbptr (s2));	/* new file */
	if (jfn2 >= 0600000) return (jfn2);
	if (rc = _RNAMF (jfn1, jfn2))
		{SYSRLJFN (jfn1);
		SYSRLJFN (jfn2);
		return (rc);
		}
	SYSRLJFN (jfn2);
	return (0);
	}

/**********************************************************************

	DELETE

**********************************************************************/

delete (s) char *s;

	{int jfn;
	jfn = SYSGTJFN (0100001000000, mkbptr (s));	/* old file */
	if (jfn < 06000000)
		{SYSDELF (jfn & 0777777);
		SYSCLOSF (jfn);
		}
	}
