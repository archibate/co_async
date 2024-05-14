#ifndef BEARSSL_IMPL_X509_SKEY_DECODER_HPP
#define BEARSSL_IMPL_X509_SKEY_DECODER_HPP

/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context;

 inline  uint32_t
_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_unsigned(const unsigned char **p)
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
_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_signed(const unsigned char **p)
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

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(x)       _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT2(x)       _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT3(x)       _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT4(x)       _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT5(x)       _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_SBYTE(x), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_FBYTE(x, 0)

/* static const unsigned char _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_datablock[]; */


void br_skey_decoder_init_main(void *t0ctx);

void br_skey_decoder_run(void *t0ctx);



#include "inner.h"





#include "inner.h"

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX   ((br_skey_decoder_context *)((unsigned char *)t0ctx - offsetof(br_skey_decoder_context, cpu)))
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_CONTEXT_NAME   br_skey_decoder_context

/* see bearssl_x509.h */
 inline void
br_skey_decoder_init(br_skey_decoder_context *ctx)
{
	memset(ctx, 0, sizeof *ctx);
	ctx->cpu.dp = &ctx->dp_stack[0];
	ctx->cpu.rp = &ctx->rp_stack[0];
	br_skey_decoder_init_main(&ctx->cpu);
	br_skey_decoder_run(&ctx->cpu);
}

/* see bearssl_x509.h */
 inline void
