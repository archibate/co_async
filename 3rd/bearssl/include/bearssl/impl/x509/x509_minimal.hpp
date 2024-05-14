#ifndef BEARSSL_IMPL_X509_X509_MINIMAL_HPP
#define BEARSSL_IMPL_X509_X509_MINIMAL_HPP

/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context;

 inline  uint32_t
_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_unsigned(const unsigned char **p)
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
_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_signed(const unsigned char **p)
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

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(x)       _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(x)       _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT3(x)       _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT4(x)       _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_FBYTE(x, 0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT5(x)       _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_SBYTE(x), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 21), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 14), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_VBYTE(x, 7), _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_FBYTE(x, 0)

/* static const unsigned char _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_datablock[]; */


void br_x509_minimal_init_main(void *t0ctx);

void br_x509_minimal_run(void *t0ctx);



#include "inner.h"





#include "inner.h"

/*
 * Implementation Notes
 * --------------------
 *
 * The C code pushes the data by chunks; all decoding is done in the
 * T0 code. The cert_length value is set to the certificate length when
 * a new certificate is started; the T0 code picks it up as outer limit,
 * and decoding functions use it to ensure that no attempt is made at
 * reading past it. The T0 code also checks that once the certificate is
 * decoded, there are no trailing bytes.
 *
 * The T0 code sets cert_length to 0 when the certificate is fully
 * decoded.
 *
 * The C code must still perform two checks:
 *
 *  -- If the certificate length is 0, then the T0 code will not be
 *  invoked at all. This invalid condition must thus be reported by the
 *  C code.
 *
 *  -- When reaching the end of certificate, the C code must verify that
 *  the certificate length has been set to 0, thereby signaling that
 *  the T0 code properly decoded a certificate.
 *
 * Processing of a chain works in the following way:
 *
 *  -- The error flag is set to a non-zero value when validation is
 *  finished. The value is either BR_ERR_X509_OK (validation is
 *  successful) or another non-zero error code. When a non-zero error
 *  code is obtained, the remaining bytes in the current certificate and
 *  the subsequent certificates (if any) are completely ignored.
 *
 *  -- Each certificate is decoded in due course, with the following
 *  "interesting points":
 *
 *     -- Start of the TBS: the multihash engine is reset and activated.
 *
 *     -- Start of the issuer DN: the secondary hash engine is started,
 *     to process the encoded issuer DN.
 *
 *     -- End of the issuer DN: the secondary hash engine is stopped. The
 *     resulting hash value is computed and then copied into the
 *     next_dn_hash[] buffer.
 *
 *     -- Start of the subject DN: the secondary hash engine is started,
 *     to process the encoded subject DN.
 *
 *     -- For the EE certificate only: the Common Name, if any, is matched
 *     against the expected server name.
 *
 *     -- End of the subject DN: the secondary hash engine is stopped. The
 *     resulting hash value is computed into the pad. It is then processed:
 *
 *        -- If this is the EE certificate, then the hash is ignored
 *        (except for direct trust processing, see later; the hash is
 *        simply left in current_dn_hash[]).
 *
 *        -- Otherwise, the hashed subject DN is compared with the saved
 *        hash value (in saved_dn_hash[]). They must match.
 *
 *     Either way, the next_dn_hash[] value is then copied into the
 *     saved_dn_hash[] value. Thus, at that point, saved_dn_hash[]
 *     contains the hash of the issuer DN for the current certificate,
 *     and current_dn_hash[] contains the hash of the subject DN for the
 *     current certificate.
 *
 *     -- Public key: it is decoded into the cert_pkey[] buffer. Unknown
 *     key types are reported at that point.
 *
 *        -- If this is the EE certificate, then the key type is compared
 *        with the expected key type (initialization parameter). The public
 *        key data is copied to ee_pkey_data[]. The key and hashed subject
 *        DN are also compared with the "direct trust" keys; if the key
 *        and DN are matched, then validation ends with a success.
 *
 *        -- Otherwise, the saved signature (cert_sig[]) is verified
 *        against the saved TBS hash (tbs_hash[]) and that freshly
 *        decoded public key. Failure here ends validation with an error.
 *
 *     -- Extensions: extension values are processed in due order.
 *
 *        -- Basic Constraints: for all certificates except EE, must be
 *        present, indicate a CA, and have a path legnth compatible with
 *        the chain length so far.
 *
 *        -- Key Usage: for the EE, if present, must allow signatures
 *        or encryption/key exchange, as required for the cipher suite.
 *        For non-EE, if present, must have the "certificate sign" bit.
 *
 *        -- Subject Alt Name: for the EE, dNSName names are matched
 *        against the server name. Ignored for non-EE.
 *
 *        -- Authority Key Identifier, Subject Key Identifier, Issuer
 *        Alt Name, Subject Directory Attributes, CRL Distribution Points
 *        Freshest CRL, Authority Info Access and Subject Info Access
 *        extensions are always ignored: they either contain only
 *        informative data, or they relate to revocation processing, which
 *        we explicitly do not support.
 *
 *        -- All other extensions are ignored if non-critical. If a
 *        critical extension other than the ones above is encountered,
 *        then a failure is reported.
 *
 *     -- End of the TBS: the multihash engine is stopped.
 *
 *     -- Signature algorithm: the signature algorithm on the
 *     certificate is decoded. A failure is reported if that algorithm
 *     is unknown. The hashed TBS corresponding to the signature hash
 *     function is computed and stored in tbs_hash[] (if not supported,
 *     then a failure is reported). The hash OID and length are stored
 *     in cert_sig_hash_oid and cert_sig_hash_len.
 *
 *     -- Signature value: the signature value is copied into the
 *     cert_sig[] array.
 *
 *     -- Certificate end: the hashed issuer DN (saved_dn_hash[]) is
 *     looked up in the trust store (CA trust anchors only); for all
 *     that match, the signature (cert_sig[]) is verified against the
 *     anchor public key (hashed TBS is in tbs_hash[]). If one of these
 *     signatures is valid, then validation ends with a success.
 *
 *  -- If the chain end is reached without obtaining a validation success,
 *  then validation is reported as failed.
 */

#ifndef _opt_libs_BearSSL_src_x509_x509_minimal_c_BR_USE_UNIX_TIME
#if defined __unix__ || defined __linux__ \
	|| defined _POSIX_SOURCE || defined _POSIX_C_SOURCE \
	|| (defined __APPLE__ && defined __MACH__)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_BR_USE_UNIX_TIME   1
#endif
#endif

#ifndef BR_USE_WIN32_TIME
#if defined _WIN32 || defined _WIN64
#define BR_USE_WIN32_TIME   1
#endif
#endif

#if _opt_libs_BearSSL_src_x509_x509_minimal_c_BR_USE_UNIX_TIME
#include <time.h>
#endif

#if BR_USE_WIN32_TIME
#include <windows.h>
#endif

void br_x509_minimal_init_main(void *ctx);
void br_x509_minimal_run(void *ctx);

/* see bearssl_x509.h */
 inline void
br_x509_minimal_init(br_x509_minimal_context *ctx,
	const br_hash_class *dn_hash_impl,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num)
{
	memset(ctx, 0, sizeof *ctx);
	ctx->vtable = &br_x509_minimal_vtable;
	ctx->dn_hash_impl = dn_hash_impl;
	ctx->trust_anchors = trust_anchors;
	ctx->trust_anchors_num = trust_anchors_num;
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_start_chain(const br_x509_class **ctx, const char *server_name)
{
	br_x509_minimal_context *cc;
	size_t u;

	cc = (br_x509_minimal_context *)ctx;
	for (u = 0; u < cc->num_name_elts; u ++) {
		cc->name_elts[u].status = 0;
		cc->name_elts[u].buf[0] = 0;
	}
	memset(&cc->pkey, 0, sizeof cc->pkey);
	cc->num_certs = 0;
	cc->err = 0;
	cc->cpu.dp = cc->dp_stack;
	cc->cpu.rp = cc->rp_stack;
	br_x509_minimal_init_main(&cc->cpu);
	if (server_name == NULL || *server_name == 0) {
		cc->server_name = NULL;
	} else {
		cc->server_name = server_name;
	}
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_start_cert(const br_x509_class **ctx, uint32_t length)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err != 0) {
		return;
	}
	if (length == 0) {
		cc->err = BR_ERR_X509_TRUNCATED;
		return;
	}
	cc->cert_length = length;
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_append(const br_x509_class **ctx, const unsigned char *buf, size_t len)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err != 0) {
		return;
	}
	cc->hbuf = buf;
	cc->hlen = len;
	br_x509_minimal_run(&cc->cpu);
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_end_cert(const br_x509_class **ctx)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err == 0 && cc->cert_length != 0) {
		cc->err = BR_ERR_X509_TRUNCATED;
	}
	cc->num_certs ++;
}

 inline  unsigned
