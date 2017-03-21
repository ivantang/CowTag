#include <xdc/runtime/Timestamp.h>
#include <stdint.h>
#include "bootTimestamp.h"
#include "global_cfg.h"
#include <stdio.h>
#include <xdc/runtime/Types.h>

// The timestamp returned by Timestamp_get32() / frequency is really just the
// number of seconds that the tag has been up for. The timestamp is not accurate
// to real world time

// TIMESTAMP_AT_BUILDTIME is a value defined in the config file, and is set
// automatically after the project builds to be the current timestamp of the
// computer that is building the project, thus adding the uptime to this will
// give us a semi-accurate timestamp to use for our packets
uint32_t TrueTimestamp() {
  Types_FreqHz frequency;
  Timestamp_getFreq(&frequency);
  uint32_t current_stamp = Timestamp_get32() / (frequency.hi << 8 | frequency.lo);

	// boot_timestamp is defined in bootTimestamp.h, and is the first thing that
	// is set in the main() function
  uint32_t uptime = current - boot_timestamp;
  uint32_t ret = TIMESTAMP_AT_BUILDTIME + uptime;

  printf("Timestamp = %i\n", ret);
  return ret;
}
