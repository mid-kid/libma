.thumb
.gcc2_compiled.:

.section .text

.section .rodata
.type PADDING, object
PADDING:
    .word 0x00000080
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
.size PADDING, .-PADDING
.section .text

.align 2
.thumb_func
.global MD5Init
MD5Init:
    mov	r1, #0
    str	r1, [r0, #20]
    str	r1, [r0, #16]
    ldr	r1, [pc, #16]
    str	r1, [r0, #0]
    ldr	r1, [pc, #16]
    str	r1, [r0, #4]
    ldr	r1, [pc, #16]
    str	r1, [r0, #8]
    ldr	r1, [pc, #16]
    str	r1, [r0, #12]
    bx	lr
.align 2
    .word 0x67452301
    .word 0xefcdab89
    .word 0x98badcfe
    .word 0x10325476
.size MD5Init, .-MD5Init

.lcomm index.6, 0x4
.lcomm partLen.7, 0x4

.align 2
.thumb_func
.global MD5Update
MD5Update:
    push	{r4, r5, r6, r7, lr}
    mov	r5, r0
    mov	r7, r1
    mov	r6, r2
    ldr	r3, [pc, #120]
    ldr	r2, [r5, #16]
    lsr	r0, r2, #3
    mov	r1, #63
    and	r0, r1
    str	r0, [r3, #0]
    lsl	r0, r6, #3
    add	r2, r2, r0
    str	r2, [r5, #16]
    cmp	r2, r0
    bcs	MD5Update+0x24
    ldr	r0, [r5, #20]
    add	r0, #1
    str	r0, [r5, #20]
    lsr	r0, r6, #29
    ldr	r1, [r5, #20]
    add	r1, r1, r0
    str	r1, [r5, #20]
    ldr	r4, [pc, #88]
    ldr	r1, [r3, #0]
    mov	r0, #64
    sub	r2, r0, r1
    str	r2, [r4, #0]
    cmp	r6, r2
    bcc	MD5Update+0x90
    mov	r0, r1
    add	r0, #24
    add	r0, r5, r0
    mov	r1, r7
    bl	MD5_memcpy
    mov	r1, r5
    add	r1, #24
    mov	r0, r5
    bl	MD5Transform
    ldr	r1, [pc, #56]
    ldr	r0, [r4, #0]
    str	r0, [r1, #0]
    add	r0, #63
    cmp	r0, r6
    bcs	MD5Update+0x76
    mov	r4, r1
    ldr	r1, [r4, #0]
    add	r1, r7, r1
    mov	r0, r5
    bl	MD5Transform
    ldr	r1, [r4, #0]
    mov	r0, r1
    add	r0, #64
    str	r0, [r4, #0]
    add	r1, #127
    cmp	r1, r6
    bcc	MD5Update+0x5e
    ldr	r1, [pc, #12]
    mov	r0, #0
    str	r0, [r1, #0]
    mov	r3, r1
    ldr	r1, [pc, #12]
    b	MD5Update+0x96
.align 2
    .word index.6
    .word partLen.7
    .word i

    ldr	r1, [pc, #24]
    mov	r0, #0
    str	r0, [r1, #0]
    ldr	r0, [r3, #0]
    add	r0, #24
    add	r0, r5, r0
    ldr	r2, [r1, #0]
    add	r1, r7, r2
    sub	r2, r6, r2
    bl	MD5_memcpy
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word i
.size MD5Update, .-MD5Update

.lcomm bits.11, 0x8
.lcomm index.12, 0x4
.lcomm padLen.13, 0x4

.align 2
.thumb_func
.global MD5Final
MD5Final:
    push	{r4, r5, lr}
    mov	r5, r0
    mov	r4, r1
    ldr	r0, [pc, #32]
    add	r1, #16
    mov	r2, #8
    bl	Encode
    ldr	r1, [pc, #24]
    ldr	r0, [r4, #16]
    lsr	r2, r0, #3
    mov	r0, #63
    and	r2, r0
    str	r2, [r1, #0]
    ldr	r1, [pc, #16]
    cmp	r2, #55
    bhi	MD5Final+0x34
    mov	r0, #56
    b	MD5Final+0x36
.align 2
    .word bits.11
    .word index.12
    .word padLen.13

    mov	r0, #120
    sub	r0, r0, r2
    str	r0, [r1, #0]
    ldr	r1, [pc, #48]
    ldr	r0, [pc, #48]
    ldr	r2, [r0, #0]
    mov	r0, r4
    bl	MD5Update
    ldr	r1, [pc, #44]
    mov	r0, r4
    mov	r2, #8
    bl	MD5Update
    mov	r0, r5
    mov	r1, r4
    mov	r2, #16
    bl	Encode
    mov	r0, r4
    mov	r1, #0
    mov	r2, #88
    bl	MD5_memset
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align
    .word PADDING
    .word padLen.13
    .word bits.11
.size MD5Final, .-MD5Final

.lcomm a.17, 0x4
.lcomm b.18, 0x4
.lcomm c.19, 0x4
.lcomm d.20, 0x4
.lcomm x.21, 0x40

.align 2
.thumb_func
MD5Transform:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    sub	sp, #16
    mov	r7, r0
    ldr	r0, [r7, #0]
    ldr	r2, [pc, #1000]
    str	r0, [r2, #0]
    ldr	r0, [r7, #4]
    ldr	r3, [pc, #1000]
    str	r0, [r3, #0]
    ldr	r0, [r7, #8]
    ldr	r4, [pc, #996]
    str	r0, [r4, #0]
    ldr	r0, [r7, #12]
    ldr	r5, [pc, #996]
    str	r0, [r5, #0]
    ldr	r6, [pc, #996]
    mov	r9, r6
    mov	r0, r9
    mov	r2, #64
    bl	Decode
    ldr	r0, [pc, #968]
    ldr	r2, [r0, #0]
    ldr	r1, [pc, #984]
    add	r2, r2, r1
    ldr	r4, [pc, #964]
    ldr	r3, [r4, #0]
    ldr	r5, [pc, #964]
    ldr	r4, [r5, #0]
    mov	r1, r3
    and	r1, r4
    ldr	r6, [pc, #960]
    ldr	r5, [r6, #0]
    mov	r0, r5
    bic	r0, r3
    orr	r1, r0
    mov	r0, r9
    ldr	r0, [r0, #0]
    add	r1, r1, r0
    add	r2, r2, r1
    mov	r1, #25
    mov	r8, r1
    ror	r2, r1
    add	r2, r2, r3
    ldr	r6, [pc, #944]
    add	r5, r5, r6
    mov	r1, r2
    and	r1, r3
    mov	r0, r4
    bic	r0, r2
    orr	r1, r0
    mov	r0, r9
    ldr	r0, [r0, #4]
    add	r1, r1, r0
    add	r5, r5, r1
    mov	r6, #20
    ror	r5, r6
    add	r5, r5, r2
    ldr	r1, [pc, #920]
    add	r1, r1, r4
    mov	sl, r1
    mov	r1, r5
    and	r1, r2
    mov	r0, r3
    bic	r0, r5
    orr	r1, r0
    mov	r4, r9
    ldr	r4, [r4, #8]
    add	r1, r1, r4
    mov	r0, sl
    add	r4, r0, r1
    mov	r1, #15
    mov	sl, r1
    ror	r4, r1
    add	r4, r4, r5
    ldr	r0, [pc, #892]
    add	r0, r0, r3
    mov	ip, r0
    mov	r1, r4
    and	r1, r5
    mov	r0, r2
    bic	r0, r4
    orr	r1, r0
    mov	r3, r9
    ldr	r3, [r3, #12]
    add	r1, r1, r3
    mov	r0, ip
    add	r3, r0, r1
    mov	r1, #10
    mov	ip, r1
    ror	r3, r1
    add	r3, r3, r4
    ldr	r0, [pc, #860]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r1, r3
    and	r1, r4
    mov	r0, r5
    bic	r0, r3
    orr	r1, r0
    mov	r2, r9
    ldr	r2, [r2, #16]
    add	r1, r1, r2
    ldr	r0, [sp, #0]
    add	r2, r0, r1
    mov	r1, r8
    ror	r2, r1
    add	r2, r2, r3
    ldr	r0, [pc, #832]
    add	r0, r5, r0
    str	r0, [sp, #4]
    mov	r1, r2
    and	r1, r3
    mov	r0, r4
    bic	r0, r2
    orr	r1, r0
    mov	r5, r9
    ldr	r5, [r5, #20]
    add	r1, r1, r5
    ldr	r0, [sp, #4]
    add	r5, r0, r1
    ror	r5, r6
    add	r5, r5, r2
    ldr	r1, [pc, #808]
    add	r1, r4, r1
    str	r1, [sp, #8]
    mov	r1, r5
    and	r1, r2
    mov	r0, r3
    bic	r0, r5
    orr	r1, r0
    mov	r4, r9
    ldr	r4, [r4, #24]
    add	r1, r1, r4
    ldr	r0, [sp, #8]
    add	r4, r0, r1
    mov	r1, sl
    ror	r4, r1
    add	r4, r4, r5
    ldr	r0, [pc, #780]
    add	r0, r3, r0
    str	r0, [sp, #12]
    mov	r1, r4
    and	r1, r5
    mov	r0, r2
    bic	r0, r4
    orr	r1, r0
    mov	r3, r9
    ldr	r3, [r3, #28]
    add	r1, r1, r3
    ldr	r0, [sp, #12]
    add	r3, r0, r1
    mov	r1, ip
    ror	r3, r1
    add	r3, r3, r4
    ldr	r0, [pc, #752]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r1, r3
    and	r1, r4
    mov	r0, r5
    bic	r0, r3
    orr	r1, r0
    mov	r2, r9
    ldr	r2, [r2, #32]
    add	r1, r1, r2
    ldr	r0, [sp, #0]
    add	r2, r0, r1
    mov	r1, r8
    ror	r2, r1
    add	r2, r2, r3
    ldr	r0, [pc, #724]
    add	r0, r5, r0
    str	r0, [sp, #4]
    mov	r1, r2
    and	r1, r3
    mov	r0, r4
    bic	r0, r2
    orr	r1, r0
    mov	r5, r9
    ldr	r5, [r5, #36]
    add	r1, r1, r5
    ldr	r0, [sp, #4]
    add	r5, r0, r1
    ror	r5, r6
    add	r5, r5, r2
    ldr	r1, [pc, #696]
    add	r1, r4, r1
    str	r1, [sp, #8]
    mov	r1, r5
    and	r1, r2
    mov	r0, r3
    bic	r0, r5
    orr	r1, r0
    mov	r4, r9
    ldr	r4, [r4, #40]
    add	r1, r1, r4
    ldr	r0, [sp, #8]
    add	r4, r0, r1
    mov	r1, sl
    ror	r4, r1
    add	r4, r4, r5
    ldr	r0, [pc, #668]
    add	r0, r3, r0
    str	r0, [sp, #12]
    mov	r1, r4
    and	r1, r5
    mov	r0, r2
    bic	r0, r4
    orr	r1, r0
    mov	r3, r9
    ldr	r3, [r3, #44]
    add	r1, r1, r3
    ldr	r0, [sp, #12]
    add	r3, r0, r1
    mov	r1, ip
    ror	r3, r1
    add	r3, r3, r4
    ldr	r0, [pc, #640]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r1, r3
    and	r1, r4
    mov	r0, r5
    bic	r0, r3
    orr	r1, r0
    mov	r2, r9
    ldr	r2, [r2, #48]
    add	r1, r1, r2
    ldr	r0, [sp, #0]
    add	r2, r0, r1
    mov	r1, r8
    ror	r2, r1
    add	r2, r2, r3
    ldr	r0, [pc, #612]
    add	r0, r0, r5
    mov	r8, r0
    mov	r1, r2
    and	r1, r3
    mov	r0, r4
    bic	r0, r2
    orr	r1, r0
    mov	r5, r9
    ldr	r5, [r5, #52]
    add	r1, r1, r5
    mov	r0, r8
    add	r5, r0, r1
    ror	r5, r6
    add	r5, r5, r2
    ldr	r1, [pc, #588]
    add	r4, r4, r1
    mov	r1, r5
    and	r1, r2
    mvn	r6, r5
    mov	r8, r6
    mov	r0, r8
    and	r0, r3
    orr	r1, r0
    mov	r0, r9
    ldr	r0, [r0, #56]
    add	r1, r1, r0
    add	r4, r4, r1
    mov	r1, sl
    ror	r4, r1
    add	r4, r4, r5
    ldr	r6, [pc, #560]
    add	r6, r6, r3
    mov	sl, r6
    mov	r1, r4
    and	r1, r5
    mvn	r6, r4
    mov	r0, r6
    and	r0, r2
    orr	r1, r0
    mov	r0, r9
    ldr	r0, [r0, #60]
    add	r1, r1, r0
    mov	r0, sl
    add	r3, r0, r1
    mov	r1, ip
    ror	r3, r1
    add	r3, r3, r4
    ldr	r0, [pc, #528]
    add	r0, r0, r2
    mov	sl, r0
    mov	r0, r3
    and	r0, r5
    mov	r1, r8
    and	r1, r4
    orr	r0, r1
    mov	r2, r9
    ldr	r2, [r2, #4]
    add	r0, r0, r2
    mov	r1, sl
    add	r2, r1, r0
    mov	r0, #27
    mov	sl, r0
    ror	r2, r0
    add	r2, r2, r3
    ldr	r1, [pc, #500]
    add	r5, r5, r1
    mov	r0, r2
    and	r0, r4
    and	r6, r3
    orr	r0, r6
    mov	r6, r9
    ldr	r6, [r6, #24]
    add	r0, r0, r6
    add	r5, r5, r0
    mov	r0, #23
    mov	ip, r0
    ror	r5, r0
    add	r5, r5, r2
    ldr	r1, [pc, #476]
    add	r4, r4, r1
    mov	r0, r5
    and	r0, r3
    mov	r1, r2
    bic	r1, r3
    orr	r0, r1
    mov	r6, r9
    ldr	r6, [r6, #44]
    add	r0, r0, r6
    add	r4, r4, r0
    mov	r0, #18
    mov	r8, r0
    ror	r4, r0
    add	r4, r4, r5
    ldr	r1, [pc, #448]
    add	r3, r3, r1
    mov	r0, r4
    and	r0, r2
    mov	r1, r5
    bic	r1, r2
    orr	r0, r1
    mov	r6, r9
    ldr	r6, [r6, #0]
    add	r0, r0, r6
    add	r3, r3, r0
    mov	r6, #12
    ror	r3, r6
    add	r3, r3, r4
    ldr	r0, [pc, #424]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r0, r3
    and	r0, r5
    mov	r1, r4
    bic	r1, r5
    orr	r0, r1
    mov	r1, r9
    ldr	r1, [r1, #20]
    add	r0, r0, r1
    ldr	r1, [sp, #0]
    add	r2, r1, r0
    mov	r0, sl
    ror	r2, r0
    add	r2, r2, r3
    ldr	r1, [pc, #396]
    add	r1, r5, r1
    str	r1, [sp, #4]
    mov	r0, r2
    and	r0, r4
    mov	r1, r3
    bic	r1, r4
    orr	r0, r1
    mov	r5, r9
    ldr	r5, [r5, #40]
    add	r0, r0, r5
    ldr	r1, [sp, #4]
    add	r5, r1, r0
    mov	r0, ip
    ror	r5, r0
    add	r5, r5, r2
    ldr	r1, [pc, #368]
    add	r1, r4, r1
    str	r1, [sp, #8]
    mov	r0, r5
    and	r0, r3
    mov	r1, r2
    bic	r1, r3
    orr	r0, r1
    mov	r4, r9
    ldr	r4, [r4, #60]
    add	r0, r0, r4
    ldr	r1, [sp, #8]
    add	r4, r1, r0
    mov	r0, r8
    ror	r4, r0
    add	r4, r4, r5
    ldr	r1, [pc, #340]
    add	r1, r3, r1
    str	r1, [sp, #12]
    mov	r0, r4
    and	r0, r2
    mov	r1, r5
    bic	r1, r2
    orr	r0, r1
    mov	r3, r9
    ldr	r3, [r3, #16]
    add	r0, r0, r3
    ldr	r1, [sp, #12]
    add	r3, r1, r0
    ror	r3, r6
    add	r3, r3, r4
    ldr	r0, [pc, #316]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r0, r3
    and	r0, r5
    mov	r1, r4
    bic	r1, r5
    orr	r0, r1
    mov	r1, r9
    ldr	r1, [r1, #36]
    add	r0, r0, r1
    ldr	r1, [sp, #0]
    add	r2, r1, r0
    mov	r0, sl
    ror	r2, r0
    add	r2, r2, r3
    ldr	r1, [pc, #288]
    add	r1, r5, r1
    str	r1, [sp, #4]
    mov	r0, r2
    and	r0, r4
    mov	r1, r3
    bic	r1, r4
    orr	r0, r1
    mov	r5, r9
    ldr	r5, [r5, #56]
    add	r0, r0, r5
    ldr	r1, [sp, #4]
    add	r5, r1, r0
    mov	r0, ip
    ror	r5, r0
    add	r5, r5, r2
    ldr	r1, [pc, #260]
    add	r1, r4, r1
    str	r1, [sp, #8]
    mov	r0, r5
    and	r0, r3
    mov	r1, r2
    bic	r1, r3
    orr	r0, r1
    mov	r4, r9
    ldr	r4, [r4, #12]
    add	r0, r0, r4
    ldr	r1, [sp, #8]
    add	r4, r1, r0
    mov	r0, r8
    ror	r4, r0
    add	r4, r4, r5
    ldr	r1, [pc, #232]
    add	r1, r3, r1
    str	r1, [sp, #12]
    mov	r0, r4
    and	r0, r2
    mov	r1, r5
    bic	r1, r2
    orr	r0, r1
    mov	r3, r9
    ldr	r3, [r3, #32]
    add	r0, r0, r3
    ldr	r1, [sp, #12]
    add	r3, r1, r0
    ror	r3, r6
    add	r3, r3, r4
    ldr	r0, [pc, #204]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r0, r3
    and	r0, r5
    mov	r1, r4
    bic	r1, r5
    orr	r0, r1
    mov	r1, r9
    ldr	r1, [r1, #52]
    add	r0, r0, r1
    ldr	r1, [sp, #0]
    add	r2, r1, r0
    mov	r0, sl
    ror	r2, r0
    add	r2, r2, r3
    ldr	r1, [pc, #176]
    add	r1, r1, r5
    mov	sl, r1
    mov	r0, r2
    and	r0, r4
    mov	r1, r3
    bic	r1, r4
    orr	r0, r1
    mov	r5, r9
    ldr	r5, [r5, #8]
    add	r0, r0, r5
    mov	r1, sl
    add	r5, r1, r0
    mov	r0, ip
    ror	r5, r0
    add	r5, r5, r2
    ldr	r1, [pc, #148]
    add	r1, r1, r4
    mov	sl, r1
    mov	r0, r5
    b	MD5Transform+0x48c
.align 2
    .word a.17
    .word b.18
    .word c.19
    .word d.20
    .word x.21
    .word 0xd76aa478
    .word 0xe8c7b756
    .word 0x242070db
    .word 0xc1bdceee
    .word 0xf57c0faf
    .word 0x4787c62a
    .word 0xa8304613
    .word 0xfd469501
    .word 0x698098d8
    .word 0x8b44f7af
    .word 0xffff5bb1
    .word 0x895cd7be
    .word 0x6b901122
    .word 0xfd987193
    .word 0xa679438e
    .word 0x49b40821
    .word 0xf61e2562
    .word 0xc040b340
    .word 0x265e5a51
    .word 0xe9b6c7aa
    .word 0xd62f105d
    .word 0x02441453
    .word 0xd8a1e681
    .word 0xe7d3fbc8
    .word 0x21e1cde6
    .word 0xc33707d6
    .word 0xf4d50d87
    .word 0x455a14ed
    .word 0xa9e3e905
    .word 0xfcefa3f8
    .word 0x676f02d9

    and	r0, r3
    mov	r1, r2
    bic	r1, r3
    orr	r0, r1
    mov	r4, r9
    ldr	r4, [r4, #28]
    add	r0, r0, r4
    mov	r1, sl
    add	r4, r1, r0
    mov	r0, r8
    ror	r4, r0
    add	r4, r4, r5
    ldr	r1, [pc, #912]
    add	r1, r1, r3
    mov	r8, r1
    mov	r0, r4
    and	r0, r2
    mov	r1, r5
    bic	r1, r2
    orr	r0, r1
    mov	r3, r9
    ldr	r3, [r3, #48]
    add	r0, r0, r3
    mov	r1, r8
    add	r3, r1, r0
    ror	r3, r6
    add	r3, r3, r4
    ldr	r6, [pc, #888]
    add	r2, r2, r6
    mov	r0, r3
    eor	r0, r4
    eor	r0, r5
    mov	r1, r9
    ldr	r1, [r1, #20]
    add	r0, r0, r1
    add	r2, r2, r0
    mov	r6, #28
    mov	sl, r6
    ror	r2, r6
    add	r2, r2, r3
    ldr	r0, [pc, #864]
    add	r5, r5, r0
    mov	r0, r2
    eor	r0, r3
    eor	r0, r4
    mov	r1, r9
    ldr	r1, [r1, #32]
    add	r0, r0, r1
    add	r5, r5, r0
    mov	r6, #21
    mov	r8, r6
    ror	r5, r6
    add	r5, r5, r2
    ldr	r0, [pc, #844]
    add	r4, r4, r0
    mov	r0, r5
    eor	r0, r2
    eor	r0, r3
    mov	r1, r9
    ldr	r1, [r1, #44]
    add	r0, r0, r1
    add	r4, r4, r0
    mov	r6, #16
    mov	ip, r6
    ror	r4, r6
    add	r4, r4, r5
    ldr	r0, [pc, #820]
    add	r0, r3, r0
    str	r0, [sp, #12]
    mov	r0, r4
    eor	r0, r5
    eor	r0, r2
    mov	r1, r9
    ldr	r1, [r1, #56]
    add	r0, r0, r1
    ldr	r6, [sp, #12]
    add	r3, r6, r0
    mov	r1, #9
    ror	r3, r1
    add	r3, r3, r4
    ldr	r0, [pc, #796]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r0, r3
    eor	r0, r4
    eor	r0, r5
    mov	r2, r9
    ldr	r2, [r2, #4]
    add	r0, r0, r2
    ldr	r6, [sp, #0]
    add	r2, r6, r0
    mov	r0, sl
    ror	r2, r0
    add	r2, r2, r3
    ldr	r6, [pc, #772]
    add	r6, r5, r6
    mov	r0, r2
    eor	r0, r3
    eor	r0, r4
    mov	r5, r9
    ldr	r5, [r5, #16]
    add	r0, r0, r5
    add	r5, r6, r0
    mov	r6, r8
    ror	r5, r6
    add	r5, r5, r2
    ldr	r0, [pc, #752]
    add	r0, r4, r0
    str	r0, [sp, #8]
    mov	r0, r5
    eor	r0, r2
    eor	r0, r3
    mov	r4, r9
    ldr	r4, [r4, #28]
    add	r0, r0, r4
    ldr	r6, [sp, #8]
    add	r4, r6, r0
    mov	r0, ip
    ror	r4, r0
    add	r4, r4, r5
    ldr	r6, [pc, #728]
    add	r6, r3, r6
    mov	r0, r4
    eor	r0, r5
    eor	r0, r2
    mov	r3, r9
    ldr	r3, [r3, #40]
    add	r0, r0, r3
    add	r3, r6, r0
    ror	r3, r1
    add	r3, r3, r4
    ldr	r6, [pc, #712]
    add	r6, r2, r6
    mov	r0, r3
    eor	r0, r4
    eor	r0, r5
    mov	r2, r9
    ldr	r2, [r2, #52]
    add	r0, r0, r2
    add	r2, r6, r0
    mov	r6, sl
    ror	r2, r6
    add	r2, r2, r3
    ldr	r0, [pc, #692]
    add	r0, r5, r0
    str	r0, [sp, #4]
    mov	r0, r2
    eor	r0, r3
    eor	r0, r4
    mov	r5, r9
    ldr	r5, [r5, #0]
    add	r0, r0, r5
    ldr	r6, [sp, #4]
    add	r5, r6, r0
    mov	r0, r8
    ror	r5, r0
    add	r5, r5, r2
    ldr	r6, [pc, #668]
    add	r6, r4, r6
    mov	r0, r5
    eor	r0, r2
    eor	r0, r3
    mov	r4, r9
    ldr	r4, [r4, #12]
    add	r0, r0, r4
    add	r4, r6, r0
    mov	r6, ip
    ror	r4, r6
    add	r4, r4, r5
    ldr	r0, [pc, #648]
    add	r0, r3, r0
    str	r0, [sp, #12]
    mov	r0, r4
    eor	r0, r5
    eor	r0, r2
    mov	r3, r9
    ldr	r3, [r3, #24]
    add	r0, r0, r3
    ldr	r6, [sp, #12]
    add	r3, r6, r0
    ror	r3, r1
    add	r3, r3, r4
    ldr	r0, [pc, #624]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mov	r0, r3
    eor	r0, r4
    eor	r0, r5
    mov	r2, r9
    ldr	r2, [r2, #36]
    add	r0, r0, r2
    ldr	r6, [sp, #0]
    add	r2, r6, r0
    mov	r0, sl
    ror	r2, r0
    add	r2, r2, r3
    ldr	r6, [pc, #600]
    add	r6, r6, r5
    mov	r0, r2
    eor	r0, r3
    eor	r0, r4
    mov	r5, r9
    ldr	r5, [r5, #48]
    add	r0, r0, r5
    add	r5, r6, r0
    mov	r6, r8
    ror	r5, r6
    add	r5, r5, r2
    ldr	r0, [pc, #580]
    add	r0, r0, r4
    mov	r8, r0
    mov	r0, r5
    eor	r0, r2
    eor	r0, r3
    mov	r4, r9
    ldr	r4, [r4, #60]
    add	r0, r0, r4
    mov	r6, r8
    add	r4, r6, r0
    mov	r0, ip
    ror	r4, r0
    add	r4, r4, r5
    ldr	r6, [pc, #556]
    add	r6, r6, r3
    mov	r0, r4
    eor	r0, r5
    eor	r0, r2
    mov	r3, r9
    ldr	r3, [r3, #8]
    add	r0, r0, r3
    add	r3, r6, r0
    ror	r3, r1
    add	r3, r3, r4
    ldr	r6, [pc, #540]
    add	r2, r2, r6
    mvn	r0, r5
    orr	r0, r3
    eor	r0, r4
    mov	r1, r9
    ldr	r1, [r1, #0]
    add	r0, r0, r1
    add	r2, r2, r0
    mov	r1, #26
    ror	r2, r1
    add	r2, r2, r3
    ldr	r6, [pc, #520]
    add	r6, r6, r5
    mvn	r0, r4
    orr	r0, r2
    eor	r0, r3
    mov	r5, r9
    ldr	r5, [r5, #28]
    add	r0, r0, r5
    add	r5, r6, r0
    mov	r6, #22
    mov	ip, r6
    ror	r5, r6
    add	r5, r5, r2
    ldr	r0, [pc, #496]
    add	r0, r0, r4
    mov	r8, r0
    mvn	r0, r3
    orr	r0, r5
    eor	r0, r2
    mov	r4, r9
    ldr	r4, [r4, #56]
    add	r0, r0, r4
    mov	r6, r8
    add	r4, r6, r0
    mov	r0, #17
    mov	r8, r0
    ror	r4, r0
    add	r4, r4, r5
    ldr	r6, [pc, #472]
    add	r6, r6, r3
    mvn	r0, r2
    orr	r0, r4
    eor	r0, r5
    mov	r3, r9
    ldr	r3, [r3, #20]
    add	r0, r0, r3
    add	r3, r6, r0
    mov	r6, #11
    mov	sl, r6
    ror	r3, r6
    add	r3, r3, r4
    ldr	r0, [pc, #448]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mvn	r0, r5
    orr	r0, r3
    eor	r0, r4
    mov	r2, r9
    ldr	r2, [r2, #48]
    add	r0, r0, r2
    ldr	r6, [sp, #0]
    add	r2, r6, r0
    ror	r2, r1
    add	r2, r2, r3
    ldr	r0, [pc, #428]
    add	r0, r5, r0
    str	r0, [sp, #4]
    mvn	r0, r4
    orr	r0, r2
    eor	r0, r3
    mov	r5, r9
    ldr	r5, [r5, #12]
    add	r0, r0, r5
    ldr	r6, [sp, #4]
    add	r5, r6, r0
    mov	r0, ip
    ror	r5, r0
    add	r5, r5, r2
    ldr	r6, [pc, #404]
    add	r6, r4, r6
    mvn	r0, r3
    orr	r0, r5
    eor	r0, r2
    mov	r4, r9
    ldr	r4, [r4, #40]
    add	r0, r0, r4
    add	r4, r6, r0
    mov	r6, r8
    ror	r4, r6
    add	r4, r4, r5
    ldr	r0, [pc, #384]
    add	r0, r3, r0
    str	r0, [sp, #12]
    mvn	r0, r2
    orr	r0, r4
    eor	r0, r5
    mov	r3, r9
    ldr	r3, [r3, #4]
    add	r0, r0, r3
    ldr	r6, [sp, #12]
    add	r3, r6, r0
    mov	r0, sl
    ror	r3, r0
    add	r3, r3, r4
    ldr	r6, [pc, #360]
    add	r6, r2, r6
    mvn	r0, r5
    orr	r0, r3
    eor	r0, r4
    mov	r2, r9
    ldr	r2, [r2, #32]
    add	r0, r0, r2
    add	r2, r6, r0
    ror	r2, r1
    add	r2, r2, r3
    ldr	r6, [pc, #340]
    add	r6, r5, r6
    mvn	r0, r4
    orr	r0, r2
    eor	r0, r3
    mov	r5, r9
    ldr	r5, [r5, #60]
    add	r0, r0, r5
    add	r5, r6, r0
    mov	r6, ip
    ror	r5, r6
    add	r5, r5, r2
    ldr	r0, [pc, #320]
    add	r0, r4, r0
    str	r0, [sp, #8]
    mvn	r0, r3
    orr	r0, r5
    eor	r0, r2
    mov	r4, r9
    ldr	r4, [r4, #24]
    add	r0, r0, r4
    ldr	r6, [sp, #8]
    add	r4, r6, r0
    mov	r0, r8
    ror	r4, r0
    add	r4, r4, r5
    ldr	r6, [pc, #296]
    add	r6, r3, r6
    mvn	r0, r2
    orr	r0, r4
    eor	r0, r5
    mov	r3, r9
    ldr	r3, [r3, #52]
    add	r0, r0, r3
    add	r3, r6, r0
    mov	r6, sl
    ror	r3, r6
    add	r3, r3, r4
    ldr	r0, [pc, #276]
    add	r0, r2, r0
    str	r0, [sp, #0]
    mvn	r0, r5
    orr	r0, r3
    eor	r0, r4
    mov	r2, r9
    ldr	r2, [r2, #16]
    add	r0, r0, r2
    ldr	r6, [sp, #0]
    add	r2, r6, r0
    ror	r2, r1
    add	r2, r2, r3
    ldr	r0, [pc, #256]
    str	r2, [r0, #0]
    ldr	r1, [pc, #256]
    add	r1, r5, r1
    mvn	r0, r4
    orr	r0, r2
    eor	r0, r3
    mov	r5, r9
    ldr	r5, [r5, #44]
    add	r0, r0, r5
    add	r5, r1, r0
    mov	r6, ip
    ror	r5, r6
    add	r5, r5, r2
    ldr	r0, [pc, #236]
    str	r5, [r0, #0]
    ldr	r1, [pc, #236]
    add	r4, r4, r1
    mvn	r0, r3
    orr	r0, r5
    eor	r0, r2
    mov	r6, r9
    ldr	r6, [r6, #8]
    add	r0, r0, r6
    add	r4, r4, r0
    mov	r0, r8
    ror	r4, r0
    add	r4, r4, r5
    ldr	r1, [pc, #216]
    str	r4, [r1, #0]
    ldr	r6, [pc, #216]
    add	r3, r3, r6
    mvn	r0, r2
    orr	r0, r4
    eor	r0, r5
    mov	r1, r9
    ldr	r1, [r1, #36]
    add	r0, r0, r1
    add	r3, r3, r0
    mov	r6, sl
    ror	r3, r6
    add	r3, r3, r4
    ldr	r0, [pc, #196]
    str	r3, [r0, #0]
    ldr	r0, [r7, #0]
    add	r0, r0, r2
    str	r0, [r7, #0]
    ldr	r0, [r7, #4]
    add	r0, r0, r3
    str	r0, [r7, #4]
    ldr	r0, [r7, #8]
    add	r0, r0, r4
    str	r0, [r7, #8]
    ldr	r0, [r7, #12]
    add	r0, r0, r5
    str	r0, [r7, #12]
    mov	r0, r9
    mov	r1, #0
    mov	r2, #64
    bl	MD5_memset
    add	sp, #16
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0x8d2a4c8a
    .word 0xfffa3942
    .word 0x8771f681
    .word 0x6d9d6122
    .word 0xfde5380c
    .word 0xa4beea44
    .word 0x4bdecfa9
    .word 0xf6bb4b60
    .word 0xbebfbc70
    .word 0x289b7ec6
    .word 0xeaa127fa
    .word 0xd4ef3085
    .word 0x04881d05
    .word 0xd9d4d039
    .word 0xe6db99e5
    .word 0x1fa27cf8
    .word 0xc4ac5665
    .word 0xf4292244
    .word 0x432aff97
    .word 0xab9423a7
    .word 0xfc93a039
    .word 0x655b59c3
    .word 0x8f0ccc92
    .word 0xffeff47d
    .word 0x85845dd1
    .word 0x6fa87e4f
    .word 0xfe2ce6e0
    .word 0xa3014314
    .word 0x4e0811a1
    .word 0xf7537e82
    .word a.17
    .word 0xbd3af235
    .word d.20
    .word 0x2ad7d2bb
    .word c.19
    .word 0xeb86d391
    .word b.18
.size MD5Transform, .-MD5Transform

.align 2
.thumb_func
Encode:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    mov	r8, r0
    mov	ip, r1
    mov	r5, r2
    ldr	r3, [pc, #64]
    mov	r0, #0
    str	r0, [r3, #0]
    ldr	r4, [pc, #64]
    str	r0, [r4, #0]
    cmp	r0, r5
    bcs	Encode+0x46
    mov	r7, r4
    mov	r6, r3
    ldr	r4, [r7, #0]
    mov	r0, r8
    add	r2, r0, r4
    ldr	r3, [r6, #0]
    lsl	r0, r3, #2
    add	r0, ip
    ldr	r1, [r0, #0]
    strb	r1, [r2, #0]
    lsr	r0, r1, #8
    strb	r0, [r2, #1]
    lsr	r0, r1, #16
    strb	r0, [r2, #2]
    lsr	r1, r1, #24
    strb	r1, [r2, #3]
    add	r3, #1
    str	r3, [r6, #0]
    add	r4, #4
    str	r4, [r7, #0]
    cmp	r4, r5
    bcc	Encode+0x1e
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word i
    .word j
.size Encode, .-Encode

.align 2
.thumb_func
Decode:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r9, r0
    mov	r8, r1
    mov	r6, r2
    ldr	r4, [pc, #80]
    mov	r0, #0
    str	r0, [r4, #0]
    ldr	r3, [pc, #76]
    str	r0, [r3, #0]
    cmp	r0, r6
    bcs	Decode+0x52
    mov	ip, r4
    mov	r7, r3
    mov	r0, ip
    ldr	r4, [r0, #0]
    lsl	r5, r4, #2
    add	r5, r9
    ldr	r3, [r7, #0]
    mov	r0, r8
    add	r2, r0, r3
    ldrb	r1, [r2, #0]
    ldrb	r0, [r2, #1]
    lsl	r0, r0, #8
    orr	r1, r0
    ldrb	r0, [r2, #2]
    lsl	r0, r0, #16
    orr	r1, r0
    ldrb	r0, [r2, #3]
    lsl	r0, r0, #24
    orr	r1, r0
    str	r1, [r5, #0]
    add	r4, #1
    mov	r0, ip
    str	r4, [r0, #0]
    add	r3, #4
    str	r3, [r7, #0]
    cmp	r3, r6
    bcc	Decode+0x20
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word i
    .word j
.size Decode, .-Decode

.align 2
.thumb_func
MD5_memcpy:
    push	{r4, r5, r6, lr}
    mov	r6, r0
    mov	r5, r1
    mov	r3, r2
    ldr	r1, [pc, #32]
    mov	r0, #0
    str	r0, [r1, #0]
    cmp	r0, r3
    bcs	MD5_memcpy+0x26
    mov	r4, r1
    ldr	r1, [r4, #0]
    add	r2, r6, r1
    add	r0, r5, r1
    ldrb	r0, [r0, #0]
    strb	r0, [r2, #0]
    add	r1, #1
    str	r1, [r4, #0]
    cmp	r1, r3
    bcc	MD5_memcpy+0x14
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word i
.size MD5_memcpy, .-MD5_memcpy

.align 2
.thumb_func
MD5_memset:
    push	{r4, r5, lr}
    mov	r4, r0
    ldr	r3, [pc, #28]
    mov	r0, #0
    str	r0, [r3, #0]
    cmp	r0, r2
    bcs	MD5_memset+0x1e
    mov	r5, r3
    mov	r3, #0
    add	r0, r4, r3
    strb	r1, [r0, #0]
    add	r3, #1
    cmp	r3, r2
    bcc	MD5_memset+0x12
    str	r3, [r5, #0]
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word i
.size MD5_memset, .-MD5_memset

.lcomm i, 0x4
.lcomm j, 0x4
