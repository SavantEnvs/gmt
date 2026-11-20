/* psxy record parsing and plotting (psxy.c, gmt_io.c, gmt_plot.c, postscriptlight). The first byte picks
 * the mode: per-record symbol codes (-S with no size: gmt_parse_symbol_option per record) with OGR/GMT
 * aspatial fields mapped to columns (-a) and colour lookup (-C); the same symbols without -a; or lines
 * with pen/fill segment headers. The rest is the table. */
#include "gmt_fuzz_common.h"

static const char *modes[] = {
	"fz.txt -R0/10/0/10 -JX10c -S -aZ=ID -Cfixed.cpt -Vq > /dev/null",
	"fz.txt -R0/10/0/10 -JX10c -S -Cfixed.cpt -Vq > /dev/null",
	"fz.txt -R0/10/0/10 -JX10c -W1p -Vq > /dev/null",
};

static void setup(void) {
	static const char cpt[] = "0 red 5 green\n5 green 10 blue\nB black\nF white\nN gray\n";
	gf_write("fixed.cpt", cpt, sizeof cpt - 1);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	const unsigned mode = size ? data[0] : 0;	/* an empty input runs mode 0 on an empty file */
	const uint8_t *body = size ? data + 1 : data;
	const size_t blen = size ? size - 1 : 0;
	gf_init(setup);
	gf_write("fz.txt", body, blen);
	gf_run("psxy", modes[mode % 3]);
	unlink("fz.txt");
	return 0;
}
