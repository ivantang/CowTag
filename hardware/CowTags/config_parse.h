#ifndef CONFIG_PARSE_H
#define CONFIG_PARSE_H

#define MAX_SIZE_OF_LINE 128
#define MAX_SIZE_SEARCH_STRING 128

// args: filename, variable to search for, return code
char* varFromConfigStrExplicitFilename(char const*, char const*, int*);

// args: variable to search for, return code
char* varFromConfigStr(char const*, int*);
int   varFromConfigInt(char const*, int*);

#endif // CONFIG_PARSE_H
