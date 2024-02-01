
#include "stringToJson.h"

// Function to split a string based on a delimiter
char** splitString(const char* input, char delimiter, int* count) {
    *count = 1;
    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] == delimiter) {
            (*count)++;
        }
    }

    char** result = (char**)malloc((*count) * sizeof(char*));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int substringIndex = 0;
    int substringStart = 0;

    for (int i = 0; i <= strlen(input); ++i) {
        if (input[i] == delimiter || input[i] == '\0') {
            result[substringIndex] = (char*)malloc((i - substringStart + 1) * sizeof(char));
            if (result[substringIndex] == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }

            strncpy(result[substringIndex], input + substringStart, i - substringStart);
            result[substringIndex][i - substringStart] = '\0';

            substringIndex++;
            substringStart = i + 1;
        }
    }

    return result;
}

// Function to free memory allocated for the array of strings
void freeStringArray(char** strings, int count) {
    for (int i = 0; i < count; ++i) {
        free(strings[i]);
    }
    free(strings);
}

// Function to print positions
void printPositions(char** result, int positionStart, int positionCount, char* formattedString) {
    sprintf(formattedString + strlen(formattedString), "\t\"positions\": [\n");

    for (int i = positionStart; i < positionStart + positionCount * 3; i += 3) {
        sprintf(formattedString + strlen(formattedString), "\t\t{\"lat\": \"%s\", \"lng\": \"%s\", \"alt\": \"%s\"}%s\n",
                result[i + 1], result[i], result[i + 2], (i + 3 < positionStart + positionCount * 3) ? "," : "");
    }

    sprintf(formattedString + strlen(formattedString), "\t]\n");
}

// Function to format input string as a JSON-like structure
char* formatInputString(const char* inputString, char delimiter) {
    int count;
    char** result = splitString(inputString, delimiter, &count);

    // Calculate the required size for the formatted string
    size_t formattedSize = 2048 + (count - 5) * 50; // Adjust the initial size and per-position size based on your requirements

    // Allocating a buffer for the formatted string
    char* formattedString = (char*)malloc(formattedSize);
    if (formattedString == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    formattedString[0] = '\0';

    // Print the formatted output
    sprintf(formattedString, "{\n");
    sprintf(formattedString + strlen(formattedString), "\t\"date\": \"%s\",\n", result[0]); // Use result[0] for date
    sprintf(formattedString + strlen(formattedString), "\t\"duration\": %s,\n", result[1]);  // Assuming duration is at index 1
    sprintf(formattedString + strlen(formattedString), "\t\"distance\": %s,\n", result[2]);  // Assuming distance is at index 2
    sprintf(formattedString + strlen(formattedString), "\t\"avg_speed\": %s,\n", result[3]); // Assuming avg_speed is at index 3
    sprintf(formattedString + strlen(formattedString), "\t\"altitude_diff\": %s,\n", result[4]); // Assuming altitude_diff is at index 4

    // Specify the number of positions
    int positionCount = (count - 5) / 3;
    printPositions(result, 5, positionCount, formattedString);

    sprintf(formattedString + strlen(formattedString), "}\n");

    // Free the allocated memory
    freeStringArray(result, count);

    return formattedString;
}

// int main() {
 //    const char* inputString = "";
//     char delimiter = ';';

//     char* formattedResult = formatInputString(inputString, delimiter);

//     printf("%s", formattedResult);

//     // Free the allocated memory
//     free(formattedResult);

//     return 0;
// }
