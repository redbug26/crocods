#include "os.h"

#if defined(_WIN32)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    UTFmax    = 4,                      /* maximum bytes per rune */
    Runesync  = 0x80,                   /* cannot represent part of a UTF sequence (<) */
    Runeself  = 0x80,                   /* rune and UTF sequences are the same (<) */
    Runeerror = 0xFFFD,                 /* decoding error in UTF */
    Runemax   = 0x10FFFF,               /* maximum rune value */
};

enum {
    Bit1  = 7,
    Bitx  = 6,
    Bit2  = 5,
    Bit3  = 4,
    Bit4  = 3,
    Bit5  = 2,

    T1    = ((1 << (Bit1 + 1)) - 1) ^ 0xFF,     /* 0000 0000 */
    Tx    = ((1 << (Bitx + 1)) - 1) ^ 0xFF,     /* 1000 0000 */
    T2    = ((1 << (Bit2 + 1)) - 1) ^ 0xFF,     /* 1100 0000 */
    T3    = ((1 << (Bit3 + 1)) - 1) ^ 0xFF,     /* 1110 0000 */
    T4    = ((1 << (Bit4 + 1)) - 1) ^ 0xFF,     /* 1111 0000 */
    T5    = ((1 << (Bit5 + 1)) - 1) ^ 0xFF,     /* 1111 1000 */

    Rune1 = (1 << (Bit1 + 0 * Bitx)) - 1,       /* 0000 0000 0000 0000 0111 1111 */
    Rune2 = (1 << (Bit2 + 1 * Bitx)) - 1,       /* 0000 0000 0000 0111 1111 1111 */
    Rune3 = (1 << (Bit3 + 2 * Bitx)) - 1,       /* 0000 0000 1111 1111 1111 1111 */
    Rune4 = (1 << (Bit4 + 3 * Bitx)) - 1,       /* 0001 1111 1111 1111 1111 1111 */

    Maskx = (1 << Bitx) - 1,                   /* 0011 1111 */
    Testx = Maskx ^ 0xFF,                      /* 1100 0000 */

    Bad   = Runeerror

};

#define FZ_REPLACEMENT_CHARACTER 0xFFFD

int
fz_chartorune(int *rune, const char *str)
{
    int c, c1, c2, c3;
    int l;

    /*
     * one character sequence
     *	00000-0007F => T1
     */
    c = *(const unsigned char *)str;
    if (c < Tx) {
        *rune = c;
        return 1;
    }

    /*
     * two character sequence
     *	0080-07FF => T2 Tx
     */
    c1 = *(const unsigned char *)(str + 1) ^ Tx;
    if (c1 & Testx)
        goto bad;
    if (c < T3) {
        if (c < T2)
            goto bad;
        l = ((c << Bitx) | c1) & Rune2;
        if (l <= Rune1)
            goto bad;
        *rune = l;
        return 2;
    }

    /*
     * three character sequence
     *	0800-FFFF => T3 Tx Tx
     */
    c2 = *(const unsigned char *)(str + 2) ^ Tx;
    if (c2 & Testx)
        goto bad;
    if (c < T4) {
        l = ((((c << Bitx) | c1) << Bitx) | c2) & Rune3;
        if (l <= Rune2)
            goto bad;
        *rune = l;
        return 3;
    }

    /*
     * four character sequence (21-bit value)
     *	10000-1FFFFF => T4 Tx Tx Tx
     */
    c3 = *(const unsigned char *)(str + 3) ^ Tx;
    if (c3 & Testx)
        goto bad;
    if (c < T5) {
        l = ((((((c << Bitx) | c1) << Bitx) | c2) << Bitx) | c3) & Rune4;
        if (l <= Rune3)
            goto bad;
        *rune = l;
        return 4;
    }
    /*
     * Support for 5-byte or longer UTF-8 would go here, but
     * since we don't have that, we'll just fall through to bad.
     */

    /*
     * bad decoding
     */
 bad:
    *rune = Bad;
    return 1;
} /* fz_chartorune */


wchar_t *
fz_wchar_from_utf8(const char *s)
{
    wchar_t *d, *r;
    int c;

    r = d = malloc((strlen(s) + 1) * sizeof(wchar_t));
    if (!r)
        return NULL;
    while (*s) {
        s += fz_chartorune(&c, s);
        /* Truncating c to a wchar_t can be problematic if c
         * is 0x10000. */
        if (c >= 0x10000)
            c = FZ_REPLACEMENT_CHARACTER;
        *d++ = c;
    }
    *d = 0;
    return r;
}

void * os_fopen(const char *name, const char *mode)
{
    wchar_t *wname, *wmode;
    FILE *file;

    wname = fz_wchar_from_utf8(name);
    if (wname == NULL) {
        return NULL;
    }

    wmode = fz_wchar_from_utf8(mode);
    if (wmode == NULL) {
        free(wname);
        return NULL;
    }

    file = _wfopen(wname, wmode);

    free(wname);
    free(wmode);
    return file;
} /* fz_fopen_utf8 */

#endif /* if defined(_WIN32) */