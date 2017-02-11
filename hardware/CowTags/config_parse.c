#include "config_parse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char* VarFromConfigStrExplicitFilename(char const* filename,
                                       char const* searchString,
                                       int* ret) {
  // Copy searchString to a mutable variable
  char search[MAX_SIZE_SEARCH_STRING];
  strcpy(search, searchString);

  // Append an "=" Sign to this new variable
  strncat(search, "=", 1);

  FILE *file = fopen(filename, "r");
  if (file != NULL) {
    char* line_p = (char *)malloc(MAX_SIZE_OF_LINE);
    char line[MAX_SIZE_OF_LINE];
    // Iterate through every line of the config file
    while (fgets (line, MAX_SIZE_OF_LINE, file) != NULL) {
      // if we get a match, close the file, set return to 1 and copy the line to
      // a char* type
      if (strstr(line, search) != NULL) {
        fclose(file);

        // Now line_p is the same as line but without all the stuff before the
        // "="
        strcpy(line_p, line);
        line_p = strstr(line_p, "=");

        // Remove the equals (this moves the pointer forward 1 character,
        // effectively eliminating the first character in the string)
        line_p++;

        // Remove the newline character at the end
        if (line_p[strlen(line_p)-1] == '\n') {
          // 0 is the null terminator
          line_p[strlen(line_p)-1] = 0;
        }

        *ret = 1;
        return line_p;
      }
    }
  }
  *ret = 0;
  return (char *)"";
}

char* varFromConfigStr(char const* searchString, int* ret) {
  return varFromConfigStrExplicitFilename("../global.conf", searchString, ret);
}

int varFromConfigInt(char const* searchString, int* ret) {
  char *var = varFromConfigStr(searchString, ret);

  return atoi(var);
}
