/* cc80.c  -- code generator tables   (C) 1981  K. Chen */

char *dec20op[] = {
	"--",	    "HRLZ",	"SETZ",	    "JUMPE",
	"ADJSP",    "HRL",	"IDIV",	    "AND",
	"HRRZ",	    "(",	")",	    "IMUL",
	"ADD",	    ",",	"SUB",	    ".",
	"IDIV",	    "SETZ",     ";",	    "CAML",
	"MOVEM",    "CAMG",	"?",	    "MOVE",
	"[",	    "]",	"XOR",	    "{",
	"IOR",	    "~",	"SETCM",    ":",
	"DPB",	    "LDB",	"ADJBP",    "IBP",
	"IDPB",	    "ILDB",	"TLO",	    "TLZ",
	"CAIN",	    "CAIG",	"CAIL",	    "CAIE",
	"CAILE",    "CAIGE",    "JRST",	    "POPJ",
	"--",	    "SKIPE",    "SKIPN",    "--",
	"JUMPN",    "JUMPG",    "JUMPL",    "JUMPE",
	"JUMPLE",   "JUMPGE",    "--",	    "--",
	"ADDB",	    "ADDB",	"IMULB",    "IDIVB",
	"--",	    "--",	"--",	    "ANDB",
	"XORB",	    "IORB",	"--",	    "--",
 	"MOVN",	    "AOS",	"SOS",	    "AOS",
	"SOS",	    "@",	"JUMPE",    "JUMPN",
	"CAME",	    "CAMLE",    "CAMGE",    "CAMN",
	"--",	    "PUSHJ",    "PUSH",     "POP",
	"ADDM",     "SUBM",     "--",	    "--",
	"--",	    "--",	"--",	    "--",
	"ROT",	    "LSH",      "LSH",      "MOVM"
~ ;

/* =     maps to MOVEM @  */
/* ident maps to MOVE     */
/* break maps to SKIPE    */
/* continue maps to SKIPN */



