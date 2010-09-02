#include <libc.h>
#include <io.h>
#include <malloc.h>
#include <file_api.h>
#include <hal.h>

static int row = 0, col = 0;

int video_putchar(int c);
int console_putchar(int ch);
int console_getchar();

struct libc_dev_ops *libc_default_ops = &libc_video_ops;

struct libc_dev_ops libc_video_ops = {
  video_putchar, 0
};

struct libc_dev_ops libc_console_ops = {
  console_putchar, console_getchar
};

static int console_stdin_fd, console_stdout_fd;

void use_vfs_console(int in, int out) {
  console_stdin_fd = in;
  console_stdout_fd = out;
  libc_default_ops = &libc_console_ops;
}

int console_putchar(int ch) {
  // this function cannot be used from >=DISPATCH level because can block
  // TODO: redirect those to kernel log 
  assert(irql_get() < IRQL_DISPATCH);

  int res;
  if((res = write(console_stdout_fd, &ch, 1)) < 0) // OCCHIO! TAKE CARE OF ENDIANNESS!!!
    return res;
  return ch;
}

int console_getchar() {
  assert(irql_get() < IRQL_DISPATCH);

  int ch = 0;
  int res;
  if((res = read(console_stdout_fd, &ch, 1)) < 0)
    return res;
  return ch;
}

///////// old libc (rewrite this or at least reorder!!!) 
///////// this is the oldest code !!

static void newline() {
  col = 0;
  row++;
  if(row >= video_get_rows())
    row = 0;
}

void cls() {
  video_cls();
  setPos(0, 0);
}

