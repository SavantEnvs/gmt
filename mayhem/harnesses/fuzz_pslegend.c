/* Legend specification parser (pslegend.c), which re-invokes psxy/pstext/psscale internally. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.txt", data, size);
	snprintf(args, sizeof args, "%s -Dx0/0+w8c -R0/1/0/1 -JX8c -Vq > /dev/null", "fz.txt");
	gf_run("pslegend", args);
	unlink("fz.txt");
	return 0;
}
