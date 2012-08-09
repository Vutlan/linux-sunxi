/**
 * common.c - common operations
 * date:    2012-2-13 8:42:56
 * author:  Aaron<leafy.myeh@allwinnertech.com>
 * history: V0.1
 */

#include "pm_types.h"
#include "pm.h"
#include <stdarg.h>

typedef unsigned int		size_t;
typedef unsigned int		ptrdiff_t;

#define NUM_TYPE long long

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static size_t _strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
	{
		/* nothing */
		;
	}
	return sc - s;
}

static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

static char *number(char *buf, unsigned NUM_TYPE num, int base, int size, int type)
{
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */
	char sign;
	char locase;
	int need_pfx = ((type & SPECIAL) && base != 10);
	char tmp[66];
	int precision;
	int i;
	
	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	sign = 0;
	if (type & SIGN) {
		if ((signed NUM_TYPE) num < 0) {
			sign = '-';
			num = - (signed NUM_TYPE) num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (need_pfx) {
		size--;
		if (base == 16)
			size--;
	}
	/* generate full string in tmp[], in reverse order */
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else if (base != 10) { /* 8 or 16 */
		int mask = base - 1;
		int shift = 3;
		if (base == 16) shift = 4;
		do {
			tmp[i++] = (digits[((unsigned char)num) & mask] | locase);
			num >>= shift;
		} while (num);
	} else { /* base 10 */
		unsigned val = (unsigned)num;
		while (val) {
			tmp[i++] = digits[val%10];
			val /= 10;
		}
	}
	
	/* printing 100 using %2d gives "100", not "00" */
	precision = i;
	/* leading space padding */
	size -= precision;
	if (!(type & (ZEROPAD+LEFT)))
		while(--size >= 0)
			*buf++ = ' ';
	/* sign */
	if (sign)
		*buf++ = sign;
	/* "0x" / "0" prefix */
	if (need_pfx) {
		*buf++ = '0';
		if (base == 16)
			*buf++ = ('X' | locase);
	}
	/* zero or space padding */
	if (!(type & LEFT)) {
		char c = (type & ZEROPAD) ? '0' : ' ';
		while (--size >= 0)
			*buf++ = c;
	}
	/* hmm even more zero padding? */
	while (i <= --precision)
		*buf++ = '0';
	/* actual digits of result */
	while (--i >= 0)
		*buf++ = tmp[i];
	/* trailing space padding */
	while (--size >= 0)
		*buf++ = ' ';
	return buf;
}

int _vsprintf(char *buf, const char *fmt, va_list args)
{
	unsigned NUM_TYPE num;
	int flags = ' ';	/* flag: *, -,  */
	int field_width;/* width of output field */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
				/* 'z' support added 23/7/1999 S.H.    */
				/* 'z' changed to 'Z' --davidm 1/25/99 */
				/* 't' added for ptrdiff_t */
	char *str = buf;
	int base;
	char* s;
	int arglen;
	
	for (; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
		
		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
			}
		
		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}
		
		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
		    *fmt == 'Z' || *fmt == 'z' || *fmt == 't') {
			qualifier = *fmt;
			++fmt;
			if (qualifier == 'l' && *fmt == 'l') {
				qualifier = 'L';
				++fmt;
			}
		}
		
		/* default base */
		base = 10;
		switch (*fmt) {
			case 'c':
				if (!(flags & LEFT))
					while (--field_width > 0)
						*str++ = ' ';
				*str++ = (unsigned char) va_arg(args, int);
				while (--field_width > 0)
					*str++ = ' ';
				continue;
				
			case 's':
				s = va_arg(args, char*);
				if (!s)
					s = "<NULL>";
				arglen = _strlen(s);
				if (!(flags & LEFT))
					while (field_width-- > arglen)
						*str++ = ' ';
				while (arglen--)
					*str++ = *s++;
				while (field_width-- > arglen)
					*str++ = ' ';
				continue;
				
			case '%':
				*str++ = '%';
				continue;
			
			/* integer number formats - set up the flags and "break" */
			case 'o':
				base = 8;
				break;
				
			case 'p':
			case 'x':
				flags |= SMALL;
			case 'X':
				base = 16;
				break;
			
			case 'd':
			case 'i':
				flags |= SIGN;
			case 'u':
				break;
	
			default:
				*str++ = '%';
				if (*fmt)
					*str++ = *fmt;
				else
					--fmt;
				continue;
		}
		
		if (qualifier == 'L')  /* "quad" for 64 bit variables */
			num = va_arg(args, unsigned long long);
		else if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'Z' || qualifier == 'z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 't') {
			num = va_arg(args, ptrdiff_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}
		str = number(str, num, base, field_width, flags);
	}
	*str = '\0';
	return str - buf;
}

void printk(const char *fmt, ...)
{
	char buf[384];
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = _vsprintf(buf, fmt, ap);
	serial_puts(buf, n);
	va_end(ap);
}

void printk_nommu(const char *fmt, ...)
{
	char buf[384];
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = _vsprintf(buf, fmt, ap);
	serial_puts_nommu(buf, n);
	va_end(ap);
}

