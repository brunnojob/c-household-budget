#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct { char month[8]; char category[48]; double total; } Bucket;
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "usage: budget <ledger.txt>\n"); return 2; }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }
    char *line = NULL; size_t cap = 0; ssize_t n; Bucket *b = NULL; size_t used = 0;
    while ((n = getline(&line, &cap, f)) >= 0) {
        if (n && line[n-1] == '\n') line[--n] = 0;
        char *date = strtok(line, "|"), *category = strtok(NULL, "|"), *amount = strtok(NULL, "|");
        if (!date || !category || !amount || strlen(date) < 7) { fprintf(stderr, "invalid row\n"); free(b); free(line); fclose(f); return 1; }
        errno = 0; char *end; double value = strtod(amount, &end);
        if (errno || end == amount || *end || value < 0) { fprintf(stderr, "invalid amount\n"); free(b); free(line); fclose(f); return 1; }
        char month[8]; memcpy(month, date, 7); month[7] = 0;
        size_t i; for (i=0; i<used; i++) if (!strcmp(b[i].month, month) && !strcmp(b[i].category, category)) break;
        if (i == used) { Bucket *next = realloc(b, (used+1)*sizeof(*b)); if (!next) return 1; b=next; snprintf(b[used].month,8,"%s",month); snprintf(b[used].category,48,"%s",category); b[used].total=0; used++; }
        b[i].total += value;
    }
    for (size_t i=0; i<used; i++) printf("%s | %-20s | %.2f\n", b[i].month,b[i].category,b[i].total);
    free(b); free(line); fclose(f); return 0;
}