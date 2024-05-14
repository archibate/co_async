#ifndef BEARSSL_IMPL_HASH_MD5_HPP
#define BEARSSL_IMPL_HASH_MD5_HPP

/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "inner.h"

#define _opt_libs_BearSSL_src_hash_md5_c_F(B, C, D)     ((((C) ^ (D)) & (B)) ^ (D))
#define _opt_libs_BearSSL_src_hash_md5_c_G(B, C, D)     ((((C) ^ (B)) & (D)) ^ (C))
#define _opt_libs_BearSSL_src_hash_md5_c_H(B, C, D)     ((B) ^ (C) ^ (D))
#define _opt_libs_BearSSL_src_hash_md5_c_I(B, C, D)     ((C) ^ ((B) | ~(D)))

#define _opt_libs_BearSSL_src_hash_md5_c_ROTL(x, n)    (((x) << (n)) | ((x) >> (32 - (n))))

/* see inner.h */
 inline const uint32_t br_md5_IV[4] = {
	0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
};

 inline static const uint32_t _opt_libs_BearSSL_src_hash_md5_c_K[64] = {
	0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
	0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
	0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
	0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,

	0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
	0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
	0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
	0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,

	0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
	0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
	0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
	0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,

	0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
	0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
	0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
	0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

 inline static const unsigned char _opt_libs_BearSSL_src_hash_md5_c_MP[48] = {
	1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
	5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
	0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
};

/* see inner.h */
 inline void
br_md5_round(const unsigned char *buf, uint32_t *val)
{
	uint32_t m[16];
	uint32_t a, b, c, d;
	int i;

	a = val[0];
	b = val[1];
	c = val[2];
	d = val[3];
	/* obsolete
	for (i = 0; i < 16; i ++) {
		m[i] = br_dec32le(buf + (i << 2));
	}
	*/
	br_range_dec32le(m, 16, buf);

	for (i = 0; i < 16; i += 4) {
		a = b + _opt_libs_BearSSL_src_hash_md5_c_ROTL(a + _opt_libs_BearSSL_src_hash_md5_c_F(b, c, d) + m[i + 0] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 0],  7);
		d = a + _opt_libs_BearSSL_src_hash_md5_c_ROTL(d + _opt_libs_BearSSL_src_hash_md5_c_F(a, b, c) + m[i + 1] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 1], 12);
		c = d + _opt_libs_BearSSL_src_hash_md5_c_ROTL(c + _opt_libs_BearSSL_src_hash_md5_c_F(d, a, b) + m[i + 2] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 2], 17);
		b = c + _opt_libs_BearSSL_src_hash_md5_c_ROTL(b + _opt_libs_BearSSL_src_hash_md5_c_F(c, d, a) + m[i + 3] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 3], 22);
	}
	for (i = 16; i < 32; i += 4) {
		a = b + _opt_libs_BearSSL_src_hash_md5_c_ROTL(a + _opt_libs_BearSSL_src_hash_md5_c_G(b, c, d) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 16]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 0],  5);
		d = a + _opt_libs_BearSSL_src_hash_md5_c_ROTL(d + _opt_libs_BearSSL_src_hash_md5_c_G(a, b, c) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 15]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 1],  9);
		c = d + _opt_libs_BearSSL_src_hash_md5_c_ROTL(c + _opt_libs_BearSSL_src_hash_md5_c_G(d, a, b) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 14]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 2], 14);
		b = c + _opt_libs_BearSSL_src_hash_md5_c_ROTL(b + _opt_libs_BearSSL_src_hash_md5_c_G(c, d, a) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 13]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 3], 20);
	}
	for (i = 32; i < 48; i += 4) {
		a = b + _opt_libs_BearSSL_src_hash_md5_c_ROTL(a + _opt_libs_BearSSL_src_hash_md5_c_H(b, c, d) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 16]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 0],  4);
		d = a + _opt_libs_BearSSL_src_hash_md5_c_ROTL(d + _opt_libs_BearSSL_src_hash_md5_c_H(a, b, c) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 15]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 1], 11);
		c = d + _opt_libs_BearSSL_src_hash_md5_c_ROTL(c + _opt_libs_BearSSL_src_hash_md5_c_H(d, a, b) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 14]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 2], 16);
		b = c + _opt_libs_BearSSL_src_hash_md5_c_ROTL(b + _opt_libs_BearSSL_src_hash_md5_c_H(c, d, a) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 13]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 3], 23);
	}
	for (i = 48; i < 64; i += 4) {
		a = b + _opt_libs_BearSSL_src_hash_md5_c_ROTL(a + _opt_libs_BearSSL_src_hash_md5_c_I(b, c, d) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 16]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 0],  6);
		d = a + _opt_libs_BearSSL_src_hash_md5_c_ROTL(d + _opt_libs_BearSSL_src_hash_md5_c_I(a, b, c) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 15]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 1], 10);
		c = d + _opt_libs_BearSSL_src_hash_md5_c_ROTL(c + _opt_libs_BearSSL_src_hash_md5_c_I(d, a, b) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 14]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 2], 15);
		b = c + _opt_libs_BearSSL_src_hash_md5_c_ROTL(b + _opt_libs_BearSSL_src_hash_md5_c_I(c, d, a) + m[_opt_libs_BearSSL_src_hash_md5_c_MP[i - 13]] + _opt_libs_BearSSL_src_hash_md5_c_K[i + 3], 21);
	}

	val[0] += a;
	val[1] += b;
	val[2] += c;
	val[3] += d;
}

