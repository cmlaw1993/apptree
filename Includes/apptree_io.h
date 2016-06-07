
/** @file apptree_io.h
 *  @brief Header file for apptree_io.c
 *  @author Dennis Law
 *  @date April 2016
 */

#ifndef APPTREE_IO_H
#define APPTREE_IO_H

#include "apptree.h"

struct apptree_io_control {
	/** @brief Non-blocking function for reading a single input.
		@param input The input character read.
		@returns Returns 0 if a new input is detected and -1 if
		otherwise.
	 */
	int (*read_input)(char *input);
	/** @brief Blocking function for writing a single output.
	 *	@param output The character to be written.
	 */
	void (*write_output)(char output);
};


void apptree_io_init(int (*read_input)(char *input),
						void (*write_output)(char output));

void apptree_print(char *format, ...);
int apptree_read(char *input);

#endif	/* APPTREE_IO_H */