br_skey_decoder_push(br_skey_decoder_context *ctx,
	const void *data, size_t len)
{
	ctx->hbuf = (unsigned char *)data;
	ctx->hlen = len;
	br_skey_decoder_run(&ctx->cpu);
}



 inline static const unsigned char _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_datablock[] = {
	0x00, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x07,
	0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x08, 0x2A, 0x86, 0x48, 0xCE,
	0x3D, 0x03, 0x01, 0x07, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22, 0x05, 0x2B,
	0x81, 0x04, 0x00, 0x23
};

 inline static const unsigned char _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_codeblock[] = {
	0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x01, 0x01, 0x08, 0x00, 0x00, 0x13,
	0x13, 0x00, 0x00, 0x01, _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_BAD_TAG_CLASS), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_BAD_TAG_VALUE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_EXTRA_ELEMENT), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_INDEFINITE_LENGTH), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_INNER_TRUNC), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_INVALID_VALUE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_LIMIT_EXCEEDED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_NOT_CONSTRUCTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_NOT_PRIMITIVE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_OVERFLOW), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_UNEXPECTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_ERR_X509_UNSUPPORTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_KEYTYPE_EC), 0x00, 0x00, 0x01, _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT1(BR_KEYTYPE_RSA),
	0x00, 0x00, 0x01, _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_skey_decoder_c_CONTEXT_NAME, key_data)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_skey_decoder_c_CONTEXT_NAME, key_type)), 0x00, 0x00,
	0x33, 0x48, 0x00, 0x00, 0x01, _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_skey_decoder_c_CONTEXT_NAME, pad)),
	0x00, 0x00, 0x01, 0x13, 0x00, 0x00, 0x01, 0x1C, 0x00, 0x00, 0x01, 0x22,
	0x00, 0x00, 0x05, 0x02, 0x2C, 0x16, 0x00, 0x00, 0x06, 0x02, 0x2D, 0x16,
	0x00, 0x00, 0x01, 0x10, 0x3D, 0x00, 0x00, 0x0D, 0x05, 0x02, 0x2F, 0x16,
	0x3A, 0x00, 0x00, 0x0D, 0x05, 0x02, 0x2F, 0x16, 0x3B, 0x00, 0x00, 0x06,
	0x02, 0x27, 0x16, 0x00, 0x01, 0x03, 0x00, 0x54, 0x57, 0x01, 0x02, 0x3E,
	0x55, 0x23, 0x06, 0x02, 0x30, 0x16, 0x57, 0x01, 0x04, 0x3E, 0x02, 0x00,
	0x41, 0x3F, 0x00, 0x02, 0x03, 0x00, 0x53, 0x14, 0x14, 0x03, 0x01, 0x48,
	0x0E, 0x06, 0x02, 0x30, 0x16, 0x33, 0x4C, 0x58, 0x01, 0x7F, 0x19, 0x0D,
	0x06, 0x04, 0x13, 0x13, 0x04, 0x29, 0x01, 0x20, 0x19, 0x0D, 0x06, 0x16,
	0x13, 0x3A, 0x53, 0x4D, 0x02, 0x00, 0x06, 0x09, 0x02, 0x00, 0x0C, 0x06,
	0x02, 0x2A, 0x16, 0x04, 0x02, 0x03, 0x00, 0x3F, 0x04, 0x0D, 0x01, 0x21,
	0x19, 0x0D, 0x06, 0x04, 0x13, 0x3A, 0x04, 0x03, 0x30, 0x16, 0x13, 0x5D,
	0x02, 0x00, 0x05, 0x02, 0x30, 0x16, 0x02, 0x00, 0x02, 0x01, 0x1D, 0x00,
	0x02, 0x53, 0x4B, 0x05, 0x02, 0x30, 0x16, 0x5B, 0x15, 0x06, 0x07, 0x5D,
	0x01, 0x7F, 0x03, 0x01, 0x04, 0x16, 0x46, 0x15, 0x06, 0x10, 0x01, 0x00,
	0x03, 0x01, 0x14, 0x06, 0x03, 0x4D, 0x04, 0x02, 0x01, 0x00, 0x03, 0x00,
	0x04, 0x02, 0x30, 0x16, 0x3F, 0x57, 0x01, 0x04, 0x3E, 0x53, 0x02, 0x01,
	0x06, 0x03, 0x43, 0x04, 0x03, 0x02, 0x00, 0x40, 0x3F, 0x5D, 0x02, 0x01,
	0x06, 0x03, 0x32, 0x04, 0x01, 0x31, 0x00, 0x00, 0x54, 0x57, 0x01, 0x02,
	0x3E, 0x55, 0x06, 0x02, 0x30, 0x16, 0x57, 0x01, 0x02, 0x3E, 0x44, 0x3F,
	0x00, 0x07, 0x35, 0x50, 0x14, 0x05, 0x02, 0x2F, 0x16, 0x23, 0x01, 0x03,
	0x0B, 0x33, 0x17, 0x47, 0x07, 0x03, 0x00, 0x4F, 0x4F, 0x35, 0x4E, 0x14,
	0x14, 0x03, 0x01, 0x03, 0x02, 0x51, 0x14, 0x03, 0x03, 0x02, 0x02, 0x07,
	0x14, 0x03, 0x02, 0x51, 0x14, 0x03, 0x04, 0x02, 0x02, 0x07, 0x14, 0x03,
	0x02, 0x51, 0x14, 0x03, 0x05, 0x02, 0x02, 0x07, 0x14, 0x03, 0x02, 0x51,
	0x03, 0x06, 0x02, 0x00, 0x02, 0x01, 0x02, 0x03, 0x02, 0x04, 0x02, 0x05,
	0x02, 0x06, 0x1E, 0x00, 0x00, 0x19, 0x19, 0x00, 0x00, 0x01, 0x0B, 0x00,
	0x00, 0x01, 0x00, 0x20, 0x14, 0x06, 0x08, 0x01, 0x01, 0x21, 0x20, 0x22,
	0x20, 0x04, 0x75, 0x13, 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INT2(3 * BR_X509_BUFSIZE_KEY), 0x00, 0x01, 0x01, 0x87, 0xFF, 0xFF,
	0x7F, 0x54, 0x57, 0x01, 0x02, 0x3E, 0x55, 0x01, 0x01, 0x0E, 0x06, 0x02,
	0x30, 0x16, 0x57, 0x01, 0x02, 0x19, 0x0D, 0x06, 0x06, 0x13, 0x3B, 0x44,
	0x32, 0x04, 0x1C, 0x01, 0x04, 0x19, 0x0D, 0x06, 0x08, 0x13, 0x3B, 0x01,
	0x00, 0x41, 0x31, 0x04, 0x0E, 0x01, 0x10, 0x19, 0x0D, 0x06, 0x05, 0x13,
	0x3A, 0x42, 0x04, 0x03, 0x30, 0x16, 0x13, 0x03, 0x00, 0x3F, 0x02, 0x00,
	0x34, 0x1F, 0x5A, 0x27, 0x16, 0x00, 0x01, 0x45, 0x0A, 0x06, 0x02, 0x29,
	0x16, 0x14, 0x03, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x57, 0x01, 0x06,
	0x3E, 0x56, 0x00, 0x00, 0x20, 0x14, 0x06, 0x07, 0x1A, 0x14, 0x06, 0x01,
	0x12, 0x04, 0x76, 0x24, 0x00, 0x00, 0x4B, 0x05, 0x02, 0x30, 0x16, 0x37,
	0x15, 0x06, 0x04, 0x01, 0x17, 0x04, 0x12, 0x38, 0x15, 0x06, 0x04, 0x01,
	0x18, 0x04, 0x0A, 0x39, 0x15, 0x06, 0x04, 0x01, 0x19, 0x04, 0x02, 0x30,
	0x16, 0x00, 0x00, 0x1C, 0x57, 0x01, 0x02, 0x3E, 0x09, 0x50, 0x00, 0x00,
	0x35, 0x4E, 0x13, 0x00, 0x03, 0x14, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02,
	0x53, 0x59, 0x14, 0x01, 0x81, 0x00, 0x0F, 0x06, 0x02, 0x2E, 0x16, 0x14,
	0x01, 0x00, 0x0D, 0x06, 0x0B, 0x13, 0x14, 0x05, 0x04, 0x13, 0x01, 0x00,
	0x00, 0x59, 0x04, 0x6F, 0x02, 0x01, 0x14, 0x05, 0x02, 0x2B, 0x16, 0x23,
	0x03, 0x01, 0x02, 0x02, 0x1F, 0x02, 0x02, 0x22, 0x03, 0x02, 0x14, 0x06,
	0x03, 0x59, 0x04, 0x68, 0x13, 0x02, 0x00, 0x02, 0x01, 0x08, 0x00, 0x00,
	0x14, 0x35, 0x1C, 0x08, 0x20, 0x1C, 0x07, 0x20, 0x4E, 0x00, 0x01, 0x59,
	0x14, 0x01, 0x81, 0x00, 0x0A, 0x06, 0x01, 0x00, 0x01, 0x81, 0x00, 0x08,
	0x14, 0x05, 0x02, 0x28, 0x16, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01,
	0x00, 0x0E, 0x06, 0x19, 0x02, 0x00, 0x23, 0x03, 0x00, 0x14, 0x01, 0x83,
	0xFF, 0xFF, 0x7F, 0x0E, 0x06, 0x02, 0x29, 0x16, 0x01, 0x08, 0x0B, 0x20,
	0x59, 0x1C, 0x07, 0x04, 0x60, 0x00, 0x00, 0x52, 0x4A, 0x00, 0x00, 0x57,
	0x3C, 0x53, 0x00, 0x01, 0x53, 0x14, 0x05, 0x02, 0x2E, 0x16, 0x59, 0x14,
	0x01, 0x81, 0x00, 0x0F, 0x06, 0x02, 0x2E, 0x16, 0x03, 0x00, 0x14, 0x06,
	0x16, 0x59, 0x02, 0x00, 0x14, 0x01, 0x87, 0xFF, 0xFF, 0x7F, 0x0F, 0x06,
	0x02, 0x2E, 0x16, 0x01, 0x08, 0x0B, 0x07, 0x03, 0x00, 0x04, 0x67, 0x13,
	0x02, 0x00, 0x00, 0x00, 0x53, 0x14, 0x01, 0x81, 0x7F, 0x0E, 0x06, 0x08,
	0x5C, 0x01, 0x00, 0x36, 0x1F, 0x01, 0x00, 0x00, 0x14, 0x36, 0x1F, 0x36,
	0x22, 0x4C, 0x01, 0x7F, 0x00, 0x01, 0x59, 0x03, 0x00, 0x02, 0x00, 0x01,
	0x05, 0x10, 0x01, 0x01, 0x11, 0x18, 0x02, 0x00, 0x01, 0x06, 0x10, 0x14,
	0x01, 0x01, 0x11, 0x06, 0x02, 0x25, 0x16, 0x01, 0x04, 0x0B, 0x02, 0x00,
	0x01, 0x1F, 0x11, 0x14, 0x01, 0x1F, 0x0D, 0x06, 0x02, 0x26, 0x16, 0x07,
	0x00, 0x00, 0x14, 0x05, 0x05, 0x01, 0x00, 0x01, 0x7F, 0x00, 0x57, 0x00,
	0x00, 0x14, 0x05, 0x02, 0x29, 0x16, 0x23, 0x5A, 0x00, 0x00, 0x1B, 0x14,
	0x01, 0x00, 0x0F, 0x06, 0x01, 0x00, 0x13, 0x12, 0x04, 0x74, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x5D, 0x13, 0x00, 0x00, 0x14, 0x06, 0x07, 0x5E, 0x14,
	0x06, 0x01, 0x12, 0x04, 0x76, 0x00, 0x00, 0x01, 0x00, 0x19, 0x1A, 0x09,
	0x24, 0x00
};

 inline static const uint16_t _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_caddr[] = {
	0,
	5,
	10,
	14,
	18,
	22,
	26,
	30,
	34,
	38,
	42,
	46,
	50,
	54,
	58,
	62,
	66,
	70,
	75,
	80,
	84,
	89,
	93,
	97,
	101,
	107,
	113,
	118,
	126,
	134,
	140,
	163,
	244,
	311,
	329,
	404,
	408,
	412,
	429,
	434,
	505,
	519,
	526,
	540,
	573,
	582,
	587,
	654,
	665,
	721,
	725,
	730,
	778,
	804,
	848,
	859,
	868,
	881,
	885,
	889,
	901
};

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INTERPRETED   34

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_codeblock[_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_caddr[(slot) - _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INTERPRETED]]; \
		t0_lnum = _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *t0ctx = (_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)ctx; \
	t0ctx->ip = &_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_codeblock[0]; \
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_DEFENTRY(br_skey_decoder_init_main, 73)

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

 inline void
