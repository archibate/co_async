#ifndef BEARSSL_IMPL_SSL_SSL_LRU_HPP
#define BEARSSL_IMPL_SSL_SSL_LRU_HPP

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
 * Each entry consists in a fixed number of bytes. Entries are concatenated
 * in the store block. "Addresses" are really offsets in the block,
 * expressed over 32 bits (so the cache may have size at most 4 GB, which
 * "ought to be enough for everyone"). The "null address" is 0xFFFFFFFF.
 * Note that since the storage block alignment is in no way guaranted, we
 * perform only accesses that can handle unaligned data.
 *
 * Two concurrent data structures are maintained:
 *
 * -- Entries are organised in a doubly-linked list; saved entries are added
 * at the head, and loaded entries are moved to the head. Eviction uses
 * the list tail (this is the LRU algorithm).
 *
 * -- Entries are indexed with a binary tree: all left descendants of a
 * node have a lower session ID (in lexicographic order), while all
 * right descendants have a higher session ID. The tree is balanced.
 *
 * Entry format:
 *
 *   session ID          32 bytes
 *   master secret       48 bytes
 *   protocol version    2 bytes (big endian)
 *   cipher suite        2 bytes (big endian)
 *   list prev           4 bytes (big endian)
 *   list next           4 bytes (big endian)
 *   tree left child     4 bytes (big endian)
 *   tree right child    4 bytes (big endian)
 *   tree node colour    1 byte (0 = red, 1 = black)
 *
 * We need to keep the tree balanced because an attacker could make
 * handshakes, selecting some specific sessions (by reusing them) to
 * try to make us make an imbalanced tree that makes lookups expensive
 * (a denial-of-service attack that would persist as long as the cache
 * remains, i.e. even after the attacker made all his connections).
 * To do that, we replace the session ID (or the start of the session ID)
 * with a HMAC value computed over the replaced part; the hash function
 * implementation and the key are obtained from the server context upon
 * first save() call.
 */
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN       32
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_MASTER_SECRET_LEN    48

#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_OFF        0
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_MASTER_SECRET_OFF    32
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_VERSION_OFF          80
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_CIPHER_SUITE_OFF     82
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_LIST_PREV_OFF        84
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_LIST_NEXT_OFF        88
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_LEFT_OFF        92
#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_RIGHT_OFF       96

#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_LRU_ENTRY_LEN       100

#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL   ((uint32_t)-1)

#define _opt_libs_BearSSL_src_ssl_ssl_lru_c_GETSET(name, off) \
static inline uint32_t get_ ## name(br_ssl_session_cache_lru *cc, uint32_t x) \
{ \
	return br_dec32be(cc->store + x + (off)); \
} \
static inline void set_ ## name(br_ssl_session_cache_lru *cc, \
	uint32_t x, uint32_t val) \
{ \
	br_enc32be(cc->store + x + (off), val); \
}

_opt_libs_BearSSL_src_ssl_ssl_lru_c_GETSET(prev, _opt_libs_BearSSL_src_ssl_ssl_lru_c_LIST_PREV_OFF)
_opt_libs_BearSSL_src_ssl_ssl_lru_c_GETSET(next, _opt_libs_BearSSL_src_ssl_ssl_lru_c_LIST_NEXT_OFF)
_opt_libs_BearSSL_src_ssl_ssl_lru_c_GETSET(left, _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_LEFT_OFF)
_opt_libs_BearSSL_src_ssl_ssl_lru_c_GETSET(right, _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_RIGHT_OFF)

/*
 * Transform the session ID by replacing the first N bytes with a HMAC
 * value computed over these bytes, using the random key K (the HMAC
 * value is truncated if needed). HMAC will use the same hash function
 * as the DRBG in the SSL server context, so with SHA-256, SHA-384,
 * or SHA-1, depending on what is available.
 *
 * The risk of collision is considered too small to be a concern; and
 * the impact of a collision is low (the handshake won't succeed). This
 * risk is much lower than any transmission error, which would lead to
 * the same consequences.
 */
 inline  void
_opt_libs_BearSSL_src_ssl_ssl_lru_c_mask_id(br_ssl_session_cache_lru *cc,
	const unsigned char *src, unsigned char *dst)
{
	br_hmac_key_context hkc;
	br_hmac_context hc;

	memcpy(dst, src, _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN);
	br_hmac_key_init(&hkc, cc->hash, cc->index_key, sizeof cc->index_key);
	br_hmac_init(&hc, &hkc, _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN);
	br_hmac_update(&hc, src, _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN);
	br_hmac_out(&hc, dst);
}

/*
 * Find a node by ID. Returned value is the node address, or _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL if
 * the node is not found.
 *
 * If addr_link is not NULL, then '*addr_link' is set to the address of the
 * last followed link. If the found node is the root, then '*addr_link' is
 * set to _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL.
 */
 inline  uint32_t
