#include "ma_sub.h"

#include <stddef.h>
#include <AgbDefine.h>
#include "ma_var.h"

static void xtoa(unsigned num, char *dest, unsigned base, int negative);

int MAU_strlen(const char *str)
{
    static const char *eos;

    eos = str;
    while (*eos++ != '\0') {}
    return (u16)(eos - str - 1);
}

void MAU_strcpy(char *dest, const char *src)
{
    static const char *pSrc;
    pSrc = src;

    *dest++ = *pSrc;
    while (*pSrc != '\0') {
        pSrc++;
        *dest++ = *pSrc;
    }
}

int MAU_strcpy_size(char *dest, const char *src, int size)
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

const char *MAU_strncpy_CRLF_LF(char *dest, const char *src, int size)
{
    if (size == 0) return NULL;

    while (size != 0) {
        if (src[0] == '\r' && src[1] == '\n') {
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

void MAU_strcat(char *dest, const char *src)
{
    while (*dest != '\0') dest++;
    for (;;) {
        *dest++ = *src;
        if (*src == '\0') break;
        src++;
    }
}

const char *MAU_strchr(const char *str, int c)
{
    while (*str != '\0' && *str != (char)c) str++;
    if (*str == (char)c) return str;
    return NULL;
}

const char *MAU_strrchr(const char *str, int c)
{
    static const char *start;
    static const char *string;

    string = str;
    start = str;
    while (*string++ != '\0') {}
    while (--string != start && *string != (char)c) {}
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

int MAU_strcmp(const char *str1, const char *str2)
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

int MAU_strncmp(const char *str1, const char *str2, int size)
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

int MAU_strnicmp(const char *str1, const char *str2, int size)
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

void MAU_memcpy(void *dest, const void *src, int size)
{
    const char *s = src;
    char *d = dest;

    while (size--) *d++ = *s++;
}

void MAU_memset(void *dest, u8 c, int size)
{
    u8 *d = dest;

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

int MAU_atoi(const char *str)
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

void MAU_DecodeEEPROMTelNo(const u8 *num, char *str)
{
    static int i;
    static u8 hi;
    static u8 lo;

    for (i = 0; i < EEPROM_TELNO_SIZE; i++) {
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

    if (i == EEPROM_TELNO_SIZE) *str = '\0';
}

int MAU_IsValidTelNoStr(const char *str)
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

int MAU_CheckCRLF(const char *str, u16 size)
{
    if (size < 3) return FALSE;
    if (str[size - 2] != '\r') return FALSE;
    if (str[size - 1] != '\n') return FALSE;
    return TRUE;
}

void MAU_Socket_Add(u8 sock)
{
    int i;
    for (i = 0; i < NUM_SOCKETS; i++) {
        if (!gMA.sockets_used[i]) {
            gMA.sockets_used[i] = TRUE;
            gMA.sockets[i] = sock;
            break;
        }
    }
}

void MAU_Socket_Delete(u8 sock)
{
    int i;

    for (i = 0; i < NUM_SOCKETS; i++) {
        if (gMA.sockets[i] == sock) {
            gMA.sockets_used[i] = FALSE;
            break;
        }
    }
}

int MAU_Socket_Search(u8 sock)
{
    int i;
    int ret;

    ret = FALSE;
    for (i = 0; i < NUM_SOCKETS; i++) {
        if (gMA.sockets[i] == sock) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

int MAU_Socket_GetNum(void)
{
    int i;
    int c;

    c = 0;
    for (i = 0; i < NUM_SOCKETS; i++) {
        if (gMA.sockets_used[i] == TRUE) c++;
    }
    return c;
}

int MAU_Socket_FreeCheck(void)
{
    int i;
    int ret;

    ret = FALSE;
    for (i = 0; i < NUM_SOCKETS; i++) {
        if (gMA.sockets_used[i] == FALSE) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

int MAU_Socket_IpAddrCheck(const u8 *addr)
{
    if (addr[0] != gMA.ipaddr[0]) return FALSE;
    if (addr[1] != gMA.ipaddr[1]) return FALSE;
    if (addr[2] != gMA.ipaddr[2]) return FALSE;
    if (addr[3] != gMA.ipaddr[3]) return FALSE;
    return TRUE;
}

void MAU_Socket_Clear(void)
{
    int i;

    for (i = 0; i < NUM_SOCKETS; i++) {
        gMA.sockets_used[i] = FALSE;
    }
    gMA.ipaddr[0] = 0;
    gMA.ipaddr[1] = 0;
    gMA.ipaddr[2] = 0;
    gMA.ipaddr[3] = 0;
}
