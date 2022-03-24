#ifndef _MA_SUB
#define _MA_SUB

#include <AgbTypes.h>

int MAU_strlen(char *str);
void MAU_strcpy(char *dest, char *src);
int MAU_strcpy_size(char *dest, char *src, int size);
char *MAU_strncpy_CRLF_LF(char *dest, char *src, int size);
char *MAU_SearchCRLF(char *str, int size);
void MAU_strcat(char *dest, char *src);
char *MAU_strchr(char *str, int c);
char *MAU_strrchr(char *str, int c);
char *MAU_FindPostBlank(char *str);
int MAU_strcmp(char *str1, char *str2);
int MAU_strncmp(char *str1, char *str2, int size);
int MAU_strnicmp(char *str1, char *str2, int size);
void MAU_memcpy(void *dest, void *src, int size);
void MAU_memset(void *dest, u8 c, int size);
char *MAU_itoa(int num, char *dest, int base);
int MAU_atoi(char *str);
void MAU_DecodeEEPROMTelNo(u8 *num, char *str);
int MAU_IsValidTelNoStr(char *str);
int MAU_CheckCRLF(char *str, u16 size);
void MAU_Socket_Add(u8 sock);
void MAU_Socket_Delete(u8 sock);
int MAU_Socket_Search(u8 sock);
int MAU_Socket_GetNum(void);
int MAU_Socket_FreeCheck(void);
int MAU_Socket_IpAddrCheck(u8 *addr);
void MAU_Socket_Clear(void);

#endif // _MA_SUB
