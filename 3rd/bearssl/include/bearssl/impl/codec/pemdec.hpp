#ifndef BEARSSL_IMPL_CODEC_PEMDEC_HPP
#define BEARSSL_IMPL_CODEC_PEMDEC_HPP

/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} _opt_libs_BearSSL_src_codec_pemdec_c_t0_context;

 inline  uint32_t
_opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_unsigned(const unsigned char **p)
{
	uint32_t x;

	x = 0;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			return x;
		}
	}
}

 inline  int32_t
_opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_signed(const unsigned char **p)
{
	int neg;
	uint32_t x;

	neg = ((**p) >> 6) & 1;
	x = (uint32_t)-neg;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			if (neg) {
				return -(int32_t)~x - 1;
			} else {
				return (int32_t)x;
			}
		}
	}
}

#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_INT1(x)       _opt_libs_BearSSL_src_codec_pemdec_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_INT2(x)       _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_codec_pemdec_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_INT3(x)       _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_codec_pemdec_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_INT4(x)       _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_codec_pemdec_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_INT5(x)       _opt_libs_BearSSL_src_codec_pemdec_c_T0_SBYTE(x), _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_codec_pemdec_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_codec_pemdec_c_T0_FBYTE(x, 0)

/* static const unsigned char _opt_libs_BearSSL_src_codec_pemdec_c_t0_datablock[]; */


void br_pem_decoder_init_main(void *t0ctx);

void br_pem_decoder_run(void *t0ctx);



#include "inner.h"

#define _opt_libs_BearSSL_src_codec_pemdec_c_CTX   ((br_pem_decoder_context *)((unsigned char *)t0ctx - offsetof(br_pem_decoder_context, cpu)))

/* see bearssl_pem.h */
 inline void
br_pem_decoder_init(br_pem_decoder_context *ctx)
{
	memset(ctx, 0, sizeof *ctx);
	ctx->cpu.dp = &ctx->dp_stack[0];
	ctx->cpu.rp = &ctx->rp_stack[0];
	br_pem_decoder_init_main(&ctx->cpu);
	br_pem_decoder_run(&ctx->cpu);
}

/* see bearssl_pem.h */
 inline size_t
br_pem_decoder_push(br_pem_decoder_context *ctx,
	const void *data, size_t len)
{
	if (ctx->event) {
		return 0;
	}
	ctx->hbuf = (unsigned char *)data;
	ctx->hlen = len;
	br_pem_decoder_run(&ctx->cpu);
	return len - ctx->hlen;
}

/* see bearssl_pem.h */
 inline int
