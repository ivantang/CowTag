#include <xdc/runtime/Timestamp.h>
#include <stdint.h>
#include "bootTimestamp.h"
#include "global_cfg.h"

uint32_t TrueTimestamp() {
  uint32_t uptime = Timestamp_get32() - boot_timestamp;
  return TIMESTAMP_AT_BUILDTIME + uptime;
}
