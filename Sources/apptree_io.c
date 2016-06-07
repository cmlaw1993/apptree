
/** @file apptree_io.c
 *  @brief Implements the io functionlities for the apptree.
 *  @author Dennis Law
 *  @date June 2016
 */

#include <stdarg.h>
#include <string.h>
#include "apptree_io.h"

static char *convert(unsigned int num, int base);

static struct apptree_io_control control;

/** Initialize the apptree_io
 *	@param read_input Function for reading an input char.
 *	@param write_output Function for writing an output char.
 */
void apptree_io_init(int (*read_input)(char *input),
						void (*write_output)(char output))
{
	control.read_input	 = read_input;
	control.write_output = write_output;
}

/** @brief converts the number base of an integer
 *	@param num The integer to be converted.
 *	@param base The base to be converted into.
 *	@returns Returns the result as a string format.
 */
static char *convert(unsigned int num, int base)
{
	static char buff[10];
	char *ptr;
	ptr = &buff[sizeof(buff)-1];
	*ptr = '\0';
	do {
		*--ptr = "0123456789abcdef"[num%base];
		num /= base;
	} while (num != 0);
	return(ptr);
}

/** @brief Writes a char to output
 *	@param c The character to be written.
 *
 *	Redirects the output from a char to the write_output function in the
 *	control struct.
 */
void apptree_putc(char c)
{
	control.write_output(c);
}

/** @brief Writes a string to output
 *	@param s The string of characters to be written.
 *
 *	Redirects the output from a string to the write_output function in the
 *	control struct.
 */
void apptree_puts(char *s)
{
	int i = 0;
	
	while (s[i] != '\0')
		control.write_output(s[i++]);
}

/** @brief An implementation of the C library printf function
 *	This function works like the printf function from the C standard library
 *	with a few missing features.
 *
 *	The following specifiers are supported by this function
 *		1. %c
 *		2. %d
 *		3. %o
 *		4. %s
 *		5. %u
 *		6. %x
 *		7. %%
 *
 *	It also supports length modifiers for %d, %o, %u and %x. All outputs of
 *	this function is redirected to the write_output function in the control
 *	struct.
 */
void apptree_print(char *format, ...)
{
	va_list arg;
	char *p;
	
	int i;
	unsigned u;
	char *s;
	
	int lmod;
	char *buff;
	int j;
	
	va_start(arg, format);
	
	p = format;
	
	for (p = format; *p != '\0'; p++)
	{
		if (*p != '%') {
			apptree_putc(*p);
			continue;
		}
		
		p++;
		
		/* Find length modifiers if available */
		if ((*p >= 49) && (*p <= 57)) {
			lmod = *p - 48;
			p++;
		} else {
			lmod = 0;
		}
		
		switch (*p) {
		case 'c':
			i = va_arg(arg, int);
			apptree_putc(i);
			break;
		case 'd':
			i = va_arg(arg, int);
			if (i < 0) {
				i = -i;
				apptree_putc('-');
			}
			buff = convert(i, 10);
			if (lmod > strlen(buff)) {
				for (j = 0; j < (lmod - strlen(buff)); j++)
					apptree_putc(' ');
			}
			apptree_puts(buff);
			break;
			
		case 'o':
			i = va_arg(arg, unsigned int);
			buff = convert(i, 8);
			if (lmod > strlen(buff)) {
				for (j = 0; j < (lmod - strlen(buff)); j++)
					apptree_putc(' ');
			}
			apptree_puts(buff);
			break;
		case 's':
			s = va_arg(arg, char *);
			apptree_puts(s);
			break;
		case 'u':
			u = va_arg(arg, unsigned int);
			buff = convert(u, 10);
			if (lmod > strlen(buff)) {
				for (j = 0; j < (lmod - strlen(buff)); j++)
					apptree_putc(' ');
			}
			apptree_puts(buff);
			break;
		case 'x':
			u = va_arg(arg, unsigned int);
			buff = convert(u, 16);
			if (lmod > strlen(buff)) {
				for (j = 0; j < (lmod - strlen(buff)); j++)
					apptree_putc(' ');
			}
			apptree_puts(buff);
			break;
		case '%':
			apptree_putc('%');
			break;
		}
	}
	
	va_end(arg);
}

/** @brief Reads a character from the binded input
 *	@input The character read.
 *	@returns Returns 0 if a new character is read and -1 if otherwise.
 */
int apptree_read(char *input)
{
	return control.read_input(input);
}
