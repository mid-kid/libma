#ifndef _MA_SUB
#define _MA_SUB

#include <AgbTypes.h>

void *MAU_memset(void *dest, u32 val, u32 size);
void *MAU_memcpy(void *dest, void *src, u32 size);
void *MAU_strcpy(char *dest, char *src);
int MAU_strlen(char *str);

#endif // _MA_SUB
