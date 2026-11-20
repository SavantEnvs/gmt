/* MGD77 supplement readers (src/mgd77/mgd77.c): header + record parsers for the MGD77 (.mgd77), MGD77T
 * (.m77t) and plain-table (.dat) flavours, chosen by file extension. First byte picks the flavour, the
 * rest is the cruise file; mgd77info reads the header, mgd77list the records. */
#include "gmt_fuzz_common.h"

static const char *ext[] = { "mgd77", "m77t", "dat" };

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char name[32], args[160];
	const unsigned mode = size ? data[0] : 0;	/* an empty input runs mode 0 on an empty file */
	const uint8_t *body = size ? data + 1 : data;
	const size_t blen = size ? size - 1 : 0;
	gf_init(NULL);
	snprintf(name, sizeof name, "fz.%s", ext[mode % 3]);
	gf_write(name, body, blen);
	snprintf(args, sizeof args, "./%s -Mf -Vq > /dev/null", name);
	gf_run("mgd77info", args);
	snprintf(args, sizeof args, "./%s -Flon,lat,depth,faa,mag -Vq > /dev/null", name);
	gf_run("mgd77list", args);
	unlink(name);
	return 0;
}
