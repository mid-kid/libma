#include "ma_sub.h"
#include "libma.h"

#include <stdlib.h>

static void xtoa(unsigned num, char *dest, unsigned base, int negative);

int MAU_strlen(char *str)
{
    static char *eos;

    eos = str;
    while (*eos++ != '\0');
    return (u16)(eos - str - 1);
}

void MAU_strcpy(char *dest, char *src)
{
    static char *pSrc;
    pSrc = src;

    *dest++ = *pSrc;
    while (*pSrc != '\0') {
        pSrc++;
        *dest++ = *pSrc;
    }
}

int MAU_strcpy_size(char *dest, char *src, int size)
{
    static int ret;

    for (ret = 0; size != 0; size--, ret++) {
        *dest = *src;
        if (*src == '\0') break;
        dest++;
        src++;
    }
    if (size != 0) ret++;
    return ret;
}

char *MAU_strncpy_CRLF_LF(char *dest, char *src, int size)
{
    if (size == 0) return NULL;

    while (size != 0) {
        if ((src[0] == '\r') && (src[1] == '\n')) {
            *dest = '\0';
            return src + 2;
        }
        if (src[0] == '\n') {
            *dest = '\0';
            return src + 1;
        }
        *dest++ = *src++;
        size--;
    }
    *dest = '\0';
    return NULL;
}

char *MAU_SearchCRLF(char *str, int size)
{
    if (size < 2) return NULL;

    while (size != 1) {
        if ((str[0] == '\r') && (str[1] == '\n')) {
            *str = '\0';
            return str + 2;
        }
        str++;
        size--;
    }
    return NULL;
}

void MAU_strcat(char *dest, char *src)
{
    while (*dest != '\0') dest++;

    for (;;) {
        *dest++ = *src;
        if (*src == '\0') break;
        src++;
    }
}

char *MAU_strchr(char *str, int c)
{
    while (*str != '\0' && *str != (char)c) str++;
    if (*str == (char)c) return str;
    return NULL;
}

char *MAU_strrchr(char *str, int c)
{
    static char *start;
    static char *string;

    string = str;
    start = str;
    while (*string++ != '\0');
    while (--string != start && *string != (char)c);
    if (*string == (char)c) return string;
    return NULL;
}

char *MAU_FindPostBlank(char *str)
{
    while (*str != '\0' && *str != ' ') str++;
    if (*str == '\0') return NULL;
    while (*str == ' ') str++;
    if (*str == '\0') return NULL;
    return str;
}

int MAU_strcmp(char *str1, char *str2)
{
    static int ret;

    ret = 0;
    for (;;) {
        ret = *str1 - *str2;
        if (ret) break;
        if (*str2 == '\0') break;
        str1++;
        str2++;
    }
    return ret;
}