/* see bearssl.h */
 inline void
br_md5_init(br_md5_context *cc)
{
	cc->vtable = &br_md5_vtable;
	memcpy(cc->val, br_md5_IV, sizeof cc->val);
	cc->count = 0;
}

/* see bearssl.h */
 inline void
br_md5_update(br_md5_context *cc, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = (unsigned char *)data;
	ptr = (size_t)cc->count & 63;
	while (len > 0) {
		size_t clen;

		clen = 64 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(cc->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		cc->count += (uint64_t)clen;
		if (ptr == 64) {
			br_md5_round(cc->buf, cc->val);
			ptr = 0;
		}
	}
}

/* see bearssl.h */
 inline void
br_md5_out(const br_md5_context *cc, void *dst)
{
	unsigned char buf[64];
	uint32_t val[4];
	size_t ptr;

	ptr = (size_t)cc->count & 63;
	memcpy(buf, cc->buf, ptr);
	memcpy(val, cc->val, sizeof val);
	buf[ptr ++] = 0x80;
	if (ptr > 56) {
		memset(buf + ptr, 0, 64 - ptr);
		br_md5_round(buf, val);
		memset(buf, 0, 56);
	} else {
		memset(buf + ptr, 0, 56 - ptr);
	}
	br_enc64le(buf + 56, cc->count << 3);
	br_md5_round(buf, val);
	br_range_enc32le(dst, val, 4);
}

/* see bearssl.h */
 inline uint64_t
br_md5_state(const br_md5_context *cc, void *dst)
{
	br_range_enc32le(dst, cc->val, 4);
	return cc->count;
}

/* see bearssl.h */
 inline void
br_md5_set_state(br_md5_context *cc, const void *stb, uint64_t count)
{
	br_range_dec32le(cc->val, 4, stb);
	cc->count = count;
}

/* see bearssl.h */
 inline const br_hash_class br_md5_vtable = {
	sizeof(br_md5_context),
	BR_HASHDESC_ID(br_md5_ID)
		| BR_HASHDESC_OUT(16)
		| BR_HASHDESC_STATE(16)
		| BR_HASHDESC_LBLEN(6)
		| BR_HASHDESC_MD_PADDING,
	(void (*)(const br_hash_class **))&br_md5_init,
	(void (*)(const br_hash_class **, const void *, size_t))&br_md5_update,
	(void (*)(const br_hash_class *const *, void *))&br_md5_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_md5_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_md5_set_state
};
#endif
