.thumb
.gcc2_compiled.:

.lcomm seq_bin.3, 0x24
.lcomm seq_text.4, 0x31
.lcomm onetime_key_bin.5, 0x24
.lcomm onetime_key_text.6, 0x2d

.align 2
.thumb_func
.global MA_MakeAuthorizationCode
MA_MakeAuthorizationCode:
    push	{r4, r5, r6, lr}
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6}
    mov	r6, r0
    mov	r5, r3
    ldr	r0, [pc, #96]
    mov	r9, r0
    mov	r0, #0
    mov	r3, r9
    strb	r0, [r3, #0]
    ldr	r3, [pc, #92]
    mov	r8, r3
    strb	r0, [r3, #0]
    strb	r0, [r5, #0]
    ldr	r4, [pc, #88]
    mov	r0, r6
    mov	r3, r4
    bl	gb_MakeSecretCode
    mov	r0, #48
    mov	r1, r6
    mov	r2, r4
    bl	gb_OutSecretCode
    mov	r0, #36
    mov	r1, r4
    mov	r2, r9
    bl	Base64_encode
    ldr	r4, [pc, #60]
    mov	r0, #48
    mov	r1, r6
    mov	r2, r4
    bl	Base64_decode
    mov	r0, #32
    mov	r1, r4
    mov	r2, r8
    bl	Base64_encode
    mov	r0, r5
    mov	r1, r8
    bl	MAU_strcpy
    add	r5, #44
    mov	r0, r5
    mov	r1, r9
    bl	MAU_strcpy
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word seq_text.4
    .word onetime_key_text.6
    .word seq_bin.3
    .word onetime_key_bin.5
.size MA_MakeAuthorizationCode, .-MA_MakeAuthorizationCode

.lcomm hash.10, 0x11
.lcomm j.11, 0x4

.align 2
.thumb_func
gb_MakeSecretCode:
    push	{r4, r5, r6, lr}
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6}
    mov	r9, r0
    mov	r6, r1
    mov	r8, r2
    mov	r5, r3
    ldr	r4, [pc, #88]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #17
    bl	MAU_memset
    mov	r0, r4
    mov	r1, r9
    mov	r2, r8
    bl	gb_CreateMD5Hash
    mov	r0, r5
    mov	r1, r4
    mov	r2, #16
    bl	MAU_memcpy
    mov	r4, r5
    add	r4, #16
    mov	r0, r6
    bl	MAU_strlen
    mov	r2, r0
    mov	r0, r4
    mov	r1, r6
    bl	MAU_memcpy
    ldr	r4, [pc, #40]
    mov	r0, r6
    bl	MAU_strlen
    add	r0, #16
    str	r0, [r4, #0]
    add	r5, r5, r0
    mov	r2, #36
    sub	r2, r2, r0
    mov	r0, r5
    mov	r1, #255
    bl	MAU_memset
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word hash.10
    .word j.11
.size gb_MakeSecretCode, .-gb_MakeSecretCode

.lcomm dest.15, 0x25
.lcomm result.16, 0x25

.align 2
.thumb_func
gb_OutSecretCode:
    push	{r4, r5, r6, lr}
    mov	r6, r8
    push	{r6}
    mov	r5, r0
    mov	r6, r1
    mov	r8, r2
    ldr	r4, [pc, #48]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #37
    bl	MAU_memset
    mov	r0, r5
    mov	r1, r6
    mov	r2, r4
    bl	Base64_decode
    ldr	r5, [pc, #32]
    mov	r0, r5
    mov	r1, r4
    bl	gb_BitHalfMove
    mov	r0, r8
    mov	r1, r5
    bl	gb_BitChangeAndRotation
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word dest.15
    .word result.16
.size gb_OutSecretCode, .-gb_OutSecretCode

.lcomm buf.20, 0x64

.align 2
.thumb_func
gb_CreateMD5Hash:
    push	{r4, r5, r6, lr}
    mov	r6, r8
    push	{r6}
    mov	r8, r0
    mov	r5, r1
    mov	r6, r2
    ldr	r4, [pc, #68]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #100
    bl	MAU_memset
    mov	r0, r4
    mov	r1, r5
    mov	r2, #48
    bl	MAU_memcpy
    mov	r0, r6
    bl	MAU_strlen
    mov	r2, r0
    ldr	r5, [pc, #44]
    str	r2, [r5, #0]
    mov	r0, r4
    add	r0, #48
    mov	r1, r6
    bl	MAU_memcpy
    mov	r0, r4
    bl	MAU_strlen
    mov	r1, r0
    str	r1, [r5, #0]
    mov	r0, r4
    mov	r2, r8
    bl	CalcValueMD5
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word buf.20
    .word len
.size gb_CreateMD5Hash, .-gb_CreateMD5Hash

.lcomm half.24, 0x4

.align 2
.thumb_func
gb_BitHalfMove:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    sub	sp, #8
    mov	r9, r0
    str	r1, [sp, #0]
    mov	r1, #0
    mov	r2, #37
    bl	MAU_memset
    ldr	r1, [pc, #280]
    mov	r0, #18
    str	r0, [r1, #0]
    ldr	r1, [pc, #280]
    mov	r0, #0
    str	r0, [r1, #0]
    mov	r6, r1
    mov	r0, #64
    mov	sl, r0
    mov	r1, #16
    mov	r8, r1
    mov	r0, #4
    mov	ip, r0
    mov	r7, #1
    ldr	r4, [r6, #0]
    lsl	r3, r4, #1
    ldr	r1, [pc, #256]
    str	r3, [r1, #0]
    mov	r0, r9
    add	r5, r0, r4
    ldr	r1, [sp, #0]
    add	r3, r1, r3
    ldrb	r2, [r3, #0]
    mov	r1, sl
    and	r1, r2
    lsl	r1, r1, #1
    ldrb	r0, [r5, #0]
    orr	r1, r0
    mov	r0, r8
    and	r0, r2
    lsl	r0, r0, #2
    orr	r1, r0
    mov	r0, ip
    and	r0, r2
    lsl	r0, r0, #3
    orr	r1, r0
    mov	r0, r7
    and	r0, r2
    lsl	r0, r0, #4
    orr	r1, r0
    ldrb	r2, [r3, #1]
    mov	r0, sl
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #27
    orr	r1, r0
    mov	r0, r8
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #26
    orr	r1, r0
    mov	r0, ip
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #25
    orr	r1, r0
    mov	r0, r7
    and	r0, r2
    orr	r1, r0
    strb	r1, [r5, #0]
    add	r4, #1
    str	r4, [r6, #0]
    cmp	r4, #18
    blt	gb_BitHalfMove+0x34
    ldr	r1, [pc, #156]
    ldr	r0, [r1, #0]
    cmp	r0, #35
    bgt	gb_BitHalfMove+0x122
    mov	ip, r1
    ldr	r0, [pc, #144]
    ldr	r0, [r0, #0]
    str	r0, [sp, #4]
    mov	r0, #128
    mov	sl, r0
    mov	r1, #32
    mov	r8, r1
    mov	r7, #8
    mov	r6, #2
    mov	r0, ip
    ldr	r4, [r0, #0]
    ldr	r1, [sp, #4]
    sub	r2, r4, r1
    lsl	r2, r2, #1
    ldr	r0, [pc, #124]
    str	r2, [r0, #0]
    mov	r1, r9
    add	r5, r1, r4
    ldr	r0, [sp, #0]
    add	r2, r0, r2
    ldrb	r3, [r2, #0]
    mov	r1, sl
    and	r1, r3
    ldrb	r0, [r5, #0]
    orr	r1, r0
    mov	r0, r8
    and	r0, r3
    lsl	r0, r0, #1
    orr	r1, r0
    mov	r0, r7
    and	r0, r3
    lsl	r0, r0, #2
    orr	r1, r0
    mov	r0, r6
    and	r0, r3
    lsl	r0, r0, #3
    orr	r1, r0
    ldrb	r2, [r2, #1]
    mov	r0, sl
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #28
    orr	r1, r0
    mov	r0, r8
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #27
    orr	r1, r0
    mov	r0, r7
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #26
    orr	r1, r0
    mov	r0, r6
    and	r0, r2
    lsl	r0, r0, #24
    lsr	r0, r0, #25
    orr	r1, r0
    strb	r1, [r5, #0]
    add	r4, #1
    mov	r1, ip
    str	r4, [r1, #0]
    cmp	r4, #35
    ble	gb_BitHalfMove+0xb4
    add	sp, #8
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word half.24
    .word i
    .word j
.size gb_BitHalfMove, .-gb_BitHalfMove

.lcomm buf.28, 0x1

.align 2
.thumb_func
gb_BitChangeAndRotation:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r0
    mov	r6, r1
    ldr	r1, [pc, #72]
    mov	r0, #0
    str	r0, [r1, #0]
    mov	r5, r1
    ldr	r0, [pc, #68]
    mov	ip, r0
    ldr	r3, [r5, #0]
    add	r4, r7, r3
    add	r0, r6, r3
    ldrb	r1, [r4, #0]
    ldrb	r0, [r0, #0]
    eor	r1, r0
    mov	r2, #182
    and	r2, r1
    mov	r0, #8
    and	r0, r1
    lsl	r0, r0, #3
    orr	r2, r0
    mov	r0, #1
    and	r0, r1
    lsl	r0, r0, #3
    orr	r2, r0
    mov	r0, #64
    and	r1, r0
    lsl	r1, r1, #24
    lsr	r1, r1, #30
    orr	r2, r1
    mov	r0, ip
    strb	r2, [r0, #0]
    strb	r2, [r4, #0]
    add	r3, #1
    str	r3, [r5, #0]
    cmp	r3, #35
    ble	gb_BitChangeAndRotation+0x12
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word i
    .word buf.28
.size gb_BitChangeAndRotation, .-gb_BitChangeAndRotation

.lcomm context.32, 0x58

.align 2
.thumb_func
CalcValueMD5:
    push	{r4, r5, r6, lr}
    mov	r6, r8
    push	{r6}
    mov	r5, r0
    mov	r6, r1
    mov	r8, r2
    ldr	r4, [pc, #44]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #88
    bl	MAU_memset
    mov	r0, r4
    bl	MD5Init
    mov	r0, r4
    mov	r1, r5
    mov	r2, r6
    bl	MD5Update
    mov	r0, r8
    mov	r1, r4
    bl	MD5Final
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word context.32
.size CalcValueMD5, .-CalcValueMD5

.section .rodata
.align 2
.type xchg, object
xchg:
    .word 0x44434241
    .word 0x48474645
    .word 0x4c4b4a49
    .word 0x504f4e4d
    .word 0x54535251
    .word 0x58575655
    .word 0x62615a59
    .word 0x66656463
    .word 0x6a696867
    .word 0x6e6d6c6b
    .word 0x7271706f
    .word 0x76757473
    .word 0x7a797877
    .word 0x33323130
    .word 0x37363534
    .word 0x2f2b3938
    .byte 0
.size xchg, .-xchg
.section .text

.align 2
.thumb_func
Base64_encode:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    mov	ip, r0
    mov	r4, r1
    ldr	r0, [pc, #44]
    mov	r1, #0
    str	r1, [r0, #0]
    cmp	r1, ip
    bge	Base64_encode+0xc8
    ldr	r5, [pc, #36]
    ldr	r6, [pc, #40]
    mov	r0, #0
    str	r0, [r5, #0]
    str	r0, [r6, #0]
    ldr	r1, [pc, #24]
    ldr	r3, [r1, #0]
    cmp	r3, ip
    blt	Base64_encode+0x48
    ldr	r0, [r6, #0]
    mov	r1, #3
    sub	r1, r1, r0
    lsl	r1, r1, #3
    ldr	r0, [r5, #0]
    lsl	r0, r1
    str	r0, [r5, #0]
    b	Base64_encode+0x64
.align 2
    .word len
    .word code.36
    .word i

    ldr	r0, [r5, #0]
    lsl	r0, r0, #8
    ldrb	r1, [r4, #0]
    orr	r0, r1
    str	r0, [r5, #0]
    add	r4, #1
    add	r0, r3, #1
    ldr	r7, [pc, #68]
    str	r0, [r7, #0]
    ldr	r0, [r6, #0]
    add	r0, #1
    str	r0, [r6, #0]
    cmp	r0, #2
    ble	Base64_encode+0x22
    mov	r0, #0
    ldr	r1, [pc, #56]
    str	r0, [r1, #0]
    mov	r3, r1
    ldr	r7, [pc, #52]
    mov	sl, r7
    ldr	r0, [pc, #52]
    mov	r9, r0
    ldr	r1, [pc, #52]
    mov	r8, r1
    ldr	r1, [r3, #0]
    mov	r7, sl
    ldr	r0, [r7, #0]
    cmp	r1, r0
    bgt	Base64_encode+0xb0
    mov	r7, #3
    sub	r0, r7, r1
    lsl	r1, r0, #1
    add	r1, r1, r0
    lsl	r1, r1, #1
    mov	r7, r8
    ldr	r0, [r7, #0]
    lsr	r0, r1
    mov	r1, #63
    and	r0, r1
    add	r0, r9
    ldrb	r0, [r0, #0]
    b	Base64_encode+0xb2
.align 2
    .word len
    .word k
    .word i
    .word xchg
    .word code.36

    mov	r0, #61
    strb	r0, [r2, #0]
    add	r2, #1
    ldr	r0, [r3, #0]
    add	r0, #1
    str	r0, [r3, #0]
    cmp	r0, #3
    ble	Base64_encode+0x78
    ldr	r1, [pc, #24]
    ldr	r0, [r1, #0]
    cmp	r0, ip
    blt	Base64_encode+0x1c
    mov	r0, #0
    strb	r0, [r2, #0]
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word len
.size Base64_encode, .-Base64_encode

.section .rodata
.type base64RevTable.40, object
base64RevTable.40:
    .word 0xffffffff
    .word 0xffffffff
    .word 0x3effffff
    .word 0x3fffffff
    .word 0x37363534
    .word 0x3b3a3938
    .word 0xffff3d3c
    .word 0xffffffff
    .word 0x020100ff
    .word 0x06050403
    .word 0x0a090807
    .word 0x0e0d0c0b
    .word 0x1211100f
    .word 0x16151413
    .word 0xff191817
    .word 0xffffffff
    .word 0x1c1b1aff
    .word 0x201f1e1d
    .word 0x24232221
    .word 0x28272625
    .word 0x2c2b2a29
    .word 0x302f2e2d
    .word 0xff333231
    .word 0xffffffff
.section .text

.lcomm code.36, 0x4
.lcomm code.41, 0x4
.lcomm c.42, 0x4
.lcomm byte.43, 0x4

.align 2
.thumb_func
Base64_decode:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    sub	sp, #4
    str	r0, [sp, #0]
    mov	r4, r1
    mov	r5, r2
    ldr	r1, [pc, #60]
    mov	r0, #0
    str	r0, [r1, #0]
    ldr	r2, [sp, #0]
    cmp	r0, r2
    bge	Base64_decode+0xe6
    ldr	r7, [pc, #52]
    ldr	r6, [pc, #52]
    mov	r8, r6
    ldr	r0, [pc, #52]
    mov	r9, r0
    mov	sl, r1
    mov	r1, #0
    str	r1, [r7, #0]
    mov	r2, r8
    str	r1, [r2, #0]
    ldrb	r0, [r4, #0]
    sub	r0, #32
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #95
    bhi	Base64_decode+0x64
    ldrb	r0, [r4, #0]
    sub	r0, #32
    ldr	r6, [pc, #28]
    add	r0, r0, r6
    ldrb	r0, [r0, #0]
    mov	r1, r9
    str	r0, [r1, #0]
    b	Base64_decode+0x6a
.align 2
    .word byte.43
    .word code.41
    .word i
    .word c.42
    .word base64RevTable.40

    mov	r0, #255
    mov	r2, r9
    str	r0, [r2, #0]
    add	r4, #1
    mov	r6, sl
    ldr	r0, [r6, #0]
    add	r0, #1
    str	r0, [r6, #0]
    mov	r0, r9
    ldr	r1, [r0, #0]
    cmp	r1, #255
    bne	Base64_decode+0x92
    mov	r1, r8
    ldr	r0, [r1, #0]
    mov	r1, #4
    sub	r1, r1, r0
    lsl	r0, r1, #1
    add	r0, r0, r1
    lsl	r0, r0, #1
    ldr	r1, [r7, #0]
    lsl	r1, r0
    str	r1, [r7, #0]
    b	Base64_decode+0xa6
    ldr	r0, [r7, #0]
    lsl	r0, r0, #6
    orr	r0, r1
    str	r0, [r7, #0]
    mov	r2, r8
    ldr	r0, [r2, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #3
    ble	Base64_decode+0x32
    mov	r6, #0
    ldr	r0, [pc, #80]
    str	r6, [r0, #0]
    mov	r2, r8
    ldr	r0, [r2, #0]
    sub	r0, #1
    cmp	r6, r0
    bge	Base64_decode+0xdc
    ldr	r1, [pc, #72]
    mov	ip, r1
    ldr	r3, [pc, #64]
    ldr	r0, [r3, #0]
    mov	r6, #2
    sub	r0, r6, r0
    lsl	r0, r0, #3
    mov	r6, ip
    ldr	r1, [r6, #0]
    lsr	r1, r0
    strb	r1, [r5, #0]
    add	r5, #1
    ldr	r1, [r3, #0]
    add	r1, #1
    str	r1, [r3, #0]
    ldr	r0, [r2, #0]
    sub	r0, #1
    cmp	r1, r0
    blt	Base64_decode+0xbc
    mov	r1, sl
    ldr	r0, [r1, #0]
    ldr	r2, [sp, #0]
    cmp	r0, r2
    blt	Base64_decode+0x2a
    mov	r0, #0
    strb	r0, [r5, #0]
    add	sp, #4
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word k
    .word code.41
.size Base64_decode, .-Base64_decode

.lcomm i, 0x4
.lcomm j, 0x4
.lcomm k, 0x4
.lcomm len, 0x4
