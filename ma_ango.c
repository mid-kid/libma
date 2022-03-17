#include "ma_ango.h"
#include "libma.h"

#include "ma_sub.h"
#include "md5.h"

static void gb_MakeSecretCode(char *key1, char *key2, char *key3, u8 *out);
static void gb_OutSecretCode(int length, char *string, u8 *out);
static void gb_CreateMD5Hash(u8 *out, char *key1, char *key3);
static void gb_BitHalfMove(u8 *out, u8 *in);
static void gb_BitChangeAndRotation(u8 *out, u8 *in);
static void CalcValueMD5(u8 *data, u32 size, u8 *out);
static void Base64_encode(int length, u8 *data, char *out);
static void Base64_decode(int length, char *string, u8 *out);

static int i, j, k;
static int len;

void MA_MakeAuthorizationCode(char *key1, char *key2, char *key3, char *out)
{
    static u8 seq_bin[0x24];
    static char seq_text[0x31];
    static u8 onetime_key_bin[0x24];
    static char onetime_key_text[0x2d];

    seq_text[0] = 0;
    onetime_key_text[0] = 0;
    out[0] = 0;

    gb_MakeSecretCode(key1, key2, key3, seq_bin);
    gb_OutSecretCode(0x30, key1, seq_bin);
    Base64_encode(0x24, seq_bin, seq_text);
    Base64_decode(0x30, key1, onetime_key_bin);
    Base64_encode(0x20, onetime_key_bin, onetime_key_text);
    MAU_strcpy(out, onetime_key_text);
    MAU_strcpy(out + 0x2c, seq_text);
}

static void gb_MakeSecretCode(char *key1, char *key2, char *key3, u8 *out)
{
    static u8 hash[0x11];
    static u32 j;

    MAU_memset(hash, 0, 0x11);
    gb_CreateMD5Hash(hash, key1, key3);

    MAU_memcpy(out, hash, 0x10);
    MAU_memcpy(out + 0x10, key2, MAU_strlen(key2));

    j = MAU_strlen(key2) + 0x10;
    MAU_memset(out + j, 0xff, 0x24 - j);
}

static void gb_OutSecretCode(int length, char *string, u8 *out)
{
    static u8 dest[0x25];
    static u8 result[0x25];

    MAU_memset(dest, 0, sizeof(dest));
    Base64_decode(length, string, dest);
    gb_BitHalfMove(result, dest);
    gb_BitChangeAndRotation(out, result);
}

static void gb_CreateMD5Hash(u8 *out, char *key1, char *key3)
{
    static u8 buf[100];

    MAU_memset(buf, 0, sizeof(buf));
    MAU_memcpy(buf, key1, 0x30);
    len = MAU_strlen(key3);
    MAU_memcpy(buf + 0x30, key3, len);
    len = MAU_strlen(buf);
    CalcValueMD5(buf, len, out);
}

static void gb_BitHalfMove(u8 *out, u8 *in)
{
    static int half;

    MAU_memset(out, 0, 0x25);
    half = 0x12;

    for (i = 0; i < half; i++) {
        j = i * 2;
        out[i] = out[i] |
            (in[j] & 0x40) << 1 |
            (in[j] & 0x10) << 2 |
            (in[j] & 0x04) << 3 |
            (in[j] & 0x01) << 4 |
            (in[j + 1] & 0x40) >> 3 |
            (in[j + 1] & 0x10) >> 2 |
            (in[j + 1] & 0x04) >> 1 |
            (in[j + 1] & 0x01) >> 0;
    }

    for (; i < 0x24; i++) {
        j = (i - half) * 2;
        out[i] = out[i] |
            (in[j] & 0x80) << 0 |
            (in[j] & 0x20) << 1 |
            (in[j] & 0x08) << 2 |
            (in[j] & 0x02) << 3 |
            (in[j + 1] & 0x80) >> 4 |
            (in[j + 1] & 0x20) >> 3 |
            (in[j + 1] & 0x08) >> 2 |
            (in[j + 1] & 0x02) >> 1;
    }
}

static void gb_BitChangeAndRotation(u8 *out, u8 *in)
{
    static u8 buf;
    u8 *outp, *inp;
    u8 c;

    for (i = 0; i < 0x24; i++) {
        outp = out + i;
        inp = in + i;

        c = *outp;
        c ^= *inp;
        buf = (c & 0xb6) |
            (c & 0x08) << 3 |
            (c & 0x01) << 3 |
            (c & 0x40) >> 6;
        *outp = buf;
    }
}

static void CalcValueMD5(u8 *data, u32 size, u8 *out)
{
    static MD5_CTX context;

    MAU_memset(&context, 0, sizeof(context));
    MD5Init(&context);
    MD5Update(&context, data, size);
    MD5Final(out, &context);
}

static const char xchg[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void Base64_encode(int length, u8 *data, char *out)
{
    static u32 code;

    len = 0;
    while (len < length) {
        // Extract up to 3 bytes (24 bits) into code
        code = 0;
        for (i = 0; i < 3; i++) {
            if (len >= length) {
                code <<= (3 - i) * 8;
                goto brk;
            }
            code = (code << 8) | *data++;
            len++;
        }
        brk:

        // Write four 6-bit symbols into the resulting string
        for (k = 0; k < 4; k++) {
            if (k <= i) {
                *out++ = xchg[(code >> ((3 - k) * 6)) % 64];
            } else {
                *out++ = '=';
            }
        }
    }

    *out = 0;
}

static void Base64_decode(int length, char *string, u8 *out)
{
    static const u8 base64RevTable[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E,
        0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
        0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02,
        0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
        0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    static u32 code;
    static int c;
    static int byte;

    for (byte = 0; byte < length;) {
        code = 0;

        // Extract up to 4 symbols from the string
        // 6 bits per character, 6*4 = 24 bits
        for (i = 0; i < 4; i++) {
            if (*string >= 0x20 && *string < 0x80) {
                c = base64RevTable[*string - 0x20];
            } else {
                c = 0xff;
            }
            string++;
            byte++;

            // If we encountered an invalid character, pad with zero
            if (c == 0xff) {
                code = code << ((4 - i) * 6);
                break;
            }
            code = code << 6 | c;
        }

        // Write the extracted code, 8 bits at a time
        for (k = 0; k < (i - 1); k++) {
            *out++ = code >> ((2 - k) * 8);
        }
    }

    *out = 0;
}
