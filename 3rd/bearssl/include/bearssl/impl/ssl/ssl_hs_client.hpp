#ifndef BEARSSL_IMPL_SSL_SSL_HS_CLIENT_HPP
#define BEARSSL_IMPL_SSL_SSL_HS_CLIENT_HPP

/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context;

 inline  uint32_t
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_unsigned(const unsigned char **p)
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
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_signed(const unsigned char **p)
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

#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(x)       _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(x)       _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT3(x)       _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT4(x)       _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT5(x)       _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_SBYTE(x), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_FBYTE(x, 0)

/* static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_datablock[]; */


void br_ssl_hs_client_init_main(void *t0ctx);

void br_ssl_hs_client_run(void *t0ctx);



#include <stddef.h>
#include <string.h>

#include "inner.h"

/*
 * This macro evaluates to a pointer to the current engine context.
 */
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG  ((br_ssl_engine_context *)((unsigned char *)t0ctx - offsetof(br_ssl_engine_context, cpu)))





/*
 * This macro evaluates to a pointer to the client context, under that
 * specific name. It must be noted that since the engine context is the
 * first field of the br_ssl_client_context structure ('eng'), then
 * pointers values of both types are interchangeable, modulo an
 * appropriate cast. This also means that "adresses" computed as offsets
 * within the structure work for both kinds of context.
 */
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX  ((br_ssl_client_context *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG)

/*
 * Generate the pre-master secret for RSA key exchange, and encrypt it
 * with the server's public key. Returned value is either the encrypted
 * data length (in bytes), or -x on error, with 'x' being an error code.
 *
 * This code assumes that the public key has been already verified (it
 * was properly obtained by the X.509 engine, and it has the right type,
 * i.e. it is of type RSA and suitable for encryption).
 */
 inline  int
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_pms_rsa(br_ssl_client_context *ctx, int prf_id)
{
	const br_x509_class **xc;
	const br_x509_pkey *pk;
	const unsigned char *n;
	unsigned char *pms;
	size_t nlen, u;

	xc = ctx->eng.x509ctx;
	pk = (*xc)->get_pkey(xc, NULL);

	/*
	 * Compute actual RSA key length, in case there are leading zeros.
	 */
	n = pk->key.rsa.n;
	nlen = pk->key.rsa.nlen;
	while (nlen > 0 && *n == 0) {
		n ++;
		nlen --;
	}

	/*
	 * We need at least 59 bytes (48 bytes for pre-master secret, and
	 * 11 bytes for the PKCS#1 type 2 padding). Note that the X.509
	 * minimal engine normally blocks RSA keys shorter than 128 bytes,
	 * so this is mostly for public keys provided explicitly by the
	 * caller.
	 */
	if (nlen < 59) {
		return -BR_ERR_X509_WEAK_PUBLIC_KEY;
	}
	if (nlen > sizeof ctx->eng.pad) {
		return -BR_ERR_LIMIT_EXCEEDED;
	}

	/*
	 * Make PMS.
	 */
	pms = ctx->eng.pad + nlen - 48;
	br_enc16be(pms, ctx->eng.version_max);
	br_hmac_drbg_generate(&ctx->eng.rng, pms + 2, 46);
	br_ssl_engine_compute_master(&ctx->eng, prf_id, pms, 48);

	/*
	 * Apply PKCS#1 type 2 padding.
	 */
	ctx->eng.pad[0] = 0x00;
	ctx->eng.pad[1] = 0x02;
	ctx->eng.pad[nlen - 49] = 0x00;
	br_hmac_drbg_generate(&ctx->eng.rng, ctx->eng.pad + 2, nlen - 51);
	for (u = 2; u < nlen - 49; u ++) {
		while (ctx->eng.pad[u] == 0) {
			br_hmac_drbg_generate(&ctx->eng.rng,
				&ctx->eng.pad[u], 1);
		}
	}

	/*
	 * Compute RSA encryption.
	 */
	if (!ctx->irsapub(ctx->eng.pad, nlen, &pk->key.rsa)) {
		return -BR_ERR_LIMIT_EXCEEDED;
	}
	return (int)nlen;
}

/*
 * OID for hash functions in RSA signatures.
 */
 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA1[] = {
	0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A
};

 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA224[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04
};

 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA256[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01
};

 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA384[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02
};

 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA512[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03
};

 inline static const unsigned char *_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID[] = {
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA1,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA224,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA256,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA384,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID_SHA512
};

/*
 * Check the RSA signature on the ServerKeyExchange message.
 *
 *   hash      hash function ID (2 to 6), or 0 for MD5+SHA-1 (with RSA only)
 *   use_rsa   non-zero for RSA signature, zero for ECDSA
 *   sig_len   signature length (in bytes); signature value is in the pad
 *
 * Returned value is 0 on success, or an error code.
 */
 inline  int
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_verify_SKE_sig(br_ssl_client_context *ctx,
	int hash, int use_rsa, size_t sig_len)
{
	const br_x509_class **xc;
	const br_x509_pkey *pk;
	br_multihash_context mhc;
	unsigned char hv[64], head[4];
	size_t hv_len;

	xc = ctx->eng.x509ctx;
	pk = (*xc)->get_pkey(xc, NULL);
	br_multihash_zero(&mhc);
	br_multihash_copyimpl(&mhc, &ctx->eng.mhash);
	br_multihash_init(&mhc);
	br_multihash_update(&mhc,
		ctx->eng.client_random, sizeof ctx->eng.client_random);
	br_multihash_update(&mhc,
		ctx->eng.server_random, sizeof ctx->eng.server_random);
	head[0] = 3;
	head[1] = 0;
	head[2] = ctx->eng.ecdhe_curve;
	head[3] = ctx->eng.ecdhe_point_len;
	br_multihash_update(&mhc, head, sizeof head);
	br_multihash_update(&mhc,
		ctx->eng.ecdhe_point, ctx->eng.ecdhe_point_len);
	if (hash) {
		hv_len = br_multihash_out(&mhc, hash, hv);
		if (hv_len == 0) {
			return BR_ERR_INVALID_ALGORITHM;
		}
	} else {
		if (!br_multihash_out(&mhc, br_md5_ID, hv)
			|| !br_multihash_out(&mhc, br_sha1_ID, hv + 16))
		{
			return BR_ERR_INVALID_ALGORITHM;
		}
		hv_len = 36;
	}
	if (use_rsa) {
		unsigned char tmp[64];
		const unsigned char *hash_oid;

		if (hash) {
			hash_oid = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_HASH_OID[hash - 2];
		} else {
			hash_oid = NULL;
		}
		if (!ctx->eng.irsavrfy(ctx->eng.pad, sig_len,
			hash_oid, hv_len, &pk->key.rsa, tmp)
			|| memcmp(tmp, hv, hv_len) != 0)
		{
			return BR_ERR_BAD_SIGNATURE;
		}
	} else {
		if (!ctx->eng.iecdsa(ctx->eng.iec, hv, hv_len, &pk->key.ec,
			ctx->eng.pad, sig_len))
		{
			return BR_ERR_BAD_SIGNATURE;
		}
	}
	return 0;
}

/*
 * Perform client-side ECDH (or ECDHE). The point that should be sent to
 * the server is written in the pad; returned value is either the point
 * length (in bytes), or -x on error, with 'x' being an error code.
 *
 * The point _from_ the server is taken from ecdhe_point[] if 'ecdhe'
 * is non-zero, or from the X.509 engine context if 'ecdhe' is zero
 * (for static ECDH).
 */
 inline  int
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_pms_ecdh(br_ssl_client_context *ctx, unsigned ecdhe, int prf_id)
{
	int curve;
	unsigned char key[66], point[133];
	const unsigned char *order, *point_src;
	size_t glen, olen, point_len, xoff, xlen;
	unsigned char mask;

	if (ecdhe) {
		curve = ctx->eng.ecdhe_curve;
		point_src = ctx->eng.ecdhe_point;
		point_len = ctx->eng.ecdhe_point_len;
	} else {
		const br_x509_class **xc;
		const br_x509_pkey *pk;

		xc = ctx->eng.x509ctx;
		pk = (*xc)->get_pkey(xc, NULL);
		curve = pk->key.ec.curve;
		point_src = pk->key.ec.q;
		point_len = pk->key.ec.qlen;
	}
	if ((ctx->eng.iec->supported_curves & ((uint32_t)1 << curve)) == 0) {
		return -BR_ERR_INVALID_ALGORITHM;
	}

	/*
	 * We need to generate our key, as a non-zero random value which
	 * is lower than the curve order, in a "large enough" range. We
	 * force top bit to 0 and bottom bit to 1, which guarantees that
	 * the value is in the proper range.
	 */
	order = ctx->eng.iec->order(curve, &olen);
	mask = 0xFF;
	while (mask >= order[0]) {
		mask >>= 1;
	}
	br_hmac_drbg_generate(&ctx->eng.rng, key, olen);
	key[0] &= mask;
	key[olen - 1] |= 0x01;

	/*
	 * Compute the common ECDH point, whose X coordinate is the
	 * pre-master secret.
	 */
	ctx->eng.iec->generator(curve, &glen);
	if (glen != point_len) {
		return -BR_ERR_INVALID_ALGORITHM;
	}

	memcpy(point, point_src, glen);
	if (!ctx->eng.iec->mul(point, glen, key, olen, curve)) {
		return -BR_ERR_INVALID_ALGORITHM;
	}

	/*
	 * The pre-master secret is the X coordinate.
	 */
	xoff = ctx->eng.iec->xoff(curve, &xlen);
	br_ssl_engine_compute_master(&ctx->eng, prf_id, point + xoff, xlen);

	ctx->eng.iec->mulgen(point, key, olen, curve);
	memcpy(ctx->eng.pad, point, glen);
	return (int)glen;
}

/*
 * Perform full static ECDH. This occurs only in the context of client
 * authentication with certificates: the server uses an EC public key,
 * the cipher suite is of type ECDH (not ECDHE), the server requested a
 * client certificate and accepts static ECDH, the client has a
 * certificate with an EC public key in the same curve, and accepts
 * static ECDH as well.
 *
 * Returned value is 0 on success, -1 on error.
 */
 inline  int
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_pms_static_ecdh(br_ssl_client_context *ctx, int prf_id)
{
	unsigned char point[133];
	size_t point_len;
	const br_x509_class **xc;
	const br_x509_pkey *pk;

	xc = ctx->eng.x509ctx;
	pk = (*xc)->get_pkey(xc, NULL);
	point_len = pk->key.ec.qlen;
	if (point_len > sizeof point) {
		return -1;
	}
	memcpy(point, pk->key.ec.q, point_len);
	if (!(*ctx->client_auth_vtable)->do_keyx(
		ctx->client_auth_vtable, point, &point_len))
	{
		return -1;
	}
	br_ssl_engine_compute_master(&ctx->eng,
		prf_id, point, point_len);
	return 0;
}

/*
 * Compute the client-side signature. This is invoked only when a
 * signature-based client authentication was selected. The computed
 * signature is in the pad; its length (in bytes) is returned. On
 * error, 0 is returned.
 */
 inline  size_t
_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_client_sign(br_ssl_client_context *ctx)
{
	size_t hv_len;

	/*
	 * Compute hash of handshake messages so far. This "cannot" fail
	 * because the list of supported hash functions provided to the
	 * client certificate handler was trimmed to include only the
	 * hash functions that the multi-hasher supports.
	 */
	if (ctx->hash_id) {
		hv_len = br_multihash_out(&ctx->eng.mhash,
			ctx->hash_id, ctx->eng.pad);
	} else {
		br_multihash_out(&ctx->eng.mhash,
			br_md5_ID, ctx->eng.pad);
		br_multihash_out(&ctx->eng.mhash,
			br_sha1_ID, ctx->eng.pad + 16);
		hv_len = 36;
	}
	return (*ctx->client_auth_vtable)->do_sign(
		ctx->client_auth_vtable, ctx->hash_id, hv_len,
		ctx->eng.pad, sizeof ctx->eng.pad);
}



 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_datablock[] = {
	0x00, 0x00, 0x0A, 0x00, 0x24, 0x00, 0x2F, 0x01, 0x24, 0x00, 0x35, 0x02,
	0x24, 0x00, 0x3C, 0x01, 0x44, 0x00, 0x3D, 0x02, 0x44, 0x00, 0x9C, 0x03,
	0x04, 0x00, 0x9D, 0x04, 0x05, 0xC0, 0x03, 0x40, 0x24, 0xC0, 0x04, 0x41,
	0x24, 0xC0, 0x05, 0x42, 0x24, 0xC0, 0x08, 0x20, 0x24, 0xC0, 0x09, 0x21,
	0x24, 0xC0, 0x0A, 0x22, 0x24, 0xC0, 0x0D, 0x30, 0x24, 0xC0, 0x0E, 0x31,
	0x24, 0xC0, 0x0F, 0x32, 0x24, 0xC0, 0x12, 0x10, 0x24, 0xC0, 0x13, 0x11,
	0x24, 0xC0, 0x14, 0x12, 0x24, 0xC0, 0x23, 0x21, 0x44, 0xC0, 0x24, 0x22,
	0x55, 0xC0, 0x25, 0x41, 0x44, 0xC0, 0x26, 0x42, 0x55, 0xC0, 0x27, 0x11,
	0x44, 0xC0, 0x28, 0x12, 0x55, 0xC0, 0x29, 0x31, 0x44, 0xC0, 0x2A, 0x32,
	0x55, 0xC0, 0x2B, 0x23, 0x04, 0xC0, 0x2C, 0x24, 0x05, 0xC0, 0x2D, 0x43,
	0x04, 0xC0, 0x2E, 0x44, 0x05, 0xC0, 0x2F, 0x13, 0x04, 0xC0, 0x30, 0x14,
	0x05, 0xC0, 0x31, 0x33, 0x04, 0xC0, 0x32, 0x34, 0x05, 0xCC, 0xA8, 0x15,
	0x04, 0xCC, 0xA9, 0x25, 0x04, 0x00, 0x00
};

 inline static const unsigned char _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x0A, 0x00, 0x00, 0x01, 0x00, 0x0D, 0x00, 0x00, 0x01,
	0x00, 0x0E, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x00, 0x00, 0x01, 0x01, 0x08,
	0x00, 0x00, 0x01, 0x01, 0x09, 0x00, 0x00, 0x01, 0x02, 0x08, 0x00, 0x00,
	0x01, 0x02, 0x09, 0x00, 0x00, 0x25, 0x25, 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_CCS), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_CIPHER_SUITE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_COMPRESSION), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_FINISHED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_FRAGLEN), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_HANDSHAKE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_HELLO_DONE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_PARAM), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_SECRENEG), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_SNI), 0x00, 0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_BAD_VERSION),
	0x00, 0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_EXTRA_EXTENSION), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_INVALID_ALGORITHM), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_LIMIT_EXCEEDED), 0x00, 0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_OK),
	0x00, 0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_OVERSIZED_ID), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_RESUME_MISMATCH), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_UNEXPECTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_UNSUPPORTED_VERSION), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_ERR_WRONG_KEY_USAGE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, action)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, alert)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, application_data)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_client_context, auth_type)), 0x00, 0x00,
	0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, cipher_suite)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, client_random)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, close_received)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, ecdhe_curve)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, ecdhe_point)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, ecdhe_point_len)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, flags)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_client_context, hash_id)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_client_context, hashes)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, log_max_frag_len)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_client_context, min_clienthello_len)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, pad)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, protocol_names_num)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, record_type_in)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, record_type_out)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, reneg)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, saved_finished)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, selected_protocol)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, server_name)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, server_random)), 0x00, 0x00,
	0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, session_id)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, session_id_len)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, shutdown_recv)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, suites_buf)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, suites_num)), 0x00, 0x00,
	0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, version)),
	0x00, 0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, version_in)),
	0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, version_max)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, version_min)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT2(offsetof(br_ssl_engine_context, version_out)),
	0x00, 0x00, 0x09, 0x26, 0x56, 0x06, 0x02, 0x66, 0x28, 0x00, 0x00, 0x06,
	0x08, 0x2C, 0x0E, 0x05, 0x02, 0x6F, 0x28, 0x04, 0x01, 0x3C, 0x00, 0x00,
	0x01, 0x01, 0x00, 0x01, 0x03, 0x00, 0x97, 0x26, 0x5C, 0x44, 0x9B, 0x26,
	0x05, 0x04, 0x5E, 0x01, 0x00, 0x00, 0x02, 0x00, 0x0E, 0x06, 0x02, 0x9B,
	0x00, 0x5C, 0x04, 0x6B, 0x00, 0x06, 0x02, 0x66, 0x28, 0x00, 0x00, 0x26,
	0x87, 0x44, 0x05, 0x03, 0x01, 0x0C, 0x08, 0x44, 0x77, 0x2C, 0xA9, 0x1C,
	0x82, 0x01, 0x0C, 0x31, 0x00, 0x00, 0x26, 0x1F, 0x01, 0x08, 0x0B, 0x44,
	0x5A, 0x1F, 0x08, 0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x75, 0x3E, 0x29,
	0x1A, 0x36, 0x06, 0x07, 0x02, 0x00, 0xCC, 0x03, 0x00, 0x04, 0x75, 0x01,
	0x00, 0xC3, 0x02, 0x00, 0x26, 0x1A, 0x17, 0x06, 0x02, 0x6D, 0x28, 0xCC,
	0x04, 0x76, 0x01, 0x01, 0x00, 0x75, 0x3E, 0x01, 0x16, 0x85, 0x3E, 0x01,
	0x00, 0x88, 0x3C, 0x34, 0xD2, 0x29, 0xB2, 0x06, 0x09, 0x01, 0x7F, 0xAD,
	0x01, 0x7F, 0xCF, 0x04, 0x80, 0x53, 0xAF, 0x77, 0x2C, 0x9F, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_KEYTYPE_SIGN), 0x17, 0x06, 0x01, 0xB3, 0xB6, 0x26, 0x01,
	0x0D, 0x0E, 0x06, 0x07, 0x25, 0xB5, 0xB6, 0x01, 0x7F, 0x04, 0x02, 0x01,
	0x00, 0x03, 0x00, 0x01, 0x0E, 0x0E, 0x05, 0x02, 0x70, 0x28, 0x06, 0x02,
	0x65, 0x28, 0x33, 0x06, 0x02, 0x70, 0x28, 0x02, 0x00, 0x06, 0x1C, 0xD0,
	0x7E, 0x2E, 0x01, 0x81, 0x7F, 0x0E, 0x06, 0x0D, 0x25, 0x01, 0x10, 0xDB,
	0x01, 0x00, 0xDA, 0x77, 0x2C, 0xA9, 0x24, 0x04, 0x04, 0xD3, 0x06, 0x01,
	0xD1, 0x04, 0x01, 0xD3, 0x01, 0x7F, 0xCF, 0x01, 0x7F, 0xAD, 0x01, 0x01,
	0x75, 0x3E, 0x01, 0x17, 0x85, 0x3E, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00,
	0x98, 0x01, 0x0C, 0x11, 0x01, 0x00, 0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_KEYTYPE_RSA | BR_KEYTYPE_KEYX), 0x04, 0x30, 0x01, 0x01,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_KEYTYPE_RSA | BR_KEYTYPE_SIGN), 0x04, 0x25, 0x01, 0x02,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_KEYTYPE_EC  | BR_KEYTYPE_SIGN), 0x04, 0x1A, 0x01, 0x03,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_KEYTYPE_EC  | BR_KEYTYPE_KEYX), 0x04, 0x0F, 0x01, 0x04,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INT1(BR_KEYTYPE_EC  | BR_KEYTYPE_KEYX), 0x04, 0x04, 0x01, 0x00,
	0x44, 0x25, 0x00, 0x00, 0x80, 0x2E, 0x01, 0x0E, 0x0E, 0x06, 0x04, 0x01,
	0x00, 0x04, 0x02, 0x01, 0x05, 0x00, 0x00, 0x40, 0x06, 0x04, 0x01, 0x06,
	0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x86, 0x2E, 0x26, 0x06, 0x08, 0x01,
	0x01, 0x09, 0x01, 0x11, 0x07, 0x04, 0x03, 0x25, 0x01, 0x05, 0x00, 0x01,
	0x41, 0x03, 0x00, 0x25, 0x01, 0x00, 0x43, 0x06, 0x03, 0x02, 0x00, 0x08,
	0x42, 0x06, 0x03, 0x02, 0x00, 0x08, 0x26, 0x06, 0x06, 0x01, 0x01, 0x0B,
	0x01, 0x06, 0x08, 0x00, 0x00, 0x89, 0x3F, 0x26, 0x06, 0x03, 0x01, 0x09,
	0x08, 0x00, 0x01, 0x40, 0x26, 0x06, 0x1E, 0x01, 0x00, 0x03, 0x00, 0x26,
	0x06, 0x0E, 0x26, 0x01, 0x01, 0x17, 0x02, 0x00, 0x08, 0x03, 0x00, 0x01,
	0x01, 0x11, 0x04, 0x6F, 0x25, 0x02, 0x00, 0x01, 0x01, 0x0B, 0x01, 0x06,
	0x08, 0x00, 0x00, 0x7D, 0x2D, 0x44, 0x11, 0x01, 0x01, 0x17, 0x35, 0x00,
	0x00, 0x9D, 0xCB, 0x26, 0x01, 0x07, 0x17, 0x01, 0x00, 0x38, 0x0E, 0x06,
	0x09, 0x25, 0x01, 0x10, 0x17, 0x06, 0x01, 0x9D, 0x04, 0x2D, 0x01, 0x01,
	0x38, 0x0E, 0x06, 0x24, 0x25, 0x25, 0x01, 0x00, 0x75, 0x3E, 0xB1, 0x86,
	0x2E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0xA6, 0x37, 0x06, 0x0F, 0x29, 0x1A,
	0x36, 0x06, 0x04, 0xCB, 0x25, 0x04, 0x78, 0x01, 0x80, 0x64, 0xC3, 0x04,
	0x01, 0x9D, 0x04, 0x03, 0x70, 0x28, 0x25, 0x04, 0xFF, 0x3C, 0x01, 0x26,
	0x03, 0x00, 0x09, 0x26, 0x56, 0x06, 0x02, 0x66, 0x28, 0x02, 0x00, 0x00,
	0x00, 0x98, 0x01, 0x0F, 0x17, 0x00, 0x00, 0x74, 0x2E, 0x01, 0x00, 0x38,
	0x0E, 0x06, 0x10, 0x25, 0x26, 0x01, 0x01, 0x0D, 0x06, 0x03, 0x25, 0x01,
	0x02, 0x74, 0x3E, 0x01, 0x00, 0x04, 0x21, 0x01, 0x01, 0x38, 0x0E, 0x06,
	0x14, 0x25, 0x01, 0x00, 0x74, 0x3E, 0x26, 0x01, 0x80, 0x64, 0x0E, 0x06,
	0x05, 0x01, 0x82, 0x00, 0x08, 0x28, 0x58, 0x04, 0x07, 0x25, 0x01, 0x82,
	0x00, 0x08, 0x28, 0x25, 0x00, 0x00, 0x01, 0x00, 0x2F, 0x06, 0x05, 0x3A,
	0xAA, 0x37, 0x04, 0x78, 0x26, 0x06, 0x04, 0x01, 0x01, 0x8D, 0x3E, 0x00,
	0x01, 0xBD, 0xA8, 0xBD, 0xA8, 0xBF, 0x82, 0x44, 0x26, 0x03, 0x00, 0xB4,
	0x99, 0x99, 0x02, 0x00, 0x4B, 0x26, 0x56, 0x06, 0x0A, 0x01, 0x03, 0xA6,
	0x06, 0x02, 0x70, 0x28, 0x25, 0x04, 0x03, 0x5A, 0x88, 0x3C, 0x00, 0x00,
	0x2F, 0x06, 0x0B, 0x84, 0x2E, 0x01, 0x14, 0x0D, 0x06, 0x02, 0x70, 0x28,
	0x04, 0x11, 0xCB, 0x01, 0x07, 0x17, 0x26, 0x01, 0x02, 0x0D, 0x06, 0x06,
	0x06, 0x02, 0x70, 0x28, 0x04, 0x70, 0x25, 0xC0, 0x01, 0x01, 0x0D, 0x33,
	0x37, 0x06, 0x02, 0x5F, 0x28, 0x26, 0x01, 0x01, 0xC6, 0x36, 0xB0, 0x00,
	0x01, 0xB6, 0x01, 0x0B, 0x0E, 0x05, 0x02, 0x70, 0x28, 0x26, 0x01, 0x03,
	0x0E, 0x06, 0x08, 0xBE, 0x06, 0x02, 0x66, 0x28, 0x44, 0x25, 0x00, 0x44,
	0x55, 0xBE, 0xA8, 0x26, 0x06, 0x23, 0xBE, 0xA8, 0x26, 0x54, 0x26, 0x06,
	0x18, 0x26, 0x01, 0x82, 0x00, 0x0F, 0x06, 0x05, 0x01, 0x82, 0x00, 0x04,
	0x01, 0x26, 0x03, 0x00, 0x82, 0x02, 0x00, 0xB4, 0x02, 0x00, 0x51, 0x04,
	0x65, 0x99, 0x52, 0x04, 0x5A, 0x99, 0x99, 0x53, 0x26, 0x06, 0x02, 0x35,
	0x00, 0x25, 0x2B, 0x00, 0x00, 0x77, 0x2C, 0x9F, 0x01, 0x7F, 0xAE, 0x26,
	0x56, 0x06, 0x02, 0x35, 0x28, 0x26, 0x05, 0x02, 0x70, 0x28, 0x38, 0x17,
	0x0D, 0x06, 0x02, 0x72, 0x28, 0x3B, 0x00, 0x00, 0x9A, 0xB6, 0x01, 0x14,
	0x0D, 0x06, 0x02, 0x70, 0x28, 0x82, 0x01, 0x0C, 0x08, 0x01, 0x0C, 0xB4,
	0x99, 0x82, 0x26, 0x01, 0x0C, 0x08, 0x01, 0x0C, 0x30, 0x05, 0x02, 0x62,
	0x28, 0x00, 0x00, 0xB7, 0x06, 0x02, 0x70, 0x28, 0x06, 0x02, 0x64, 0x28,
	0x00, 0x0A, 0xB6, 0x01, 0x02, 0x0E, 0x05, 0x02, 0x70, 0x28, 0xBD, 0x03,
	0x00, 0x02, 0x00, 0x93, 0x2C, 0x0A, 0x02, 0x00, 0x92, 0x2C, 0x0F, 0x37,
	0x06, 0x02, 0x71, 0x28, 0x02, 0x00, 0x91, 0x2C, 0x0D, 0x06, 0x02, 0x69,
	0x28, 0x02, 0x00, 0x94, 0x3C, 0x8A, 0x01, 0x20, 0xB4, 0x01, 0x00, 0x03,
	0x01, 0xBF, 0x03, 0x02, 0x02, 0x02, 0x01, 0x20, 0x0F, 0x06, 0x02, 0x6E,
	0x28, 0x82, 0x02, 0x02, 0xB4, 0x02, 0x02, 0x8C, 0x2E, 0x0E, 0x02, 0x02,
	0x01, 0x00, 0x0F, 0x17, 0x06, 0x0B, 0x8B, 0x82, 0x02, 0x02, 0x30, 0x06,
	0x04, 0x01, 0x7F, 0x03, 0x01, 0x8B, 0x82, 0x02, 0x02, 0x31, 0x02, 0x02,
	0x8C, 0x3E, 0x02, 0x00, 0x90, 0x02, 0x01, 0x96, 0xBD, 0x26, 0xC1, 0x56,
	0x06, 0x02, 0x60, 0x28, 0x77, 0x02, 0x01, 0x96, 0xBF, 0x06, 0x02, 0x61,
	0x28, 0x26, 0x06, 0x81, 0x45, 0xBD, 0xA8, 0xA4, 0x03, 0x03, 0xA2, 0x03,
	0x04, 0xA0, 0x03, 0x05, 0xA3, 0x03, 0x06, 0xA5, 0x03, 0x07, 0xA1, 0x03,
	0x08, 0x27, 0x03, 0x09, 0x26, 0x06, 0x81, 0x18, 0xBD, 0x01, 0x00, 0x38,
	0x0E, 0x06, 0x0F, 0x25, 0x02, 0x03, 0x05, 0x02, 0x6A, 0x28, 0x01, 0x00,
	0x03, 0x03, 0xBC, 0x04, 0x80, 0x7F, 0x01, 0x01, 0x38, 0x0E, 0x06, 0x0F,
	0x25, 0x02, 0x05, 0x05, 0x02, 0x6A, 0x28, 0x01, 0x00, 0x03, 0x05, 0xBA,
	0x04, 0x80, 0x6A, 0x01, 0x83, 0xFE, 0x01, 0x38, 0x0E, 0x06, 0x0F, 0x25,
	0x02, 0x04, 0x05, 0x02, 0x6A, 0x28, 0x01, 0x00, 0x03, 0x04, 0xBB, 0x04,
	0x80, 0x53, 0x01, 0x0D, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x02, 0x06, 0x05,
	0x02, 0x6A, 0x28, 0x01, 0x00, 0x03, 0x06, 0xB8, 0x04, 0x3F, 0x01, 0x0A,
	0x38, 0x0E, 0x06, 0x0E, 0x25, 0x02, 0x07, 0x05, 0x02, 0x6A, 0x28, 0x01,
	0x00, 0x03, 0x07, 0xB8, 0x04, 0x2B, 0x01, 0x0B, 0x38, 0x0E, 0x06, 0x0E,
	0x25, 0x02, 0x08, 0x05, 0x02, 0x6A, 0x28, 0x01, 0x00, 0x03, 0x08, 0xB8,
	0x04, 0x17, 0x01, 0x10, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x02, 0x09, 0x05,
	0x02, 0x6A, 0x28, 0x01, 0x00, 0x03, 0x09, 0xAC, 0x04, 0x03, 0x6A, 0x28,
	0x25, 0x04, 0xFE, 0x64, 0x02, 0x04, 0x06, 0x0D, 0x02, 0x04, 0x01, 0x05,
	0x0F, 0x06, 0x02, 0x67, 0x28, 0x01, 0x01, 0x86, 0x3E, 0x99, 0x99, 0x02,
	0x01, 0x00, 0x04, 0xB6, 0x01, 0x0C, 0x0E, 0x05, 0x02, 0x70, 0x28, 0xBF,
	0x01, 0x03, 0x0E, 0x05, 0x02, 0x6B, 0x28, 0xBD, 0x26, 0x7A, 0x3E, 0x26,
	0x01, 0x20, 0x10, 0x06, 0x02, 0x6B, 0x28, 0x40, 0x44, 0x11, 0x01, 0x01,
	0x17, 0x05, 0x02, 0x6B, 0x28, 0xBF, 0x26, 0x01, 0x81, 0x05, 0x0F, 0x06,
	0x02, 0x6B, 0x28, 0x26, 0x7C, 0x3E, 0x7B, 0x44, 0xB4, 0x90, 0x2C, 0x01,
	0x86, 0x03, 0x10, 0x03, 0x00, 0x77, 0x2C, 0xC9, 0x03, 0x01, 0x01, 0x02,
	0x03, 0x02, 0x02, 0x00, 0x06, 0x21, 0xBF, 0x26, 0x26, 0x01, 0x02, 0x0A,
	0x44, 0x01, 0x06, 0x0F, 0x37, 0x06, 0x02, 0x6B, 0x28, 0x03, 0x02, 0xBF,
	0x02, 0x01, 0x01, 0x01, 0x0B, 0x01, 0x03, 0x08, 0x0E, 0x05, 0x02, 0x6B,
	0x28, 0x04, 0x08, 0x02, 0x01, 0x06, 0x04, 0x01, 0x00, 0x03, 0x02, 0xBD,
	0x26, 0x03, 0x03, 0x26, 0x01, 0x84, 0x00, 0x0F, 0x06, 0x02, 0x6C, 0x28,
	0x82, 0x44, 0xB4, 0x02, 0x02, 0x02, 0x01, 0x02, 0x03, 0x4E, 0x26, 0x06,
	0x01, 0x28, 0x25, 0x99, 0x00, 0x02, 0x03, 0x00, 0x03, 0x01, 0x02, 0x00,
	0x95, 0x02, 0x01, 0x02, 0x00, 0x39, 0x26, 0x01, 0x00, 0x0E, 0x06, 0x02,
	0x5E, 0x00, 0xCD, 0x04, 0x74, 0x02, 0x01, 0x00, 0x03, 0x00, 0xBF, 0xA8,
	0x26, 0x06, 0x80, 0x43, 0xBF, 0x01, 0x01, 0x38, 0x0E, 0x06, 0x06, 0x25,
	0x01, 0x81, 0x7F, 0x04, 0x2E, 0x01, 0x80, 0x40, 0x38, 0x0E, 0x06, 0x07,
	0x25, 0x01, 0x83, 0xFE, 0x00, 0x04, 0x20, 0x01, 0x80, 0x41, 0x38, 0x0E,
	0x06, 0x07, 0x25, 0x01, 0x84, 0x80, 0x00, 0x04, 0x12, 0x01, 0x80, 0x42,
	0x38, 0x0E, 0x06, 0x07, 0x25, 0x01, 0x88, 0x80, 0x00, 0x04, 0x04, 0x01,
	0x00, 0x44, 0x25, 0x02, 0x00, 0x37, 0x03, 0x00, 0x04, 0xFF, 0x39, 0x99,
	0x77, 0x2C, 0xC7, 0x05, 0x09, 0x02, 0x00, 0x01, 0x83, 0xFF, 0x7F, 0x17,
	0x03, 0x00, 0x90, 0x2C, 0x01, 0x86, 0x03, 0x10, 0x06, 0x3A, 0xB9, 0x26,
	0x7F, 0x3D, 0x41, 0x25, 0x26, 0x01, 0x08, 0x0B, 0x37, 0x01, 0x8C, 0x80,
	0x00, 0x37, 0x17, 0x02, 0x00, 0x17, 0x02, 0x00, 0x01, 0x8C, 0x80, 0x00,
	0x17, 0x06, 0x19, 0x26, 0x01, 0x81, 0x7F, 0x17, 0x06, 0x05, 0x01, 0x84,
	0x80, 0x00, 0x37, 0x26, 0x01, 0x83, 0xFE, 0x00, 0x17, 0x06, 0x05, 0x01,
	0x88, 0x80, 0x00, 0x37, 0x03, 0x00, 0x04, 0x09, 0x02, 0x00, 0x01, 0x8C,
	0x88, 0x01, 0x17, 0x03, 0x00, 0x16, 0xBD, 0xA8, 0x26, 0x06, 0x23, 0xBD,
	0xA8, 0x26, 0x15, 0x26, 0x06, 0x18, 0x26, 0x01, 0x82, 0x00, 0x0F, 0x06,
	0x05, 0x01, 0x82, 0x00, 0x04, 0x01, 0x26, 0x03, 0x01, 0x82, 0x02, 0x01,
	0xB4, 0x02, 0x01, 0x12, 0x04, 0x65, 0x99, 0x13, 0x04, 0x5A, 0x99, 0x14,
	0x99, 0x02, 0x00, 0x2A, 0x00, 0x00, 0xB7, 0x26, 0x58, 0x06, 0x07, 0x25,
	0x06, 0x02, 0x64, 0x28, 0x04, 0x74, 0x00, 0x00, 0xC0, 0x01, 0x03, 0xBE,
	0x44, 0x25, 0x44, 0x00, 0x00, 0xBD, 0xC4, 0x00, 0x03, 0x01, 0x00, 0x03,
	0x00, 0xBD, 0xA8, 0x26, 0x06, 0x80, 0x50, 0xBF, 0x03, 0x01, 0xBF, 0x03,
	0x02, 0x02, 0x01, 0x01, 0x08, 0x0E, 0x06, 0x16, 0x02, 0x02, 0x01, 0x0F,
	0x0C, 0x06, 0x0D, 0x01, 0x01, 0x02, 0x02, 0x01, 0x10, 0x08, 0x0B, 0x02,
	0x00, 0x37, 0x03, 0x00, 0x04, 0x2A, 0x02, 0x01, 0x01, 0x02, 0x10, 0x02,
	0x01, 0x01, 0x06, 0x0C, 0x17, 0x02, 0x02, 0x01, 0x01, 0x0E, 0x02, 0x02,
	0x01, 0x03, 0x0E, 0x37, 0x17, 0x06, 0x11, 0x02, 0x00, 0x01, 0x01, 0x02,
	0x02, 0x5B, 0x01, 0x02, 0x0B, 0x02, 0x01, 0x08, 0x0B, 0x37, 0x03, 0x00,
	0x04, 0xFF, 0x2C, 0x99, 0x02, 0x00, 0x00, 0x00, 0xBD, 0x01, 0x01, 0x0E,
	0x05, 0x02, 0x63, 0x28, 0xBF, 0x01, 0x08, 0x08, 0x80, 0x2E, 0x0E, 0x05,
	0x02, 0x63, 0x28, 0x00, 0x00, 0xBD, 0x86, 0x2E, 0x05, 0x15, 0x01, 0x01,
	0x0E, 0x05, 0x02, 0x67, 0x28, 0xBF, 0x01, 0x00, 0x0E, 0x05, 0x02, 0x67,
	0x28, 0x01, 0x02, 0x86, 0x3E, 0x04, 0x1C, 0x01, 0x19, 0x0E, 0x05, 0x02,
	0x67, 0x28, 0xBF, 0x01, 0x18, 0x0E, 0x05, 0x02, 0x67, 0x28, 0x82, 0x01,
	0x18, 0xB4, 0x87, 0x82, 0x01, 0x18, 0x30, 0x05, 0x02, 0x67, 0x28, 0x00,
	0x00, 0xBD, 0x06, 0x02, 0x68, 0x28, 0x00, 0x00, 0x01, 0x02, 0x95, 0xC0,
	0x01, 0x08, 0x0B, 0xC0, 0x08, 0x00, 0x00, 0x01, 0x03, 0x95, 0xC0, 0x01,
	0x08, 0x0B, 0xC0, 0x08, 0x01, 0x08, 0x0B, 0xC0, 0x08, 0x00, 0x00, 0x01,
	0x01, 0x95, 0xC0, 0x00, 0x00, 0x3A, 0x26, 0x56, 0x05, 0x01, 0x00, 0x25,
	0xCD, 0x04, 0x76, 0x02, 0x03, 0x00, 0x8F, 0x2E, 0x03, 0x01, 0x01, 0x00,
	0x26, 0x02, 0x01, 0x0A, 0x06, 0x10, 0x26, 0x01, 0x01, 0x0B, 0x8E, 0x08,
	0x2C, 0x02, 0x00, 0x0E, 0x06, 0x01, 0x00, 0x5A, 0x04, 0x6A, 0x25, 0x01,
	0x7F, 0x00, 0x00, 0x01, 0x15, 0x85, 0x3E, 0x44, 0x50, 0x25, 0x50, 0x25,
	0x29, 0x00, 0x00, 0x01, 0x01, 0x44, 0xC2, 0x00, 0x00, 0x44, 0x38, 0x95,
	0x44, 0x26, 0x06, 0x05, 0xC0, 0x25, 0x5B, 0x04, 0x78, 0x25, 0x00, 0x00,
	0x26, 0x01, 0x81, 0xAC, 0x00, 0x0E, 0x06, 0x04, 0x25, 0x01, 0x7F, 0x00,
	0x98, 0x57, 0x00, 0x02, 0x03, 0x00, 0x77, 0x2C, 0x98, 0x03, 0x01, 0x02,
	0x01, 0x01, 0x0F, 0x17, 0x02, 0x01, 0x01, 0x04, 0x11, 0x01, 0x0F, 0x17,
	0x02, 0x01, 0x01, 0x08, 0x11, 0x01, 0x0F, 0x17, 0x01, 0x00, 0x38, 0x0E,
	0x06, 0x10, 0x25, 0x01, 0x00, 0x01, 0x18, 0x02, 0x00, 0x06, 0x03, 0x47,
	0x04, 0x01, 0x48, 0x04, 0x80, 0x68, 0x01, 0x01, 0x38, 0x0E, 0x06, 0x10,
	0x25, 0x01, 0x01, 0x01, 0x10, 0x02, 0x00, 0x06, 0x03, 0x47, 0x04, 0x01,
	0x48, 0x04, 0x80, 0x52, 0x01, 0x02, 0x38, 0x0E, 0x06, 0x0F, 0x25, 0x01,
	0x01, 0x01, 0x20, 0x02, 0x00, 0x06, 0x03, 0x47, 0x04, 0x01, 0x48, 0x04,
	0x3D, 0x01, 0x03, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x25, 0x01, 0x10, 0x02,
	0x00, 0x06, 0x03, 0x45, 0x04, 0x01, 0x46, 0x04, 0x29, 0x01, 0x04, 0x38,
	0x0E, 0x06, 0x0E, 0x25, 0x25, 0x01, 0x20, 0x02, 0x00, 0x06, 0x03, 0x45,
	0x04, 0x01, 0x46, 0x04, 0x15, 0x01, 0x05, 0x38, 0x0E, 0x06, 0x0C, 0x25,
	0x25, 0x02, 0x00, 0x06, 0x03, 0x49, 0x04, 0x01, 0x4A, 0x04, 0x03, 0x66,
	0x28, 0x25, 0x00, 0x00, 0x98, 0x01, 0x0C, 0x11, 0x01, 0x02, 0x0F, 0x00,
	0x00, 0x98, 0x01, 0x0C, 0x11, 0x26, 0x59, 0x44, 0x01, 0x03, 0x0A, 0x17,
	0x00, 0x00, 0x98, 0x01, 0x0C, 0x11, 0x01, 0x01, 0x0E, 0x00, 0x00, 0x98,
	0x01, 0x0C, 0x11, 0x58, 0x00, 0x00, 0x1B, 0x01, 0x00, 0x73, 0x2E, 0x26,
	0x06, 0x1F, 0x01, 0x01, 0x38, 0x0E, 0x06, 0x06, 0x25, 0x01, 0x00, 0x9C,
	0x04, 0x11, 0x01, 0x02, 0x38, 0x0E, 0x06, 0x0A, 0x25, 0x75, 0x2E, 0x06,
	0x03, 0x01, 0x10, 0x37, 0x04, 0x01, 0x25, 0x04, 0x01, 0x25, 0x79, 0x2E,
	0x05, 0x33, 0x2F, 0x06, 0x30, 0x84, 0x2E, 0x01, 0x14, 0x38, 0x0E, 0x06,
	0x06, 0x25, 0x01, 0x02, 0x37, 0x04, 0x22, 0x01, 0x15, 0x38, 0x0E, 0x06,
	0x09, 0x25, 0xAB, 0x06, 0x03, 0x01, 0x7F, 0x9C, 0x04, 0x13, 0x01, 0x16,
	0x38, 0x0E, 0x06, 0x06, 0x25, 0x01, 0x01, 0x37, 0x04, 0x07, 0x25, 0x01,
	0x04, 0x37, 0x01, 0x00, 0x25, 0x1A, 0x06, 0x03, 0x01, 0x08, 0x37, 0x00,
	0x00, 0x1B, 0x26, 0x05, 0x0F, 0x2F, 0x06, 0x0C, 0x84, 0x2E, 0x01, 0x15,
	0x0E, 0x06, 0x04, 0x25, 0xAB, 0x04, 0x01, 0x20, 0x00, 0x00, 0xCB, 0x01,
	0x07, 0x17, 0x01, 0x01, 0x0F, 0x06, 0x02, 0x70, 0x28, 0x00, 0x01, 0x03,
	0x00, 0x29, 0x1A, 0x06, 0x05, 0x02, 0x00, 0x85, 0x3E, 0x00, 0xCB, 0x25,
	0x04, 0x74, 0x00, 0x01, 0x14, 0xCE, 0x01, 0x01, 0xDB, 0x29, 0x26, 0x01,
	0x00, 0xC6, 0x01, 0x16, 0xCE, 0xD4, 0x29, 0x00, 0x00, 0x01, 0x0B, 0xDB,
	0x4C, 0x26, 0x26, 0x01, 0x03, 0x08, 0xDA, 0xDA, 0x18, 0x26, 0x56, 0x06,
	0x02, 0x25, 0x00, 0xDA, 0x1D, 0x26, 0x06, 0x05, 0x82, 0x44, 0xD5, 0x04,
	0x77, 0x25, 0x04, 0x6C, 0x00, 0x21, 0x01, 0x0F, 0xDB, 0x26, 0x90, 0x2C,
	0x01, 0x86, 0x03, 0x10, 0x06, 0x0C, 0x01, 0x04, 0x08, 0xDA, 0x7E, 0x2E,
	0xDB, 0x76, 0x2E, 0xDB, 0x04, 0x02, 0x5C, 0xDA, 0x26, 0xD9, 0x82, 0x44,
	0xD5, 0x00, 0x02, 0xA2, 0xA4, 0x08, 0xA0, 0x08, 0xA3, 0x08, 0xA5, 0x08,
	0xA1, 0x08, 0x27, 0x08, 0x03, 0x00, 0x01, 0x01, 0xDB, 0x01, 0x27, 0x8C,
	0x2E, 0x08, 0x8F, 0x2E, 0x01, 0x01, 0x0B, 0x08, 0x02, 0x00, 0x06, 0x04,
	0x5C, 0x02, 0x00, 0x08, 0x81, 0x2C, 0x38, 0x09, 0x26, 0x59, 0x06, 0x24,
	0x02, 0x00, 0x05, 0x04, 0x44, 0x5C, 0x44, 0x5D, 0x01, 0x04, 0x09, 0x26,
	0x56, 0x06, 0x03, 0x25, 0x01, 0x00, 0x26, 0x01, 0x04, 0x08, 0x02, 0x00,
	0x08, 0x03, 0x00, 0x44, 0x01, 0x04, 0x08, 0x38, 0x08, 0x44, 0x04, 0x03,
	0x25, 0x01, 0x7F, 0x03, 0x01, 0xDA, 0x92, 0x2C, 0xD9, 0x78, 0x01, 0x04,
	0x19, 0x78, 0x01, 0x04, 0x08, 0x01, 0x1C, 0x32, 0x78, 0x01, 0x20, 0xD5,
	0x8B, 0x8C, 0x2E, 0xD7, 0x8F, 0x2E, 0x26, 0x01, 0x01, 0x0B, 0xD9, 0x8E,
	0x44, 0x26, 0x06, 0x0F, 0x5B, 0x38, 0x2C, 0x26, 0xC5, 0x05, 0x02, 0x60,
	0x28, 0xD9, 0x44, 0x5C, 0x44, 0x04, 0x6E, 0x5E, 0x01, 0x01, 0xDB, 0x01,
	0x00, 0xDB, 0x02, 0x00, 0x06, 0x81, 0x5A, 0x02, 0x00, 0xD9, 0xA2, 0x06,
	0x0E, 0x01, 0x83, 0xFE, 0x01, 0xD9, 0x87, 0xA2, 0x01, 0x04, 0x09, 0x26,
	0xD9, 0x5B, 0xD7, 0xA4, 0x06, 0x16, 0x01, 0x00, 0xD9, 0x89, 0xA4, 0x01,
	0x04, 0x09, 0x26, 0xD9, 0x01, 0x02, 0x09, 0x26, 0xD9, 0x01, 0x00, 0xDB,
	0x01, 0x03, 0x09, 0xD6, 0xA0, 0x06, 0x0C, 0x01, 0x01, 0xD9, 0x01, 0x01,
	0xD9, 0x80, 0x2E, 0x01, 0x08, 0x09, 0xDB, 0xA3, 0x06, 0x19, 0x01, 0x0D,
	0xD9, 0xA3, 0x01, 0x04, 0x09, 0x26, 0xD9, 0x01, 0x02, 0x09, 0xD9, 0x42,
	0x06, 0x03, 0x01, 0x03, 0xD8, 0x43, 0x06, 0x03, 0x01, 0x01, 0xD8, 0xA5,
	0x26, 0x06, 0x36, 0x01, 0x0A, 0xD9, 0x01, 0x04, 0x09, 0x26, 0xD9, 0x5D,
	0xD9, 0x40, 0x01, 0x00, 0x26, 0x01, 0x82, 0x80, 0x80, 0x80, 0x00, 0x17,
	0x06, 0x0A, 0x01, 0xFD, 0xFF, 0xFF, 0xFF, 0x7F, 0x17, 0x01, 0x1D, 0xD9,
	0x26, 0x01, 0x20, 0x0A, 0x06, 0x0C, 0x9E, 0x11, 0x01, 0x01, 0x17, 0x06,
	0x02, 0x26, 0xD9, 0x5A, 0x04, 0x6E, 0x5E, 0x04, 0x01, 0x25, 0xA1, 0x06,
	0x0A, 0x01, 0x0B, 0xD9, 0x01, 0x02, 0xD9, 0x01, 0x82, 0x00, 0xD9, 0x27,
	0x26, 0x06, 0x1F, 0x01, 0x10, 0xD9, 0x01, 0x04, 0x09, 0x26, 0xD9, 0x5D,
	0xD9, 0x83, 0x2C, 0x01, 0x00, 0x9E, 0x0F, 0x06, 0x0A, 0x26, 0x1E, 0x26,
	0xDB, 0x82, 0x44, 0xD5, 0x5A, 0x04, 0x72, 0x5E, 0x04, 0x01, 0x25, 0x02,
	0x01, 0x56, 0x05, 0x11, 0x01, 0x15, 0xD9, 0x02, 0x01, 0x26, 0xD9, 0x26,
	0x06, 0x06, 0x5B, 0x01, 0x00, 0xDB, 0x04, 0x77, 0x25, 0x00, 0x00, 0x01,
	0x10, 0xDB, 0x77, 0x2C, 0x26, 0xCA, 0x06, 0x0C, 0xA9, 0x23, 0x26, 0x5C,
	0xDA, 0x26, 0xD9, 0x82, 0x44, 0xD5, 0x04, 0x0D, 0x26, 0xC8, 0x44, 0xA9,
	0x22, 0x26, 0x5A, 0xDA, 0x26, 0xDB, 0x82, 0x44, 0xD5, 0x00, 0x00, 0x9A,
	0x01, 0x14, 0xDB, 0x01, 0x0C, 0xDA, 0x82, 0x01, 0x0C, 0xD5, 0x00, 0x00,
	0x4F, 0x26, 0x01, 0x00, 0x0E, 0x06, 0x02, 0x5E, 0x00, 0xCB, 0x25, 0x04,
	0x73, 0x00, 0x26, 0xD9, 0xD5, 0x00, 0x00, 0x26, 0xDB, 0xD5, 0x00, 0x01,
	0x03, 0x00, 0x41, 0x25, 0x26, 0x01, 0x10, 0x17, 0x06, 0x06, 0x01, 0x04,
	0xDB, 0x02, 0x00, 0xDB, 0x26, 0x01, 0x08, 0x17, 0x06, 0x06, 0x01, 0x03,
	0xDB, 0x02, 0x00, 0xDB, 0x26, 0x01, 0x20, 0x17, 0x06, 0x06, 0x01, 0x05,
	0xDB, 0x02, 0x00, 0xDB, 0x26, 0x01, 0x80, 0x40, 0x17, 0x06, 0x06, 0x01,
	0x06, 0xDB, 0x02, 0x00, 0xDB, 0x01, 0x04, 0x17, 0x06, 0x06, 0x01, 0x02,
	0xDB, 0x02, 0x00, 0xDB, 0x00, 0x00, 0x26, 0x01, 0x08, 0x4D, 0xDB, 0xDB,
	0x00, 0x00, 0x26, 0x01, 0x10, 0x4D, 0xDB, 0xD9, 0x00, 0x00, 0x26, 0x50,
	0x06, 0x02, 0x25, 0x00, 0xCB, 0x25, 0x04, 0x76
};

 inline static const uint16_t _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_caddr[] = {
	0,
	5,
	10,
	15,
	20,
	25,
	30,
	35,
	40,
	44,
	48,
	52,
	56,
	60,
	64,
	68,
	72,
	76,
	80,
	84,
	88,
	92,
	96,
	100,
	104,
	108,
	112,
	116,
	120,
	124,
	129,
	134,
	139,
	144,
	149,
	154,
	159,
	164,
	169,
	174,
	179,
	184,
	189,
	194,
	199,
	204,
	209,
	214,
	219,
	224,
	229,
	234,
	239,
	244,
	249,
	254,
	259,
	264,
	269,
	274,
	279,
	284,
	289,
	294,
	303,
	316,
	320,
	345,
	351,
	370,
	381,
	415,
	535,
	539,
	604,
	619,
	630,
	648,
	677,
	687,
	723,
	733,
	803,
	817,
	823,
	882,
	901,
	936,
	985,
	1061,
	1088,
	1119,
	1130,
	1455,
	1602,
	1626,
	1842,
	1856,
	1865,
	1869,
	1964,
	1985,
	2041,
	2048,
	2059,
	2075,
	2081,
	2092,
	2127,
	2139,
	2145,
	2160,
	2176,
	2332,
	2341,
	2354,
	2363,
	2370,
	2473,
	2494,
	2507,
	2523,
	2541,
	2573,
	2607,
	2975,
	3011,
	3024,
	3038,
	3043,
	3048,
	3114,
	3122,
	3130
};

