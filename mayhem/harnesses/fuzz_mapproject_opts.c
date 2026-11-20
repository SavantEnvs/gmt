/* Module option parsing (gmt_api.c option/key handling, gmt_init.c -R/-J/common-option parsers,
 * gmt_map.c/gmt_proj.c map setup) via mapproject on a fixed 3-point table. The input is split into
 * whitespace-separated words; each word must be an option ("-<letter>...") whose letter is allow-listed
 * and whose characters are limited to an allow-listed set, so no word can name an output file, a remote
 * (@) dataset, a redirect or a module-level long option. */
#include "gmt_fuzz_common.h"
#include <ctype.h>

static void setup(void) {
	static const char pts[] = "10 10\n20 25\n-5 80\n";
	gf_write("pts.txt", pts, sizeof pts - 1);
}

static int ok_letter(int c) { return c && strchr("RJACDEFGIJLNQSTWZfjnpsx", c) != NULL; }
static int ok_char(int c) { return isalnum(c) || (c && strchr("/+-.,:_^%~", c)); }

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[2048] = "pts.txt -Vq";
	size_t len = strlen(args), i = 0;
	gf_init(setup);
	while (i < size) {
		char word[256];
		size_t w = 0;
		while (i < size && isspace(data[i])) i++;
		while (i < size && !isspace(data[i])) {
			if (ok_char(data[i]) && w < sizeof word - 1) word[w++] = (char)data[i];
			i++;
		}
		word[w] = '\0';
		if (w < 2 || word[0] != '-' || !ok_letter(word[1])) continue;
		if (len + w + 2 >= sizeof args - 16) break;
		args[len++] = ' ';
		memcpy(args + len, word, w + 1);
		len += w;
	}
	strcat(args, " > /dev/null");
	gf_run("mapproject", args);
	return 0;
}
