/* Binary GSHHG polygon reader (src/gshhg/gshhg.c) via gshhg. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.b", data, size);
	snprintf(args, sizeof args, "%s -Vq > /dev/null", "fz.b");
	gf_run("gshhg", args);
	unlink("fz.b");
	return 0;
}