br_pem_decoder_event(br_pem_decoder_context *ctx)
{
	int event;

	event = ctx->event;
	ctx->event = 0;
	return event;
}



 inline static const unsigned char _opt_libs_BearSSL_src_codec_pemdec_c_t0_datablock[] = {
	0x00, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20,
	0x00, 0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x00
};

 inline static const unsigned char _opt_libs_BearSSL_src_codec_pemdec_c_t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x01,
	0x01, 0x08, 0x00, 0x00, 0x13, 0x13, 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_INT2(offsetof(br_pem_decoder_context, event)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_INT2(offsetof(br_pem_decoder_context, name)), 0x00, 0x00, 0x05,
	0x14, 0x2C, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x03, 0x13, 0x04, 0x76, 0x01,
	0x2D, 0x0C, 0x06, 0x05, 0x2E, 0x01, 0x03, 0x2D, 0x00, 0x01, 0x0D, 0x27,
	0x05, 0x04, 0x01, 0x03, 0x2D, 0x00, 0x15, 0x2E, 0x01, 0x02, 0x2D, 0x00,
	0x01, 0x01, 0x7F, 0x03, 0x00, 0x24, 0x01, 0x00, 0x17, 0x0D, 0x06, 0x03,
	0x13, 0x04, 0x3C, 0x01, 0x7F, 0x17, 0x0D, 0x06, 0x13, 0x13, 0x02, 0x00,
	0x05, 0x06, 0x2E, 0x01, 0x03, 0x2D, 0x04, 0x03, 0x01, 0x7F, 0x22, 0x01,
	0x00, 0x00, 0x04, 0x23, 0x01, 0x01, 0x17, 0x0D, 0x06, 0x09, 0x13, 0x01,
	0x00, 0x22, 0x01, 0x00, 0x00, 0x04, 0x14, 0x01, 0x02, 0x17, 0x0D, 0x06,
	0x06, 0x13, 0x01, 0x7F, 0x00, 0x04, 0x08, 0x13, 0x01, 0x03, 0x2D, 0x01,
	0x00, 0x00, 0x13, 0x01, 0x00, 0x03, 0x00, 0x04, 0xFF, 0x33, 0x01, 0x2C,
	0x14, 0x01, 0x2D, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x7F, 0x00, 0x14, 0x31,
	0x06, 0x02, 0x13, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13, 0x01,
	0x02, 0x00, 0x25, 0x14, 0x1C, 0x06, 0x05, 0x13, 0x2E, 0x01, 0x03, 0x00,
	0x03, 0x00, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x03,
	0x00, 0x25, 0x14, 0x1C, 0x06, 0x05, 0x13, 0x2E, 0x01, 0x03, 0x00, 0x02,
	0x00, 0x01, 0x06, 0x0A, 0x07, 0x03, 0x00, 0x29, 0x14, 0x01, 0x0A, 0x0D,
	0x06, 0x04, 0x13, 0x01, 0x03, 0x00, 0x14, 0x01, 0x3D, 0x0D, 0x06, 0x2E,
	0x13, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x03, 0x00,
	0x2F, 0x05, 0x04, 0x13, 0x01, 0x03, 0x00, 0x01, 0x3D, 0x0C, 0x06, 0x03,
	0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x0F, 0x10, 0x06, 0x03, 0x01, 0x03,
	0x00, 0x02, 0x00, 0x01, 0x04, 0x0F, 0x1B, 0x01, 0x01, 0x00, 0x25, 0x14,
	0x1C, 0x06, 0x05, 0x13, 0x2E, 0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x06,
	0x0A, 0x07, 0x03, 0x00, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13,
	0x01, 0x03, 0x00, 0x14, 0x01, 0x3D, 0x0D, 0x06, 0x20, 0x13, 0x2F, 0x05,
	0x03, 0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x03, 0x10, 0x06, 0x03, 0x01,
	0x03, 0x00, 0x02, 0x00, 0x01, 0x0A, 0x0F, 0x1B, 0x02, 0x00, 0x01, 0x02,
	0x0F, 0x1B, 0x01, 0x01, 0x00, 0x25, 0x14, 0x1C, 0x06, 0x05, 0x13, 0x2E,
	0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x06, 0x0A, 0x07, 0x03, 0x00, 0x02,
	0x00, 0x01, 0x10, 0x0F, 0x1B, 0x02, 0x00, 0x01, 0x08, 0x0F, 0x1B, 0x02,
	0x00, 0x1B, 0x01, 0x00, 0x00, 0x00, 0x14, 0x14, 0x01, 0x80, 0x41, 0x0E,
	0x1A, 0x01, 0x80, 0x5A, 0x0B, 0x10, 0x06, 0x05, 0x01, 0x80, 0x41, 0x08,
	0x00, 0x14, 0x14, 0x01, 0x80, 0x61, 0x0E, 0x1A, 0x01, 0x80, 0x7A, 0x0B,
	0x10, 0x06, 0x05, 0x01, 0x80, 0x47, 0x08, 0x00, 0x14, 0x14, 0x01, 0x30,
	0x0E, 0x1A, 0x01, 0x39, 0x0B, 0x10, 0x06, 0x04, 0x01, 0x04, 0x07, 0x00,
	0x14, 0x01, 0x2B, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x3E, 0x00, 0x14, 0x01,
	0x2F, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x3F, 0x00, 0x01, 0x3D, 0x0C, 0x1E,
	0x00, 0x00, 0x28, 0x01, 0x01, 0x2D, 0x23, 0x06, 0x02, 0x04, 0x7B, 0x04,
	0x75, 0x00, 0x14, 0x12, 0x2A, 0x14, 0x05, 0x04, 0x1F, 0x01, 0x7F, 0x00,
	0x2C, 0x2A, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x05, 0x13, 0x1F, 0x01, 0x00,
	0x00, 0x0D, 0x05, 0x05, 0x13, 0x2E, 0x01, 0x00, 0x00, 0x1D, 0x04, 0x5E,
	0x00, 0x01, 0x01, 0x27, 0x06, 0x0B, 0x21, 0x01, 0x80, 0x7F, 0x2B, 0x14,
	0x06, 0x02, 0x30, 0x00, 0x13, 0x04, 0x6E, 0x00, 0x2C, 0x14, 0x31, 0x05,
	0x01, 0x00, 0x13, 0x04, 0x77, 0x00, 0x14, 0x14, 0x01, 0x80, 0x61, 0x0E,
	0x1A, 0x01, 0x80, 0x7A, 0x0B, 0x10, 0x06, 0x03, 0x01, 0x20, 0x08, 0x00,
	0x01, 0x14, 0x03, 0x00, 0x1A, 0x17, 0x05, 0x05, 0x1F, 0x2E, 0x01, 0x00,
	0x00, 0x2C, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x06, 0x1F, 0x02, 0x00, 0x1A,
	0x08, 0x00, 0x14, 0x01, 0x0D, 0x0D, 0x06, 0x03, 0x13, 0x04, 0x03, 0x2A,
	0x17, 0x19, 0x1D, 0x1A, 0x1E, 0x1A, 0x04, 0x59, 0x00, 0x18, 0x14, 0x1C,
	0x05, 0x01, 0x00, 0x13, 0x11, 0x04, 0x76, 0x00, 0x20, 0x19, 0x11, 0x00,
	0x00, 0x2C, 0x01, 0x0A, 0x0C, 0x06, 0x02, 0x04, 0x78, 0x00, 0x01, 0x01,
	0x7F, 0x03, 0x00, 0x2C, 0x14, 0x01, 0x0A, 0x0C, 0x06, 0x09, 0x31, 0x05,
	0x04, 0x01, 0x00, 0x03, 0x00, 0x04, 0x70, 0x13, 0x02, 0x00, 0x00, 0x00,
	0x14, 0x06, 0x14, 0x1E, 0x14, 0x21, 0x07, 0x16, 0x01, 0x2D, 0x0C, 0x06,
	0x08, 0x21, 0x07, 0x1D, 0x01, 0x00, 0x1A, 0x19, 0x00, 0x04, 0x69, 0x21,
	0x19, 0x00, 0x00, 0x14, 0x01, 0x0A, 0x0C, 0x1A, 0x01, 0x20, 0x0B, 0x10,
	0x00
};

 inline static const uint16_t _opt_libs_BearSSL_src_codec_pemdec_c_t0_caddr[] = {
	0,
	5,
	10,
	15,
	19,
	24,
	29,
	67,
	149,
	384,
	464,
	476,
	511,
	530,
	540,
	559,
	603,
	614,
	619,
	629,
	654,
	681
};

