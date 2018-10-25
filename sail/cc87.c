/* cc87.c -- Code generator TOPS-20 (contd)   (C) 1981  K. Chen */

#define	    sd   extern
#include    "cc.g"


/* --------------------------------------------- */
/*	address immediate -> register codes      */
/* --------------------------------------------- */

code3(op, r, s)
struct SYMBOL *s;
{
  if (op==PLUS && previous->ptype==IMMED && previous->preg==r && optimize) {
     switch (previous->pop) {
     case PLUS:
     case IDENT:
          previous->ptype = GADDR;
	  previous->poffset = previous->pvalue;
          previous->pptr = s;
	  previous->pindex = 0;
	  return;
     ~
  ~
  previous = &codes[maxcode&(MAXCODE-1)];
  previous->ptype = GADDR;
  previous->pop = op;
  previous->preg = r;
  previous->pptr = s;
  previous->poffset = 0;
  previous->pindex = 0;
  maxcode++;
~

/* ------------------------------------ */
/*	register-register indirect      */
/*	(transformed to indexed)	*/
/* ------------------------------------ */

code4(op, r, s)
{
  struct PSEUDO *p, *b, *before();
  struct SYMBOL *ssymbol;
  int    ty, o, soffset;

  if (previous->ptype == IMMED && previous->preg == s) {

      switch (previous->pop) {

          /* fold:   MOVNI  s,const  */
          /*	     OP     r,@s     */
          /*			     */
	  /* into:   OP	    r,const  */

      case NEG:
	   previous->pvalue = -previous->pvalue;
	   /* fall through */

          /* fold:   MOVEI  s,const  */
          /*	     OP     r,@s     */
          /*			     */
	  /* into:   OP	    r,const  */

      case IDENT:
           previous->ptype = RCONST;
           previous->preg = r;
           previous->pop = op;
           release(s);
           return;
      ~
  ~

  ssymbol = NULL;
  soffset = 0;
  p = previous;
  while (p != NULL) {
      if (p->preg == s) {
          if (p->pop == PLUS) {
	      switch (p->ptype) {
	      case IMMED:
	           soffset += p->pvalue;
	           p->pop = NOP;
		   break;
	      case GADDR:
	           if (ssymbol == NULL) {
		       p->pop = NOP;
		       ssymbol = p->pptr;
		       soffset += p->poffset;
		   ~
	      ~
	  ~
          else if (p->pop < 1000) break;
      ~
      p = before(p);
  ~
  if (soffset || ssymbol != NULL) {
      o = PLUS;
      if (p->pop == IDENT && p->preg == s && p->ptype == IMMED) {
          soffset += p->pvalue;
	  p->pop = NOP;
	  o = IDENT;
      ~
      code3(o, s, ssymbol);
      previous->poffset = soffset;
  ~
  if ((p = previous) != NULL && p->preg == s && optimize) {
      if (op == IDENT || tok[op].ttype == ASOP) {
          switch (p->ptype) {
	  case GADDR:
	       switch (p->pop) {
	       case IDENT:
		    if (op == ASGN) {
		        b = before(p);
			if (b->preg == r) {
		            if (b->ptype == GLOBAL && b->pptr == p->pptr) {
		                o = b->pop;
			        if (o==PLUS || o==MINUS || o==MPLY ||
			            o==AND  || o==OR || o==XOR || o==DIV) {
	
			            /* fold:  	OP    R,addr */
			            /*		MOVEM R,addr */
				    /*			     */
		   	            /* into:	OPB   R,addr */

			            b->ptype = BOTH;
				    b->poffset = p->poffset;
				    b->pindex = 0;
			            p->pop = NOP;	/* poof! */
			            return r;
			        ~
			    ~
			    else  if (b->ptype == ONEREG && b->pop == SETZ) {

				/* fold:    SETZ   R,     */
				/*	    MOVEM  R,...  */
				/*			  */
				/* to:      SETZB  R,...  */

				p->pop = NOP;		/* poof! */
				b->ptype = BOTH;
				b->pptr = p->pptr;
				b->pindex = 0;
				b->poffset = p->poffset;
				return r;
			    ~
			~
		    ~

		    /*  fold:    	MOVEI   S,addr */
		    /*		        MOVE(M) R,@S   */
		    /*  into:		MOVE(M) R,addr */

  	            p->ptype = GLOBAL;
               	    if (s != r) regis[s] = -1;
	       	    p->pop = op;
	 	    p->preg = r;
		    p->pindex = 0;
		    return;

	       case PLUS:

		    /*  fold:    	ADDI    S,addr+offset    */
		    /*		        MOVE(M) R,@S             */
		    /*  into:		MOVE(M) R,addr+offset(S) */

	       	    p->ptype = MINDEXED;
	            p->pop = op;
	            p->preg = r;
	            p->pindex = s;

		    if (op == ASGN) {
		        /* one more chance ... */
		        b = before(p);
		        if (b->ptype==ONEREG && b->pop==SETZ && b->preg==r) {

			    /* fold:    SETZ   R,       */
			    /*          MOVEM  R,...    */
			    /*				*/
			    /* into:    SETZB  R,...    */

		            b->pop = NOP;   /* poof! */
		    	    p->pop = SETZ;
			    p->ptype = BOTH;
			~
		    ~
	            return;
	       ~
	  case IINDEXED:
	       if (p->pop == IDENT) {
	           
		   /* fold:      MOVEI	  S,offset(SP)  */
		   /*		 MOVE(M)  R,@S		*/
		   /*					*/
		   /* into:	 MOVE(M)  R,offset(SP)	*/
		   
		   p->ptype = INDEXED;
		   if (s != r) regis[s] = -1;
		   p->pop = op;
		   p->preg = r;

		   if (op == ASGN) {   /* one more chance ... */
		       b = before(p);
		       if (b->ptype==ONEREG && b->pop==SETZ && b->preg==r) {

			   /* fold:    SETZ   R,       */
			   /*          MOVEM  R,...    */
			   /*				*/
			   /* into:    SETZB  R,...    */

		           b->pop = NOP;   /* poof! */
		    	   p->pop = SETZ;
			   p->ptype = BOTH;
		       ~
		   ~
		   return;
	       ~
	       break;

	  case GLOBAL:
	  case INDEXED:
	       if (p->pop == IDENT) {


		    /* transform:	MOVE    S,loc    */
		    /*			MOVE(M) R,@S	 */
		    /* to:		MOVE(M) R,@loc   */

		   p->ptype = MINDIRECT;
		   if (s != r) regis[s] = -1;
		   p->pop = op;
		   p->preg = r;
		   return;
	       ~
	       break;

	  case IMMED:
	       if (p->pop == PLUS) {

		    /*  transform:	ADDI    S,Offset    */
		    /*		 	MOVE(M) R,@S        */
		    /*  to:		MOVE(M) R,Offset(S) */

      	           p->ptype = INDEXED;
                   if (s != r) regis[s] = -1;
	           p->pop = op;
	           p->preg = r;
	           p->pindex = s;
	           return;
		~
	  ~
      ~
      else {
          if (p->pop == IDENT) {
              ty = p->ptype;
	      if (ty == GLOBAL || ty == GADDR) {
  	          if (s != r) regis[s] = -1;
	          p->ptype = (ty == GLOBAL) ? MINDIRECT : GLOBAL;
	          p->pop = op;
	          p->preg = r;
		  p->poffset = 0;
		  p->pindex = 0;
	          return;
	      ~
	  ~
      ~
  ~
  previous = &codes[maxcode&(MAXCODE-1)];
  previous->ptype = INDEXED;
  previous->pop = op;
  previous->preg = r;
  previous->poffset = 0;
  previous->pindex = s;
  maxcode++;
  release(s);
~
