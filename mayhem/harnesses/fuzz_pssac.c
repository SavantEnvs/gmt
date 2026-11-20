/* SAC seismogram reader (src/seis/sacio.c) via pssac. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.sac", data, size);
	snprintf(args, sizeof args, "%s -JX10c/5c -R0/100/-1/1 -Vq > /dev/null", "fz.sac");
	gf_run("pssac", args);
	unlink("fz.sac");
	return 0;
}