#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_INTERPRETED   28

#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &_opt_libs_BearSSL_src_codec_pemdec_c_t0_codeblock[_opt_libs_BearSSL_src_codec_pemdec_c_t0_caddr[(slot) - _opt_libs_BearSSL_src_codec_pemdec_c_T0_INTERPRETED]]; \
		t0_lnum = _opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &_opt_libs_BearSSL_src_codec_pemdec_c_t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *t0ctx = (_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)ctx; \
	t0ctx->ip = &_opt_libs_BearSSL_src_codec_pemdec_c_t0_codeblock[0]; \
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

_opt_libs_BearSSL_src_codec_pemdec_c_T0_DEFENTRY(br_pem_decoder_init_main, 38)

#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

 inline void
br_pem_decoder_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_LOCAL(x)    (*(rp - 2 - (x)))
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP()       (*-- dp)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi()      (*(int32_t *)(-- dp))
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_PEEK(x)     (*(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_RPOP()      (*-- rp)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_RPOPi()     (*(int32_t *)(-- rp))
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(_opt_libs_BearSSL_src_codec_pemdec_c_T0_PEEK(t0depth)); \
} while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define _opt_libs_BearSSL_src_codec_pemdec_c_T0_RET()        goto t0_next

	dp = ((_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)t0ctx)->dp;
	rp = ((_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)t0ctx)->rp;
	ip = ((_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = _opt_libs_BearSSL_src_codec_pemdec_c_T0_NEXT(&ip);
		if (t0x < _opt_libs_BearSSL_src_codec_pemdec_c_T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = _opt_libs_BearSSL_src_codec_pemdec_c_T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &_opt_libs_BearSSL_src_codec_pemdec_c_t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSHi(_opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(_opt_libs_BearSSL_src_codec_pemdec_c_T0_LOCAL(_opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				_opt_libs_BearSSL_src_codec_pemdec_c_T0_LOCAL(_opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_unsigned(&ip)) = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
				break;
			case 4: /* jump */
				t0off = _opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = _opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_signed(&ip);
				if (_opt_libs_BearSSL_src_codec_pemdec_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = _opt_libs_BearSSL_src_codec_pemdec_c_t0_parse7E_signed(&ip);
				if (!_opt_libs_BearSSL_src_codec_pemdec_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* + */

	uint32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(a + b);

				}
				break;
			case 8: {
				/* - */

	uint32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(a - b);

				}
				break;
			case 9: {
				/* < */

	int32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 10: {
				/* << */

	int c = (int)_opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	uint32_t x = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(x << c);

				}
				break;
			case 11: {
				/* <= */

	int32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 12: {
				/* <> */

	uint32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 13: {
				/* = */

	uint32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 14: {
				/* >= */

	int32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 15: {
				/* >> */

	int c = (int)_opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	int32_t x = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POPi();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSHi(x >> c);

				}
				break;
			case 16: {
				/* and */

	uint32_t b = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(a & b);

				}
				break;
			case 17: {
				/* co */
 _opt_libs_BearSSL_src_codec_pemdec_c_T0_CO(); 
				}
				break;
			case 18: {
				/* data-get8 */

	size_t addr = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(_opt_libs_BearSSL_src_codec_pemdec_c_t0_datablock[addr]);

				}
				break;
			case 19: {
				/* drop */
 (void)_opt_libs_BearSSL_src_codec_pemdec_c_T0_POP(); 
				}
				break;
			case 20: {
				/* dup */
 _opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(_opt_libs_BearSSL_src_codec_pemdec_c_T0_PEEK(0)); 
				}
				break;
			case 21: {
				/* flush-buf */

	if (_opt_libs_BearSSL_src_codec_pemdec_c_CTX->ptr > 0) {
		_opt_libs_BearSSL_src_codec_pemdec_c_CTX->dest(_opt_libs_BearSSL_src_codec_pemdec_c_CTX->dest_ctx, _opt_libs_BearSSL_src_codec_pemdec_c_CTX->buf, _opt_libs_BearSSL_src_codec_pemdec_c_CTX->ptr);
		_opt_libs_BearSSL_src_codec_pemdec_c_CTX->ptr = 0;
	}

				}
				break;
			case 22: {
				/* get8 */

	size_t addr = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(*((unsigned char *)_opt_libs_BearSSL_src_codec_pemdec_c_CTX + addr));

				}
				break;
			case 23: {
				/* over */
 _opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(_opt_libs_BearSSL_src_codec_pemdec_c_T0_PEEK(1)); 
				}
				break;
			case 24: {
				/* read8-native */

	if (_opt_libs_BearSSL_src_codec_pemdec_c_CTX->hlen > 0) {
		_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSH(*_opt_libs_BearSSL_src_codec_pemdec_c_CTX->hbuf ++);
		_opt_libs_BearSSL_src_codec_pemdec_c_CTX->hlen --;
	} else {
		_opt_libs_BearSSL_src_codec_pemdec_c_T0_PUSHi(-1);
	}

				}
				break;
			case 25: {
				/* set8 */

	size_t addr = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	unsigned x = _opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	*((unsigned char *)_opt_libs_BearSSL_src_codec_pemdec_c_CTX + addr) = x;

				}
				break;
			case 26: {
				/* swap */
 _opt_libs_BearSSL_src_codec_pemdec_c_T0_SWAP(); 
				}
				break;
			case 27: {
				/* write8 */

	unsigned char x = (unsigned char)_opt_libs_BearSSL_src_codec_pemdec_c_T0_POP();
	_opt_libs_BearSSL_src_codec_pemdec_c_CTX->buf[_opt_libs_BearSSL_src_codec_pemdec_c_CTX->ptr ++] = x;
	if (_opt_libs_BearSSL_src_codec_pemdec_c_CTX->ptr == sizeof _opt_libs_BearSSL_src_codec_pemdec_c_CTX->buf) {
		if (_opt_libs_BearSSL_src_codec_pemdec_c_CTX->dest) {
			_opt_libs_BearSSL_src_codec_pemdec_c_CTX->dest(_opt_libs_BearSSL_src_codec_pemdec_c_CTX->dest_ctx, _opt_libs_BearSSL_src_codec_pemdec_c_CTX->buf, sizeof _opt_libs_BearSSL_src_codec_pemdec_c_CTX->buf);
		}
		_opt_libs_BearSSL_src_codec_pemdec_c_CTX->ptr = 0;
	}

				}
				break;
			}

		} else {
			_opt_libs_BearSSL_src_codec_pemdec_c_T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)t0ctx)->dp = dp;
	((_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)t0ctx)->rp = rp;
	((_opt_libs_BearSSL_src_codec_pemdec_c_t0_context *)t0ctx)->ip = ip;
}
#endif