_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_end_chain(const br_x509_class **ctx)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err == 0) {
		if (cc->num_certs == 0) {
			cc->err = BR_ERR_X509_EMPTY_CHAIN;
		} else {
			cc->err = BR_ERR_X509_NOT_TRUSTED;
		}
	} else if (cc->err == BR_ERR_X509_OK) {
		return 0;
	}
	return (unsigned)cc->err;
}

 inline  const br_x509_pkey *
_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_get_pkey(const br_x509_class *const *ctx, unsigned *usages)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err == BR_ERR_X509_OK
		|| cc->err == BR_ERR_X509_NOT_TRUSTED)
	{
		if (usages != NULL) {
			*usages = cc->key_usages;
		}
		return &((br_x509_minimal_context *)ctx)->pkey;
	} else {
		return NULL;
	}
}

/* see bearssl_x509.h */
 inline const br_x509_class br_x509_minimal_vtable = {
	sizeof(br_x509_minimal_context),
	_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_start_chain,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_start_cert,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_append,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_end_cert,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_end_chain,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_xm_get_pkey
};

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX   ((br_x509_minimal_context *)((unsigned char *)t0ctx - offsetof(br_x509_minimal_context, cpu)))
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME   br_x509_minimal_context

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_DNHASH_LEN   ((_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash_impl->desc >> BR_HASHDESC_OUT_OFF) & BR_HASHDESC_OUT_MASK)

/*
 * Hash a DN (from a trust anchor) into the provided buffer. This uses the
 * DN hash implementation and context structure from the X.509 engine
 * context.
 */
 inline  void
_opt_libs_BearSSL_src_x509_x509_minimal_c_hash_dn(br_x509_minimal_context *ctx, const void *dn, size_t len,
	unsigned char *out)
{
	ctx->dn_hash_impl->init(&ctx->dn_hash.vtable);
	ctx->dn_hash_impl->update(&ctx->dn_hash.vtable, dn, len);
	ctx->dn_hash_impl->out(&ctx->dn_hash.vtable, out);
}

/*
 * Compare two big integers for equality. The integers use unsigned big-endian
 * encoding; extra leading bytes (of value 0) are allowed.
 */
 inline  int
_opt_libs_BearSSL_src_x509_x509_minimal_c_eqbigint(const unsigned char *b1, size_t len1,
	const unsigned char *b2, size_t len2)
{
	while (len1 > 0 && *b1 == 0) {
		b1 ++;
		len1 --;
	}
	while (len2 > 0 && *b2 == 0) {
		b2 ++;
		len2 --;
	}
	if (len1 != len2) {
		return 0;
	}
	return memcmp(b1, b2, len1) == 0;
}

/*
 * Compare two strings for equality, in a case-insensitive way. This
 * function handles casing only for ASCII letters.
 */
 inline  int
