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
#define TESTING_DONT_WRITE_TO_FILE 0

int getCurrentTime() {
  return (int)time(NULL);
}

// Returns the byte offset to get to the edit point
int Search_in_File(FILE *fp, char *str) {
  char current_line[512];
  int byte_count=0;
  char c;

  while (fgets(current_line, 512, fp) != NULL) {
    if ((strstr(current_line, str)) != NULL) {
      // String found - Take the current count, and add the length of the search
      // string to the byte_count (to get the byte_count to the end of the line)
      byte_count+=strlen(str)+1;
      break;
    }
    // Count the number of bytes in the line if the line does not match
    for (int i = 0; i < sizeof(current_line); i++) {
      // If the current character is a newline, that is the end of the line, so
      // the current index+1 is the number of characters on that line
      if (current_line[i] == '\n') {
        byte_count+=i+1;
        break;
      }
    }
  }
  return byte_count;
}

int removeExecutableFilenameFromString(char *str) {
  // Start checking for the first forward slash from the end of the string
  for (int i = strlen(str)-1; i >= 0; i--) {
    if (str[i] == '/') {
      // When we find the forward slash, replace it with a null character (ie,
      // end the string here)
      str[i] = 0;
      return 1;
    }
  }
  return 0;
}

int main() {
  char path[MAX_PATH];

  // Get the path of the config executable
#ifdef __WINDOWS__
  GetModuleFileName(NULL, path, MAX_PATH);
#elif  __gnu_linux__
  readlink("/proc/self/exe", path, MAX_PATH);
#endif

  // Remove this executable name from the end of the path
  // path is returned as the updated string from this function
  int ret = removeExecutableFilenameFromString(path);
  if (!ret) {
    printf("Error occurred trying to extract filepath");
    return 1;
  }
  // Append the rest of the path to the config file
  strcat(path, "/hardware/CowTags/global_cfg.h");

  // Open config file for writing
  FILE *fp = fopen(path, "r+");

  int byte_count = Search_in_File(fp, "#define TIMESTAMP_AT_BUILDTIME");
  // Start editing at the byte_count character:
  // C works by needing to count the number of bytes from the start of the file
  // to the point you want to edit. byte_count is that number of bytes
  fseek (fp, byte_count, SEEK_SET);

  // Convert the number received from getCurrentTime() to a string with snprintf
  // That string gets saved into "current_time_str"
  char current_time_str[EPOCH_TIME_CHARS];
  snprintf(current_time_str, EPOCH_TIME_CHARS, "%d", getCurrentTime());

  if (!TESTING_DONT_WRITE_TO_FILE) {
    // Write the number to the file
    fputs(current_time_str, fp);
  }

  fclose(fp);
  return 0;
}