void itoa (char *buf, int base, int d) {
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
    
  /* If %d is specified and D is minus, put -' in the head. */
  if (base == 'd' && d < 0)
    {
	
      *p++ = '-';
      buf++;
      ud = -d;
    }
    
  else if (base == 'x')
    divisor = 16;
    
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
	
      int remainder = ud % divisor;
	
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
    
  while (ud /= divisor);
    
  /* Terminate BUF. */
  *p = 0;
    
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

int video_putchar(int c) {
  if(c == '\n' || c == '\r') 
    newline();
  else {
    video_draw_char(c, row, col);
    
    col++;
    if(col >= video_get_columns()) 
      newline();
  }
  return c;
}

int dputchar(struct libc_dev_ops* ops, int c) {
  if(ops->f_putchar)
    return ops->f_putchar(c);
  return -1;
}

int dputs(struct libc_dev_ops* ops, const char *str) {
  const char *s = str;
  while(*s)
    dputchar(ops, *s++);
  dputchar(ops, '\n');
  return s-str;
}

int dprintf(struct libc_dev_ops* ops, const char *format, ...) {
  char **arg = (char **) &format;
  int c;
  char buf[20];
  int zero_prefix = 0;
  int pad = 0;
  int escaped = 0;  


  arg++;
    
  while ((c = *format++) != 0) {
    if(c == '%') {
      escaped = 1;
      c = *format++;
    }
      
    if (!escaped) {
      zero_prefix = 0;
      pad = 0;
      dputchar (ops, c);
    } else {     
      char *p;
      escaped = 0;
 
      switch (c) {
	
      case '0':
	if(!(zero_prefix || pad)) {
	  zero_prefix = 1;
	  escaped = 1;
	  break;
	} 
	// fall through
      case '1' ... '9':
	pad = pad * 10 + (c-'0');
	escaped = 1;
	break; 
	  
      case 'p':
	dputchar(ops, '0');
	dputchar(ops, 'x');
	c = 'x';
	// fall though
      case 'd':
      case 'u':
      case 'x':
	itoa (buf, c, *((int *) arg++));
	p = buf;
	if(strlen(p) < pad) {
	  int len = pad - strlen(p);
	  while(len--)
	    dputchar(ops, zero_prefix ? '0' : ' ');
	}
	goto string;
	break;
	
	
      case 's':
	p = *arg++;
	if (! p)
	  p = "(null)";
	
      string:
	escaped = 0;
	zero_prefix = 0;
	pad = 0;
	while (*p)
	  dputchar (ops, *p++);
	break;
	
      default:
	dputchar (ops, *((int *) arg++));
	break;
      }
    }
  }
  return 0;
}

typedef unsigned char u_char;
typedef unsigned long u_long;
static u_char *std_sccl();

/*
 * Flags used during conversion.
 */
#define LONG            0x01    /* l: long or double */
#define LONGDBL         0x02    /* L: long double; unimplemented */
#define SHORT           0x04    /* h: short */
#define SUPPRESS        0x08    /* suppress assignment */
#define POINTER         0x10    /* weird %p pointer (`fake hex') */
#define NOSKIP          0x20    /* do not skip blanks */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define SIGNOK          0x40    /* +/- is (still) legal */
#define NDIGITS         0x80    /* no digits detected */

#define DPTOK           0x100   /* (float) decimal point is still legal */
#define EXPOK           0x200   /* (float) exponent (e+3, etc) still legal */

#define PFXOK           0x100   /* 0x prefix is (still) legal */
#define NZDIGITS        0x200   /* no zero digits detected */

#ifdef FLOATING_POINT
#ifdef bsd
#include "floatio.h"
#else
#include <float.h>
#define MAXEXP FLT_MAX_EXP
#define	MAXFRACT FLT_MANT_DIG
#endif
#define	BUF	(MAXEXP+MAXFRACT+3)	/* 3 = sign + decimal point + NUL */
#else
#define	BUF	40
#endif

/*
 * Conversion types.
 */
#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* integer, i.e., strtol or strtoul */
#define	CT_FLOAT	4	/* floating, i.e., strtod */

/* compat. mk */
#define NULL 0
#define EOF -1

int std_vxscanf(int(*fgetc)(void*),int(*ungetc)(int,void*),void *info,
                const char *fmt0, va_list ap) {
	register u_char *fmt = (u_char *)fmt0;
	register int c;		/* character from format, or conversion */
	register size_t width;	/* field width, or 0 */
	register char *p;	/* points into all kinds of strings */
	register int n;		/* handy integer */
	register int flags;	/* flags as defined above */
	register char *p0;	/* saves original value of p when necessary */
	register int cin;	/* One character lookahead */
	int nassigned;		/* number of fields assigned */
	int nread;		/* number of characters consumed from fp */
	int base;		/* base argument to strtol/strtoul */
	u_long (*ccfn)();	/* conversion function (strtol/strtoul) */
	char ccltab[256];	/* character class table for %[...] */
	char buf[BUF];		/* buffer for numeric conversions */

	/* `basefix' is used to avoid `if' tests in the integer scanner */
	static short basefix[17] =
		{ 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	nassigned = 0;
	nread = 0;
	base = 0;		/* XXX just to keep gcc happy */
	ccfn = NULL;		/* XXX just to keep gcc happy */

	/* Begin lookahead */
	cin = (*fgetc)(info);
	
	for (;;) {
		c = *fmt++;
		if (c == 0)
		{
			if (cin != EOF) (void) (*ungetc)(cin,info);
			return (nassigned);
		}
		if (isspace(c)) {
			for (;;) {
				if (cin == EOF)
					return (nassigned);
				if (!isspace(cin))
					break;
				cin = (*fgetc)(info);
				nread++;
			}
			continue;
		}
		if (c != '%')
			goto literal;
		width = 0;
		flags = 0;
		/*
		 * switch on the format.  continue if done;
		 * break once format type is derived.
		 */
again:		c = *fmt++;
		switch (c) {
		case '%':
literal:
			if (cin == EOF)
				goto input_failure;
			if (cin != c)
				goto match_failure;
			cin = (*fgetc)(info);
			nread++;
			continue;

		case '*':
			flags |= SUPPRESS;
			goto again;
		case 'l':
			flags |= LONG;
			goto again;
		case 'L':
			flags |= LONGDBL;
			goto again;
		case 'h':
			flags |= SHORT;
			goto again;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			width = width * 10 + c - '0';
			goto again;

		/*
		 * Conversions.
		 * Those marked `compat' are for 4.[123]BSD compatibility.
		 *
		 * (According to ANSI, E and X formats are supposed
		 * to the same as e and x.  Sorry about that.)
		 */
		case 'D':	/* compat */
			flags |= LONG;
			/* FALLTHROUGH */
		case 'd':
			c = CT_INT;
			ccfn = (u_long (*)())strtol;
			base = 10;
			break;

		case 'i':
			c = CT_INT;
			ccfn = (u_long (*)())strtol;
			base = 0;
			break;

		case 'O':	/* compat */
			flags |= LONG;
			/* FALLTHROUGH */
		case 'o':
			c = CT_INT;
			ccfn = (u_long(*)())strtoul;
			base = 8;
			break;

		case 'u':
			c = CT_INT;
			ccfn = (u_long(*)())strtoul;
			base = 10;
			break;

		case 'X':	/* compat   XXX */
			flags |= LONG;
			/* FALLTHROUGH */
		case 'x':
			flags |= PFXOK;	/* enable 0x prefixing */
			c = CT_INT;
			ccfn = (u_long(*)())strtoul;
			base = 16;
			break;

#ifdef FLOATING_POINT
		case 'E':	/* compat   XXX */
		case 'F':	/* compat */
			flags |= LONG;
			/* FALLTHROUGH */
		case 'e': case 'f': case 'g':
			c = CT_FLOAT;
			break;
#endif

		case 's':
			c = CT_STRING;
			break;

		case '[':
			fmt = std_sccl(ccltab, fmt);
			flags |= NOSKIP;
			c = CT_CCL;
			break;

		case 'c':
			flags |= NOSKIP;
			c = CT_CHAR;
			break;

		case 'p':	/* pointer format is like hex */
			flags |= POINTER | PFXOK;
			c = CT_INT;
			ccfn = (u_long(*)())strtoul;
			base = 16;
			break;

		case 'n':
			if (flags & SUPPRESS)	/* ??? */
				continue;
			if (flags & SHORT)
				*va_arg(ap, short *) = nread;
			else if (flags & LONG)
				*va_arg(ap, long *) = nread;
			else
				*va_arg(ap, int *) = nread;
			continue;

		/*
		 * Disgusting backwards compatibility hacks.	XXX
		 */
		case '\0':	/* compat */
			return (EOF);

		default:	/* compat */
			if (isupper(c))
				flags |= LONG;
			c = CT_INT;
			ccfn = (u_long (*)())strtol;
			base = 10;
			break;
		}

		/*
		 * We have a conversion that requires input.
		 */
		if (cin == EOF)
			goto input_failure;

		/*
		 * Consume leading white space, except for formats
		 * that suppress this.
		 */
		if ((flags & NOSKIP) == 0) {
			while (isspace(cin)) {
				nread++;
				cin = (*fgetc)(info);
				if (cin == EOF)
					goto input_failure;
			}
			/*
			 * Note that there is at least one character in
			 * the buffer, so conversions that do not set NOSKIP
			 * can no longer result in an input failure.
			 */
		}

		/*
		 * Do the conversion.
		 */
		switch (c) {

		case CT_CHAR:
			/* scan arbitrary characters (sets NOSKIP) */
			if (width == 0)
				width = 1;
			if (flags & SUPPRESS) {
			  //				size_t sum = 0;
				while (width > 0)
				{
					cin = (*fgetc)(info);
					if (cin == EOF) goto input_failure;
					nread++;
					width--;
				}
			} else {
				char	*ptr;

				ptr = (char*) va_arg(ap,char*);
				while (width > 0)
				{
					cin = (*fgetc)(info);
					if (cin == EOF) goto input_failure;
					*ptr++ = (char) cin;
					nread++;
					width--;
				}
				nassigned++;
			}
			break;

		case CT_CCL:
			/* scan a (nonempty) character class (sets NOSKIP) */
			if (width == 0)
				width = ~0;	/* `infinity' */
			/* take only those things in the class */
			if (flags & SUPPRESS) {
				n = 0;
				while (ccltab[cin]) {
					n++, cin = (*fgetc)(info);
					if (--width == 0)
						break;
					if (cin == EOF) {
						if (n == 0)
							goto input_failure;
						break;
					}
				}
				if (n == 0)
					goto match_failure;
			} else {
				p0 = p = va_arg(ap, char *);
				while (ccltab[cin]) {
					*p++ = cin;
					cin = (*fgetc)(info);
					if (--width == 0)
						break;
					if (cin == EOF) {
						if (p == p0)
							goto input_failure;
						break;
					}
				}
				n = p - p0;
				if (n == 0)
					goto match_failure;
				*p = 0;
				nassigned++;
			}
			nread += n;
			break;

		case CT_STRING:
			/* like CCL, but zero-length string OK, & no NOSKIP */
			if (width == 0)
				width = ~0;
			if (flags & SUPPRESS) {
				n = 0;
				while (!isspace(cin)) {
					n++, cin = (*fgetc)(info);
					if (--width == 0)
						break;
					if (cin == EOF)
						break;
				}
				nread += n;
			} else {
				p0 = p = va_arg(ap, char *);
				while (!isspace(cin)) {
					*p++ = cin;
					cin = (*fgetc)(info);
					if (--width == 0)
						break;
					if (cin == EOF);
						break;
				}
				*p = 0;
				nread += p - p0;
				nassigned++;
			}
			continue;

		case CT_INT:
			/* scan an integer as if by strtol/strtoul */
#ifdef hardway
			if (width == 0 || width > sizeof(buf) - 1)
				width = sizeof(buf) - 1;
#else
			/* size_t is unsigned, hence this optimisation */
			if (--width > sizeof(buf) - 2)
				width = sizeof(buf) - 2;
			width++;
#endif
			flags |= SIGNOK | NDIGITS | NZDIGITS;
			for (p = buf; width; width--) {
				c = cin;
				/*
				 * Switch on the character; `goto ok'
				 * if we accept it as a part of number.
				 */
				switch (c) {

				/*
				 * The digit 0 is always legal, but is
				 * special.  For %i conversions, if no
				 * digits (zero or nonzero) have been
				 * scanned (only signs), we will have
				 * base==0.  In that case, we should set
				 * it to 8 and enable 0x prefixing.
				 * Also, if we have not scanned zero digits
				 * before this, do not turn off prefixing
				 * (someone else will turn it off if we
				 * have scanned any nonzero digits).
				 */
				case '0':
					if (base == 0) {
						base = 8;
						flags |= PFXOK;
					}
					if (flags & NZDIGITS)
					    flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
					else
					    flags &= ~(SIGNOK|PFXOK|NDIGITS);
					goto ok;

				/* 1 through 7 always legal */
				case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					base = basefix[base];
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* digits 8 and 9 ok iff decimal or hex */
				case '8': case '9':
					base = basefix[base];
					if (base <= 8)
						break;	/* not legal here */
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* letters ok iff hex */
				case 'A': case 'B': case 'C':
				case 'D': case 'E': case 'F':
				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					/* no need to fix base here */
					if (base <= 10)
						break;	/* not legal here */
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* sign ok only as first character */
				case '+': case '-':
					if (flags & SIGNOK) {
						flags &= ~SIGNOK;
						goto ok;
					}
					break;

				/* x ok iff flag still set & 2nd char */
				case 'x': case 'X':
					if (flags & PFXOK && p == buf + 1) {
						base = 16;	/* if %i */
						flags &= ~PFXOK;
						goto ok;
					}
					break;
				}

				/*
				 * If we got here, c is not a legal character
				 * for a number.  Stop accumulating digits.
				 */
				break;
		ok:
				/*
				 * c is legal: store it and look at the next.
				 */
				*p++ = c;
				cin = (*fgetc)(info);
				if (cin == EOF)
					break;		/* EOF */
			}
			/*
			 * If we had only a sign, it is no good; push
			 * back the sign.  If the number ends in `x',
			 * it was [sign] '0' 'x', so push back the x
			 * and treat it as [sign] '0'.
			 */
			if (flags & NDIGITS) {
				if (p > buf)
					(void) (*ungetc)(*(u_char *)--p, info);
				goto match_failure;
			}
			c = ((u_char *)p)[-1];
			if (c == 'x' || c == 'X') {
				--p;
				(void) (*ungetc)(c, info);
			}
			if ((flags & SUPPRESS) == 0) {
				u_long res;

				*p = 0;
				res = (*ccfn)(buf, (char **)NULL, base);
				if (flags & POINTER)
					*va_arg(ap, void **) = (void *)res;
				else if (flags & SHORT)
					*va_arg(ap, short *) = res;
				else if (flags & LONG)
					*va_arg(ap, long *) = res;
				else
					*va_arg(ap, int *) = res;
				nassigned++;
			}
			nread += p - buf;
			break;

#ifdef FLOATING_POINT
		case CT_FLOAT:
			/* scan a floating point number as if by strtod */
#ifdef hardway
			if (width == 0 || width > sizeof(buf) - 1)
				width = sizeof(buf) - 1;
#else
			/* size_t is unsigned, hence this optimisation */
			if (--width > sizeof(buf) - 2)
				width = sizeof(buf) - 2;
			width++;
#endif
			flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
			for (p = buf; width; width--) {
				c = cin;
				/*
				 * This code mimicks the integer conversion
				 * code, but is much simpler.
				 */
				switch (c) {

				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					flags &= ~(SIGNOK | NDIGITS);
					goto fok;

				case '+': case '-':
					if (flags & SIGNOK) {
						flags &= ~SIGNOK;
						goto fok;
					}
					break;
				case '.':
					if (flags & DPTOK) {
						flags &= ~(SIGNOK | DPTOK);
						goto fok;
					}
					break;
				case 'e': case 'E':
					/* no exponent without some digits */
					if ((flags&(NDIGITS|EXPOK)) == EXPOK) {
						flags =
						    (flags & ~(EXPOK|DPTOK)) |
						    SIGNOK | NDIGITS;
						goto fok;
					}
					break;
				}
				break;
		fok:
				*p++ = c;
				cin = (*fgetc)(info);
				if (cin == EOF)
					break;	/* EOF */
			}
			/*
			 * If no digits, might be missing exponent digits
			 * (just give back the exponent) or might be missing
			 * regular digits, but had sign and/or decimal point.
			 */
			if (flags & NDIGITS) {
				if (flags & EXPOK) {
					/* no digits at all */
					while (p > buf)
						(*ungetc)(*(u_char *)--p, info);
					goto match_failure;
				}
				/* just a bad exponent (e and maybe sign) */
				c = *(u_char *)--p;
				if (c != 'e' && c != 'E') {
					(void) (*ungetc)(c, info);/* sign */
					c = *(u_char *)--p;
				}
				(void) (*ungetc)(c, info);
			}
			if ((flags & SUPPRESS) == 0) {
				double res;

				*p = 0;
				res = atof(buf);
				if (flags & LONG)
					*va_arg(ap, double *) = res;
				else
					*va_arg(ap, float *) = res;
				nassigned++;
			}
			nread += p - buf;
			break;
#endif /* FLOATING_POINT */
		}
	}
input_failure:
	return (nassigned ? nassigned : -1);
match_failure:
	(void) (*ungetc)(cin,info);
	return (nassigned);
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */
static u_char *
std_sccl(tab, fmt)
	register char *tab;
	register u_char *fmt;
{
	register int c, n, v;

	/* first `clear' the whole table */
	c = *fmt++;		/* first char hat => negated scanset */
	if (c == '^') {
		v = 1;		/* default => accept */
		c = *fmt++;	/* get new first char */
	} else
		v = 0;		/* default => reject */
	/* should probably use memset here */
	for (n = 0; n < 256; n++)
		tab[n] = v;
	if (c == 0)
		return (fmt - 1);/* format ended before closing ] */

	/*
	 * Now set the entries corresponding to the actual scanset
	 * to the opposite of the above.
	 *
	 * The first character may be ']' (or '-') without being special;
	 * the last character may be '-'.
	 */
	v = 1 - v;
	for (;;) {
		tab[c] = v;		/* take character c */
doswitch:
		n = *fmt++;		/* and examine the next */
		switch (n) {

		case 0:			/* format ended too soon */
			return (fmt - 1);

		case '-':
			/*
			 * A scanset of the form
			 *	[01+-]
			 * is defined as `the digit 0, the digit 1,
			 * the character +, the character -', but
			 * the effect of a scanset such as
			 *	[a-zA-Z0-9]
			 * is implementation defined.  The V7 Unix
			 * scanf treats `a-z' as `the letters a through
			 * z', but treats `a-a' as `the letter a, the
			 * character -, and the letter a'.
			 *
			 * For compatibility, the `-' is not considerd
			 * to define a range if the character following
			 * it is either a close bracket (required by ANSI)
			 * or is not numerically greater than the character
			 * we just stored in the table (c).
			 */
			n = *fmt;
			if (n == ']' || n < c) {
				c = '-';
				break;	/* resume the for(;;) */
			}
			fmt++;
			do {		/* fill in the range */
				tab[++c] = v;
			} while (c < n);
#if 1	/* XXX another disgusting compatibility hack */
			/*
			 * Alas, the V7 Unix scanf also treats formats
			 * such as [a-c-e] as `the letters a through e'.
			 * This too is permitted by the standard....
			 */
			goto doswitch;
#else
			c = *fmt++;
			if (c == 0)
				return (fmt - 1);
			if (c == ']')
				return (fmt);
#endif
			break;

		case ']':		/* end of scanset */
			return (fmt);

		default:		/* just another character */
			c = n;
			break;
		}
	}
	/* NOTREACHED */
}

/// string scanf

struct sscanf_info {
  const char* str;
};

static int sscanf_fgetc(void* stream) {
  return *((struct sscanf_info*)stream)->str++;
}

static int sscanf_ungetc(int ch, void* stream) {
  ((struct sscanf_info*)stream)->str--;
  return ch;
}

int sscanf(const char *str, const char *format, ...) {
  struct sscanf_info info = {str};
  va_list args;
  va_start(args, format);
  return std_vxscanf(sscanf_fgetc, sscanf_ungetc, &info,
	      format, args);
}

/// file descriptor scanf

struct fdscanf_info {
  int fd;
  int unget_buffer;
};

static int fdscanf_fgetc(void* _stream) {
  struct fdscanf_info *stream = _stream;
  int res;
  int err;
  if((res = stream->unget_buffer) != -1) {
    stream->unget_buffer = -1;
    return res;
  }
  err = read(stream->fd, &res, 1);
  if(err == -1) {
    uprintf("fdscan fgetc I/O error %d\n", err);
    return -1;
  }
  return res;
}

static int fdscanf_ungetc(int ch, void* _stream) {
  struct fdscanf_info *stream = _stream;
  stream->unget_buffer = ch;
  return ch;
}

int fdscanf(int fd, const char *format, ...) {
  struct fdscanf_info info = {fd, -1};
  va_list args;
  va_start(args, format);
  return std_vxscanf(fdscanf_fgetc, fdscanf_ungetc, &info,
	      format, args);
}

int yscanf(const char *format, ...) {
  struct fdscanf_info info = {stdin_fd, -1};
  va_list args;
  va_start(args, format);
  return std_vxscanf(fdscanf_fgetc, fdscanf_ungetc, &info,
	      format, args);
}

///// 

void setPos(int r, int c) {
  row = r;
  col = c;
}

int getRow() {
  return row;
}

int getCol() {
  return col;
}

unsigned char bcd2int(unsigned char bcd) {
  return (bcd >> 4) * 10 + (bcd & 0xF);
}

int strcmp(const char* s1, const char*s2) {
  while(*s1 && *s2) {
    if(*s1 != *s2)
      return *s2 - *s1;
    else
      s1++, s2++;
  }
  
  if(*s1 == *s2)
    return 0;
  else
    return *s1 ? 1 : -1;
}

int strncmp(const char* s1, const char*s2, size_t size) {
  while(*s1 && *s2 && size--) {
    if(*s1 != *s2)
      return *s2 - *s1;
    else
      s1++, s2++;
  }
  
  if(*s1 == *s2 || size == 0)
    return 0;
  else
    return *s1 ? 1 : -1;
}

size_t strlen(const char *s) {
  int i=0;
  while(*s++) 
    i++;
  return i;
}

char *strstr(const char *haystack, const char *needle) {
  /** I don't have time to think ...*/
  int hi = strlen(haystack);
  int len = strlen(needle);
  int i;
  for(i=0; i < hi; i++)
    if(strncmp(haystack+i, needle, len) == 0)
      return (char*)(haystack+i);
  return 0;
}

char *strchr(const char *haystack, int c) {
  while(*haystack) {
    if(*haystack == c)
      return (char*)haystack;
    else 
      haystack++;
  }
  return 0;
}

void *memcpy (void *__restrict dest,
	      __const void *__restrict src, size_t n) {
  void *d=dest;
  while(n--)
    *((char*)dest)++ = *((char*)src)++;
  return d;
}

void *memset (void *s, int c, size_t n) {
  while(n--)
    *((char*)s)++ = c;
  return s;
}

char *strcpy(char *dest, const char *str) {
  int n = strlen(str) + 1;
  memcpy(dest, str, n);
  return dest;
}

char *strncpy(char *dest, const char *str, size_t n) {
  int len = strlen(str) + 1;
  n = len < n ? len : n;
  memcpy(dest, str, n);
  return dest;
}

char *strcat(char *dest, const char *src) {
  strcpy(dest + strlen(dest), src);
  return dest;
}

char *strdup(const char *src) {
  char* res = malloc(strlen(src) + 1);
  strcpy(res, src);
  return res;
}

void hexdump(u_int8_t *data, int len) {
  u_int8_t line[16];
  int lines = len / sizeof(line);
  int l,c;

  for(l=0; l<lines; l++) {
    xprintf("%08x  ", l*sizeof(line));

    for(c=0; c<sizeof(line); c++) {
      xprintf("%02x ", data[l*sizeof(line) + c]);
      if(c == (sizeof(line)/2-1))
	xprintf(" ");
    }

    xprintf(" |");

    for(c=0; c<sizeof(line); c++) {
      u_int8_t d = data[l*sizeof(line) + c];
      xprintf("%c", isprint(d) ? d : '.');
    }

    xprintf("|\n");
  }
}
