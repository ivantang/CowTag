#include <stdio.h>
#include <string.h>
#include <time.h>

#define EPOCH_TIME_CHARS 15
#define TESTING_DONT_WRITE_TO_FILE 0
#define DEBUG 0

int getCurrentTime() {
  return (int)time(NULL);
}

// Returns the byte offset to get to the edit point
int Search_in_File(FILE *fp, char *str) {
  char current_line[512];
  int byte_count=0;
  int i;
#ifdef __WIN32
  int current_line_number=0;
#endif

  while (fgets(current_line, 512, fp) != NULL) {
#ifdef __WIN32
    current_line_number++;
#endif
    if ((strstr(current_line, str)) != NULL) {
      // String found - Take the current count, and add the length of the search
      // string to the byte_count (to get the byte_count to the end of the line)
      byte_count+=strlen(str)+1;
      break;
    }
    // Count the number of bytes in the line if the line does not match
    for (i = 0; i < sizeof(current_line); i++) {
      // If the current character is a newline, that is the end of the line, so
      // the current index+1 is the number of characters on that line
      if (current_line[i] == '\n') {
        byte_count+=i+1;
        break;
      }
    }
  }
#ifdef __WIN32
  // Need to do this for windows, because for some reason windows thought it
  // would be a good idea to use 2 characters for a newline instead of 1 like
  // everybody else. So for every line above the matched line we need to add 1
  // more character.
  return byte_count + current_line_number - 1;
#else
  return byte_count;
#endif
}


int main(int argc, char *argv[]) {
  // argv[1] should be the path to the config file
  if (argc != 2) {
    printf("Expects the path to the config file as argument\n");
    return 1;
  }

  if (DEBUG) {
    printf("config path: %s\n", argv[1]);
  }

  // Open config file for writing. r+ specifies read and write, but the write
  // does not delete the contents of the file
  FILE *fp = fopen(argv[1], "r+");

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
