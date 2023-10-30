/*
    Random generator helpers
*/

#ifndef _ERANDOM_H_
#define _ERANDOM_H_

/*----------------------------------------------------------------------*/

/* random characters */
char erandalpha();
char erandalnum();

/* random strings */
void erandalpha_str(char* str_out, size_t str_len);
void erandalnum_str(char* str_out, size_t str_len);
void erand_str(char* str_out, size_t str_len, const char* alphabet, size_t alphabet_len);

/*----------------------------------------------------------------------*/

#endif /* _ERANDOM_H_ */

