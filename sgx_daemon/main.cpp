#include "daemon.h"

void main() {
	daemon sgxd(5556, 5555);
	sgxd.start_daemon();
}
