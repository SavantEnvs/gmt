/* GMT's own binary/ASCII grid formats (gmt_customio.c native bm/bb/bs/bi/bf/bd, Sun raster rb, GEODAS rf,
 * Surfer sf/sd; gmt_agc_io.c af; gmt_esri_io.c ei/ef). These formats cannot be autodetected, so the
 * caller names them with "<file>=<id>" (GMT's documented grid-name syntax): the first input byte picks
 * the id, the rest is the grid file. */
#include "gmt_fuzz_common.h"

static const char *ids[] = { "bm", "bb", "bs", "bi", "bf", "bd", "rb", "rf", "sf", "sd", "af", "ei", "ef" };

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[128];
	const char *id;
	const unsigned mode = size ? data[0] : 0;	/* an empty input runs mode 0 on an empty file */
	const uint8_t *body = size ? data + 1 : data;
	const size_t blen = size ? size - 1 : 0;
	gf_init(NULL);
	id = ids[mode % (sizeof ids / sizeof ids[0])];
	gf_write("fz.grd", body, blen);
	snprintf(args, sizeof args, "fz.grd=%s -C -Vq > /dev/null", id);
	gf_run("grdinfo", args);
	snprintf(args, sizeof args, "fz.grd=%s -Vq > /dev/null", id);
	gf_run("grd2xyz", args);
	unlink("fz.grd");
	return 0;
}
