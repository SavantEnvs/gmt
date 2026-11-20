/* SEG-Y reel/trace header and data reader (src/segy/segy_io.c) via pssegy. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.segy", data, size);
	snprintf(args, sizeof args, "%s -R0/100/0/5 -JX5c/-5c -D0.05 -F -Vq > /dev/null", "fz.segy");
	gf_run("pssegy", args);
	unlink("fz.segy");
	return 0;
}
