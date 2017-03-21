#include <xdc/runtime/Timestamp.h>
#include <stdint.h>
#include "bootTimestamp.h"
#include "global_cfg.h"
#include <stdio.h>
#include <xdc/runtime/Types.h>

uint32_t TrueTimestamp() {
  /* uint32_t uptime = Timestamp_get32() - boot_timestamp; */
	Types_FreqHz frequency;
	Timestamp_getFreq(&frequency);
  uint32_t current = Timestamp_get32() / (frequency.hi << 8 | frequency.lo);
  uint32_t uptime = current - boot_timestamp;
  uint32_t ret = TIMESTAMP_AT_BUILDTIME + uptime;
  /* return TIMESTAMP_AT_BUILDTIME + uptime; */

  printf("current = %i\n", current);
  printf("boot_timestamp = %i\n", boot_timestamp);
  printf("uptime = %i\n", uptime);
  printf("TIMESTAMP_AT_BUILDTIME = %i\n", TIMESTAMP_AT_BUILDTIME);
  printf("ret = %i\n", ret);
  return ret;
}
