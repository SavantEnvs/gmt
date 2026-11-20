/* CPT colour-table parser (gmtlib_read_cpt) via makecpt -C<file>. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[512];
	gf_init(NULL);
	gf_write("fz.cpt", data, size);
	snprintf(args, sizeof args, "-C%s -T0/1/0.25 -Vq > /dev/null", "fz.cpt");
	gf_run("makecpt", args);
	unlink("fz.cpt");
	return 0;
}