_opt_libs_BearSSL_src_x509_x509_minimal_c_eqnocase(const void *s1, const void *s2, size_t len)
{
	const unsigned char *buf1, *buf2;

	buf1 = (unsigned char *)s1;
	buf2 = (unsigned char *)s2;
	while (len -- > 0) {
		int x1, x2;

		x1 = *buf1 ++;
		x2 = *buf2 ++;
		if (x1 >= 'A' && x1 <= 'Z') {
			x1 += 'a' - 'A';
		}
		if (x2 >= 'A' && x2 <= 'Z') {
			x2 += 'a' - 'A';
		}
		if (x1 != x2) {
			return 0;
		}
	}
	return 1;
}

 inline  int _opt_libs_BearSSL_src_x509_x509_minimal_c_verify_signature(br_x509_minimal_context *ctx,
	const br_x509_pkey *pk);



 inline static const unsigned char _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_datablock[] = {
	0x00, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x09,
	0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05, 0x09, 0x2A, 0x86,
	0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0E, 0x09, 0x2A, 0x86, 0x48, 0x86,
	0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D,
	0x01, 0x01, 0x0C, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
	0x0D, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x09, 0x60, 0x86, 0x48, 0x01,
	0x65, 0x03, 0x04, 0x02, 0x04, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03,
	0x04, 0x02, 0x01, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
	0x02, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x07,
	0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x08, 0x2A, 0x86, 0x48, 0xCE,
	0x3D, 0x03, 0x01, 0x07, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22, 0x05, 0x2B,
	0x81, 0x04, 0x00, 0x23, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01,
	0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x01, 0x08, 0x2A, 0x86,
	0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
	0x04, 0x03, 0x03, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x04,
	0x03, 0x55, 0x04, 0x03, 0x00, 0x1F, 0x03, 0xFC, 0x07, 0x7F, 0x0B, 0x5E,
	0x0F, 0x1F, 0x12, 0xFE, 0x16, 0xBF, 0x1A, 0x9F, 0x1E, 0x7E, 0x22, 0x3F,
	0x26, 0x1E, 0x29, 0xDF, 0x00, 0x1F, 0x03, 0xFD, 0x07, 0x9F, 0x0B, 0x7E,
	0x0F, 0x3F, 0x13, 0x1E, 0x16, 0xDF, 0x1A, 0xBF, 0x1E, 0x9E, 0x22, 0x5F,
	0x26, 0x3E, 0x29, 0xFF, 0x03, 0x55, 0x1D, 0x13, 0x03, 0x55, 0x1D, 0x0F,
	0x03, 0x55, 0x1D, 0x11, 0x03, 0x55, 0x1D, 0x23, 0x03, 0x55, 0x1D, 0x0E,
	0x03, 0x55, 0x1D, 0x12, 0x03, 0x55, 0x1D, 0x09, 0x03, 0x55, 0x1D, 0x1F,
	0x03, 0x55, 0x1D, 0x2E, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01,
	0x01, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x0B
};

 inline static const unsigned char _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x0D, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x01,
	0x00, 0x11, 0x00, 0x00, 0x01, 0x01, 0x09, 0x00, 0x00, 0x01, 0x01, 0x0A,
	0x00, 0x00, 0x24, 0x24, 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_BAD_BOOLEAN), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_BAD_DN), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_BAD_SERVER_NAME), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_BAD_TAG_CLASS), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_BAD_TAG_VALUE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_BAD_TIME), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_CRITICAL_EXTENSION), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_DN_MISMATCH), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_EXPIRED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_EXTRA_ELEMENT), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_FORBIDDEN_KEY_USAGE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_INDEFINITE_LENGTH), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_INNER_TRUNC), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_LIMIT_EXCEEDED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_NOT_CA), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_NOT_CONSTRUCTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_NOT_PRIMITIVE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_OVERFLOW), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_PARTIAL_BYTE), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_UNEXPECTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_UNSUPPORTED), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_ERR_X509_WEAK_PUBLIC_KEY), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_KEYTYPE_EC), 0x00, 0x00, 0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT1(BR_KEYTYPE_RSA),
	0x00, 0x00, 0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, cert_length)), 0x00,
	0x00, 0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, cert_sig)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, cert_sig_hash_len)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, cert_sig_hash_oid)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, cert_sig_len)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, cert_signer_key_type)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, current_dn_hash)), 0x00, 0x00,
	0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, key_usages)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(br_x509_minimal_context, pkey_data)), 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(BR_X509_BUFSIZE_KEY), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, min_rsa_size)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, next_dn_hash)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, num_certs)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, pad)), 0x00, 0x00, 0x01,
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(offsetof(_opt_libs_BearSSL_src_x509_x509_minimal_c_CONTEXT_NAME, saved_dn_hash)), 0x00, 0x00, 0xC6, 0x71,
	0x00, 0x00, 0x01, 0x80, 0x73, 0x00, 0x00, 0x01, 0x80, 0x7C, 0x00, 0x00,
	0x01, 0x81, 0x02, 0x00, 0x00, 0x90, 0x05, 0x05, 0x34, 0x42, 0x01, 0x00,
	0x00, 0x34, 0x01, 0x0A, 0x0E, 0x09, 0x01, 0x9A, 0xFF, 0xB8, 0x00, 0x0A,
	0x00, 0x00, 0x01, 0x82, 0x0C, 0x00, 0x00, 0x01, 0x81, 0x74, 0x00, 0x00,
	0x01, 0x81, 0x68, 0x00, 0x04, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02, 0x03,
	0x03, 0x02, 0x03, 0x02, 0x01, 0x11, 0x06, 0x07, 0x02, 0x02, 0x02, 0x00,
	0x0D, 0x04, 0x05, 0x02, 0x03, 0x02, 0x01, 0x0D, 0x00, 0x02, 0x03, 0x00,
	0x03, 0x01, 0x25, 0x02, 0x01, 0x13, 0x3B, 0x02, 0x00, 0x0F, 0x15, 0x00,
	0x00, 0x05, 0x02, 0x52, 0x28, 0x00, 0x00, 0x06, 0x02, 0x53, 0x28, 0x00,
	0x00, 0x01, 0x10, 0x76, 0x00, 0x00, 0x11, 0x05, 0x02, 0x56, 0x28, 0x73,
	0x00, 0x00, 0x11, 0x05, 0x02, 0x56, 0x28, 0x74, 0x00, 0x00, 0x06, 0x02,
	0x4C, 0x28, 0x00, 0x00, 0x01, 0x82, 0x04, 0x00, 0x00, 0x25, 0x20, 0x01,
	0x08, 0x0E, 0x3B, 0x40, 0x20, 0x09, 0x00, 0x09, 0x03, 0x00, 0x5B, 0x2B,
	0xAC, 0x39, 0xAC, 0xB0, 0x25, 0x01, 0x20, 0x11, 0x06, 0x11, 0x24, 0x73,
	0xAA, 0xB0, 0x01, 0x02, 0x77, 0xAD, 0x01, 0x02, 0x12, 0x06, 0x02, 0x57,
	0x28, 0x78, 0xB0, 0x01, 0x02, 0x77, 0xAB, 0xAC, 0xBF, 0x99, 0x65, 0x61,
	0x21, 0x16, 0xAC, 0xA4, 0x29, 0x69, 0x06, 0x02, 0x4B, 0x28, 0xA4, 0x29,
	0x71, 0x06, 0x02, 0x4B, 0x28, 0x78, 0x02, 0x00, 0x06, 0x05, 0x9A, 0x03,
	0x01, 0x04, 0x09, 0x99, 0x61, 0x68, 0x21, 0x27, 0x05, 0x02, 0x4A, 0x28,
	0x68, 0x65, 0x21, 0x16, 0xAC, 0xAC, 0x9B, 0x05, 0x02, 0x57, 0x28, 0xB9,
	0x26, 0x06, 0x27, 0xBF, 0xA1, 0xAC, 0x63, 0xA7, 0x03, 0x03, 0x63, 0x3B,
	0x02, 0x03, 0x09, 0x3B, 0x02, 0x03, 0x0A, 0xA7, 0x03, 0x04, 0x78, 0x64,
	0x2A, 0x01, 0x81, 0x00, 0x09, 0x02, 0x03, 0x12, 0x06, 0x02, 0x58, 0x28,
	0x78, 0x5A, 0x03, 0x02, 0x04, 0x3A, 0x87, 0x26, 0x06, 0x34, 0x9B, 0x05,
	0x02, 0x57, 0x28, 0x6A, 0x26, 0x06, 0x04, 0x01, 0x17, 0x04, 0x12, 0x6B,
	0x26, 0x06, 0x04, 0x01, 0x18, 0x04, 0x0A, 0x6C, 0x26, 0x06, 0x04, 0x01,
	0x19, 0x04, 0x02, 0x57, 0x28, 0x03, 0x05, 0x78, 0xA1, 0x25, 0x03, 0x06,
	0x25, 0x63, 0x34, 0x0D, 0x06, 0x02, 0x50, 0x28, 0xA2, 0x59, 0x03, 0x02,
	0x04, 0x02, 0x57, 0x28, 0x78, 0x02, 0x00, 0x06, 0x21, 0x02, 0x02, 0x5A,
	0x30, 0x11, 0x06, 0x08, 0x24, 0x02, 0x03, 0x02, 0x04, 0x1D, 0x04, 0x10,
	0x59, 0x30, 0x11, 0x06, 0x08, 0x24, 0x02, 0x05, 0x02, 0x06, 0x1C, 0x04,
	0x03, 0x57, 0x28, 0x24, 0x04, 0x24, 0x02, 0x02, 0x5A, 0x30, 0x11, 0x06,
	0x08, 0x24, 0x02, 0x03, 0x02, 0x04, 0x23, 0x04, 0x10, 0x59, 0x30, 0x11,
	0x06, 0x08, 0x24, 0x02, 0x05, 0x02, 0x06, 0x22, 0x04, 0x03, 0x57, 0x28,
	0x24, 0x25, 0x06, 0x01, 0x28, 0x24, 0x01, 0x00, 0x03, 0x07, 0xB1, 0x01,
	0x21, 0x8D, 0x01, 0x22, 0x8D, 0x25, 0x01, 0x23, 0x11, 0x06, 0x81, 0x17,
	0x24, 0x73, 0xAA, 0xAC, 0x25, 0x06, 0x81, 0x0B, 0x01, 0x00, 0x03, 0x08,
	0xAC, 0x9B, 0x24, 0xB0, 0x25, 0x01, 0x01, 0x11, 0x06, 0x04, 0xA3, 0x03,
	0x08, 0xB0, 0x01, 0x04, 0x77, 0xAA, 0x70, 0x26, 0x06, 0x0F, 0x02, 0x00,
	0x06, 0x03, 0xC0, 0x04, 0x05, 0x97, 0x01, 0x7F, 0x03, 0x07, 0x04, 0x80,
	0x5D, 0x8F, 0x26, 0x06, 0x06, 0x02, 0x00, 0x98, 0x04, 0x80, 0x53, 0xC2,
	0x26, 0x06, 0x10, 0x02, 0x00, 0x06, 0x09, 0x01, 0x00, 0x03, 0x01, 0x96,
	0x03, 0x01, 0x04, 0x01, 0xC0, 0x04, 0x3F, 0x6F, 0x26, 0x06, 0x03, 0xC0,
	0x04, 0x38, 0xC5, 0x26, 0x06, 0x03, 0xC0, 0x04, 0x31, 0x8E, 0x26, 0x06,
	0x03, 0xC0, 0x04, 0x2A, 0xC3, 0x26, 0x06, 0x03, 0xC0, 0x04, 0x23, 0x79,
	0x26, 0x06, 0x03, 0xC0, 0x04, 0x1C, 0x84, 0x26, 0x06, 0x03, 0xC0, 0x04,
	0x15, 0x6E, 0x26, 0x06, 0x03, 0xC0, 0x04, 0x0E, 0xC4, 0x26, 0x06, 0x03,
	0xC0, 0x04, 0x07, 0x02, 0x08, 0x06, 0x02, 0x49, 0x28, 0xC0, 0x78, 0x78,
	0x04, 0xFE, 0x71, 0x78, 0x78, 0x04, 0x08, 0x01, 0x7F, 0x11, 0x05, 0x02,
	0x56, 0x28, 0x24, 0x78, 0x3A, 0x02, 0x00, 0x06, 0x08, 0x02, 0x01, 0x3C,
	0x2F, 0x05, 0x02, 0x45, 0x28, 0x02, 0x00, 0x06, 0x01, 0x17, 0x02, 0x00,
	0x02, 0x07, 0x2F, 0x05, 0x02, 0x51, 0x28, 0xB0, 0x75, 0xAA, 0x9B, 0x06,
	0x80, 0x77, 0xBA, 0x26, 0x06, 0x07, 0x01, 0x02, 0x5A, 0x88, 0x04, 0x80,
	0x5E, 0xBB, 0x26, 0x06, 0x07, 0x01, 0x03, 0x5A, 0x89, 0x04, 0x80, 0x53,
	0xBC, 0x26, 0x06, 0x07, 0x01, 0x04, 0x5A, 0x8A, 0x04, 0x80, 0x48, 0xBD,
	0x26, 0x06, 0x06, 0x01, 0x05, 0x5A, 0x8B, 0x04, 0x3E, 0xBE, 0x26, 0x06,
	0x06, 0x01, 0x06, 0x5A, 0x8C, 0x04, 0x34, 0x7E, 0x26, 0x06, 0x06, 0x01,
	0x02, 0x59, 0x88, 0x04, 0x2A, 0x7F, 0x26, 0x06, 0x06, 0x01, 0x03, 0x59,
	0x89, 0x04, 0x20, 0x80, 0x26, 0x06, 0x06, 0x01, 0x04, 0x59, 0x8A, 0x04,
	0x16, 0x81, 0x26, 0x06, 0x06, 0x01, 0x05, 0x59, 0x8B, 0x04, 0x0C, 0x82,
	0x26, 0x06, 0x06, 0x01, 0x06, 0x59, 0x8C, 0x04, 0x02, 0x57, 0x28, 0x5E,
	0x35, 0x60, 0x37, 0x1B, 0x25, 0x05, 0x02, 0x57, 0x28, 0x5D, 0x37, 0x04,
	0x02, 0x57, 0x28, 0xBF, 0xA1, 0x25, 0x01, _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INT2(BR_X509_BUFSIZE_SIG),
	0x12, 0x06, 0x02, 0x50, 0x28, 0x25, 0x5F, 0x35, 0x5C, 0xA2, 0x78, 0x78,
	0x01, 0x00, 0x5B, 0x36, 0x18, 0x00, 0x00, 0x01, 0x30, 0x0A, 0x25, 0x01,
	0x00, 0x01, 0x09, 0x72, 0x05, 0x02, 0x48, 0x28, 0x00, 0x00, 0x30, 0x30,
	0x00, 0x00, 0x01, 0x81, 0x08, 0x00, 0x00, 0x01, 0x81, 0x10, 0x00, 0x00,
	0x01, 0x81, 0x19, 0x00, 0x00, 0x01, 0x81, 0x22, 0x00, 0x00, 0x01, 0x81,
	0x2B, 0x00, 0x01, 0x7D, 0x01, 0x01, 0x11, 0x3B, 0x01, 0x83, 0xFD, 0x7F,
	0x11, 0x15, 0x06, 0x03, 0x3B, 0x24, 0x00, 0x3B, 0x25, 0x03, 0x00, 0x25,
	0xC7, 0x05, 0x04, 0x42, 0x01, 0x00, 0x00, 0x25, 0x01, 0x81, 0x00, 0x0D,
	0x06, 0x04, 0x94, 0x04, 0x80, 0x49, 0x25, 0x01, 0x90, 0x00, 0x0D, 0x06,
	0x0F, 0x01, 0x06, 0x14, 0x01, 0x81, 0x40, 0x2F, 0x94, 0x02, 0x00, 0x01,
	0x00, 0x95, 0x04, 0x33, 0x25, 0x01, 0x83, 0xFF, 0x7F, 0x0D, 0x06, 0x14,
	0x01, 0x0C, 0x14, 0x01, 0x81, 0x60, 0x2F, 0x94, 0x02, 0x00, 0x01, 0x06,
	0x95, 0x02, 0x00, 0x01, 0x00, 0x95, 0x04, 0x17, 0x01, 0x12, 0x14, 0x01,
	0x81, 0x70, 0x2F, 0x94, 0x02, 0x00, 0x01, 0x0C, 0x95, 0x02, 0x00, 0x01,
	0x06, 0x95, 0x02, 0x00, 0x01, 0x00, 0x95, 0x00, 0x00, 0x01, 0x82, 0x08,
	0x00, 0x00, 0x25, 0x01, 0x83, 0xB0, 0x00, 0x01, 0x83, 0xB7, 0x7F, 0x72,
	0x00, 0x00, 0x01, 0x81, 0x34, 0x00, 0x00, 0x01, 0x80, 0x6B, 0x00, 0x00,
	0x01, 0x3D, 0x00, 0x00, 0x01, 0x80, 0x43, 0x00, 0x00, 0x01, 0x80, 0x4D,
	0x00, 0x00, 0x01, 0x80, 0x57, 0x00, 0x00, 0x01, 0x80, 0x61, 0x00, 0x00,
	0x30, 0x11, 0x06, 0x04, 0x42, 0xAA, 0xBF, 0xB1, 0x00, 0x00, 0x01, 0x81,
	0x7C, 0x00, 0x00, 0x01, 0x81, 0x6C, 0x00, 0x00, 0x25, 0x01, 0x83, 0xB8,
	0x00, 0x01, 0x83, 0xBF, 0x7F, 0x72, 0x00, 0x00, 0x01, 0x30, 0x62, 0x37,
	0x01, 0x7F, 0x7B, 0x19, 0x01, 0x00, 0x7B, 0x19, 0x04, 0x7A, 0x00, 0x01,
	0x81, 0x38, 0x00, 0x01, 0x7D, 0x0D, 0x06, 0x02, 0x4F, 0x28, 0x25, 0x03,
	0x00, 0x0A, 0x02, 0x00, 0x00, 0x00, 0x30, 0x25, 0x3F, 0x3B, 0x01, 0x82,
	0x00, 0x13, 0x2F, 0x06, 0x04, 0x42, 0x01, 0x00, 0x00, 0x30, 0x67, 0x09,
	0x37, 0x40, 0x00, 0x00, 0x14, 0x01, 0x3F, 0x15, 0x01, 0x81, 0x00, 0x2F,
	0x94, 0x00, 0x02, 0x01, 0x00, 0x03, 0x00, 0xAC, 0x25, 0x06, 0x80, 0x59,
	0xB0, 0x01, 0x20, 0x30, 0x11, 0x06, 0x17, 0x24, 0x73, 0xAA, 0x9B, 0x24,
	0x01, 0x7F, 0x2E, 0x03, 0x01, 0xB0, 0x01, 0x20, 0x76, 0xAA, 0xAF, 0x02,
	0x01, 0x1F, 0x78, 0x78, 0x04, 0x38, 0x01, 0x21, 0x30, 0x11, 0x06, 0x08,
	0x24, 0x74, 0xB3, 0x01, 0x01, 0x1E, 0x04, 0x2A, 0x01, 0x22, 0x30, 0x11,
	0x06, 0x11, 0x24, 0x74, 0xB3, 0x25, 0x06, 0x06, 0x2C, 0x02, 0x00, 0x2F,
	0x03, 0x00, 0x01, 0x02, 0x1E, 0x04, 0x13, 0x01, 0x26, 0x30, 0x11, 0x06,
	0x08, 0x24, 0x74, 0xB3, 0x01, 0x06, 0x1E, 0x04, 0x05, 0x42, 0xAB, 0x01,
	0x00, 0x24, 0x04, 0xFF, 0x23, 0x78, 0x02, 0x00, 0x00, 0x00, 0xAC, 0xB1,
	0x25, 0x01, 0x01, 0x11, 0x06, 0x08, 0xA3, 0x05, 0x02, 0x51, 0x28, 0xB1,
	0x04, 0x02, 0x51, 0x28, 0x25, 0x01, 0x02, 0x11, 0x06, 0x0C, 0x24, 0x74,
	0xAD, 0x66, 0x2B, 0x41, 0x0D, 0x06, 0x02, 0x51, 0x28, 0xB1, 0x01, 0x7F,
	0x10, 0x06, 0x02, 0x56, 0x28, 0x24, 0x78, 0x00, 0x02, 0x03, 0x00, 0xB0,
	0x01, 0x03, 0x77, 0xAA, 0xB7, 0x03, 0x01, 0x02, 0x01, 0x01, 0x07, 0x12,
	0x06, 0x02, 0x56, 0x28, 0x25, 0x01, 0x00, 0x30, 0x11, 0x06, 0x05, 0x24,
	0x4D, 0x28, 0x04, 0x15, 0x01, 0x01, 0x30, 0x11, 0x06, 0x0A, 0x24, 0xB7,
	0x02, 0x01, 0x14, 0x02, 0x01, 0x0E, 0x04, 0x05, 0x24, 0xB7, 0x01, 0x00,
	0x24, 0x02, 0x00, 0x06, 0x19, 0x01, 0x00, 0x30, 0x01, 0x38, 0x15, 0x06,
	0x03, 0x01, 0x10, 0x2F, 0x3B, 0x01, 0x81, 0x40, 0x15, 0x06, 0x03, 0x01,
	0x20, 0x2F, 0x62, 0x37, 0x04, 0x07, 0x01, 0x04, 0x15, 0x05, 0x02, 0x4D,
	0x28, 0xBF, 0x00, 0x00, 0x38, 0xAC, 0xBF, 0x1A, 0x00, 0x03, 0x01, 0x00,
	0x03, 0x00, 0x38, 0xAC, 0x25, 0x06, 0x30, 0xB0, 0x01, 0x11, 0x76, 0xAA,
	0x25, 0x05, 0x02, 0x44, 0x28, 0x25, 0x06, 0x20, 0xAC, 0x9B, 0x24, 0x86,
	0x26, 0x03, 0x01, 0x01, 0x00, 0x2E, 0x03, 0x02, 0xAF, 0x25, 0x02, 0x01,
	0x15, 0x06, 0x07, 0x2C, 0x06, 0x04, 0x01, 0x7F, 0x03, 0x00, 0x02, 0x02,
	0x1F, 0x78, 0x04, 0x5D, 0x78, 0x04, 0x4D, 0x78, 0x1A, 0x02, 0x00, 0x00,
	0x00, 0xB0, 0x01, 0x06, 0x77, 0xAE, 0x00, 0x00, 0xB5, 0x85, 0x06, 0x0E,
	0x3B, 0x25, 0x05, 0x06, 0x42, 0x01, 0x00, 0x01, 0x00, 0x00, 0xB5, 0x6D,
	0x04, 0x08, 0x90, 0x06, 0x05, 0x24, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00,
	0xB6, 0x85, 0x06, 0x0E, 0x3B, 0x25, 0x05, 0x06, 0x42, 0x01, 0x00, 0x01,
	0x00, 0x00, 0xB6, 0x6D, 0x04, 0x08, 0x90, 0x06, 0x05, 0x24, 0x01, 0x00,
	0x04, 0x00, 0x00, 0x00, 0xB7, 0x25, 0x01, 0x81, 0x00, 0x0D, 0x06, 0x04,
	0x00, 0x04, 0x80, 0x55, 0x25, 0x01, 0x81, 0x40, 0x0D, 0x06, 0x07, 0x24,
	0x01, 0x00, 0x00, 0x04, 0x80, 0x47, 0x25, 0x01, 0x81, 0x60, 0x0D, 0x06,
	0x0E, 0x01, 0x1F, 0x15, 0x01, 0x01, 0xA0, 0x01, 0x81, 0x00, 0x01, 0x8F,
	0x7F, 0x04, 0x32, 0x25, 0x01, 0x81, 0x70, 0x0D, 0x06, 0x0F, 0x01, 0x0F,
	0x15, 0x01, 0x02, 0xA0, 0x01, 0x90, 0x00, 0x01, 0x83, 0xFF, 0x7F, 0x04,
	0x1C, 0x25, 0x01, 0x81, 0x78, 0x0D, 0x06, 0x11, 0x01, 0x07, 0x15, 0x01,
	0x03, 0xA0, 0x01, 0x84, 0x80, 0x00, 0x01, 0x80, 0xC3, 0xFF, 0x7F, 0x04,
	0x04, 0x24, 0x01, 0x00, 0x00, 0x72, 0x05, 0x03, 0x24, 0x01, 0x00, 0x00,
	0x00, 0x3B, 0x25, 0x05, 0x06, 0x42, 0x01, 0x00, 0x01, 0x7F, 0x00, 0xB7,
	0x34, 0x25, 0x3D, 0x06, 0x03, 0x3B, 0x24, 0x00, 0x01, 0x06, 0x0E, 0x3B,
	0x25, 0x01, 0x06, 0x14, 0x01, 0x02, 0x10, 0x06, 0x04, 0x42, 0x01, 0x7F,
	0x00, 0x01, 0x3F, 0x15, 0x09, 0x00, 0x00, 0x25, 0x06, 0x06, 0x0B, 0x9F,
	0x34, 0x41, 0x04, 0x77, 0x24, 0x25, 0x00, 0x00, 0xB0, 0x01, 0x03, 0x77,
	0xAA, 0xB7, 0x06, 0x02, 0x55, 0x28, 0x00, 0x00, 0x3B, 0x25, 0x06, 0x07,
	0x31, 0x25, 0x06, 0x01, 0x19, 0x04, 0x76, 0x42, 0x00, 0x00, 0x01, 0x01,
	0x77, 0xA9, 0x01, 0x01, 0x10, 0x06, 0x02, 0x43, 0x28, 0xB7, 0x3E, 0x00,
	0x04, 0xB0, 0x25, 0x01, 0x17, 0x01, 0x18, 0x72, 0x05, 0x02, 0x48, 0x28,
	0x01, 0x18, 0x11, 0x03, 0x00, 0x74, 0xAA, 0xA5, 0x02, 0x00, 0x06, 0x0C,
	0x01, 0x80, 0x64, 0x08, 0x03, 0x01, 0xA5, 0x02, 0x01, 0x09, 0x04, 0x0E,
	0x25, 0x01, 0x32, 0x0D, 0x06, 0x04, 0x01, 0x80, 0x64, 0x09, 0x01, 0x8E,
	0x6C, 0x09, 0x03, 0x01, 0x02, 0x01, 0x01, 0x82, 0x6D, 0x08, 0x02, 0x01,
	0x01, 0x03, 0x09, 0x01, 0x04, 0x0C, 0x09, 0x02, 0x01, 0x01, 0x80, 0x63,
	0x09, 0x01, 0x80, 0x64, 0x0C, 0x0A, 0x02, 0x01, 0x01, 0x83, 0x0F, 0x09,
	0x01, 0x83, 0x10, 0x0C, 0x09, 0x03, 0x03, 0x01, 0x01, 0x01, 0x0C, 0xA6,
	0x41, 0x01, 0x01, 0x0E, 0x02, 0x01, 0x01, 0x04, 0x07, 0x3F, 0x02, 0x01,
	0x01, 0x80, 0x64, 0x07, 0x3E, 0x02, 0x01, 0x01, 0x83, 0x10, 0x07, 0x3F,
	0x2F, 0x15, 0x06, 0x03, 0x01, 0x18, 0x09, 0x92, 0x09, 0x7A, 0x25, 0x01,
	0x05, 0x14, 0x02, 0x03, 0x09, 0x03, 0x03, 0x01, 0x1F, 0x15, 0x01, 0x01,
	0x3B, 0xA6, 0x02, 0x03, 0x09, 0x41, 0x03, 0x03, 0x01, 0x00, 0x01, 0x17,
	0xA6, 0x01, 0x9C, 0x10, 0x08, 0x03, 0x02, 0x01, 0x00, 0x01, 0x3B, 0xA6,
	0x01, 0x3C, 0x08, 0x02, 0x02, 0x09, 0x03, 0x02, 0x01, 0x00, 0x01, 0x3C,
	0xA6, 0x02, 0x02, 0x09, 0x03, 0x02, 0xB7, 0x25, 0x01, 0x2E, 0x11, 0x06,
	0x0D, 0x24, 0xB7, 0x25, 0x01, 0x30, 0x01, 0x39, 0x72, 0x06, 0x03, 0x24,
	0x04, 0x74, 0x01, 0x80, 0x5A, 0x10, 0x06, 0x02, 0x48, 0x28, 0x78, 0x02,
	0x03, 0x02, 0x02, 0x00, 0x01, 0xB7, 0x7C, 0x01, 0x0A, 0x08, 0x03, 0x00,
	0xB7, 0x7C, 0x02, 0x00, 0x09, 0x00, 0x02, 0x03, 0x00, 0x03, 0x01, 0xA5,
	0x25, 0x02, 0x01, 0x02, 0x00, 0x72, 0x05, 0x02, 0x48, 0x28, 0x00, 0x00,
	0x34, 0xB0, 0x01, 0x02, 0x77, 0x0B, 0xA8, 0x00, 0x03, 0x25, 0x03, 0x00,
	0x03, 0x01, 0x03, 0x02, 0xAA, 0xB7, 0x25, 0x01, 0x81, 0x00, 0x13, 0x06,
	0x02, 0x54, 0x28, 0x25, 0x01, 0x00, 0x11, 0x06, 0x0B, 0x24, 0x25, 0x05,
	0x04, 0x24, 0x01, 0x00, 0x00, 0xB7, 0x04, 0x6F, 0x02, 0x01, 0x25, 0x05,
	0x02, 0x50, 0x28, 0x41, 0x03, 0x01, 0x02, 0x02, 0x37, 0x02, 0x02, 0x40,
	0x03, 0x02, 0x25, 0x06, 0x03, 0xB7, 0x04, 0x68, 0x24, 0x02, 0x00, 0x02,
	0x01, 0x0A, 0x00, 0x01, 0xB7, 0x25, 0x01, 0x81, 0x00, 0x0D, 0x06, 0x01,
	0x00, 0x01, 0x81, 0x00, 0x0A, 0x25, 0x05, 0x02, 0x4E, 0x28, 0x03, 0x00,
	0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x12, 0x06, 0x19, 0x02, 0x00, 0x41,
	0x03, 0x00, 0x25, 0x01, 0x83, 0xFF, 0xFF, 0x7F, 0x12, 0x06, 0x02, 0x4F,
	0x28, 0x01, 0x08, 0x0E, 0x3B, 0xB7, 0x34, 0x09, 0x04, 0x60, 0x00, 0x00,
	0xA9, 0x93, 0x00, 0x00, 0xAA, 0xBF, 0x00, 0x00, 0xB0, 0x75, 0xAA, 0x00,
	0x01, 0xAA, 0x25, 0x05, 0x02, 0x54, 0x28, 0xB7, 0x25, 0x01, 0x81, 0x00,
	0x13, 0x06, 0x02, 0x54, 0x28, 0x03, 0x00, 0x25, 0x06, 0x16, 0xB7, 0x02,
	0x00, 0x25, 0x01, 0x87, 0xFF, 0xFF, 0x7F, 0x13, 0x06, 0x02, 0x54, 0x28,
	0x01, 0x08, 0x0E, 0x09, 0x03, 0x00, 0x04, 0x67, 0x24, 0x02, 0x00, 0x00,
	0x00, 0xAA, 0x25, 0x01, 0x81, 0x7F, 0x12, 0x06, 0x08, 0xBF, 0x01, 0x00,
	0x67, 0x37, 0x01, 0x00, 0x00, 0x25, 0x67, 0x37, 0x67, 0x40, 0xA2, 0x01,
	0x7F, 0x00, 0x00, 0xB0, 0x01, 0x0C, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74,
	0xB3, 0x04, 0x3E, 0x01, 0x12, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4,
	0x04, 0x33, 0x01, 0x13, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4, 0x04,
	0x28, 0x01, 0x14, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4, 0x04, 0x1D,
	0x01, 0x16, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4, 0x04, 0x12, 0x01,
	0x1E, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB2, 0x04, 0x07, 0x42, 0xAB,
	0x01, 0x00, 0x01, 0x00, 0x24, 0x00, 0x01, 0xB7, 0x03, 0x00, 0x02, 0x00,
	0x01, 0x05, 0x14, 0x01, 0x01, 0x15, 0x2D, 0x02, 0x00, 0x01, 0x06, 0x14,
	0x25, 0x01, 0x01, 0x15, 0x06, 0x02, 0x46, 0x28, 0x01, 0x04, 0x0E, 0x02,
	0x00, 0x01, 0x1F, 0x15, 0x25, 0x01, 0x1F, 0x11, 0x06, 0x02, 0x47, 0x28,
	0x09, 0x00, 0x00, 0x25, 0x05, 0x05, 0x01, 0x00, 0x01, 0x7F, 0x00, 0xB0,
	0x00, 0x01, 0xAA, 0x25, 0x05, 0x05, 0x67, 0x37, 0x01, 0x7F, 0x00, 0x01,
	0x01, 0x03, 0x00, 0x9C, 0x25, 0x01, 0x83, 0xFF, 0x7E, 0x11, 0x06, 0x16,
	0x24, 0x25, 0x06, 0x10, 0x9D, 0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00,
	0x00, 0x02, 0x00, 0x83, 0x03, 0x00, 0x04, 0x6D, 0x04, 0x1B, 0x25, 0x05,
	0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x02, 0x00, 0x83, 0x03, 0x00, 0x25,
	0x06, 0x0B, 0x9C, 0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x04,
	0x6D, 0x24, 0x02, 0x00, 0x25, 0x05, 0x01, 0x00, 0x41, 0x67, 0x37, 0x01,
	0x7F, 0x00, 0x01, 0xAA, 0x01, 0x01, 0x03, 0x00, 0x25, 0x06, 0x10, 0x9E,
	0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x02, 0x00, 0x83, 0x03,
	0x00, 0x04, 0x6D, 0x24, 0x02, 0x00, 0x25, 0x05, 0x01, 0x00, 0x41, 0x67,
	0x37, 0x01, 0x7F, 0x00, 0x01, 0xAA, 0x01, 0x01, 0x03, 0x00, 0x25, 0x06,
	0x10, 0xB7, 0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x02, 0x00,
	0x83, 0x03, 0x00, 0x04, 0x6D, 0x24, 0x02, 0x00, 0x25, 0x05, 0x01, 0x00,
	0x41, 0x67, 0x37, 0x01, 0x7F, 0x00, 0x00, 0xB7, 0x01, 0x08, 0x0E, 0x3B,
	0xB7, 0x34, 0x09, 0x00, 0x00, 0xB7, 0x3B, 0xB7, 0x01, 0x08, 0x0E, 0x34,
	0x09, 0x00, 0x00, 0x25, 0x05, 0x02, 0x4F, 0x28, 0x41, 0xB8, 0x00, 0x00,
	0x32, 0x25, 0x01, 0x00, 0x13, 0x06, 0x01, 0x00, 0x24, 0x19, 0x04, 0x74,
	0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x01, 0x15, 0x00,
	0x00, 0x01, 0x1F, 0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x01, 0x33, 0x00,
	0x00, 0xC0, 0x24, 0x00, 0x00, 0x25, 0x06, 0x07, 0xC1, 0x25, 0x06, 0x01,
	0x19, 0x04, 0x76, 0x00, 0x00, 0x01, 0x00, 0x30, 0x31, 0x0B, 0x42, 0x00,
	0x00, 0x01, 0x81, 0x70, 0x00, 0x00, 0x01, 0x82, 0x00, 0x00, 0x00, 0x01,
	0x82, 0x15, 0x00, 0x00, 0x01, 0x81, 0x78, 0x00, 0x00, 0x01, 0x03, 0x33,
	0x01, 0x03, 0x33, 0x00, 0x00, 0x25, 0x01, 0x83, 0xFB, 0x50, 0x01, 0x83,
	0xFD, 0x5F, 0x72, 0x06, 0x04, 0x24, 0x01, 0x00, 0x00, 0x25, 0x01, 0x83,
	0xB0, 0x00, 0x01, 0x83, 0xBF, 0x7F, 0x72, 0x06, 0x04, 0x24, 0x01, 0x00,
	0x00, 0x01, 0x83, 0xFF, 0x7F, 0x15, 0x01, 0x83, 0xFF, 0x7E, 0x0D, 0x00
};

 inline static const uint16_t _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_caddr[] = {
	0,
	5,
	10,
	15,
	20,
	25,
	29,
	33,
	37,
	41,
	45,
	49,
	53,
	57,
	61,
	65,
	69,
	73,
	77,
	81,
	85,
	89,
	93,
	97,
	101,
	105,
	109,
	113,
	117,
	121,
	125,
	130,
	135,
	140,
	145,
	150,
	155,
	160,
	165,
	173,
	178,
	183,
	188,
	193,
	198,
	202,
	207,
	212,
	217,
	238,
	243,
	248,
	253,
	282,
	297,
	303,
	309,
	314,
	322,
	330,
	336,
	341,
	352,
	972,
	987,
	991,
	996,
	1001,
	1006,
	1011,
	1016,
	1130,
	1135,
	1147,
	1152,
	1157,
	1161,
	1166,
	1171,
	1176,
	1181,
	1191,
	1196,
	1201,
	1213,
	1228,
	1233,
	1247,
	1269,
	1280,
	1383,
	1430,
	1521,
	1527,
	1590,
	1597,
	1625,
	1653,
	1758,
	1800,
	1813,
	1825,
	1839,
	1854,
	2074,
	2088,
	2105,
	2114,
	2181,
	2237,
	2241,
	2245,
	2250,
	2298,
	2324,
	2400,
	2444,
	2455,
	2540,
	2578,
	2616,
	2626,
	2636,
	2645,
	2658,
	2662,
	2666,
	2670,
	2674,
	2678,
	2682,
	2686,
	2698,
	2706,
	2711,
	2716,
	2721,
	2726,
	2734
};

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INTERPRETED   61

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_codeblock[_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_caddr[(slot) - _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INTERPRETED]]; \
		t0_lnum = _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *t0ctx = (_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)ctx; \
	t0ctx->ip = &_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_codeblock[0]; \
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_DEFENTRY(br_x509_minimal_init_main, 145)

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

 inline void
