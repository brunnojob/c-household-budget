#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char month[8], category[49];
    int64_t income, expense;
    size_t entries;
} Bucket;
typedef struct {
    Bucket *items;
    size_t count, capacity;
} Ledger;

static int valid_date(const char *s) {
    int y, m, d;
    char tail;
    if (strlen(s) != 10 || sscanf(s, "%4d-%2d-%2d%c", &y, &m, &d, &tail) != 3 || s[4] != '-' ||
        s[7] != '-' || y < 1900 || m < 1 || m > 12)
        return 0;
    for (int i = 0; i < 10; i++)
        if (i != 4 && i != 7 && (s[i] < '0' || s[i] > '9'))
            return 0;
    const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
    return d >= 1 && d <= days[m] + (m == 2 && leap);
}
static int parse_money(const char *s, int64_t *out) {
    int64_t whole = 0, fraction = 0;
    int digits = 0;
    if (!s || *s < '0' || *s > '9')
        return 0;
    while (*s >= '0' && *s <= '9') {
        int digit = *s++ - '0';
        if (whole > (INT64_MAX / 100 - digit) / 10)
            return 0;
        whole = whole * 10 + digit;
    }
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9' && digits < 2) {
            fraction = fraction * 10 + (*s++ - '0');
            digits++;
        }
        if (!digits)
            return 0;
        if (digits == 1)
            fraction *= 10;
    }
    if (*s || whole > (INT64_MAX - fraction) / 100)
        return 0;
    *out = whole * 100 + fraction;
    return 1;
}
static int add(Ledger *ledger, char *line) {
    char *f[5], *start = line;
    size_t n = 0;
    for (char *p = line;; p++)
        if (*p == '|' || !*p) {
            int done = !*p;
            if (n == 5)
                return 0;
            f[n++] = start;
            if (done)
                break;
            *p = 0;
            start = p + 1;
        }
    if (n != 4 && n != 5)
        return 0;
    int income = n == 5 && !strcmp(f[4], "income");
    int64_t amount;
    if (n == 5 && !income && strcmp(f[4], "expense"))
        return 0;
    if (!valid_date(f[0]) || !*f[1] || strlen(f[1]) > 48 || !*f[3] || !parse_money(f[2], &amount))
        return 0;
    for (const char *p = f[1]; *p; p++)
        if ((unsigned char)*p < 32 || *p == '"' || *p == '\\')
            return 0;
    size_t i;
    for (i = 0; i < ledger->count; i++)
        if (!strncmp(ledger->items[i].month, f[0], 7) && !strcmp(ledger->items[i].category, f[1]))
            break;
    if (i == ledger->count) {
        if (ledger->count == ledger->capacity) {
            size_t next = ledger->capacity ? ledger->capacity * 2 : 16;
            if (next > SIZE_MAX / sizeof(Bucket))
                return -1;
            Bucket *items = realloc(ledger->items, next * sizeof(Bucket));
            if (!items)
                return -1;
            ledger->items = items;
            ledger->capacity = next;
        }
        Bucket b = {0};
        memcpy(b.month, f[0], 7);
        strcpy(b.category, f[1]);
        ledger->items[ledger->count++] = b;
    }
    int64_t *total = income ? &ledger->items[i].income : &ledger->items[i].expense;
    if (*total > INT64_MAX - amount)
        return 0;
    *total += amount;
    ledger->items[i].entries++;
    return 1;
}
static int compare(const void *a, const void *b) {
    const Bucket *x = a, *y = b;
    int c = strcmp(x->month, y->month);
    return c ? c : strcmp(x->category, y->category);
}
int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: budget ledger.txt [YYYY-MM]\n");
        return 2;
    }
    if (argc == 3) {
        char date[11];
        if (strlen(argv[2]) != 7)
            return 2;
        snprintf(date, sizeof date, "%s-01", argv[2]);
        if (!valid_date(date))
            return 2;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("ledger");
        return 1;
    }
    Ledger ledger = {0};
    char *line = NULL;
    size_t cap = 0, row = 0;
    ssize_t len;
    int rejected = 0;
    while ((len = getline(&line, &cap, file)) >= 0) {
        row++;
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = 0;
        if (!len || line[0] == '#')
            continue;
        if (len > 4096 || memchr(line, 0, (size_t)len)) {
            rejected = 1;
            continue;
        }
        int result = add(&ledger, line);
        if (result <= 0) {
            fprintf(stderr, "invalid row %zu\n", row);
            rejected = 1;
            if (result < 0)
                break;
        }
    }
    rejected |= ferror(file);
    free(line);
    fclose(file);
    if (rejected) {
        free(ledger.items);
        return 2;
    }
    if (ledger.count)
        qsort(ledger.items, ledger.count, sizeof(Bucket), compare);
    printf("{\"currency\":\"BRL\",\"buckets\":[");
    size_t printed = 0;
    for (size_t i = 0; i < ledger.count; i++) {
        Bucket *b = &ledger.items[i];
        if (argc == 3 && strcmp(argv[2], b->month))
            continue;
        printf("%s{\"month\":\"%s\",\"category\":\"%s\",\"incomeMinor\":%" PRId64
               ",\"expenseMinor\":%" PRId64 ",\"netMinor\":%" PRId64 ",\"entries\":%zu}",
               printed++ ? "," : "", b->month, b->category, b->income, b->expense,
               b->income - b->expense, b->entries);
    }
    puts("]}");
    free(ledger.items);
    return 0;
}
