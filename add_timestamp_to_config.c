#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// OS specific libraries
#ifdef __WINDOWS__
#elif  __gnu_linux__
#include <unistd.h>
#endif

#define MAX_PATH 200
#define EPOCH_TIME_CHARS 15

int getCurrentTime() {
  return (int)time(NULL);
}

int main() {
  char path[MAX_PATH];

  // Get the path of the config executable
#ifdef __WINDOWS__
  GetModuleFileName(NULL, path, MAX_PATH);
#elif  __gnu_linux__
  readlink("/proc/self/exe", path, MAX_PATH);
#endif

  // Remove "/config" from the end of the path
  path[strlen(path)-7]=0;
  // Append the rest of the path to the config file
  strcat(path, "/hardware/CowTags/global_cfg.h");

  // Open config file for writing
  FILE *fp = fopen(path, "r+");

  // Start editing at the 242th character
  fseek (fp, 242, SEEK_SET);

  // Convert the number received from getCurrentTime() to a string with snprintf
  // That string gets saved into "current_time_str"
  char current_time_str[EPOCH_TIME_CHARS];
  snprintf(current_time_str, EPOCH_TIME_CHARS, "%d", getCurrentTime());

  // Write the number to the file
  /* fprintf(fp, current_time_str); */
  fputs(current_time_str, fp);

  fclose(fp);
  return 0;
}
