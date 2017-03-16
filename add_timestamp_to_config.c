#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define EPOCH_TIME_CHARS 15

int getCurrentTime() {
  return (int)time(NULL);
}

int main() {
  // Open config file for writing
  FILE *fp = fopen("./hardware/CowTags/global_cfg.h", "r+");

  // Start editing at the 242th character
  fseek (fp, 242, SEEK_SET);

  // Convert the number received from getCurrentTime() to a string with snprintf
  // That string gets saved into "current_time_str"
  char current_time_str[EPOCH_TIME_CHARS];
  snprintf(current_time_str, EPOCH_TIME_CHARS, "%d", getCurrentTime());

  // Write the number to the file
  fprintf(fp, current_time_str);

  fclose(fp);
  return 0;
}
