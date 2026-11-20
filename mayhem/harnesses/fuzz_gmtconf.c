/* gmt.conf defaults parser (gmt_init.c gmt_getdefaults / gmtlib_setparameter, hundreds of keywords:
 * fonts, pens, formats, map settings, ellipsoids) via gmtget -G<file> and a dump of all parameters. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	gf_init(NULL);
	gf_write("fz.conf", data, size);
	gf_run("gmtget", "-Gfz.conf FONT_TITLE MAP_FRAME_PEN FORMAT_FLOAT_OUT PROJ_ELLIPSOID TIME_EPOCH -Vq > /dev/null");
	unlink("fz.conf");
	return 0;
}
