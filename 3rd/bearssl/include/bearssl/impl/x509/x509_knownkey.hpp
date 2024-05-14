#ifndef BEARSSL_IMPL_X509_X509_KNOWNKEY_HPP
#define BEARSSL_IMPL_X509_X509_KNOWNKEY_HPP

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

/* see bearssl_x509.h */
 inline void
br_x509_knownkey_init_rsa(br_x509_knownkey_context *ctx,
	const br_rsa_public_key *pk, unsigned usages)
{
	ctx->vtable = &br_x509_knownkey_vtable;
	ctx->pkey.key_type = BR_KEYTYPE_RSA;
	ctx->pkey.key.rsa = *pk;
	ctx->usages = usages;
}

/* see bearssl_x509.h */
 inline void
br_x509_knownkey_init_ec(br_x509_knownkey_context *ctx,
	const br_ec_public_key *pk, unsigned usages)
{
	ctx->vtable = &br_x509_knownkey_vtable;
	ctx->pkey.key_type = BR_KEYTYPE_EC;
	ctx->pkey.key.ec = *pk;
	ctx->usages = usages;
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_start_chain(const br_x509_class **ctx, const char *server_name)
{
	(void)ctx;
	(void)server_name;
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_start_cert(const br_x509_class **ctx, uint32_t length)
{
	(void)ctx;
	(void)length;
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_append(const br_x509_class **ctx, const unsigned char *buf, size_t len)
{
	(void)ctx;
	(void)buf;
	(void)len;
}

 inline  void
_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_end_cert(const br_x509_class **ctx)
{
	(void)ctx;
}

 inline  unsigned
_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_end_chain(const br_x509_class **ctx)
{
	(void)ctx;
	return 0;
}

 inline  const br_x509_pkey *
_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_get_pkey(const br_x509_class *const *ctx, unsigned *usages)
{
	const br_x509_knownkey_context *xc;

	xc = (const br_x509_knownkey_context *)ctx;
	if (usages != NULL) {
		*usages = xc->usages;
	}
	return &xc->pkey;
}

/* see bearssl_x509.h */
 inline const br_x509_class br_x509_knownkey_vtable = {
	sizeof(br_x509_knownkey_context),
	_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_start_chain,
	_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_start_cert,
	_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_append,
	_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_end_cert,
	_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_end_chain,
	_opt_libs_BearSSL_src_x509_x509_knownkey_c_kk_get_pkey
};
#endif
