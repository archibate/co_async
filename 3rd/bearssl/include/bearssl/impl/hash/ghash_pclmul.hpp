#ifndef BEARSSL_IMPL_HASH_GHASH_PCLMUL_HPP
#define BEARSSL_IMPL_HASH_GHASH_PCLMUL_HPP

/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
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

/*
 * This is the GHASH implementation that leverages the pclmulqdq opcode
 * (from the AES-NI instructions).
 */

#if BR_AES_X86NI

#if BR_AES_X86NI_GCC
#if BR_AES_X86NI_GCC_OLD
#pragma GCC push_options
#pragma GCC target("sse2,ssse3,pclmul")
#pragma GCC diagnostic ignored "-Wpsabi"
#endif
#include <tmmintrin.h>
#include <wmmintrin.h>
#include <cpuid.h>
#endif

#if BR_AES_X86NI_MSC
#include <intrin.h>
#endif

/*
 * GHASH is defined over elements of GF(2^128) with "full little-endian"
 * representation: leftmost byte is least significant, and, within each
 * byte, leftmost _bit_ is least significant. The natural ordering in
 * x86 is "mixed little-endian": bytes are ordered from least to most
 * significant, but bits within a byte are in most-to-least significant
 * order. Going to full little-endian representation would require
 * reversing bits within each byte, which is doable but expensive.
 *
 * Instead, we go to full big-endian representation, by swapping bytes
 * around, which is done with a single _mm_shuffle_epi8() opcode (it
 * comes with SSSE3; all CPU that offer pclmulqdq also have SSSE3). We
 * can use a full big-endian representation because in a carryless
 * multiplication, we have a nice bit reversal property:
 *
 *    rev_128(x) * rev_128(y) = rev_255(x * y)
 *
 * So by using full big-endian, we still get the right result, except
 * that it is right-shifted by 1 bit. The left-shift is relatively
 * inexpensive, and it can be mutualised.
 *
 *
 * Since SSE2 opcodes do not have facilities for shitfting full 128-bit
 * values with bit precision, we have to break down values into 64-bit
 * chunks. We number chunks from 0 to 3 in left to right order.
 */

/*
 * From a 128-bit value kw, compute kx as the XOR of the two 64-bit
 * halves of kw (into the right half of kx; left half is unspecified).
 */
#define _opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(kw, kx)   do { \
		kx = _mm_xor_si128(kw, _mm_shuffle_epi32(kw, 0x0E)); \
	} while (0)

/*
 * Combine two 64-bit values (k0:k1) into a 128-bit (kw) value and
 * the XOR of the two values (kx).
 */
#define _opt_libs_BearSSL_src_hash_ghash_pclmul_c_PBK(k0, k1, kw, kx)   do { \
		kw = _mm_unpacklo_epi64(k1, k0); \
		kx = _mm_xor_si128(k0, k1); \
	} while (0)

/*
 * Left-shift by 1 bit a 256-bit value (in four 64-bit words).
 */
#define _opt_libs_BearSSL_src_hash_ghash_pclmul_c_SL_256(x0, x1, x2, x3)   do { \
		x0 = _mm_or_si128( \
			_mm_slli_epi64(x0, 1), \
			_mm_srli_epi64(x1, 63)); \
		x1 = _mm_or_si128( \
			_mm_slli_epi64(x1, 1), \
			_mm_srli_epi64(x2, 63)); \
		x2 = _mm_or_si128( \
			_mm_slli_epi64(x2, 1), \
			_mm_srli_epi64(x3, 63)); \
		x3 = _mm_slli_epi64(x3, 1); \
	} while (0)

/*
 * Perform reduction in GF(2^128). The 256-bit value is in x0..x3;
 * result is written in x0..x1.
 */
#define _opt_libs_BearSSL_src_hash_ghash_pclmul_c_REDUCE_F128(x0, x1, x2, x3)   do { \
		x1 = _mm_xor_si128( \
			x1, \
			_mm_xor_si128( \
				_mm_xor_si128( \
					x3, \
					_mm_srli_epi64(x3, 1)), \
				_mm_xor_si128( \
					_mm_srli_epi64(x3, 2), \
					_mm_srli_epi64(x3, 7)))); \
		x2 = _mm_xor_si128( \
			_mm_xor_si128( \
				x2, \
				_mm_slli_epi64(x3, 63)), \
			_mm_xor_si128( \
				_mm_slli_epi64(x3, 62), \
				_mm_slli_epi64(x3, 57))); \
		x0 = _mm_xor_si128( \
			x0, \
			_mm_xor_si128( \
				_mm_xor_si128( \
					x2, \
					_mm_srli_epi64(x2, 1)), \
				_mm_xor_si128( \
					_mm_srli_epi64(x2, 2), \
					_mm_srli_epi64(x2, 7)))); \
		x1 = _mm_xor_si128( \
			_mm_xor_si128( \
				x1, \
				_mm_slli_epi64(x2, 63)), \
			_mm_xor_si128( \
				_mm_slli_epi64(x2, 62), \
				_mm_slli_epi64(x2, 57))); \
	} while (0)

