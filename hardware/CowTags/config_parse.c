#include "config_parse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int VarFromConfigStrExplicitFilename(char const* filename,
                                       char const* searchString,
                                       char* output) {
  // Copy searchString to a mutable variable
  char search[MAX_SIZE_SEARCH_STRING];
  strcpy(search, searchString);

  // Append an "=" Sign to this new variable
  strncat(search, "=", 1);

  FILE *file = fopen(filename, "r");
  if (file != NULL) {
    // Iterate through every line of the config file
    while (fgets (output, MAX_SIZE_OF_LINE, file) != NULL) {
      // if we get a match, close the file, set return to 1 and copy the line to
      // a char* type
      if (strstr(output, search) != NULL) {
        fclose(file);

        // Now line_p is the same as line but without all the stuff before the
        // "="
        strcpy(output, strstr(output, "="));

        // Remove the equals (this moves the pointer forward 1 character,
        // effectively eliminating the first character in the string)
        strcpy(output, output+1);

        // Remove the newline character at the end
        if (output[strlen(output)-1] == '\n') {
          // 0 is the null terminator
          output[strlen(output)-1] = 0;
        }

        return 1;
      }
    }
  }
  fclose(file);
  return 0;
}

int varFromConfigStr(char const* searchString, char* output) {
  return VarFromConfigStrExplicitFilename("../global.conf", searchString, output);
}

int varFromConfigInt(char const* searchString, int* output) {
  char out[MAX_SIZE_OF_LINE];
  int ret;

  ret = varFromConfigStr(searchString, out);

  *output = atoi(out);

  return ret;
}