#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INTERPRETED   86

#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_codeblock[_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_caddr[(slot) - _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INTERPRETED]]; \
		t0_lnum = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *t0ctx = (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)ctx; \
	t0ctx->ip = &_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_codeblock[0]; \
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_DEFENTRY(br_ssl_hs_client_init_main, 167)

#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

 inline void
br_ssl_hs_client_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_LOCAL(x)    (*(rp - 2 - (x)))
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP()       (*-- dp)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi()      (*(int32_t *)(-- dp))
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PEEK(x)     (*(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RPOP()      (*-- rp)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RPOPi()     (*(int32_t *)(-- rp))
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PEEK(t0depth)); \
} while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RET()        goto t0_next

	dp = ((_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)t0ctx)->dp;
	rp = ((_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)t0ctx)->rp;
	ip = ((_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_NEXT(&ip);
		if (t0x < _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_LOCAL(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_LOCAL(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_unsigned(&ip)) = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
				break;
			case 4: /* jump */
				t0off = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_signed(&ip);
				if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_parse7E_signed(&ip);
				if (!_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* * */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(a * b);

				}
				break;
			case 8: {
				/* + */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(a + b);

				}
				break;
			case 9: {
				/* - */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(a - b);

				}
				break;
			case 10: {
				/* < */

	int32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 11: {
				/* << */

	int c = (int)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	uint32_t x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x << c);

				}
				break;
			case 12: {
				/* <= */

	int32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 13: {
				/* <> */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 14: {
				/* = */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 15: {
				/* > */

	int32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 16: {
				/* >= */

	int32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 17: {
				/* >> */

	int c = (int)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	int32_t x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(x >> c);

				}
				break;
			case 18: {
				/* anchor-dn-append-name */

	size_t len;

	len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable != NULL) {
		(*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable)->append_name(
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad, len);
	}

				}
				break;
			case 19: {
				/* anchor-dn-end-name */

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable != NULL) {
		(*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable)->end_name(
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable);
	}

				}
				break;
			case 20: {
				/* anchor-dn-end-name-list */

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable != NULL) {
		(*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable)->end_name_list(
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable);
	}

				}
				break;
			case 21: {
				/* anchor-dn-start-name */

	size_t len;

	len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable != NULL) {
		(*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable)->start_name(
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable, len);
	}

				}
				break;
			case 22: {
				/* anchor-dn-start-name-list */

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable != NULL) {
		(*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable)->start_name_list(
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable);
	}

				}
				break;
			case 23: {
				/* and */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(a & b);

				}
				break;
			case 24: {
				/* begin-cert */

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain_len == 0) {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-1);
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_cur = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain->data;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain->data_len;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain ++;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain_len --;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_len);
	}

				}
				break;
			case 25: {
				/* bzero */

	size_t len = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	void *addr = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	memset(addr, 0, len);

				}
				break;
			case 26: {
				/* can-output? */

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_out > 0));

				}
				break;
			case 27: {
				/* co */
 _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO(); 
				}
				break;
			case 28: {
				/* compute-Finished-inner */

	int prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	int from_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	unsigned char seed[48];
	size_t seed_len;

	br_tls_prf_impl prf = br_ssl_engine_get_PRF(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, prf_id);
	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->session.version >= BR_TLS12) {
		seed_len = br_multihash_out(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, prf_id, seed);
	} else {
		br_multihash_out(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, br_md5_ID, seed);
		br_multihash_out(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, br_sha1_ID, seed + 16);
		seed_len = 36;
	}
	prf(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad, 12, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->session.master_secret,
		sizeof _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->session.master_secret,
		from_client ? "client finished" : "server finished",
		seed, seed_len);

				}
				break;
			case 29: {
				/* copy-cert-chunk */

	size_t clen;

	clen = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_len;
	if (clen > sizeof _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad) {
		clen = sizeof _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad;
	}
	memcpy(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_cur, clen);
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_cur += clen;
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->cert_len -= clen;
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(clen);

				}
				break;
			case 30: {
				/* copy-protocol-name */

	size_t idx = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	size_t len = strlen(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names[idx]);
	memcpy(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names[idx], len);
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(len);

				}
				break;
			case 31: {
				/* data-get8 */

	size_t addr = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_datablock[addr]);

				}
				break;
			case 32: {
				/* discard-input */

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in = 0;

				}
				break;
			case 33: {
				/* do-client-sign */

	size_t sig_len;

	sig_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_client_sign(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX);
	if (sig_len == 0) {
		br_ssl_engine_fail(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, BR_ERR_INVALID_ALGORITHM);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO();
	}
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(sig_len);

				}
				break;
			case 34: {
				/* do-ecdh */

	unsigned prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	unsigned ecdhe = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	int x;

	x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_pms_ecdh(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX, ecdhe, prf_id);
	if (x < 0) {
		br_ssl_engine_fail(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, -x);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO();
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x);
	}

				}
				break;
			case 35: {
				/* do-rsa-encrypt */

	int x;

	x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_pms_rsa(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP());
	if (x < 0) {
		br_ssl_engine_fail(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, -x);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO();
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x);
	}

				}
				break;
			case 36: {
				/* do-static-ecdh */

	unsigned prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_make_pms_static_ecdh(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX, prf_id) < 0) {
		br_ssl_engine_fail(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, BR_ERR_INVALID_ALGORITHM);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO();
	}

				}
				break;
			case 37: {
				/* drop */
 (void)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP(); 
				}
				break;
			case 38: {
				/* dup */
 _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PEEK(0)); 
				}
				break;
			case 39: {
				/* ext-ALPN-length */

	size_t u, len;

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names_num == 0) {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(0);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RET();
	}
	len = 6;
	for (u = 0; u < _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names_num; u ++) {
		len += 1 + strlen(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names[u]);
	}
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(len);

				}
				break;
			case 40: {
				/* fail */

	br_ssl_engine_fail(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, (int)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi());
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_CO();

				}
				break;
			case 41: {
				/* flush-record */

	br_ssl_engine_flush_record(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG);

				}
				break;
			case 42: {
				/* get-client-chain */

	uint32_t auth_types;

	auth_types = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable != NULL) {
		br_ssl_client_certificate ux;

		(*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable)->choose(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->client_auth_vtable,
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX, auth_types, &ux);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->auth_type = (unsigned char)ux.auth_type;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->hash_id = (unsigned char)ux.hash_id;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain = ux.chain;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain_len = ux.chain_len;
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->hash_id = 0;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain_len = 0;
	}

				}
				break;
			case 43: {
				/* get-key-type-usages */

	const br_x509_class *xc;
	const br_x509_pkey *pk;
	unsigned usages;

	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	pk = xc->get_pkey(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx, &usages);
	if (pk == NULL) {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(0);
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(pk->key_type | usages);
	}

				}
				break;
			case 44: {
				/* get16 */

	size_t addr = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(*(uint16_t *)((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr));

				}
				break;
			case 45: {
				/* get32 */

	size_t addr = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(*(uint32_t *)((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr));

				}
				break;
			case 46: {
				/* get8 */

	size_t addr = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(*((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr));

				}
				break;
			case 47: {
				/* has-input? */

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in != 0));

				}
				break;
			case 48: {
				/* memcmp */

	size_t len = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	void *addr2 = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	void *addr1 = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	int x = memcmp(addr1, addr2, len);
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH((uint32_t)-(x == 0));

				}
				break;
			case 49: {
				/* memcpy */

	size_t len = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	void *src = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	void *dst = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	memcpy(dst, src, len);

				}
				break;
			case 50: {
				/* mkrand */

	size_t len = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	void *addr = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_hmac_drbg_generate(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->rng, addr, len);

				}
				break;
			case 51: {
				/* more-incoming-bytes? */

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in != 0 || !br_ssl_engine_recvrec_finished(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG));

				}
				break;
			case 52: {
				/* multihash-init */

	br_multihash_init(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash);

				}
				break;
			case 53: {
				/* neg */

	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(-a);

				}
				break;
			case 54: {
				/* not */

	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(~a);

				}
				break;
			case 55: {
				/* or */

	uint32_t b = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(a | b);

				}
				break;
			case 56: {
				/* over */
 _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PEEK(1)); 
				}
				break;
			case 57: {
				/* read-chunk-native */

	size_t clen = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in;
	if (clen > 0) {
		uint32_t addr, len;

		len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
		addr = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
		if ((size_t)len < clen) {
			clen = (size_t)len;
		}
		memcpy((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_in, clen);
		if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->record_type_in == BR_SSL_HANDSHAKE) {
			br_multihash_update(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_in, clen);
		}
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(addr + (uint32_t)clen);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(len - (uint32_t)clen);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_in += clen;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in -= clen;
	}

				}
				break;
			case 58: {
				/* read8-native */

	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in > 0) {
		unsigned char x;

		x = *_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_in ++;
		if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->record_type_in == BR_SSL_HANDSHAKE) {
			br_multihash_update(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, &x, 1);
		}
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_in --;
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-1);
	}

				}
				break;
			case 59: {
				/* set-server-curve */

	const br_x509_class *xc;
	const br_x509_pkey *pk;

	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	pk = xc->get_pkey(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx, NULL);
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX->server_curve =
		(pk->key_type == BR_KEYTYPE_EC) ? pk->key.ec.curve : 0;

				}
				break;
			case 60: {
				/* set16 */

	size_t addr = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	*(uint16_t *)((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr) = (uint16_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();

				}
				break;
			case 61: {
				/* set32 */

	size_t addr = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	*(uint32_t *)((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr) = (uint32_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();

				}
				break;
			case 62: {
				/* set8 */

	size_t addr = (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	*((unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr) = (unsigned char)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();

				}
				break;
			case 63: {
				/* strlen */

	void *str = (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + (size_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH((uint32_t)strlen((char *)str));

				}
				break;
			case 64: {
				/* supported-curves */

	uint32_t x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iec == NULL ? 0 : _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iec->supported_curves;
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x);

				}
				break;
			case 65: {
				/* supported-hash-functions */

	int i;
	unsigned x, num;

	x = 0;
	num = 0;
	for (i = br_sha1_ID; i <= br_sha512_ID; i ++) {
		if (br_multihash_getimpl(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, i)) {
			x |= 1U << i;
			num ++;
		}
	}
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x);
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(num);

				}
				break;
			case 66: {
				/* supports-ecdsa? */

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iecdsa != 0));

				}
				break;
			case 67: {
				/* supports-rsa-sign? */

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->irsavrfy != 0));

				}
				break;
			case 68: {
				/* swap */
 _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_SWAP(); 
				}
				break;
			case 69: {
				/* switch-aesgcm-in */

	int is_client, prf_id;
	unsigned cipher_key_len;

	cipher_key_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	is_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_ssl_engine_switch_gcm_in(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, is_client, prf_id,
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iaes_ctr, cipher_key_len);

				}
				break;
			case 70: {
				/* switch-aesgcm-out */

	int is_client, prf_id;
	unsigned cipher_key_len;

	cipher_key_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	is_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_ssl_engine_switch_gcm_out(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, is_client, prf_id,
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iaes_ctr, cipher_key_len);

				}
				break;
			case 71: {
				/* switch-cbc-in */

	int is_client, prf_id, mac_id, aes;
	unsigned cipher_key_len;

	cipher_key_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	aes = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	mac_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	is_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_ssl_engine_switch_cbc_in(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, is_client, prf_id, mac_id,
		aes ? _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iaes_cbcdec : _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->ides_cbcdec, cipher_key_len);

				}
				break;
			case 72: {
				/* switch-cbc-out */

	int is_client, prf_id, mac_id, aes;
	unsigned cipher_key_len;

	cipher_key_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	aes = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	mac_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	is_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_ssl_engine_switch_cbc_out(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, is_client, prf_id, mac_id,
		aes ? _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->iaes_cbcenc : _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->ides_cbcenc, cipher_key_len);

				}
				break;
			case 73: {
				/* switch-chapol-in */

	int is_client, prf_id;

	prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	is_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_ssl_engine_switch_chapol_in(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, is_client, prf_id);

				}
				break;
			case 74: {
				/* switch-chapol-out */

	int is_client, prf_id;

	prf_id = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	is_client = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	br_ssl_engine_switch_chapol_out(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG, is_client, prf_id);

				}
				break;
			case 75: {
				/* test-protocol-name */

	size_t len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	size_t u;

	for (u = 0; u < _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names_num; u ++) {
		const char *name;

		name = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->protocol_names[u];
		if (len == strlen(name) && memcmp(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad, name, len) == 0) {
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(u);
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_RET();
		}
	}
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-1);

				}
				break;
			case 76: {
				/* total-chain-length */

	size_t u;
	uint32_t total;

	total = 0;
	for (u = 0; u < _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain_len; u ++) {
		total += 3 + (uint32_t)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->chain[u].data_len;
	}
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(total);

				}
				break;
			case 77: {
				/* u>> */

	int c = (int)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	uint32_t x = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(x >> c);

				}
				break;
			case 78: {
				/* verify-SKE-sig */

	size_t sig_len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	int use_rsa = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();
	int hash = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POPi();

	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_verify_SKE_sig(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_CTX, hash, use_rsa, sig_len));

				}
				break;
			case 79: {
				/* write-blob-chunk */

	size_t clen = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_out;
	if (clen > 0) {
		uint32_t addr, len;

		len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
		addr = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
		if ((size_t)len < clen) {
			clen = (size_t)len;
		}
		memcpy(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_out, (unsigned char *)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG + addr, clen);
		if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->record_type_out == BR_SSL_HANDSHAKE) {
			br_multihash_update(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_out, clen);
		}
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(addr + (uint32_t)clen);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(len - (uint32_t)clen);
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_out += clen;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_out -= clen;
	}

				}
				break;
			case 80: {
				/* write8-native */

	unsigned char x;

	x = (unsigned char)_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_out > 0) {
		if (_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->record_type_out == BR_SSL_HANDSHAKE) {
			br_multihash_update(&_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->mhash, &x, 1);
		}
		*_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hbuf_out ++ = x;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->hlen_out --;
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(-1);
	} else {
		_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSHi(0);
	}

				}
				break;
			case 81: {
				/* x509-append */

	const br_x509_class *xc;
	size_t len;

	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	len = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	xc->append(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->pad, len);

				}
				break;
			case 82: {
				/* x509-end-cert */

	const br_x509_class *xc;

	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	xc->end_cert(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);

				}
				break;
			case 83: {
				/* x509-end-chain */

	const br_x509_class *xc;

	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_PUSH(xc->end_chain(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx));

				}
				break;
			case 84: {
				/* x509-start-cert */

	const br_x509_class *xc;

	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	xc->start_cert(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx, _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP());

				}
				break;
			case 85: {
				/* x509-start-chain */

	const br_x509_class *xc;
	uint32_t bc;

	bc = _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_POP();
	xc = *(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx);
	xc->start_chain(_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->x509ctx, bc ? _opt_libs_BearSSL_src_ssl_ssl_hs_client_c_ENG->server_name : NULL);

				}
				break;
			}

		} else {
			_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)t0ctx)->dp = dp;
	((_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)t0ctx)->rp = rp;
	((_opt_libs_BearSSL_src_ssl_ssl_hs_client_c_t0_context *)t0ctx)->ip = ip;
}
#endif
