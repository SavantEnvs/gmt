/* netCDF/COARDS grid reader with format autodetection (gmt_grdio.c, gmt_nc.c gmtnc_grd_info /
 * gmtnc_grd_prep_io, gmt_cdf.c): grdinfo reads the header, grd2xyz reads the data. */
#include "gmt_fuzz_common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	gf_init(NULL);
	gf_write("fz.nc", data, size);
	gf_run("grdinfo", "fz.nc -C -Vq > /dev/null");
	gf_run("grd2xyz", "fz.nc -Vq > /dev/null");
	unlink("fz.nc");
	return 0;
}
