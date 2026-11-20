/* Allow-listed token alphabet for the gmtmath/grdmath RPN harnesses: each input byte becomes one
 * operand or operator from a fixed list, so the fuzzer explores the stack machines without ever naming
 * a file, a remote (@) dataset or an output sink. */
#ifndef RPN_TOKENS_H
#define RPN_TOKENS_H
static const char *rpn_operands[] = {
	"0", "1", "2", "-1", "0.5", "3", "10", "-7.25", "1e300", "-1e-300", "NaN", "PI", "E", "EULER",
	"PHI", "F_EPS", "TMIN", "TMAX", "TINC", "N", "T", "TNORM", "TROW", "1000000", "65536", "-0", "4",
	"180", "90", "0.001", "255", "2147483648"
};
static const char *rpn_ops[] = {
	"ABS", "ACOS", "ACOSH", "ADD", "AND", "ASIN", "ASINH", "ATAN", "ATAN2", "ATANH", "BCDF", "BEI", "BER",
	"BITAND", "BITLEFT", "BITNOT", "BITOR", "BITRIGHT", "BITTEST", "BITXOR", "BPDF", "CEIL", "CHICRIT",
	"CHICDF", "CHIPDF", "CORRCOEFF", "COS", "COSD", "COSH", "COT", "COTD", "CSC", "CUMSUM",
	"D2DT2", "D2R", "DDT", "DEG2KM", "DENAN", "DIFF", "DILOG", "DIV", "DUP", "ECDF", "ECRIT", "EPDF", "ERF",
	"ERFC", "ERFINV", "EQ", "EXCH", "EXP", "FACT", "FCDF", "FCRIT", "FLIPUD", "FLOOR", "FMOD", "FPDF", "GE",
	"GT", "HYPOT", "I0", "I1", "IFELSE", "IN", "INRANGE", "INT", "INV", "ISFINITE", "ISNAN", "J0", "J1",
	"JN", "K0", "K1", "KEI", "KER", "KM2DEG", "KN", "KURT", "LE", "LMSSCL", "LMSSCLW",
	"LOG", "LOG10", "LOG1P", "LOG2", "LOWER", "LPDF", "LRAND", "LT", "MAD", "MAX", "MEAN", "MEDIAN", "MIN",
	"MOD", "MODE", "MUL", "NAN", "NEG", "NEQ", "NORM", "NOT", "NRAND", "OR", "PCDF", "PLM", "PLMg",
	"POP", "POW", "PPDF", "PQUANT", "PSI", "PV", "QV", "R2", "R2D", "RAND", "RINT", "RMS",
	"ROLL", "ROTT", "SEC", "SECD", "SIGN", "SIN", "SINC", "SIND", "SINH", "SKEW", "SQR", "SQRT",
	"STD", "STEP", "STEPT", "SUB", "SUM", "TAN", "TAND", "TANH", "TAPER", "TCDF", "TCRIT", "TPDF", "UPPER",
	"VAR", "XOR", "Y0", "Y1", "YN", "ZCDF", "ZCRIT", "ZPDF", 
	"ROOTS", "PFQUANT", "ADD", "SUB", "MUL", "DIV"
};
#define RPN_N_OPERANDS (sizeof rpn_operands / sizeof rpn_operands[0])
#define RPN_N_OPS (sizeof rpn_ops / sizeof rpn_ops[0])

/* Bytes 0-79 pick an operand, 80-239 an operator (160 of them), 240-255 a caller-supplied extra token
 * (grid operands for grdmath; operators again when there are none).
 * Append up to max_tokens tokens decoded from data to buf (NUL-terminated, never overflows). */
static void rpn_decode(const uint8_t *data, size_t size, const char **extra, size_t n_extra,
                       char *buf, size_t cap, size_t max_tokens) {
	size_t len = strlen(buf);
	for (size_t i = 0; i < size && i < max_tokens; i++) {
		const char *tok;
		uint8_t b = data[i];
		if (b < 80) tok = rpn_operands[b % RPN_N_OPERANDS];
		else if (n_extra && b >= 240) tok = extra[(b - 240) % n_extra];
		else tok = rpn_ops[(b - 80) % RPN_N_OPS];
		if (len + strlen(tok) + 2 >= cap) break;
		buf[len++] = ' ';
		strcpy(buf + len, tok);
		len += strlen(tok);
	}
}
#endif
