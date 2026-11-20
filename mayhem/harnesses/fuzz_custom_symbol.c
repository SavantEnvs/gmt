/* Custom-symbol macro files (gmt_support.c custom .def parser + the macro interpreter in gmt_plot.c):
 * the input is fz.def, plotted at a fixed point with psxy -Skfz/1c. */
#include "gmt_fuzz_common.h"

static void setup(void) {
	static const char pt[] = "1 1\n2 2 \n";
	gf_write("pt.txt", pt, sizeof pt - 1);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	gf_init(setup);
	gf_write("fz.def", data, size);
	gf_run("psxy", "pt.txt -R0/3/0/3 -JX5c -Skfz/1c -W0.5p -Gred -Vq > /dev/null");
	unlink("fz.def");
	return 0;
}
