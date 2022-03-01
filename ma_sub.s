.thumb
.gcc2_compiled.:

.lcomm eos.3, 0x4

.align 2
.thumb_func
.global MAU_strlen
MAU_strlen:
    push	{r4, lr}
    mov	r3, r0
    ldr	r0, [pc, #40]
    ldrb	r2, [r3, #0]
    add	r1, r3, #1
    str	r1, [r0, #0]
    mov	r4, r0
    cmp	r2, #0
    beq	MAU_strlen+0x20
    mov	r2, r4
    ldr	r0, [r2, #0]
    ldrb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r1, #0
    bne	MAU_strlen+0x14
    ldr	r0, [r4, #0]
    sub	r0, r0, r3
    sub	r0, #1
    lsl	r0, r0, #16
    lsr	r0, r0, #16
    pop	{r4}
    pop	{r1}
    bx	r1
.align 2
    .word eos.3
.size MAU_strlen, .-MAU_strlen

.lcomm pSrc.7, 0x4

.align 2
.thumb_func
.global MAU_strcpy
MAU_strcpy:
    mov	r2, r0
    ldr	r3, [pc, #36]
    str	r1, [r3, #0]
    ldrb	r0, [r1, #0]
    strb	r0, [r2, #0]
    add	r2, #1
    lsl	r0, r0, #24
    cmp	r0, #0
    beq	MAU_strcpy+0x24
    ldr	r1, [r3, #0]
    add	r0, r1, #1
    str	r0, [r3, #0]
    ldrb	r0, [r1, #1]
    strb	r0, [r2, #0]
    add	r2, #1
    ldrb	r0, [r1, #1]
    cmp	r0, #0
    bne	MAU_strcpy+0x12
    bx	lr
.align 2
    .word pSrc.7
.size MAU_strcpy, .-MAU_strcpy

.lcomm ret.11, 0x4

.align 2
.thumb_func
.global MAU_strcpy_size
MAU_strcpy_size:
    push	{r4, r5, lr}
    mov	r4, r0
    mov	r3, r1
    ldr	r1, [pc, #8]
    mov	r0, #0
    str	r0, [r1, #0]
    mov	r5, r1
    b	MAU_strcpy_size+0x20
.align 2
    .word ret.11

    add	r4, #1
    add	r3, #1
    sub	r2, #1
    ldr	r0, [r1, #0]
    add	r0, #1
    str	r0, [r1, #0]
    cmp	r2, #0
    beq	MAU_strcpy_size+0x38
    ldrb	r0, [r3, #0]
    strb	r0, [r4, #0]
    lsl	r0, r0, #24
    cmp	r0, #0
    bne	MAU_strcpy_size+0x14
    cmp	r2, #0
    beq	MAU_strcpy_size+0x38
    ldr	r0, [r5, #0]
    add	r0, #1
    str	r0, [r5, #0]
    ldr	r0, [r5, #0]
    pop	{r4, r5}
    pop	{r1}
    bx	r1
.size MAU_strcpy_size, .-MAU_strcpy_size

.align 2
.thumb_func
.global MAU_strncpy_CRLF_LF
MAU_strncpy_CRLF_LF:
    push	{r4, r5, lr}
    mov	r4, r0
    cmp	r2, #0
    bne	MAU_strncpy_CRLF_LF+0x1a
    b	MAU_strncpy_CRLF_LF+0x3a
    mov	r0, #0
    strb	r0, [r4, #0]
    add	r0, r1, #2
    b	MAU_strncpy_CRLF_LF+0x3c
    mov	r0, #0
    strb	r0, [r4, #0]
    add	r0, r1, #1
    b	MAU_strncpy_CRLF_LF+0x3c
    ldrb	r5, [r1, #0]
    mov	r3, r5
    cmp	r3, #13
    bne	MAU_strncpy_CRLF_LF+0x28
    ldrb	r0, [r1, #1]
    cmp	r0, #10
    beq	MAU_strncpy_CRLF_LF+0xa
    cmp	r3, #10
    beq	MAU_strncpy_CRLF_LF+0x12
    strb	r5, [r4, #0]
    add	r1, #1
    add	r4, #1
    sub	r2, #1
    cmp	r2, #0
    bne	MAU_strncpy_CRLF_LF+0x1a
    strb	r2, [r4, #0]
    mov	r0, #0
    pop	{r4, r5}
    pop	{r1}
    bx	r1
.size MAU_strncpy_CRLF_LF, .-MAU_strncpy_CRLF_LF

.align 2
.thumb_func
.global MAU_SearchCRLF
MAU_SearchCRLF:
    mov	r2, r0
    cmp	r1, #1
    bgt	MAU_SearchCRLF+0x10
    b	MAU_SearchCRLF+0x24
    mov	r0, #0
    strb	r0, [r2, #0]
    add	r0, r2, #2
    b	MAU_SearchCRLF+0x26
    ldrb	r0, [r2, #0]
    cmp	r0, #13
    bne	MAU_SearchCRLF+0x1c
    ldrb	r0, [r2, #1]
    cmp	r0, #10
    beq	MAU_SearchCRLF+0x8
    add	r2, #1
    sub	r1, #1
    cmp	r1, #1
    bne	MAU_SearchCRLF+0x10
    mov	r0, #0
    bx	lr
.size MAU_SearchCRLF, .-MAU_SearchCRLF

.align 2
.thumb_func
.global MAU_strcat
MAU_strcat:
    mov	r2, r0
    b	MAU_strcat+0x6
    add	r2, #1
    ldrb	r0, [r2, #0]
    cmp	r0, #0
    bne	MAU_strcat+0x4
    b	MAU_strcat+0x10
    add	r1, #1
    ldrb	r0, [r1, #0]
    strb	r0, [r2, #0]
    add	r2, #1
    lsl	r0, r0, #24
    cmp	r0, #0
    bne	MAU_strcat+0xe
    bx	lr
.size MAU_strcat, .-MAU_strcat

.align 2
.thumb_func
.global MAU_strchr
MAU_strchr:
    mov	r2, r0
    mov	r3, r1
    ldrb	r1, [r2, #0]
    cmp	r1, #0
    beq	MAU_strchr+0x1e
    lsl	r0, r3, #24
    lsr	r0, r0, #24
    cmp	r1, r0
    beq	MAU_strchr+0x2c
    add	r2, #1
    ldrb	r1, [r2, #0]
    cmp	r1, #0
    beq	MAU_strchr+0x1e
    cmp	r1, r0
    bne	MAU_strchr+0x12
    ldrb	r1, [r2, #0]
    lsl	r0, r3, #24
    lsr	r0, r0, #24
    cmp	r1, r0
    beq	MAU_strchr+0x2c
    mov	r0, #0
    b	MAU_strchr+0x2e
    mov	r0, r2
    bx	lr
.size MAU_strchr, .-MAU_strchr

.lcomm start.27, 0x4
.lcomm string.28, 0x4

.align 2
.thumb_func
.global MAU_strrchr
MAU_strrchr:
    push	{r4, r5, lr}
    mov	r3, r1
    ldr	r2, [pc, #4]
    ldr	r1, [pc, #8]
    str	r0, [r1, #0]
    b	MAU_strrchr+0x16
.align 2
    .word string.28
    .word start.27

    ldr	r0, [r2, #0]
    ldrb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r1, #0
    bne	MAU_strrchr+0x14
    lsl	r5, r3, #24
    ldr	r1, [pc, #36]
    ldr	r0, [pc, #36]
    ldr	r4, [r0, #0]
    lsr	r3, r5, #24
    ldr	r0, [r1, #0]
    sub	r2, r0, #1
    str	r2, [r1, #0]
    cmp	r2, r4
    beq	MAU_strrchr+0x3a
    ldrb	r0, [r2, #0]
    cmp	r0, r3
    bne	MAU_strrchr+0x2a
    ldrb	r1, [r2, #0]
    lsr	r0, r5, #24
    cmp	r1, r0
    beq	MAU_strrchr+0x50
    mov	r0, #0
    b	MAU_strrchr+0x52
.align 2
    .word string.28
    .word start.27

    mov	r0, r2
    pop	{r4, r5}
    pop	{r1}
    bx	r1
.size MAU_strrchr, .-MAU_strrchr

.align 2
.thumb_func
.global MAU_FindPostBlank
MAU_FindPostBlank:
    mov	r1, r0
    b	MAU_FindPostBlank+0x6
    add	r1, #1
    ldrb	r0, [r1, #0]
    cmp	r0, #0
    beq	MAU_FindPostBlank+0x2a
    cmp	r0, #32
    bne	MAU_FindPostBlank+0x4
    ldrb	r0, [r1, #0]
    cmp	r0, #0
    beq	MAU_FindPostBlank+0x2a
    b	MAU_FindPostBlank+0x1c
    add	r1, #1
    ldrb	r0, [r1, #0]
    cmp	r0, #32
    beq	MAU_FindPostBlank+0x18
    ldrb	r0, [r1, #0]
    cmp	r0, #0
    beq	MAU_FindPostBlank+0x2a
    mov	r0, r1
    b	MAU_FindPostBlank+0x2c
    mov	r0, #0
    bx	lr
.size MAU_FindPostBlank, .-MAU_FindPostBlank

.lcomm ret.35, 0x4

.align 2
.thumb_func
.global MAU_strcmp
MAU_strcmp:
    push	{r4, r5, lr}
    mov	r4, r0
    mov	r3, r1
    ldr	r2, [pc, #20]
    mov	r0, #0
    str	r0, [r2, #0]
    ldrb	r0, [r4, #0]
    ldrb	r1, [r3, #0]
    sub	r0, r0, r1
    str	r0, [r2, #0]
    mov	r5, r2
    cmp	r0, #0
    bne	MAU_strcmp+0x36
    b	MAU_strcmp+0x30
.align 2
    .word ret.35

    add	r4, #1
    add	r3, #1
    ldrb	r1, [r4, #0]
    ldrb	r0, [r3, #0]
    sub	r1, r1, r0
    str	r1, [r2, #0]
    cmp	r1, #0
    bne	MAU_strcmp+0x36
    ldrb	r0, [r3, #0]
    cmp	r0, #0
    bne	MAU_strcmp+0x20
    ldr	r0, [r5, #0]
    pop	{r4, r5}
    pop	{r1}
    bx	r1
.size MAU_strcmp, .-MAU_strcmp

.align 2
.thumb_func
.global MAU_strncmp
MAU_strncmp:
    push	{r4, lr}
    cmp	r2, #0
    bne	MAU_strncmp+0xe
    mov	r0, #0
    b	MAU_strncmp+0x26
    add	r0, #1
    add	r1, #1
    sub	r2, #1
    cmp	r2, #0
    beq	MAU_strncmp+0x20
    ldrb	r3, [r0, #0]
    cmp	r3, #0
    beq	MAU_strncmp+0x20
    ldrb	r4, [r1, #0]
    cmp	r3, r4
    beq	MAU_strncmp+0xa
    ldrb	r0, [r0, #0]
    ldrb	r1, [r1, #0]
    sub	r0, r0, r1
    pop	{r4}
    pop	{r1}
    bx	r1
.size MAU_strncmp, .-MAU_strncmp

.lcomm f.42, 0x4
.lcomm l.43, 0x4

.align 2
.thumb_func
.global MAU_strnicmp
MAU_strnicmp:
    push	{r4, r5, r6, lr}
    mov	r6, r0
    mov	r5, r1
    cmp	r2, #0
    beq	MAU_strnicmp+0x56
    ldr	r4, [pc, #4]
    ldr	r1, [pc, #4]
    b	MAU_strnicmp+0x24
.align 2
    .word f.42
    .word l.43

    ldr	r3, [r4, #0]
    cmp	r3, #0
    beq	MAU_strnicmp+0x4e
    ldr	r0, [r1, #0]
    cmp	r3, r0
    bne	MAU_strnicmp+0x4e
    ldrb	r0, [r6, #0]
    str	r0, [r4, #0]
    add	r6, #1
    cmp	r0, #64
    ble	MAU_strnicmp+0x36
    cmp	r0, #90
    bgt	MAU_strnicmp+0x36
    add	r0, #32
    str	r0, [r4, #0]
    ldrb	r0, [r5, #0]
    str	r0, [r1, #0]
    add	r5, #1
    cmp	r0, #64
    ble	MAU_strnicmp+0x48
    cmp	r0, #90
    bgt	MAU_strnicmp+0x48
    add	r0, #32
    str	r0, [r1, #0]
    sub	r2, #1
    cmp	r2, #0
    bne	MAU_strnicmp+0x18
    ldr	r0, [r4, #0]
    ldr	r1, [r1, #0]
    sub	r0, r0, r1
    b	MAU_strnicmp+0x58
    mov	r0, #0
    pop	{r4, r5, r6}
    pop	{r1}
    bx	r1
.size MAU_strnicmp, .-MAU_strnicmp

.align 2
.thumb_func
.global MAU_memcpy
MAU_memcpy:
    push	{r4, lr}
    mov	r3, r0
    sub	r2, #1
    mov	r0, #1
    neg	r0, r0
    cmp	r2, r0
    beq	MAU_memcpy+0x1e
    mov	r4, r0
    ldrb	r0, [r1, #0]
    strb	r0, [r3, #0]
    add	r1, #1
    add	r3, #1
    sub	r2, #1
    cmp	r2, r4
    bne	MAU_memcpy+0x10
    pop	{r4}
    pop	{r0}
    bx	r0
.size MAU_memcpy, .-MAU_memcpy

.align 2
.thumb_func
.global MAU_memset
MAU_memset:
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    sub	r2, #1
    mov	r3, #1
    neg	r3, r3
    cmp	r2, r3
    beq	MAU_memset+0x18
    strb	r1, [r0, #0]
    add	r0, #1
    sub	r2, #1
    cmp	r2, r3
    bne	MAU_memset+0xe
    bx	lr
.size MAU_memset, .-MAU_memset

.lcomm p.53, 0x4
.lcomm firstdig.54, 0x4
.lcomm temp.55, 0x1
.lcomm digval.56, 0x4

.align 2
.thumb_func
xtoa:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r5, r0
    mov	r7, r2
    ldr	r0, [pc, #68]
    str	r1, [r0, #0]
    mov	r8, r0
    cmp	r3, #0
    beq	xtoa+0x22
    mov	r0, #45
    strb	r0, [r1, #0]
    add	r0, r1, #1
    mov	r1, r8
    str	r0, [r1, #0]
    neg	r5, r5
    ldr	r1, [pc, #52]
    mov	r2, r8
    ldr	r0, [r2, #0]
    str	r0, [r1, #0]
    ldr	r0, [pc, #48]
    mov	r9, r0
    mov	r6, r8
    mov	r0, r5
    mov	r1, r7
    bl	__umodsi3
    mov	r4, r0
    mov	r1, r9
    str	r4, [r1, #0]
    mov	r0, r5
    mov	r1, r7
    bl	__udivsi3
    mov	r5, r0
    cmp	r4, #9
    bls	xtoa+0x60
    ldr	r1, [r6, #0]
    mov	r0, r4
    add	r0, #87
    b	xtoa+0x66
.align 2
    .word p.53
    .word firstdig.54
    .word digval.56

    ldr	r1, [r6, #0]
    mov	r0, r4
    add	r0, #48
    strb	r0, [r1, #0]
    add	r1, #1
    str	r1, [r6, #0]
    cmp	r5, #0
    bne	xtoa+0x30
    mov	r2, r8
    ldr	r0, [r2, #0]
    strb	r5, [r0, #0]
    sub	r0, #1
    str	r0, [r2, #0]
    ldr	r4, [pc, #52]
    mov	r3, r8
    ldr	r2, [pc, #52]
    ldr	r1, [r3, #0]
    ldrb	r0, [r1, #0]
    strb	r0, [r4, #0]
    ldr	r0, [r2, #0]
    ldrb	r0, [r0, #0]
    strb	r0, [r1, #0]
    ldr	r1, [r2, #0]
    ldrb	r0, [r4, #0]
    strb	r0, [r1, #0]
    ldr	r1, [r3, #0]
    sub	r1, #1
    str	r1, [r3, #0]
    ldr	r0, [r2, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r0, r1
    bcc	xtoa+0x80
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word temp.55
    .word firstdig.54
.size xtoa, .-xtoa

.align 2
.thumb_func
.global MAU_itoa
MAU_itoa:
    push	{r4, lr}
    mov	r4, r1
    cmp	r2, #10
    bne	MAU_itoa+0x16
    cmp	r0, #0
    bge	MAU_itoa+0x16
    mov	r2, #10
    mov	r3, #1
    bl	xtoa
    b	MAU_itoa+0x1e
    mov	r1, r4
    mov	r3, #0
    bl	xtoa
    mov	r0, r4
    pop	{r4}
    pop	{r1}
    bx	r1
.size MAU_itoa, .-MAU_itoa

.lcomm c.63, 0x1
.lcomm sign.64, 0x1
.lcomm total.65, 0x4

.align 2
.thumb_func
.global MAU_atoi
MAU_atoi:
    push	{r4, r5, r6, r7, lr}
    mov	r4, r0
    ldr	r0, [pc, #96]
    ldrb	r1, [r4, #0]
    strb	r1, [r0, #0]
    add	r4, #1
    ldr	r2, [pc, #92]
    strb	r1, [r2, #0]
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    mov	r3, r0
    mov	r6, r2
    cmp	r1, #45
    beq	MAU_atoi+0x20
    cmp	r1, #43
    bne	MAU_atoi+0x26
    ldrb	r0, [r4, #0]
    strb	r0, [r3, #0]
    add	r4, #1
    ldr	r1, [pc, #72]
    mov	r0, #0
    str	r0, [r1, #0]
    ldrb	r0, [r3, #0]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    mov	r2, r1
    cmp	r0, #9
    bhi	MAU_atoi+0x5e
    mov	r5, r2
    mov	r1, r3
    mov	r3, #0
    lsl	r0, r3, #2
    add	r0, r0, r3
    lsl	r0, r0, #1
    sub	r0, #48
    ldrb	r7, [r1, #0]
    add	r3, r0, r7
    ldrb	r0, [r4, #0]
    strb	r0, [r1, #0]
    add	r4, #1
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bls	MAU_atoi+0x40
    str	r3, [r5, #0]
    ldrb	r0, [r6, #0]
    cmp	r0, #45
    beq	MAU_atoi+0x74
    ldr	r0, [r2, #0]
    b	MAU_atoi+0x78
.align 2
    .word c.63
    .word sign.64
    .word total.65

    ldr	r0, [r2, #0]
    neg	r0, r0
    pop	{r4, r5, r6, r7}
    pop	{r1}
    bx	r1
.size MAU_atoi, .-MAU_atoi

.section .rodata
.align 2
.type telCharTable, object
telCharTable:
    .word 0x33323130
    .word 0x37363534
    .word 0x2a233938
    .byte 0
.size telCharTable, .-telCharTable
.section .text

.lcomm i.69, 0x4
.lcomm hi.70, 0x1
.lcomm lo.71, 0x1
.lcomm i.75, 0x4

.align 2
.thumb_func
.global MAU_DecodeEEPROMTelNo
MAU_DecodeEEPROMTelNo:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    mov	r4, r0
    mov	r3, r1
    ldr	r1, [pc, #72]
    mov	r0, #0
    str	r0, [r1, #0]
    ldr	r0, [pc, #68]
    mov	r9, r0
    ldr	r1, [pc, #68]
    mov	r8, r1
    mov	sl, r9
    mov	r7, r8
    mov	r0, #0
    mov	ip, r0
    ldr	r6, [pc, #48]
    ldr	r5, [pc, #60]
    ldrb	r0, [r4, #0]
    lsr	r2, r0, #4
    mov	r1, sl
    strb	r2, [r1, #0]
    mov	r1, #15
    and	r1, r0
    strb	r1, [r7, #0]
    add	r4, #1
    cmp	r2, #15
    beq	MAU_DecodeEEPROMTelNo+0x50
    mov	r1, r9
    ldrb	r0, [r1, #0]
    add	r0, r0, r5
    ldrb	r0, [r0, #0]
    strb	r0, [r3, #0]
    add	r3, #1
    mov	r1, r8
    ldrb	r0, [r1, #0]
    cmp	r0, #15
    bne	MAU_DecodeEEPROMTelNo+0x68
    mov	r0, ip
    strb	r0, [r3, #0]
    b	MAU_DecodeEEPROMTelNo+0x7c
.align 2
    .word i.69
    .word hi.70
    .word lo.71
    .word telCharTable

    ldrb	r0, [r7, #0]
    add	r0, r0, r5
    ldrb	r0, [r0, #0]
    strb	r0, [r3, #0]
    add	r3, #1
    ldr	r0, [r6, #0]
    add	r0, #1
    str	r0, [r6, #0]
    cmp	r0, #7
    ble	MAU_DecodeEEPROMTelNo+0x28
    ldr	r1, [pc, #24]
    ldr	r0, [r1, #0]
    cmp	r0, #8
    bne	MAU_DecodeEEPROMTelNo+0x88
    mov	r0, #0
    strb	r0, [r3, #0]
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word i.69
.size MAU_DecodeEEPROMTelNo, .-MAU_DecodeEEPROMTelNo

.align 2
.thumb_func
.global MAU_IsValidTelNoStr
MAU_IsValidTelNoStr:
    push	{r4, r5, r6, lr}
    mov	r2, r0
    ldrb	r1, [r2, #0]
    cmp	r1, #0
    beq	MAU_IsValidTelNoStr+0x4c
    ldr	r5, [pc, #48]
    ldr	r0, [pc, #48]
    ldrb	r6, [r0, #0]
    mov	r0, #0
    str	r0, [r5, #0]
    cmp	r1, r6
    beq	MAU_IsValidTelNoStr+0x30
    ldr	r3, [pc, #32]
    ldr	r4, [pc, #36]
    ldr	r0, [r3, #0]
    add	r0, #1
    str	r0, [r3, #0]
    cmp	r0, #12
    bhi	MAU_IsValidTelNoStr+0x30
    add	r1, r0, r4
    ldrb	r0, [r2, #0]
    ldrb	r1, [r1, #0]
    cmp	r0, r1
    bne	MAU_IsValidTelNoStr+0x1c
    ldr	r0, [r5, #0]
    cmp	r0, #13
    bne	MAU_IsValidTelNoStr+0x44
    mov	r0, #0
    b	MAU_IsValidTelNoStr+0x4e
.align 2
    .word i.75
    .word telCharTable

    add	r2, #1
    ldrb	r1, [r2, #0]
    cmp	r1, #0
    bne	MAU_IsValidTelNoStr+0x10
    mov	r0, #1
    pop	{r4, r5, r6}
    pop	{r1}
    bx	r1
.size MAU_IsValidTelNoStr, .-MAU_IsValidTelNoStr

.align 2
.thumb_func
.global MAU_CheckCRLF
MAU_CheckCRLF:
    mov	r2, r0
    lsl	r1, r1, #16
    lsr	r0, r1, #16
    cmp	r0, #2
    bls	MAU_CheckCRLF+0x20
    add	r1, r0, r2
    sub	r0, r1, #2
    ldrb	r0, [r0, #0]
    cmp	r0, #13
    bne	MAU_CheckCRLF+0x20
    sub	r0, r1, #1
    ldrb	r0, [r0, #0]
    cmp	r0, #10
    bne	MAU_CheckCRLF+0x20
    mov	r0, #1
    b	MAU_CheckCRLF+0x22
    mov	r0, #0
    bx	lr
.size MAU_CheckCRLF, .-MAU_CheckCRLF

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
