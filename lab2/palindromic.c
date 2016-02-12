#include <omp.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXWORKERS 8   /* maximum number of workers */
#define MAXWORDS 500000
#define MAXWORDLEN 120

void reverse(char* str, char* target) {
  int len = strlen(str)-1;
  target[len+1] = 0;
  for (size_t i = 0; i <= len; i++) {
    target[len-i] = str[i];
  }
}

int64_t binary_search(char* str, char** dict, size_t words) {
  int64_t min = 0;
  int64_t max = words;
  int cmp;
  while (min < max) {
    int64_t pos = min + (max-min)/2;
    char* val = dict[pos];
    cmp = strcmp(str, val);
    if (cmp < 0) {
      max = pos;
    } else if (cmp == 0) {
      return pos;
    } else if (cmp > 0) {
      min = pos + 1;
    }
#ifdef DEBUG
    printf("[%lu %lu] %s x %s, %lu\n", min, max, str, val, pos);
#endif
  }
  return -1;
}

int main(int argc, char *argv[]) {
  int64_t numWorkers;
  size_t words, len;
  char** buffer = malloc(sizeof(char*) * MAXWORDS);
  bool* palindromic = calloc(MAXWORDS, sizeof(bool));

  char tmp[MAXWORDLEN + 1];
  memset(tmp, 0, MAXWORDLEN+1);

  if (argc < 2) {
    printf("Usage: %s <input file> [<number of workers>]\n", argv[0]);
    return -1;
  }

  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  FILE* file = fopen(argv[1], "r");
  for (words = 0; words < MAXWORDS; words++) {
    buffer[words] = calloc(1, MAXWORDLEN + 1); // Allocate space for every word
    buffer[words][MAXWORDLEN] = 0; // Padding

    // Read word from file
    char* rv = fgets(buffer[words], MAXWORDLEN, file);

    // End loop when EOF is reached
    if (rv == NULL) {
      break;
    }
  }
  fclose(file);

  omp_set_num_threads(numWorkers);
  double start_time, end_time;
  start_time = omp_get_wtime();
#pragma omp parallel for
  for (size_t pos = 0; pos < words; pos++) {
    len = strlen(buffer[pos]);
    // Replace the new line character with \0
    buffer[pos][len-1] = 0;
    for (int i = 0; i < len-1; i++) {
      // Make all characters lowercase
      buffer[pos][i] = tolower(buffer[pos][i]);
    }
  }
#pragma omp parallel for private(tmp)
  for (size_t pos = 0; pos < words; pos++) {
    reverse(buffer[pos], tmp);
    int64_t found = binary_search(tmp, buffer, words);
    if (found > 0)
      palindromic[pos] = true;
  }
  end_time = omp_get_wtime();

  FILE* output = fopen("output", "w");
  for (size_t pos = 0; pos <= words; pos++) {
    if (palindromic[pos])
      fprintf(output, "%s\n", buffer[pos]);
    free(buffer[pos]);
  }

  fclose(output);
  free(buffer);
  free(palindromic);

  printf("Operation took %g s with %li threads\n",
      end_time - start_time, numWorkers);
}
