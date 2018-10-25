/*
	test file name stuff

*/

main ()

	{while (test ());}

test ()

	{char buf[200], temp[200];
	cprint ("Enter file name: ");
	gets (buf);
	if (buf[0] == 0) return (0);
	cprint (" device is '%s'\n", fngdv (buf, temp));
	cprint (" directory is '%s'\n", fngdr (buf, temp));
	cprint (" name is '%s'\n", fngnm (buf, temp));
	cprint (" type is '%s'\n", fngtp (buf, temp));
	cprint (" generation is '%s'\n", fnggn (buf, temp));
	cprint (" attributes are '%s'\n", fngat (buf, temp));
	cprint ("\n");
	fnsdf (buf, buf, 0, "R", 0, "XGP", 0, 0);
	cprint (" after defaults set: %s\n", buf);
	fnsfd (buf, buf, "DSK", "CLU", 0, 0, 0, 0);
	cprint (" after fields set: %s\n", buf);
	return (1);
	}
