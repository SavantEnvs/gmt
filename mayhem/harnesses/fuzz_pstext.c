/* pstext record parsing: per-record font/angle/justification (-F+f+a+j), text escape sequences
 * (@~ @+ @% ...) rendered by postscriptlight, and paragraph mode (-M). */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	gf_init(NULL);
	gf_write("fz.txt", data, size);
	gf_run("pstext", "fz.txt -R0/10/0/10 -JX10c -F+f+a+j -Vq > /dev/null");
	gf_run("pstext", "fz.txt -R0/10/0/10 -JX10c -M -Vq > /dev/null");
	unlink("fz.txt");
	return 0;
}
