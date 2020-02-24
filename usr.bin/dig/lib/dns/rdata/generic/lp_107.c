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

#ifndef RDATA_GENERIC_LP_107_C
#define RDATA_GENERIC_LP_107_C

#include <string.h>

#include <isc/net.h>

#define RRTYPE_LP_ATTRIBUTES (0)

static inline isc_result_t
totext_lp(ARGS_TOTEXT) {
	isc_region_t region;
	dns_name_t name;
	dns_name_t prefix;
	isc_boolean_t sub;
	char buf[sizeof("64000")];
	unsigned short num;

	REQUIRE(rdata->type == dns_rdatatype_lp);
	REQUIRE(rdata->length != 0);

	dns_name_init(&name, NULL);
	dns_name_init(&prefix, NULL);

	dns_rdata_toregion(rdata, &region);
	num = uint16_fromregion(&region);
	isc_region_consume(&region, 2);
	snprintf(buf, sizeof(buf), "%u", num);
	RETERR(str_totext(buf, target));

	RETERR(str_totext(" ", target));

	dns_name_fromregion(&name, &region);
	sub = name_prefix(&name, tctx->origin, &prefix);
	return (dns_name_totext(&prefix, sub, target));
}

static inline isc_result_t
fromwire_lp(ARGS_FROMWIRE) {
	dns_name_t name;
	isc_region_t sregion;

	REQUIRE(type == dns_rdatatype_lp);

	UNUSED(type);
	UNUSED(rdclass);

	dns_decompress_setmethods(dctx, DNS_COMPRESS_GLOBAL14);

	dns_name_init(&name, NULL);

	isc_buffer_activeregion(source, &sregion);
	if (sregion.length < 2)
		return (ISC_R_UNEXPECTEDEND);
	RETERR(mem_tobuffer(target, sregion.base, 2));
	isc_buffer_forward(source, 2);
	return (dns_name_fromwire(&name, source, dctx, options, target));
}

static inline isc_result_t
towire_lp(ARGS_TOWIRE) {

	REQUIRE(rdata->type == dns_rdatatype_lp);
	REQUIRE(rdata->length != 0);

	UNUSED(cctx);

	return (mem_tobuffer(target, rdata->data, rdata->length));
}


static inline isc_result_t
fromstruct_lp(ARGS_FROMSTRUCT) {
	dns_rdata_lp_t *lp = source;
	isc_region_t region;

	REQUIRE(type == dns_rdatatype_lp);
	REQUIRE(source != NULL);
	REQUIRE(lp->common.rdtype == type);
	REQUIRE(lp->common.rdclass == rdclass);

	UNUSED(type);
	UNUSED(rdclass);

	RETERR(uint16_tobuffer(lp->pref, target));
	dns_name_toregion(&lp->lp, &region);
	return (isc_buffer_copyregion(target, &region));
}

static inline isc_result_t
tostruct_lp(ARGS_TOSTRUCT) {
	isc_region_t region;
	dns_rdata_lp_t *lp = target;
	dns_name_t name;

	REQUIRE(rdata->type == dns_rdatatype_lp);
	REQUIRE(target != NULL);
	REQUIRE(rdata->length != 0);

	lp->common.rdclass = rdata->rdclass;
	lp->common.rdtype = rdata->type;
	ISC_LINK_INIT(&lp->common, link);

	dns_name_init(&name, NULL);
	dns_rdata_toregion(rdata, &region);
	lp->pref = uint16_fromregion(&region);
	isc_region_consume(&region, 2);
	dns_name_fromregion(&name, &region);
	dns_name_init(&lp->lp, NULL);
	RETERR(name_duporclone(&name, &lp->lp));
	return (ISC_R_SUCCESS);
}

static inline void
freestruct_lp(ARGS_FREESTRUCT) {
	dns_rdata_lp_t *lp = source;

	REQUIRE(source != NULL);
	REQUIRE(lp->common.rdtype == dns_rdatatype_lp);

	dns_name_free(&lp->lp);
}

static inline isc_boolean_t
checkowner_lp(ARGS_CHECKOWNER) {

	REQUIRE(type == dns_rdatatype_lp);

	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(name);
	UNUSED(wildcard);

	return (ISC_TRUE);
}


#endif	/* RDATA_GENERIC_LP_107_C */
