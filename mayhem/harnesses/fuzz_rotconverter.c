/* Plate-rotation parsing and algebra (src/spotter/spotter.c spotter_init and the pole parsers,
 * rotconverter.c). The input's prefix picks the mode:
 *   "@F": the rest is a rotation file (stage or total reconstruction poles, optional covariances), run
 *         as `rotconverter fz.rot`;
 *   anything else (including the empty input): the input after its first byte is split into
 *         whitespace-separated words, each kept only if it is "+"/"-" or a rotation given on the command
 *         line (characters limited to digits, letters and "/.:-+"), so rotconverter parses and
 *         adds/subtracts the rotations without any file being named.
 * Why a two-byte selector and not one bit: spotter_init dereferences a NULL e[0] whenever a rotation
 * file holds no rotations (spotter.c:616, the "Extend oldest stage pole" block), so almost every file
 * libFuzzer derives from a short input crashes there. With a one-bit selector the second or third
 * mutation of the empty input already hit it, and Mayhem's start-up check ("libFuzzer target failed to
 * fuzz for 5 iterations") failed the run. Every input still runs; the file mode is reached from the
 * "@F" seeds and libFuzzer's compare tracing, and that crash stays a reported defect. */
#include "gmt_fuzz_common.h"
#include <ctype.h>

static int ok_char(int c) { return isalnum(c) || (c && strchr("/.:-+", c)); }

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[2048] = "-Vq";
	size_t len = strlen(args), i = 1;
	gf_init(NULL);
	if (size >= 2 && data[0] == '@' && data[1] == 'F') {
		gf_write("fz.rot", data + 2, size - 2);
		gf_run("rotconverter", "fz.rot -Vq > /dev/null");
		unlink("fz.rot");
		return 0;
	}
	while (i < size) {
		char word[256];
		size_t w = 0;
		int good = 1;
		while (i < size && isspace(data[i])) i++;
		while (i < size && !isspace(data[i])) {
			if (!ok_char(data[i])) good = 0;
			if (w < sizeof word - 1) word[w++] = (char)data[i];
			i++;
		}
		word[w] = '\0';
		if (!w || !good || access(word, F_OK) == 0) continue;	/* never name a file */
		if (len + w + 2 >= sizeof args - 16) break;
		args[len++] = ' ';
		memcpy(args + len, word, w + 1);
		len += w;
	}
	strcat(args, " > /dev/null");
	gf_run("rotconverter", args);
	return 0;
}