br_skey_decoder_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_LOCAL(x)    (*(rp - 2 - (x)))
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP()       (*-- dp)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi()      (*(int32_t *)(-- dp))
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PEEK(x)     (*(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_RPOP()      (*-- rp)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_RPOPi()     (*(int32_t *)(-- rp))
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PEEK(t0depth)); \
} while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_RET()        goto t0_next

	dp = ((_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)t0ctx)->dp;
	rp = ((_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)t0ctx)->rp;
	ip = ((_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_NEXT(&ip);
		if (t0x < _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSHi(_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_LOCAL(_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_LOCAL(_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_unsigned(&ip)) = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
				break;
			case 4: /* jump */
				t0off = _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_signed(&ip);
				if (_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = _opt_libs_BearSSL_src_x509_skey_decoder_c_t0_parse7E_signed(&ip);
				if (!_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* + */

	uint32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(a + b);

				}
				break;
			case 8: {
				/* - */

	uint32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(a - b);

				}
				break;
			case 9: {
				/* -rot */
 _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_NROT(); 
				}
				break;
			case 10: {
				/* < */

	int32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 11: {
				/* << */

	int c = (int)_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	uint32_t x = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(x << c);

				}
				break;
			case 12: {
				/* <> */

	uint32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 13: {
				/* = */

	uint32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 14: {
				/* > */

	int32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 15: {
				/* >= */

	int32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 16: {
				/* >> */

	int c = (int)_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	int32_t x = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSHi(x >> c);

				}
				break;
			case 17: {
				/* and */

	uint32_t b = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(a & b);

				}
				break;
			case 18: {
				/* co */
 _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_CO(); 
				}
				break;
			case 19: {
				/* drop */
 (void)_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP(); 
				}
				break;
			case 20: {
				/* dup */
 _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PEEK(0)); 
				}
				break;
			case 21: {
				/* eqOID */

	const unsigned char *a2 = &_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_datablock[_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP()];
	const unsigned char *a1 = &_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->pad[0];
	size_t len = a1[0];
	int x;
	if (len == a2[0]) {
		x = -(memcmp(a1 + 1, a2 + 1, len) == 0);
	} else {
		x = 0;
	}
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH((uint32_t)x);

				}
				break;
			case 22: {
				/* fail */

	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->err = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_CO();

				}
				break;
			case 23: {
				/* get8 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(*((unsigned char *)_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX + addr));

				}
				break;
			case 24: {
				/* neg */

	uint32_t a = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(-a);

				}
				break;
			case 25: {
				/* over */
 _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PEEK(1)); 
				}
				break;
			case 26: {
				/* read-blob-inner */

	uint32_t len = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t addr = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	size_t clen = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hlen;
	if (clen > len) {
		clen = (size_t)len;
	}
	if (addr != 0) {
		memcpy((unsigned char *)_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX + addr, _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hbuf, clen);
	}
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hbuf += clen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hlen -= clen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(addr + clen);
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(len - clen);

				}
				break;
			case 27: {
				/* read8-low */

	if (_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hlen == 0) {
		_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSHi(-1);
	} else {
		_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hlen --;
		_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(*_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->hbuf ++);
	}

				}
				break;
			case 28: {
				/* rot */
 _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_ROT(); 
				}
				break;
			case 29: {
				/* set-ec-key */

	size_t xlen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t curve = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.ec.curve = curve;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.ec.x = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key_data;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.ec.xlen = xlen;

				}
				break;
			case 30: {
				/* set-rsa-key */

	size_t iqlen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	size_t dqlen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	size_t dplen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	size_t qlen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	size_t plen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	uint32_t n_bitlen = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	size_t off;

	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.n_bitlen = n_bitlen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.p = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key_data;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.plen = plen;
	off = plen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.q = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key_data + off;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.qlen = qlen;
	off += qlen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.dp = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key_data + off;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.dplen = dplen;
	off += dplen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.dq = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key_data + off;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.dqlen = dqlen;
	off += dqlen;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.iq = _opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key_data + off;
	_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX->key.rsa.iqlen = iqlen;

				}
				break;
			case 31: {
				/* set8 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	*((unsigned char *)_opt_libs_BearSSL_src_x509_skey_decoder_c_CTX + addr) = (unsigned char)_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();

				}
				break;
			case 32: {
				/* swap */
 _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_SWAP(); 
				}
				break;
			case 33: {
				/* u>> */

	int c = (int)_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POPi();
	uint32_t x = _opt_libs_BearSSL_src_x509_skey_decoder_c_T0_POP();
	_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_PUSH(x >> c);

				}
				break;
			}

		} else {
			_opt_libs_BearSSL_src_x509_skey_decoder_c_T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)t0ctx)->dp = dp;
	((_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)t0ctx)->rp = rp;
	((_opt_libs_BearSSL_src_x509_skey_decoder_c_t0_context *)t0ctx)->ip = ip;
}
#endif
