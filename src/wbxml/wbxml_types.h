/*
    WBXML decoder/encoder types
    Refer to WBXML documentation: http://www.w3.org/1999/06/NOTE-wbxml-19990624/
*/

#ifndef _WBXML_TYPES_H_
#define _WBXML_TYPES_H_

/*----------------------------------------------------------------------*/
/* tag flags */
#define WBXML_TAG_WITH_ATTRIBUTES       0x80        /* bit 7 Indicates whether attributes follow the tag code.
                                                       If this bit is zero, the tag contains no attributes.
                                                       If this bit is one, the tag is followed immediately by one or more attributes.
                                                       The attribute list is terminated by an END token.*/
#define WBXML_TAG_WITH_CONTENT          0x40        /* bit 6 Indicates whether this tag begins an element containing content.
                                                       If this bit is zero, the tag contains no content and no end tag.
                                                       If this bit is one, the tag is followed by any content it contains and
                                                       is terminated by an END token.*/
#define WBXML_TAG_MASK                  0x1F        /* bits 5-0 indicate the tag identity.*/

/* tag flags */
#define WBXML_MULTIBYTE_CONTINUE        0x80        /* multibyte continuation flag */
#define WBXML_MULTIBYTE_CONTENT_MASK    0x7F        /* byte content mask */

/*----------------------------------------------------------------------*/
/* codepage */
typedef struct {

    /* lookup table to encode tags from codes */
    const euint8_t*     tags_table;
    size_t              tags_count;

    /* lookup table to decode tags to codes */
    const euint8_t*     codes_table;
    size_t              codes_count;

} wbxml_codepage_t;

/*----------------------------------------------------------------------*/
/* reserved tokens */
/*----------------------------------------------------------------------*/

#define WBXML_TOKEN_SWITCH_PAGE         0x00    /* Change the code page for the current token state. Followed by a single u_int8 indicating the new code page number. */
#define WBXML_TOKEN_END                 0x01    /* Indicates the end of an attribute list or the end of an element.*/
#define WBXML_TOKEN_ENTITY              0x02    /* A character entity. Followed by a mb_u_int32 encoding the character entity number.*/
#define WBXML_TOKEN_STR_I               0x03    /* Inline string. Followed by a termstr.*/
#define WBXML_TOKEN_LITERAL             0x04    /* An unknown tag or attribute name. Followed by an mb_u_int32 that encodes an offset into the string table.*/
#define WBXML_TOKEN_EXT_I_0             0x40    /* Inline string document-type-specific extension token. Token is followed by a termstr.*/
#define WBXML_TOKEN_EXT_I_1             0x41    /* Inline string document-type-specific extension token. Token is followed by a termstr.*/
#define WBXML_TOKEN_EXT_I_2             0x42    /* Inline string document-type-specific extension token. Token is followed by a termstr.*/
#define WBXML_TOKEN_PI                  0x43    /* Processing instruction.*/
#define WBXML_TOKEN_LITERAL_C           0x44    /* Unknown tag, with content.*/
#define WBXML_TOKEN_EXT_T_0             0x80    /* Inline integer document-type-specific extension token. Token is followed by a mb_uint_32.*/
#define WBXML_TOKEN_EXT_T_1             0x81    /* Inline integer document-type-specific extension token. Token is followed by a mb_uint_32.*/
#define WBXML_TOKEN_EXT_T_2             0x82    /* Inline integer document-type-specific extension token. Token is followed by a mb_uint_32.*/
#define WBXML_TOKEN_STR_T               0x83    /* String table reference. Followed by a mb_u_int32 encoding a byte offset from the beginning of the string table.*/
#define WBXML_TOKEN_LITERAL_A           0x84    /* Unknown tag, with attributes.*/
#define WBXML_TOKEN_EXT_0               0xC0    /* Single-byte document-type-specific extension token.*/
#define WBXML_TOKEN_EXT_1               0xC1    /* Single-byte document-type-specific extension token.*/
#define WBXML_TOKEN_EXT_2               0xC2    /* Single-byte document-type-specific extension token.*/
#define WBXML_TOKEN_OPAQUE              0xC3    /* Opaque document-type-specific data.*/
#define WBXML_TOKEN_LITERAL_AC          0xC4    /* Unknown tag, with content and attributes.*/

/*----------------------------------------------------------------------*/
/* public identifiers */
/*----------------------------------------------------------------------*/

#define WBXML_PUBLICID_LITERAL          0x00    /* String table index follows; public identifier is encoded as a literal in the string table. */
#define WBXML_PUBLICID_UNKNOWN          0x01    /* Unknown or missing public identifier. */
#define WBXML_PUBLICID_WML10            0x02	/* "-//WAPFORUM//DTD WML 1.0//EN" (WML 1.0) */
#define WBXML_PUBLICID_WTA10            0x03	/* "-//WAPFORUM//DTD WTA 1.0//EN" (WTA Event 1.0) */
#define WBXML_PUBLICID_WML11            0x04	/* "-//WAPFORUM//DTD WML 1.1//EN" (WML 1.1) */
#define WBXML_PUBLICID_RESERVED_BEGIN   0x05
#define WBXML_PUBLICID_RESERVED_END     0x7F

/*----------------------------------------------------------------------*/

#endif /* _WBXML_TYPES_H_ */

