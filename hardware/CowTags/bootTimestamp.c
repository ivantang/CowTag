#include <stdint.h>
#include "bootTimestamp.h"
#include "global_cfg.h"
#include <stdio.h>
#include <ti/sysbios/knl/Clock.h>
#include "debug.h"

// The timestamp returned by Timestamp_get32() / frequency is really just the
// number of seconds that the tag has been up for. The timestamp is not accurate
// to real world time

// TIMESTAMP_AT_BUILDTIME is a value defined in the config file, and is set
// automatically after the project builds to be the current timestamp of the
// computer that is building the project, thus adding the uptime to this will
// give us a semi-accurate timestamp to use for our packets
uint32_t TrueTimestamp() {
  uint32_t current_stamp = Clock_getTicks();

	// boot_timestamp is defined in bootTimestamp.h, and is the first thing that
	// is set in the main() function
  uint32_t uptime = (current_stamp - boot_timestamp) / 100000;
  if (verbose_uptime) {
    printf("uptime = %u\n", uptime);
  }

  uint32_t ret = TIMESTAMP_AT_BUILDTIME + uptime;
  if (verbose_timestamp) {
    printf("Timestamp = %u\n", ret);
  }

  return ret;
}