br_x509_minimal_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_LOCAL(x)    (*(rp - 2 - (x)))
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP()       (*-- dp)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi()      (*(int32_t *)(-- dp))
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PEEK(x)     (*(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RPOP()      (*-- rp)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RPOPi()     (*(int32_t *)(-- rp))
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PEEK(t0depth)); \
} while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RET()        goto t0_next

	dp = ((_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)t0ctx)->dp;
	rp = ((_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)t0ctx)->rp;
	ip = ((_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_NEXT(&ip);
		if (t0x < _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_LOCAL(_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_LOCAL(_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_unsigned(&ip)) = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
				break;
			case 4: /* jump */
				t0off = _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_signed(&ip);
				if (_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = _opt_libs_BearSSL_src_x509_x509_minimal_c_t0_parse7E_signed(&ip);
				if (!_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* %25 */

	int32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(a % b);

				}
				break;
			case 8: {
				/* * */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(a * b);

				}
				break;
			case 9: {
				/* + */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(a + b);

				}
				break;
			case 10: {
				/* - */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(a - b);

				}
				break;
			case 11: {
				/* -rot */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_NROT(); 
				}
				break;
			case 12: {
				/* / */

	int32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(a / b);

				}
				break;
			case 13: {
				/* < */

	int32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 14: {
				/* << */

	int c = (int)_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	uint32_t x = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(x << c);

				}
				break;
			case 15: {
				/* <= */

	int32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 16: {
				/* <> */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 17: {
				/* = */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 18: {
				/* > */

	int32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 19: {
				/* >= */

	int32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 20: {
				/* >> */

	int c = (int)_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int32_t x = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(x >> c);

				}
				break;
			case 21: {
				/* and */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(a & b);

				}
				break;
			case 22: {
				/* blobcopy */

	size_t len = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	unsigned char *src = (unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	unsigned char *dst = (unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	memcpy(dst, src, len);

				}
				break;
			case 23: {
				/* check-direct-trust */

	size_t u;

	for (u = 0; u < _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->trust_anchors_num; u ++) {
		const br_x509_trust_anchor *ta;
		unsigned char hashed_DN[64];
		int kt;

		ta = &_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->trust_anchors[u];
		if (ta->flags & BR_X509_TA_CA) {
			continue;
		}
		_opt_libs_BearSSL_src_x509_x509_minimal_c_hash_dn(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX, ta->dn.data, ta->dn.len, hashed_DN);
		if (memcmp(hashed_DN, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->current_dn_hash, _opt_libs_BearSSL_src_x509_x509_minimal_c_DNHASH_LEN)) {
			continue;
		}
		kt = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key_type;
		if ((ta->pkey.key_type & 0x0F) != kt) {
			continue;
		}
		switch (kt) {

		case BR_KEYTYPE_RSA:
			if (!_opt_libs_BearSSL_src_x509_x509_minimal_c_eqbigint(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.n,
				_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.nlen,
				ta->pkey.key.rsa.n,
				ta->pkey.key.rsa.nlen)
				|| !_opt_libs_BearSSL_src_x509_x509_minimal_c_eqbigint(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.e,
				_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.elen,
				ta->pkey.key.rsa.e,
				ta->pkey.key.rsa.elen))
			{
				continue;
			}
			break;

		case BR_KEYTYPE_EC:
			if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.ec.curve != ta->pkey.key.ec.curve
				|| _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.ec.qlen != ta->pkey.key.ec.qlen
				|| memcmp(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.ec.q,
					ta->pkey.key.ec.q,
					ta->pkey.key.ec.qlen) != 0)
			{
				continue;
			}
			break;

		default:
			continue;
		}

		/*
		 * Direct trust match!
		 */
		_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->err = BR_ERR_X509_OK;
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_CO();
	}

				}
				break;
			case 24: {
				/* check-trust-anchor-CA */

	size_t u;

	for (u = 0; u < _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->trust_anchors_num; u ++) {
		const br_x509_trust_anchor *ta;
		unsigned char hashed_DN[64];

		ta = &_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->trust_anchors[u];
		if (!(ta->flags & BR_X509_TA_CA)) {
			continue;
		}
		_opt_libs_BearSSL_src_x509_x509_minimal_c_hash_dn(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX, ta->dn.data, ta->dn.len, hashed_DN);
		if (memcmp(hashed_DN, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->saved_dn_hash, _opt_libs_BearSSL_src_x509_x509_minimal_c_DNHASH_LEN)) {
			continue;
		}
		if (_opt_libs_BearSSL_src_x509_x509_minimal_c_verify_signature(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX, &ta->pkey) == 0) {
			_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->err = BR_ERR_X509_OK;
			_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_CO();
		}
	}

				}
				break;
			case 25: {
				/* co */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_CO(); 
				}
				break;
			case 26: {
				/* compute-dn-hash */

	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash_impl->out(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash.vtable, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->current_dn_hash);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_dn_hash = 0;

				}
				break;
			case 27: {
				/* compute-tbs-hash */

	int id = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	size_t len;
	len = br_multihash_out(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->mhash, id, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->tbs_hash);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(len);

				}
				break;
			case 28: {
				/* copy-ee-ec-pkey */

	size_t qlen = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t curve = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	memcpy(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->ee_pkey_data, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey_data, qlen);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key_type = BR_KEYTYPE_EC;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.ec.curve = curve;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.ec.q = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->ee_pkey_data;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.ec.qlen = qlen;

				}
				break;
			case 29: {
				/* copy-ee-rsa-pkey */

	size_t elen = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	size_t nlen = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	memcpy(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->ee_pkey_data, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey_data, nlen + elen);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key_type = BR_KEYTYPE_RSA;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.n = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->ee_pkey_data;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.nlen = nlen;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.e = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->ee_pkey_data + nlen;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey.key.rsa.elen = elen;

				}
				break;
			case 30: {
				/* copy-name-SAN */

	unsigned tag = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	unsigned ok = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	size_t u, len;

	len = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[0];
	for (u = 0; u < _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->num_name_elts; u ++) {
		br_name_element *ne;

		ne = &_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->name_elts[u];
		if (ne->status == 0 && ne->oid[0] == 0 && ne->oid[1] == tag) {
			if (ok && ne->len > len) {
				memcpy(ne->buf, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad + 1, len);
				ne->buf[len] = 0;
				ne->status = 1;
			} else {
				ne->status = -1;
			}
			break;
		}
	}

				}
				break;
			case 31: {
				/* copy-name-element */

	size_t len;
	int32_t off = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	int ok = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();

	if (off >= 0) {
		br_name_element *ne = &_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->name_elts[off];

		if (ok) {
			len = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[0];
			if (len < ne->len) {
				memcpy(ne->buf, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad + 1, len);
				ne->buf[len] = 0;
				ne->status = 1;
			} else {
				ne->status = -1;
			}
		} else {
			ne->status = -1;
		}
	}

				}
				break;
			case 32: {
				/* data-get8 */

	size_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_datablock[addr]);

				}
				break;
			case 33: {
				/* dn-hash-length */

	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_DNHASH_LEN);

				}
				break;
			case 34: {
				/* do-ecdsa-vrfy */

	size_t qlen = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	int curve = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	br_x509_pkey pk;

	pk.key_type = BR_KEYTYPE_EC;
	pk.key.ec.curve = curve;
	pk.key.ec.q = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey_data;
	pk.key.ec.qlen = qlen;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_verify_signature(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX, &pk));

				}
				break;
			case 35: {
				/* do-rsa-vrfy */

	size_t elen = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	size_t nlen = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	br_x509_pkey pk;

	pk.key_type = BR_KEYTYPE_RSA;
	pk.key.rsa.n = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey_data;
	pk.key.rsa.nlen = nlen;
	pk.key.rsa.e = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pkey_data + nlen;
	pk.key.rsa.elen = elen;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_verify_signature(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX, &pk));

				}
				break;
			case 36: {
				/* drop */
 (void)_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP(); 
				}
				break;
			case 37: {
				/* dup */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PEEK(0)); 
				}
				break;
			case 38: {
				/* eqOID */

	const unsigned char *a2 = &_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_datablock[_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP()];
	const unsigned char *a1 = &_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[0];
	size_t len = a1[0];
	int x;
	if (len == a2[0]) {
		x = -(memcmp(a1 + 1, a2 + 1, len) == 0);
	} else {
		x = 0;
	}
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH((uint32_t)x);

				}
				break;
			case 39: {
				/* eqblob */

	size_t len = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	const unsigned char *a2 = (const unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	const unsigned char *a1 = (const unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(-(memcmp(a1, a2, len) == 0));

				}
				break;
			case 40: {
				/* fail */

	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->err = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POPi();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_CO();

				}
				break;
			case 41: {
				/* get-system-date */

	if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->days == 0 && _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->seconds == 0) {
#if _opt_libs_BearSSL_src_x509_x509_minimal_c_BR_USE_UNIX_TIME
		time_t x = time(NULL);

		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH((uint32_t)(x / 86400) + 719528);
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH((uint32_t)(x % 86400));
#elif BR_USE_WIN32_TIME
		FILETIME ft;
		uint64_t x;

		GetSystemTimeAsFileTime(&ft);
		x = ((uint64_t)ft.dwHighDateTime << 32)
			+ (uint64_t)ft.dwLowDateTime;
		x = (x / 10000000);
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH((uint32_t)(x / 86400) + 584754);
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH((uint32_t)(x % 86400));
#else
		_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->err = BR_ERR_X509_TIME_UNKNOWN;
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_CO();
#endif
	} else {
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->days);
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->seconds);
	}

				}
				break;
			case 42: {
				/* get16 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(*(uint16_t *)((unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + addr));

				}
				break;
			case 43: {
				/* get32 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(*(uint32_t *)((unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + addr));

				}
				break;
			case 44: {
				/* match-server-name */

	size_t n1, n2;

	if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->server_name == NULL) {
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(0);
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RET();
	}
	n1 = strlen(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->server_name);
	n2 = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[0];
	if (n1 == n2 && _opt_libs_BearSSL_src_x509_x509_minimal_c_eqnocase(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[1], _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->server_name, n1)) {
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(-1);
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RET();
	}
	if (n2 >= 2 && _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[1] == '*' && _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[2] == '.') {
		size_t u;

		u = 0;
		while (u < n1 && _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->server_name[u] != '.') {
			u ++;
		}
		u ++;
		n1 -= u;
		if ((n2 - 2) == n1
			&& _opt_libs_BearSSL_src_x509_x509_minimal_c_eqnocase(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[3], _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->server_name + u, n1))
		{
			_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(-1);
			_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RET();
		}
	}
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(0);

				}
				break;
			case 45: {
				/* neg */

	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(-a);

				}
				break;
			case 46: {
				/* offset-name-element */

	unsigned san = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	size_t u;

	for (u = 0; u < _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->num_name_elts; u ++) {
		if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->name_elts[u].status == 0) {
			const unsigned char *oid;
			size_t len, off;

			oid = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->name_elts[u].oid;
			if (san) {
				if (oid[0] != 0 || oid[1] != 0) {
					continue;
				}
				off = 2;
			} else {
				off = 0;
			}
			len = oid[off];
			if (len != 0 && len == _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad[0]
				&& memcmp(oid + off + 1,
					_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->pad + 1, len) == 0)
			{
				_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(u);
				_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_RET();
			}
		}
	}
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(-1);

				}
				break;
			case 47: {
				/* or */

	uint32_t b = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t a = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(a | b);

				}
				break;
			case 48: {
				/* over */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PEEK(1)); 
				}
				break;
			case 49: {
				/* read-blob-inner */

	uint32_t len = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	uint32_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	size_t clen = _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hlen;
	if (clen > len) {
		clen = (size_t)len;
	}
	if (addr != 0) {
		memcpy((unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + addr, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hbuf, clen);
	}
	if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_mhash) {
		br_multihash_update(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->mhash, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hbuf, clen);
	}
	if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_dn_hash) {
		_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash_impl->update(
			&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash.vtable, _opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hbuf, clen);
	}
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hbuf += clen;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hlen -= clen;
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(addr + clen);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(len - clen);

				}
				break;
			case 50: {
				/* read8-low */

	if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hlen == 0) {
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(-1);
	} else {
		unsigned char x = *_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hbuf ++;
		if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_mhash) {
			br_multihash_update(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->mhash, &x, 1);
		}
		if (_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_dn_hash) {
			_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash_impl->update(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash.vtable, &x, 1);
		}
		_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->hlen --;
		_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSH(x);
	}

				}
				break;
			case 51: {
				/* roll */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ROLL(_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP()); 
				}
				break;
			case 52: {
				/* rot */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ROT(); 
				}
				break;
			case 53: {
				/* set16 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	*(uint16_t *)((unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + addr) = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();

				}
				break;
			case 54: {
				/* set32 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	*(uint32_t *)((unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + addr) = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();

				}
				break;
			case 55: {
				/* set8 */

	uint32_t addr = _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();
	*((unsigned char *)_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX + addr) = (unsigned char)_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_POP();

				}
				break;
			case 56: {
				/* start-dn-hash */

	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash_impl->init(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->dn_hash.vtable);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_dn_hash = 1;

				}
				break;
			case 57: {
				/* start-tbs-hash */

	br_multihash_init(&_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->mhash);
	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_mhash = 1;

				}
				break;
			case 58: {
				/* stop-tbs-hash */

	_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->do_mhash = 0;

				}
				break;
			case 59: {
				/* swap */
 _opt_libs_BearSSL_src_x509_x509_minimal_c_T0_SWAP(); 
				}
				break;
			case 60: {
				/* zero-server-name */

	_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_PUSHi(-(_opt_libs_BearSSL_src_x509_x509_minimal_c_CTX->server_name == NULL));

				}
				break;
			}

		} else {
			_opt_libs_BearSSL_src_x509_x509_minimal_c_T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)t0ctx)->dp = dp;
	((_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)t0ctx)->rp = rp;
	((_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_context *)t0ctx)->ip = ip;
}



