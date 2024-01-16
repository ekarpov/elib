/*
 *  ELib core functionality examples
 */


#include "../src/elib.h"

void examples_core()
{
	/*** trace in DEBUG builds ***/

	/* simple message trace */
	ETRACE("Some message here");

	/* trace with arguments */
	int int_value = 5;
	const char* char_value = "text";
	ETRACE("int_value=%d, char_value=%s", int_value, char_value);

}