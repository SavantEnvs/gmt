/* KML reader (kml2gmt.c): kml2gmt <file>, in-process form of the legacy CLI target. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.kml", data, size);
	snprintf(args, sizeof args, "%s -Vq > /dev/null", "fz.kml");
	gf_run("kml2gmt", args);
	unlink("fz.kml");
	return 0;
}
