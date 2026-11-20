/* ISF seismic bulletin reader (src/seis/read_isf.c) via gmtisf. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.isf", data, size);
	snprintf(args, sizeof args, "%s -R-180/180/-90/90 -Vq > /dev/null", "fz.isf");
	gf_run("gmtisf", args);
	unlink("fz.isf");
	return 0;
}
