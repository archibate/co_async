#ifndef BEARSSL_IMPL_HASH_DIG_OID_HPP
#define BEARSSL_IMPL_HASH_DIG_OID_HPP

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

/*
 * This file contains the encoded OID for the standard hash functions.
 * Such OID appear in, for instance, the PKCS#1 v1.5 padding for RSA
 * signatures.
 */

 inline static const unsigned char _opt_libs_BearSSL_src_hash_dig_oid_c_md5_OID[] = {
	0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05
};

 inline static const unsigned char _opt_libs_BearSSL_src_hash_dig_oid_c_sha1_OID[] = {
	0x2B, 0x0E, 0x03, 0x02, 0x1A
};

 inline static const unsigned char _opt_libs_BearSSL_src_hash_dig_oid_c_sha224_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04
};

 inline static const unsigned char _opt_libs_BearSSL_src_hash_dig_oid_c_sha256_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01
};

 inline static const unsigned char _opt_libs_BearSSL_src_hash_dig_oid_c_sha384_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02
};

 inline static const unsigned char _opt_libs_BearSSL_src_hash_dig_oid_c_sha512_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03
};

/* see inner.h */
 inline const unsigned char *
br_digest_OID(int digest_id, size_t *len)
{
	switch (digest_id) {
	case br_md5_ID:
		*len = sizeof _opt_libs_BearSSL_src_hash_dig_oid_c_md5_OID;
		return _opt_libs_BearSSL_src_hash_dig_oid_c_md5_OID;
	case br_sha1_ID:
		*len = sizeof _opt_libs_BearSSL_src_hash_dig_oid_c_sha1_OID;
		return _opt_libs_BearSSL_src_hash_dig_oid_c_sha1_OID;
	case br_sha224_ID:
		*len = sizeof _opt_libs_BearSSL_src_hash_dig_oid_c_sha224_OID;
		return _opt_libs_BearSSL_src_hash_dig_oid_c_sha224_OID;
	case br_sha256_ID:
		*len = sizeof _opt_libs_BearSSL_src_hash_dig_oid_c_sha256_OID;
		return _opt_libs_BearSSL_src_hash_dig_oid_c_sha256_OID;
	case br_sha384_ID:
		*len = sizeof _opt_libs_BearSSL_src_hash_dig_oid_c_sha384_OID;
		return _opt_libs_BearSSL_src_hash_dig_oid_c_sha384_OID;
	case br_sha512_ID:
		*len = sizeof _opt_libs_BearSSL_src_hash_dig_oid_c_sha512_OID;
		return _opt_libs_BearSSL_src_hash_dig_oid_c_sha512_OID;
	default:
		*len = 0;
		return NULL;
	}
}
#endif