_opt_libs_BearSSL_src_ssl_ssl_lru_c_find_node(br_ssl_session_cache_lru *cc, const unsigned char *id,
	uint32_t *addr_link)
{
	uint32_t x, y;

	x = cc->root;
	y = _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
	while (x != _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		int r;

		r = memcmp(id, cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_OFF, _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN);
		if (r < 0) {
			y = x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_LEFT_OFF;
			x = get_left(cc, x);
		} else if (r == 0) {
			if (addr_link != NULL) {
				*addr_link = y;
			}
			return x;
		} else {
			y = x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_RIGHT_OFF;
			x = get_right(cc, x);
		}
	}
	if (addr_link != NULL) {
		*addr_link = y;
	}
	return _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
}

/*
 * For node x, find its replacement upon removal.
 *
 *  -- If node x has no child, then this returns _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL.
 *  -- Otherwise, if node x has a left child, then the replacement is the
 *     rightmost left-descendent.
 *  -- Otherwise, the replacement is the leftmost right-descendent.
 *
 * If a node is returned, then '*al' is set to the address of the field
 * that points to that node.
 */
 inline  uint32_t
_opt_libs_BearSSL_src_ssl_ssl_lru_c_find_replacement_node(br_ssl_session_cache_lru *cc, uint32_t x, uint32_t *al)
{
	uint32_t y1, y2;

	y1 = get_left(cc, x);
	if (y1 != _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		y2 = x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_LEFT_OFF;
		for (;;) {
			uint32_t z;

			z = get_right(cc, y1);
			if (z == _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
				*al = y2;
				return y1;
			}
			y2 = y1 + _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_RIGHT_OFF;
			y1 = z;
		}
	}
	y1 = get_right(cc, x);
	if (y1 != _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		y2 = x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_RIGHT_OFF;
		for (;;) {
			uint32_t z;

			z = get_left(cc, y1);
			if (z == _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
				*al = y2;
				return y1;
			}
			y2 = y1 + _opt_libs_BearSSL_src_ssl_ssl_lru_c_TREE_LEFT_OFF;
			y1 = z;
		}
	}
	*al = _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
	return _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
}

 inline void
_opt_libs_BearSSL_src_ssl_ssl_lru_c_set_link(br_ssl_session_cache_lru *cc, uint32_t alx, uint32_t x)
{
	if (alx == _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		cc->root = x;
	} else {
		br_enc32be(cc->store + alx, x);
	}
}

 inline  void
_opt_libs_BearSSL_src_ssl_ssl_lru_c_remove_node(br_ssl_session_cache_lru *cc, uint32_t x)
{
	uint32_t alx, y, aly;

	/*
	 * Find node back and its ancestor link.
	 */
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_find_node(cc, cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_OFF, &alx);

	/*
	 * Find replacement node.
	 */
	y = _opt_libs_BearSSL_src_ssl_ssl_lru_c_find_replacement_node(cc, x, &aly);

	/*
	 * Unlink replacement node.
	 */
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_set_link(cc, aly, _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL);

	/*
	 * Link the replacement node in its new place.
	 */
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_set_link(cc, alx, y);
}

 inline  void