/*
 * Verify the signature on the certificate with the provided public key.
 * This function checks the public key type with regards to the expected
 * type. Returned value is either 0 on success, or a non-zero error code.
 */
 inline  int
_opt_libs_BearSSL_src_x509_x509_minimal_c_verify_signature(br_x509_minimal_context *ctx, const br_x509_pkey *pk)
{
	int kt;

	kt = ctx->cert_signer_key_type;
	if ((pk->key_type & 0x0F) != kt) {
		return BR_ERR_X509_WRONG_KEY_TYPE;
	}
	switch (kt) {
		unsigned char tmp[64];

	case BR_KEYTYPE_RSA:
		if (ctx->irsa == 0) {
			return BR_ERR_X509_UNSUPPORTED;
		}
		if (!ctx->irsa(ctx->cert_sig, ctx->cert_sig_len,
			&_opt_libs_BearSSL_src_x509_x509_minimal_c_t0_datablock[ctx->cert_sig_hash_oid],
			ctx->cert_sig_hash_len, &pk->key.rsa, tmp))
		{
			return BR_ERR_X509_BAD_SIGNATURE;
		}
		if (memcmp(ctx->tbs_hash, tmp, ctx->cert_sig_hash_len) != 0) {
			return BR_ERR_X509_BAD_SIGNATURE;
		}
		return 0;

	case BR_KEYTYPE_EC:
		if (ctx->iecdsa == 0) {
			return BR_ERR_X509_UNSUPPORTED;
		}
		if (!ctx->iecdsa(ctx->iec, ctx->tbs_hash,
			ctx->cert_sig_hash_len, &pk->key.ec,
			ctx->cert_sig, ctx->cert_sig_len))
		{
			return BR_ERR_X509_BAD_SIGNATURE;
		}
		return 0;

	default:
		return BR_ERR_X509_UNSUPPORTED;
	}
}


#endif
