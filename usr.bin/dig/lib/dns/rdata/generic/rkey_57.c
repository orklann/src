/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef RDATA_GENERIC_RKEY_57_C
#define RDATA_GENERIC_RKEY_57_C

#define RRTYPE_RKEY_ATTRIBUTES 0

static inline isc_result_t
totext_rkey(ARGS_TOTEXT) {

	REQUIRE(rdata != NULL);
	REQUIRE(rdata->type == dns_rdatatype_rkey);

	return (generic_totext_key(rdata, tctx, target));
}

static inline isc_result_t
fromwire_rkey(ARGS_FROMWIRE) {

	REQUIRE(type == dns_rdatatype_rkey);

	return (generic_fromwire_key(rdclass, type, source, dctx,
				     options, target));
}

static inline isc_result_t
towire_rkey(ARGS_TOWIRE) {
	isc_region_t sr;

	REQUIRE(rdata != NULL);
	REQUIRE(rdata->type == dns_rdatatype_rkey);
	REQUIRE(rdata->length != 0);

	UNUSED(cctx);

	dns_rdata_toregion(rdata, &sr);
	return (mem_tobuffer(target, sr.base, sr.length));
}


static inline isc_result_t
fromstruct_rkey(ARGS_FROMSTRUCT) {

	REQUIRE(type == dns_rdatatype_rkey);

	return (generic_fromstruct_key(rdclass, type, source, target));
}

static inline isc_result_t
tostruct_rkey(ARGS_TOSTRUCT) {
	dns_rdata_rkey_t *rkey = target;

	REQUIRE(rkey != NULL);
	REQUIRE(rdata != NULL);
	REQUIRE(rdata->type == dns_rdatatype_rkey);

	rkey->common.rdclass = rdata->rdclass;
	rkey->common.rdtype = rdata->type;
	ISC_LINK_INIT(&rkey->common, link);

	return (generic_tostruct_key(rdata, target));
}

static inline void
freestruct_rkey(ARGS_FREESTRUCT) {
	dns_rdata_rkey_t *rkey = (dns_rdata_rkey_t *) source;

	REQUIRE(rkey != NULL);
	REQUIRE(rkey->common.rdtype == dns_rdatatype_rkey);

	generic_freestruct_key(source);
}

static inline isc_boolean_t
checkowner_rkey(ARGS_CHECKOWNER) {

	REQUIRE(type == dns_rdatatype_rkey);

	UNUSED(name);
	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(wildcard);

	return (ISC_TRUE);
}


#endif	/* RDATA_GENERIC_RKEY_57_C */