_opt_libs_BearSSL_src_ssl_ssl_lru_c_lru_save(const br_ssl_session_cache_class **ctx,
	br_ssl_server_context *server_ctx,
	const br_ssl_session_parameters *params)
{
	br_ssl_session_cache_lru *cc;
	unsigned char id[_opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN];
	uint32_t x, alx;

	cc = (br_ssl_session_cache_lru *)ctx;

	/*
	 * If the buffer is too small, we don't record anything. This
	 * test avoids problems in subsequent code.
	 */
	if (cc->store_len < _opt_libs_BearSSL_src_ssl_ssl_lru_c_LRU_ENTRY_LEN) {
		return;
	}

	/*
	 * Upon the first save in a session cache instance, we obtain
	 * a random key for our indexing.
	 */
	if (!cc->init_done) {
		br_hmac_drbg_generate(&server_ctx->eng.rng,
			cc->index_key, sizeof cc->index_key);
		cc->hash = br_hmac_drbg_get_hash(&server_ctx->eng.rng);
		cc->init_done = 1;
	}
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_mask_id(cc, params->session_id, id);

	/*
	 * Look for the node in the tree. If the same ID is already used,
	 * then reject it. This is a collision event, which should be
	 * exceedingly rare.
	 * Note: we do NOT record the emplacement here, because the
	 * removal of an entry may change the tree topology.
	 */
	if (_opt_libs_BearSSL_src_ssl_ssl_lru_c_find_node(cc, id, NULL) != _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		return;
	}

	/*
	 * Find some room for the new parameters. If the cache is not
	 * full yet, add it to the end of the area and bump the pointer up.
	 * Otherwise, evict the list tail entry. Note that we already
	 * filtered out the case of a ridiculously small buffer that
	 * cannot hold any entry at all; thus, if there is no room for an
	 * extra entry, then the cache cannot be empty.
	 */
	if (cc->store_ptr > (cc->store_len - _opt_libs_BearSSL_src_ssl_ssl_lru_c_LRU_ENTRY_LEN)) {
		/*
		 * Evict tail. If the buffer has room for a single entry,
		 * then this may also be the head.
		 */
		x = cc->tail;
		cc->tail = get_prev(cc, x);
		if (cc->tail == _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
			cc->head = _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
		} else {
			set_next(cc, cc->tail, _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL);
		}

		/*
		 * Remove the node from the tree.
		 */
		_opt_libs_BearSSL_src_ssl_ssl_lru_c_remove_node(cc, x);
	} else {
		/*
		 * Allocate room for new node.
		 */
		x = cc->store_ptr;
		cc->store_ptr += _opt_libs_BearSSL_src_ssl_ssl_lru_c_LRU_ENTRY_LEN;
	}

	/*
	 * Find the emplacement for the new node, and link it.
	 */
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_find_node(cc, id, &alx);
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_set_link(cc, alx, x);
	set_left(cc, x, _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL);
	set_right(cc, x, _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL);

	/*
	 * New entry becomes new list head. It may also become the list
	 * tail if the cache was empty at that point.
	 */
	if (cc->head == _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		cc->tail = x;
	} else {
		set_prev(cc, cc->head, x);
	}
	set_prev(cc, x, _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL);
	set_next(cc, x, cc->head);
	cc->head = x;

	/*
	 * Fill data in the entry.
	 */
	memcpy(cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_OFF, id, _opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN);
	memcpy(cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_MASTER_SECRET_OFF,
		params->master_secret, _opt_libs_BearSSL_src_ssl_ssl_lru_c_MASTER_SECRET_LEN);
	br_enc16be(cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_VERSION_OFF, params->version);
	br_enc16be(cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_CIPHER_SUITE_OFF, params->cipher_suite);
}

 inline  int
_opt_libs_BearSSL_src_ssl_ssl_lru_c_lru_load(const br_ssl_session_cache_class **ctx,
	br_ssl_server_context *server_ctx,
	br_ssl_session_parameters *params)
{
	br_ssl_session_cache_lru *cc;
	unsigned char id[_opt_libs_BearSSL_src_ssl_ssl_lru_c_SESSION_ID_LEN];
	uint32_t x;

	(void)server_ctx;
	cc = (br_ssl_session_cache_lru *)ctx;
	if (!cc->init_done) {
		return 0;
	}
	_opt_libs_BearSSL_src_ssl_ssl_lru_c_mask_id(cc, params->session_id, id);
	x = _opt_libs_BearSSL_src_ssl_ssl_lru_c_find_node(cc, id, NULL);
	if (x != _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
		params->version = br_dec16be(
			cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_VERSION_OFF);
		params->cipher_suite = br_dec16be(
			cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_CIPHER_SUITE_OFF);
		memcpy(params->master_secret,
			cc->store + x + _opt_libs_BearSSL_src_ssl_ssl_lru_c_MASTER_SECRET_OFF,
			_opt_libs_BearSSL_src_ssl_ssl_lru_c_MASTER_SECRET_LEN);
		if (x != cc->head) {
			/*
			 * Found node is not at list head, so move
			 * it to the head.
			 */
			uint32_t p, n;

			p = get_prev(cc, x);
			n = get_next(cc, x);
			set_next(cc, p, n);
			if (n == _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL) {
				cc->tail = p;
			} else {
				set_prev(cc, n, p);
			}
			set_prev(cc, cc->head, x);
			set_next(cc, x, cc->head);
			set_prev(cc, x, _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL);
			cc->head = x;
		}
		return 1;
	}
	return 0;
}

 inline static const br_ssl_session_cache_class _opt_libs_BearSSL_src_ssl_ssl_lru_c_lru_class = {
	sizeof(br_ssl_session_cache_lru),
	&_opt_libs_BearSSL_src_ssl_ssl_lru_c_lru_save,
	&_opt_libs_BearSSL_src_ssl_ssl_lru_c_lru_load
};

/* see inner.h */
 inline void
br_ssl_session_cache_lru_init(br_ssl_session_cache_lru *cc,
	unsigned char *store, size_t store_len)
{
	cc->vtable = &_opt_libs_BearSSL_src_ssl_ssl_lru_c_lru_class;
	cc->store = store;
	cc->store_len = store_len;
	cc->store_ptr = 0;
	cc->init_done = 0;
	cc->head = _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
	cc->tail = _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
	cc->root = _opt_libs_BearSSL_src_ssl_ssl_lru_c_ADDR_NULL;
}
#endif
