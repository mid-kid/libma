#include "ma_ango.h"

#include "ma_sub.h"
#include "md5.h"

#define HASH_SIZE 0x10
#define KEY_SIZE_BIN 0x24
#define KEY_SIZE_TEXT 0x30

static void gb_MakeSecretCode(const char *key, const char *userid,
    const char *password, u8 *out);
static void gb_OutSecretCode(int length, const char *key, u8 *out);
static void gb_CreateMD5Hash(u8 *out, const char *key, const char *password);
static void gb_BitHalfMove(u8 *out, const u8 *in);
static void gb_BitChangeAndRotation(u8 *out, const u8 *in);
static void CalcValueMD5(u8 *data, u32 size, u8 *out);
static void Base64_encode(int length, u8 *data, char *out);
static void Base64_decode(int length, const char *string, u8 *out);

static int i, j, k;
static int len;

void MA_MakeAuthorizationCode(const char *key, const char *userid,
    const char *password, char *out)
{
    static u8 seq_bin[KEY_SIZE_BIN];
    static char seq_text[KEY_SIZE_TEXT + 1];
    static u8 onetime_key_bin[KEY_SIZE_BIN];
    static char onetime_key_text[KEY_SIZE_TEXT - 4 + 1];

    seq_text[0] = '\0';
    onetime_key_text[0] = '\0';
    out[0] = '\0';

    gb_MakeSecretCode(key, userid, password, seq_bin);
    gb_OutSecretCode(KEY_SIZE_TEXT, key, seq_bin);
    Base64_encode(KEY_SIZE_BIN, seq_bin, seq_text);
    Base64_decode(KEY_SIZE_TEXT, key, onetime_key_bin);
    Base64_encode(KEY_SIZE_BIN - 4, onetime_key_bin, onetime_key_text);
    MAU_strcpy(out, onetime_key_text);
    MAU_strcpy(&out[KEY_SIZE_TEXT - 4], seq_text);
}

static void gb_MakeSecretCode(const char *key, const char *userid,
    const char *password, u8 *out)
{
    static u8 hash[HASH_SIZE + 1];
    static int j;

    MAU_memset(hash, 0, sizeof(hash));

    gb_CreateMD5Hash(hash, key, password);
    MAU_memcpy(out, hash, HASH_SIZE);
    MAU_memcpy(&out[HASH_SIZE], userid, MAU_strlen(userid));

    j = MAU_strlen(userid) + HASH_SIZE;
    MAU_memset(&out[j], 0xff, KEY_SIZE_BIN - j);
}

static void gb_OutSecretCode(int length, const char *key, u8 *out)
{
    static u8 dest[KEY_SIZE_BIN + 1];
    static u8 result[KEY_SIZE_BIN + 1];

    MAU_memset(dest, 0, sizeof(dest));

    Base64_decode(length, key, dest);
    gb_BitHalfMove(result, dest);
    gb_BitChangeAndRotation(out, result);
}

static void gb_CreateMD5Hash(u8 *out, const char *key, const char *password)
{
    static char buf[100];

    MAU_memset(buf, 0, sizeof(buf));

    MAU_memcpy(buf, key, KEY_SIZE_TEXT);
    len = MAU_strlen(password);
    MAU_memcpy(&buf[KEY_SIZE_TEXT], password, len);
    len = MAU_strlen(buf);
    CalcValueMD5(buf, len, out);
}

static void gb_BitHalfMove(u8 *out, const u8 *in)
{
    static int half;

    MAU_memset(out, 0, KEY_SIZE_BIN + 1);

    half = KEY_SIZE_BIN / 2;
    for (i = 0; i < half; i++) {
        j = i * 2;
        out[i] = out[i]
            | (in[j] & 0x40) << 1
            | (in[j] & 0x10) << 2
            | (in[j] & 0x04) << 3
            | (in[j] & 0x01) << 4
            | (in[j + 1] & 0x40) >> 3
            | (in[j + 1] & 0x10) >> 2
            | (in[j + 1] & 0x04) >> 1
            | (in[j + 1] & 0x01) >> 0;
    }

    for (; i < KEY_SIZE_BIN; i++) {
        j = (i - half) * 2;
        out[i] = out[i]
            | (in[j] & 0x80) << 0
            | (in[j] & 0x20) << 1
            | (in[j] & 0x08) << 2
            | (in[j] & 0x02) << 3
            | (in[j + 1] & 0x80) >> 4
            | (in[j + 1] & 0x20) >> 3
            | (in[j + 1] & 0x08) >> 2
            | (in[j + 1] & 0x02) >> 1;
    }
}

static void gb_BitChangeAndRotation(u8 *out, const u8 *in)
{
    static u8 buf;

    for (i = 0; i < KEY_SIZE_BIN; i++) {
        u8 *outp = &out[i];
        const u8 *inp = &in[i];

        u8 c = *outp;
        c ^= *inp;
        buf = (c & 0xb6)
            | (c & 0x08) << 3
            | (c & 0x01) << 3
            | (c & 0x40) >> 6;
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

static const char xchg[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

static void Base64_decode(int length, const char *string, u8 *out)
{
    static const u8 base64RevTable[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
        0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
        0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
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
