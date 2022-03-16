#ifndef _MD5C_H
#define _MD5C_H

#include <AgbTypes.h>

typedef struct {
    char _[88];
} MD5_CTX;

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, u8 *data, u32 size);
void MD5Final(u8 *out, MD5_CTX *context);

#endif // _MD5C_H
