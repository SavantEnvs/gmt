/* Shared scaffolding for the GMT module harnesses.
 *
 * Every harness drives GMT through its public API exactly as a caller would:
 *   GMT_Create_Session -> GMT_Call_Module(API, "<module>", GMT_MODULE_CMD, "<args>") -> GMT_Destroy_Session
 * with the fuzz input written to a file in a private scratch directory (the cwd of the process).
 *
 * State kept across inputs, on purpose: one extra reference to the supplements plugin (see gf_init); the private scratch dir (created once under $TMPDIR, made the
 * cwd, and used as HOME/GMT_USERDIR/GMT_TMPDIR so GMT never touches the real home) and fixed helper
 * files written there once (read-only inputs such as a CPT). Each input gets a fresh GMT session, and
 * every file a module may leave behind in the cwd (gmt.conf, gmt.history, outputs) is removed after it,
 * so an input's result depends only on that input.
 */
#ifndef GMT_FUZZ_COMMON_H
#define GMT_FUZZ_COMMON_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <gmt.h>

static int gf_ready = 0;

static void gf_write(const char *name, const void *data, size_t size) {
	FILE *fp = fopen(name, "wb");
	if (!fp) abort();
	if (size && fwrite(data, 1, size, fp) != size) abort();
	fclose(fp);
}

static void gf_init(void (*setup)(void)) {
	static char dir[4096];
	const char *t;
	if (gf_ready) return;
	t = getenv("TMPDIR");
	if (!t || !*t) t = "/tmp";
	snprintf(dir, sizeof dir, "%s/gmtfuzz.XXXXXX", t);
	if (!mkdtemp(dir) || chdir(dir) != 0) abort();
	setenv("HOME", dir, 1);
	setenv("GMT_USERDIR", dir, 1);
	setenv("GMT_TMPDIR", dir, 1);
	setenv("GMT_CACHEDIR", dir, 1);
	setenv("GMT_DATADIR", dir, 1);
	unsetenv("GMT_SESSION_NAME");
#ifdef GMT_FUZZ_PLUGIN
	/* GMT dlopen()s the supplements plugin in every session and dlclose()s it in GMT_Destroy_Session.
	 * The plugin is SanCov-instrumented, and libFuzzer keeps pointers to its counters, so unmapping it
	 * crashes libFuzzer on the next input. Holding one extra reference (RTLD_NODELETE) keeps it mapped;
	 * GMT still loads and initialises it per session exactly as before. */
	(void)dlopen(GMT_FUZZ_PLUGIN, RTLD_NOW | RTLD_NODELETE);
#endif
	if (setup) setup();
	gf_ready = 1;
}

/* Files a module may write into the cwd; removed after every input. */
static void gf_cleanup(void) {
	static const char *junk[] = { "gmt.conf", "gmt.history", "gmt.cpt", "fz_out.nc", "fz_out.grd", NULL };
	for (int i = 0; junk[i]; i++) unlink(junk[i]);
}

static void gf_run(const char *module, const char *args) {
	void *API = GMT_Create_Session("fuzz", 2U, GMT_SESSION_NOEXIT | GMT_SESSION_NOHISTORY, NULL);
	if (API == NULL) abort();	/* session creation does not depend on the input */
	(void)GMT_Call_Module(API, module, GMT_MODULE_CMD, (void *)args);
	GMT_Destroy_Session(API);
	gf_cleanup();
}

#endif
