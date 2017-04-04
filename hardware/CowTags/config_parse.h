#ifndef CONFIG_PARSE_H
#define CONFIG_PARSE_H

#define MAX_SIZE_OF_LINE 128
#define MAX_SIZE_SEARCH_STRING 128

// args: filename, variable to search for, returned value
// Function returns return code
int varFromConfigStrExplicitFilename(char const*, char const*, char*);

// args: variable to search for, returned value
// Function returns return code 1 for success, 0 for failure
// Example Usage (assuming cfg_variable is a variable defined in global.cfg):
// int var;
// if (varFromConfigInt("cfg_variable", &var)) {
//   var was set
// }
// else {
//   var was not set
// }
int varFromConfigStr(char const*, char*);
int varFromConfigInt(char const*, int*);

#endif // CONFIG_PARSE_H