/*
 * Square value kw into (dw,dx).
 */
#define _opt_libs_BearSSL_src_hash_ghash_pclmul_c_SQUARE_F128(kw, dw, dx)   do { \
		__m128i z0, z1, z2, z3; \
		z1 = _mm_clmulepi64_si128(kw, kw, 0x11); \
		z3 = _mm_clmulepi64_si128(kw, kw, 0x00); \
		z0 = _mm_shuffle_epi32(z1, 0x0E); \
		z2 = _mm_shuffle_epi32(z3, 0x0E); \
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_SL_256(z0, z1, z2, z3); \
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_REDUCE_F128(z0, z1, z2, z3); \
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_PBK(z0, z1, dw, dx); \
	} while (0)

/* see bearssl_hash.h */
BR_TARGET("ssse3,pclmul")
void
br_ghash_pclmul(void *y, const void *h, const void *data, size_t len)
{
	const unsigned char *buf1, *buf2;
	unsigned char tmp[64];
	size_t num4, num1;
	__m128i yw, h1w, h1x;
	__m128i byteswap_index;

	/*
	 * We split data into two chunks. First chunk starts at buf1
	 * and contains num4 blocks of 64-byte values. Second chunk
	 * starts at buf2 and contains num1 blocks of 16-byte values.
	 * We want the first chunk to be as large as possible.
	 */
	buf1 = (unsigned char *)data;
	num4 = len >> 6;
	len &= 63;
	buf2 = buf1 + (num4 << 6);
	num1 = (len + 15) >> 4;
	if ((len & 15) != 0) {
		memcpy(tmp, buf2, len);
		memset(tmp + len, 0, (num1 << 4) - len);
		buf2 = tmp;
	}

	/*
	 * Constant value to perform endian conversion.
	 */
	byteswap_index = _mm_set_epi8(
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

	/*
	 * Load y and h.
	 */
	yw = _mm_loadu_si128((__m128i const *)y);
	h1w = _mm_loadu_si128((__m128i const *)h);
	yw = _mm_shuffle_epi8(yw, byteswap_index);
	h1w = _mm_shuffle_epi8(h1w, byteswap_index);
	_opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(h1w, h1x);

	if (num4 > 0) {
		__m128i h2w, h2x, h3w, h3x, h4w, h4x;
		__m128i t0, t1, t2, t3;

		/*
		 * Compute h2 = h^2.
		 */
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_SQUARE_F128(h1w, h2w, h2x);

		/*
		 * Compute h3 = h^3 = h*(h^2).
		 */
		t1 = _mm_clmulepi64_si128(h1w, h2w, 0x11);
		t3 = _mm_clmulepi64_si128(h1w, h2w, 0x00);
		t2 = _mm_xor_si128(_mm_clmulepi64_si128(h1x, h2x, 0x00),
			_mm_xor_si128(t1, t3));
		t0 = _mm_shuffle_epi32(t1, 0x0E);
		t1 = _mm_xor_si128(t1, _mm_shuffle_epi32(t2, 0x0E));
		t2 = _mm_xor_si128(t2, _mm_shuffle_epi32(t3, 0x0E));
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_SL_256(t0, t1, t2, t3);
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_REDUCE_F128(t0, t1, t2, t3);
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_PBK(t0, t1, h3w, h3x);

		/*
		 * Compute h4 = h^4 = (h^2)^2.
		 */
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_SQUARE_F128(h2w, h4w, h4x);

		while (num4 -- > 0) {
			__m128i aw0, aw1, aw2, aw3;
			__m128i ax0, ax1, ax2, ax3;

			aw0 = _mm_loadu_si128((const __m128i *)(buf1 +  0));
			aw1 = _mm_loadu_si128((const __m128i *)(buf1 + 16));
			aw2 = _mm_loadu_si128((const __m128i *)(buf1 + 32));
			aw3 = _mm_loadu_si128((const __m128i *)(buf1 + 48));
			aw0 = _mm_shuffle_epi8(aw0, byteswap_index);
			aw1 = _mm_shuffle_epi8(aw1, byteswap_index);
			aw2 = _mm_shuffle_epi8(aw2, byteswap_index);
			aw3 = _mm_shuffle_epi8(aw3, byteswap_index);
			buf1 += 64;

			aw0 = _mm_xor_si128(aw0, yw);
			_opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(aw1, ax1);
			_opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(aw2, ax2);
			_opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(aw3, ax3);
			_opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(aw0, ax0);

			t1 = _mm_xor_si128(
				_mm_xor_si128(
					_mm_clmulepi64_si128(aw0, h4w, 0x11),
					_mm_clmulepi64_si128(aw1, h3w, 0x11)),
				_mm_xor_si128(
					_mm_clmulepi64_si128(aw2, h2w, 0x11),
					_mm_clmulepi64_si128(aw3, h1w, 0x11)));
			t3 = _mm_xor_si128(
				_mm_xor_si128(
					_mm_clmulepi64_si128(aw0, h4w, 0x00),
					_mm_clmulepi64_si128(aw1, h3w, 0x00)),
				_mm_xor_si128(
					_mm_clmulepi64_si128(aw2, h2w, 0x00),
					_mm_clmulepi64_si128(aw3, h1w, 0x00)));
			t2 = _mm_xor_si128(
				_mm_xor_si128(
					_mm_clmulepi64_si128(ax0, h4x, 0x00),
					_mm_clmulepi64_si128(ax1, h3x, 0x00)),
				_mm_xor_si128(
					_mm_clmulepi64_si128(ax2, h2x, 0x00),
					_mm_clmulepi64_si128(ax3, h1x, 0x00)));
			t2 = _mm_xor_si128(t2, _mm_xor_si128(t1, t3));
			t0 = _mm_shuffle_epi32(t1, 0x0E);
			t1 = _mm_xor_si128(t1, _mm_shuffle_epi32(t2, 0x0E));
			t2 = _mm_xor_si128(t2, _mm_shuffle_epi32(t3, 0x0E));
			_opt_libs_BearSSL_src_hash_ghash_pclmul_c_SL_256(t0, t1, t2, t3);
			_opt_libs_BearSSL_src_hash_ghash_pclmul_c_REDUCE_F128(t0, t1, t2, t3);
			yw = _mm_unpacklo_epi64(t1, t0);
		}
	}

	while (num1 -- > 0) {
		__m128i aw, ax;
		__m128i t0, t1, t2, t3;

		aw = _mm_loadu_si128((__m128i const *)buf2);
		aw = _mm_shuffle_epi8(aw, byteswap_index);
		buf2 += 16;

		aw = _mm_xor_si128(aw, yw);
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_BK(aw, ax);

		t1 = _mm_clmulepi64_si128(aw, h1w, 0x11);
		t3 = _mm_clmulepi64_si128(aw, h1w, 0x00);
		t2 = _mm_clmulepi64_si128(ax, h1x, 0x00);
		t2 = _mm_xor_si128(t2, _mm_xor_si128(t1, t3));
		t0 = _mm_shuffle_epi32(t1, 0x0E);
		t1 = _mm_xor_si128(t1, _mm_shuffle_epi32(t2, 0x0E));
		t2 = _mm_xor_si128(t2, _mm_shuffle_epi32(t3, 0x0E));
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_SL_256(t0, t1, t2, t3);
		_opt_libs_BearSSL_src_hash_ghash_pclmul_c_REDUCE_F128(t0, t1, t2, t3);
		yw = _mm_unpacklo_epi64(t1, t0);
	}

	yw = _mm_shuffle_epi8(yw, byteswap_index);
	_mm_storeu_si128((__m128i *)y, yw);
}

/*
 * Test CPU support for PCLMULQDQ.
 */
 inline  int
_opt_libs_BearSSL_src_hash_ghash_pclmul_c_pclmul_supported(void)
{
	/*
	 * Bit mask for features in ECX:
	 *    1   PCLMULQDQ support
	 */
#define _opt_libs_BearSSL_src_hash_ghash_pclmul_c_MASK   0x00000002

#if BR_AES_X86NI_GCC
	unsigned eax, ebx, ecx, edx;

	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
		return (ecx & _opt_libs_BearSSL_src_hash_ghash_pclmul_c_MASK) == _opt_libs_BearSSL_src_hash_ghash_pclmul_c_MASK;
	} else {
		return 0;
	}
#elif BR_AES_X86NI_MSC
	int info[4];

	__cpuid(info, 1);
	return ((uint32_t)info[2] & _opt_libs_BearSSL_src_hash_ghash_pclmul_c_MASK) == _opt_libs_BearSSL_src_hash_ghash_pclmul_c_MASK;
#else
	return 0;
#endif

#undef _opt_libs_BearSSL_src_hash_ghash_pclmul_c_MASK
}

/* see bearssl_hash.h */
 inline br_ghash
br_ghash_pclmul_get(void)
{
	return _opt_libs_BearSSL_src_hash_ghash_pclmul_c_pclmul_supported() ? &br_ghash_pclmul : 0;
}

#if BR_AES_X86NI_GCC && BR_AES_X86NI_GCC_OLD
#pragma GCC pop_options
#endif

#else

/* see bearssl_hash.h */
br_ghash
br_ghash_pclmul_get(void)
{
	return 0;
}

#endif
#endif
