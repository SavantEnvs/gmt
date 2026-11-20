/* gmtmath RPN stack machine (gmtmath.c operator table, gmt_stat.c, gmt_common_math.c). The input is
 * decoded into an allow-listed token sequence (rpn_tokens.h); the result table goes to /dev/null. */
#include "gmt_fuzz_common.h"
#include "rpn_tokens.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char args[4096] = "-T0/4/1 -Vq";
	gf_init(NULL);
	rpn_decode(data, size, NULL, 0, args, sizeof args - 32, 200);
	strcat(args, " = > /dev/null");
	gf_run("gmtmath", args);
	return 0;
}
