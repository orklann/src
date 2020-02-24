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

/* $Id: px_26.c,v 1.4 2020/02/24 12:06:14 florian Exp $ */

/* Reviewed: Mon Mar 20 10:44:27 PST 2000 */

/* RFC2163 */

#ifndef RDATA_IN_1_PX_26_C
#define RDATA_IN_1_PX_26_C

#define RRTYPE_PX_ATTRIBUTES (0)

static inline isc_result_t
totext_in_px(ARGS_TOTEXT) {
	isc_region_t region;
	dns_name_t name;
	dns_name_t prefix;
	isc_boolean_t sub;
	char buf[sizeof("64000")];
	unsigned short num;

	REQUIRE(rdata->type == dns_rdatatype_px);
	REQUIRE(rdata->rdclass == dns_rdataclass_in);
	REQUIRE(rdata->length != 0);

	dns_name_init(&name, NULL);
	dns_name_init(&prefix, NULL);

	/*
	 * Preference.
	 */
	dns_rdata_toregion(rdata, &region);
	num = uint16_fromregion(&region);
	isc_region_consume(&region, 2);
	snprintf(buf, sizeof(buf), "%u", num);
	RETERR(str_totext(buf, target));
	RETERR(str_totext(" ", target));

	/*
	 * MAP822.
	 */
	dns_name_fromregion(&name, &region);
	sub = name_prefix(&name, tctx->origin, &prefix);
	isc_region_consume(&region, name_length(&name));
	RETERR(dns_name_totext(&prefix, sub, target));
	RETERR(str_totext(" ", target));

	/*
	 * MAPX400.
	 */
	dns_name_fromregion(&name, &region);
	sub = name_prefix(&name, tctx->origin, &prefix);
	return(dns_name_totext(&prefix, sub, target));
}

static inline isc_result_t
fromwire_in_px(ARGS_FROMWIRE) {
	dns_name_t name;
	isc_region_t sregion;

	REQUIRE(type == dns_rdatatype_px);
	REQUIRE(rdclass == dns_rdataclass_in);

	UNUSED(type);
	UNUSED(rdclass);

	dns_decompress_setmethods(dctx, DNS_COMPRESS_NONE);

	dns_name_init(&name, NULL);

	/*
	 * Preference.
	 */
	isc_buffer_activeregion(source, &sregion);
	if (sregion.length < 2)
		return (ISC_R_UNEXPECTEDEND);
	RETERR(mem_tobuffer(target, sregion.base, 2));
	isc_buffer_forward(source, 2);

	/*
	 * MAP822.
	 */
	RETERR(dns_name_fromwire(&name, source, dctx, options, target));

	/*
	 * MAPX400.
	 */
	return (dns_name_fromwire(&name, source, dctx, options, target));
}

static inline isc_result_t
towire_in_px(ARGS_TOWIRE) {
	dns_name_t name;
	dns_offsets_t offsets;
	isc_region_t region;

	REQUIRE(rdata->type == dns_rdatatype_px);
	REQUIRE(rdata->rdclass == dns_rdataclass_in);
	REQUIRE(rdata->length != 0);

	dns_compress_setmethods(cctx, DNS_COMPRESS_NONE);
	/*
	 * Preference.
	 */
	dns_rdata_toregion(rdata, &region);
	RETERR(mem_tobuffer(target, region.base, 2));
	isc_region_consume(&region, 2);

	/*
	 * MAP822.
	 */
	dns_name_init(&name, offsets);
	dns_name_fromregion(&name, &region);
	RETERR(dns_name_towire(&name, cctx, target));
	isc_region_consume(&region, name_length(&name));

	/*
	 * MAPX400.
	 */
	dns_name_init(&name, offsets);
	dns_name_fromregion(&name, &region);
	return (dns_name_towire(&name, cctx, target));
}


static inline isc_result_t
fromstruct_in_px(ARGS_FROMSTRUCT) {
	dns_rdata_in_px_t *px = source;
	isc_region_t region;

	REQUIRE(type == dns_rdatatype_px);
	REQUIRE(rdclass == dns_rdataclass_in);
	REQUIRE(source != NULL);
	REQUIRE(px->common.rdtype == type);
	REQUIRE(px->common.rdclass == rdclass);

	UNUSED(type);
	UNUSED(rdclass);

	RETERR(uint16_tobuffer(px->preference, target));
	dns_name_toregion(&px->map822, &region);
	RETERR(isc_buffer_copyregion(target, &region));
	dns_name_toregion(&px->mapx400, &region);
	return (isc_buffer_copyregion(target, &region));
}

static inline isc_result_t
tostruct_in_px(ARGS_TOSTRUCT) {
	dns_rdata_in_px_t *px = target;
	dns_name_t name;
	isc_region_t region;
	isc_result_t result;

	REQUIRE(rdata->type == dns_rdatatype_px);
	REQUIRE(rdata->rdclass == dns_rdataclass_in);
	REQUIRE(target != NULL);
	REQUIRE(rdata->length != 0);

	px->common.rdclass = rdata->rdclass;
	px->common.rdtype = rdata->type;
	ISC_LINK_INIT(&px->common, link);

	dns_name_init(&name, NULL);
	dns_rdata_toregion(rdata, &region);

	px->preference = uint16_fromregion(&region);
	isc_region_consume(&region, 2);

	dns_name_fromregion(&name, &region);

	dns_name_init(&px->map822, NULL);
	RETERR(name_duporclone(&name, &px->map822));
	isc_region_consume(&region, name_length(&px->map822));

	dns_name_init(&px->mapx400, NULL);
	result = name_duporclone(&name, &px->mapx400);
	if (result != ISC_R_SUCCESS)
		goto cleanup;

	return (result);

 cleanup:
	dns_name_free(&px->map822);
	return (ISC_R_NOMEMORY);
}

static inline void
freestruct_in_px(ARGS_FREESTRUCT) {
	dns_rdata_in_px_t *px = source;

	REQUIRE(source != NULL);
	REQUIRE(px->common.rdclass == dns_rdataclass_in);
	REQUIRE(px->common.rdtype == dns_rdatatype_px);

	dns_name_free(&px->map822);
	dns_name_free(&px->mapx400);
}

static inline isc_boolean_t
checkowner_in_px(ARGS_CHECKOWNER) {

	REQUIRE(type == dns_rdatatype_px);
	REQUIRE(rdclass == dns_rdataclass_in);

	UNUSED(name);
	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(wildcard);

	return (ISC_TRUE);
}


#endif	/* RDATA_IN_1_PX_26_C */
