/* grdmath grid RPN machine (grdmath.c: its own, larger operator table plus grid operators), on a tiny
 * 4x4 region so each input stays fast. Tokens come from an allow-list (rpn_tokens.h + grid operands);
 * the result is written to fz_out.nc in the private scratch dir and removed after the input. */
#include "gmt_fuzz_common.h"
#include "rpn_tokens.h"

static const char *grid_tokens[] = { "X", "Y", "XNORM", "YNORM", "XCOL", "YROW", "NODE",
	"CDIST", "AZ", "CAZ", "EXTREMA", "CURV", "D2DX2", "D2DY2", "DDX", "DDY" };

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[4096] = "-R0/3/0/3 -I1 -Vq";
	gf_init(NULL);
	rpn_decode(data, size, grid_tokens, sizeof grid_tokens / sizeof grid_tokens[0], args, sizeof args - 32, 200);
	strcat(args, " = fz_out.nc");
	gf_run("grdmath", args);
	return 0;
}
