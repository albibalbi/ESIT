#ifndef __STRINGTOJSON_H__
#define __STRINGTOJSON_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** splitString(const char* input, char delimiter, int* count) ;
void freeStringArray(char** strings, int count);
void printPositions(char** result, int positionStart, int positionCount, char* formattedString);
char* formatInputString(const char* inputString, char delimiter) ;

#endif
