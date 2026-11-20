/* Leak detection is off for every target (SPEC §6.2 item 15); ASan + UBSan stay fully active. */
int __lsan_is_turned_off(void) { return 1; }
