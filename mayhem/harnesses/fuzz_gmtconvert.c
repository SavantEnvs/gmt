/* ASCII data-table reader (gmt_io.c gmtlib_read_table / gmtio_ascii_input / gmt_scanf*, segment and
 * OGR/GMT aspatial headers) via gmtconvert. The first byte picks one of a fixed set of option strings
 * (plain read, OGR aspatial fields mapped to columns, geographic/time column parsing with segment
 * selection); the rest is the table. */
#include "gmt_fuzz_common.h"

static const char *modes[] = {
	"fz.txt -Vq > /dev/null",
	"fz.txt -aZ=ID,name -Vq > /dev/null",
	"fz.txt -fg -f0T -s -Q1 -o1,0,2 -Vq > /dev/null",
};

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	const unsigned mode = size ? data[0] : 0;	/* an empty input runs mode 0 on an empty file */
	const uint8_t *body = size ? data + 1 : data;
	const size_t blen = size ? size - 1 : 0;
	gf_init(NULL);
	gf_write("fz.txt", body, blen);
	gf_run("gmtconvert", modes[mode % 3]);
	unlink("fz.txt");
	return 0;
}