int MAU_strncmp(char *str1, char *str2, int size)
{
    if (size == 0) return 0;
    while (--size != 0) {
        if (*str1 == '\0') break;
        if (*str1 != *str2) break;
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

int MAU_strnicmp(char *str1, char *str2, int size)
{
    static int f;
    static int l;

    if (size != 0) {
        do {
            f = *str1++;
            if ('A' <= f) if (f <= 'Z') f += 'a' - 'A';
            l = *str2++;
            if ('A' <= l) if (l <= 'Z') l += 'a' - 'A';
        } while (--size != 0 && f != '\0' && f == l);
        return f - l;
    }
    return 0;
}

void MAU_memcpy(void *dest, void *src, int size)
{
    char *d = dest;
    char *s = src;
    while (size--) *d++ = *s++;
}

void MAU_memset(void *dest, u8 c, int size)
{
    char *d = dest;
    while (size--) *d++ = c;
}

static void xtoa(unsigned num, char *dest, unsigned base, int negative)
{
    static char *p;
    static char *firstdig;
    static char temp;
    static unsigned digval;

    p = dest;
    if (negative) {
        *p++ = '-';
        num = -num;
    }

    // Write number into dest backwards
    firstdig = p;
    do {
        digval = num % base;
        num = num / base;
        if (digval >= 10) {
            *p++ = digval + 'a' - 10;
        } else {
            *p++ = digval + '0';
        }
    } while (num != 0);

    // Reverse the string in dest
    *p-- = '\0';
    do {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        p--;
        firstdig++;
    } while (firstdig < p);
}

char *MAU_itoa(int num, char *dest, int base)
{
    if (base == 10 && num < 0) {
        xtoa(num, dest, 10, TRUE);
    } else {
        xtoa(num, dest, base, FALSE);
    }
    return dest;
}

int MAU_atoi(char *str)
{
    static char c;
    static char sign;
    static int total;

    c = *str++;
    sign = c;
    if (sign == '-' || sign == '+') {
        c = *str++;
    }

    total = 0;
    while (c >= '0' && c <= '9') {
        total = total * 10 + (c - '0');
        c = *str++;
    }

    if (sign == '-') return -total;
    return total;
}

static const char telCharTable[] = "0123456789#*";

void MAU_DecodeEEPROMTelNo(u8 *num, char *str)
{
    static int i;
    static u8 hi;
    static u8 lo;

    for (i = 0; i < 8; i++) {
        hi = (*num >> 4) & 0xf;
        lo = (*num >> 0) & 0xf;
        num++;
        if (hi == 0xf) {
            *str = '\0';
            break;
        }
        *str++ = telCharTable[hi];
        if (lo == 0xf) {
            *str = '\0';
            break;
        }
        *str++ = telCharTable[lo];
    }

    if (i == 8) *str = '\0';
    return;
}

int MAU_IsValidTelNoStr(char *str)
{
    static unsigned i;

    for (; *str != '\0'; str++) {
        for (i = 0; i < sizeof(telCharTable); i++) {
            if (*str == telCharTable[i]) break;
        }
        if (i == sizeof(telCharTable)) return 0;
    }
    return 1;
}

int MAU_CheckCRLF(char *str, u16 size)
{
    if (size < 3) return FALSE;
    if (str[size - 2] != '\r') return FALSE;
    if (str[size - 1] != '\n') return FALSE;
    return TRUE;
}

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_Add
MAU_Socket_Add:
    push	{r4, r5, r6, lr}
    lsl	r0, r0, #24
    lsr	r4, r0, #24
    mov	r2, #0
    ldr	r3, [pc, #20]
    mov	r5, #1
    mov	r6, r3
    sub	r6, #105
    add	r1, r2, r3
    ldrb	r0, [r1, #0]
    cmp	r0, #0
    bne	MAU_Socket_Add+0x24
    strb	r5, [r1, #0]
    add	r0, r2, r6
    strb	r4, [r0, #0]
    b	MAU_Socket_Add+0x2a
.align 2
    .word gMA+0xcc

    add	r2, #1
    cmp	r2, #1
    ble	MAU_Socket_Add+0x10
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.size MAU_Socket_Add, .-MAU_Socket_Add
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_Delete
MAU_Socket_Delete:
    lsl	r0, r0, #24
    lsr	r3, r0, #24
    mov	r2, #0
    ldr	r1, [pc, #20]
    mov	r0, r1
    add	r0, #99
    ldrb	r0, [r0, #0]
    cmp	r0, r3
    bne	MAU_Socket_Delete+0x20
    mov	r0, r1
    add	r0, #204
    strb	r2, [r0, #0]
    b	MAU_Socket_Delete+0x3c
.align 2
    .word gMA

    add	r2, #1
    cmp	r2, #1
    bgt	MAU_Socket_Delete+0x3c
    mov	r0, r1
    add	r0, #99
    add	r0, r2, r0
    ldrb	r0, [r0, #0]
    cmp	r0, r3
    bne	MAU_Socket_Delete+0x20
    mov	r0, r1
    add	r0, #204
    add	r0, r2, r0
    mov	r1, #0
    strb	r1, [r0, #0]
    bx	lr
.size MAU_Socket_Delete, .-MAU_Socket_Delete
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_Search
MAU_Socket_Search:
    push	{r4, lr}
    lsl	r0, r0, #24
    lsr	r3, r0, #24
    mov	r4, #0
    mov	r1, #0
    ldr	r2, [pc, #8]
    mov	r0, r2
    add	r0, #99
    b	MAU_Socket_Search+0x24
.align 2
    .word gMA

    add	r1, #1
    cmp	r1, #1
    bgt	MAU_Socket_Search+0x2c
    mov	r0, r2
    add	r0, #99
    add	r0, r1, r0
    ldrb	r0, [r0, #0]
    cmp	r0, r3
    bne	MAU_Socket_Search+0x18
    mov	r4, #1
    mov	r0, r4
    pop	{r4}
    pop	{r1}
    bx	r1
.size MAU_Socket_Search, .-MAU_Socket_Search
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_GetNum
MAU_Socket_GetNum:
    mov	r2, #0
    mov	r1, #0
    ldr	r3, [pc, #20]
    add	r0, r1, r3
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	MAU_Socket_GetNum+0x10
    add	r2, #1
    add	r1, #1
    cmp	r1, #1
    ble	MAU_Socket_GetNum+0x6
    mov	r0, r2
    bx	lr
.align 2
    .word gMA+0xcc
.size MAU_Socket_GetNum, .-MAU_Socket_GetNum
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_FreeCheck
MAU_Socket_FreeCheck:
    mov	r3, #0
    mov	r1, #0
    ldr	r2, [pc, #4]
    mov	r0, r2
    add	r0, #204
    b	MAU_Socket_FreeCheck+0x1c
.align 2
    .word gMA

    add	r1, #1
    cmp	r1, #1
    bgt	MAU_Socket_FreeCheck+0x24
    mov	r0, r2
    add	r0, #204
    add	r0, r1, r0
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    bne	MAU_Socket_FreeCheck+0x10
    mov	r3, #1
    mov	r0, r3
    bx	lr
.size MAU_Socket_FreeCheck, .-MAU_Socket_FreeCheck
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_IpAddrCheck
MAU_Socket_IpAddrCheck:
    mov	r3, r0
    ldr	r2, [pc, #52]
    mov	r1, r2
    add	r1, #106
    ldrb	r0, [r3, #0]
    ldrb	r1, [r1, #0]
    cmp	r0, r1
    bne	MAU_Socket_IpAddrCheck+0x3c
    mov	r1, r2
    add	r1, #107
    ldrb	r0, [r3, #1]
    ldrb	r1, [r1, #0]
    cmp	r0, r1
    bne	MAU_Socket_IpAddrCheck+0x3c
    mov	r1, r2
    add	r1, #108
    ldrb	r0, [r3, #2]
    ldrb	r1, [r1, #0]
    cmp	r0, r1
    bne	MAU_Socket_IpAddrCheck+0x3c
    mov	r1, r2
    add	r1, #109
    ldrb	r0, [r3, #3]
    ldrb	r1, [r1, #0]
    cmp	r0, r1
    bne	MAU_Socket_IpAddrCheck+0x3c
    mov	r0, #1
    b	MAU_Socket_IpAddrCheck+0x3e
.align 2
    .word gMA

    mov	r0, #0
    bx	lr
.size MAU_Socket_IpAddrCheck, .-MAU_Socket_IpAddrCheck
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MAU_Socket_Clear
MAU_Socket_Clear:
    ldr	r2, [pc, #40]
    mov	r3, #0
    mov	r1, #1
    mov	r0, r2
    add	r0, #205
    strb	r3, [r0, #0]
    sub	r0, #1
    sub	r1, #1
    cmp	r1, #0
    bge	MAU_Socket_Clear+0xa
    mov	r1, r2
    add	r1, #106
    mov	r0, #0
    strb	r0, [r1, #0]
    add	r1, #1
    strb	r0, [r1, #0]
    add	r1, #1
    strb	r0, [r1, #0]
    add	r1, #1
    strb	r0, [r1, #0]
    bx	lr
.align 2
    .word gMA
.size MAU_Socket_Clear, .-MAU_Socket_Clear
");
#endif
