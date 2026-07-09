/*
 * wc.c - a reimplementation of the Unix wc utility in C
 *
 * Supports:
 *   -l   print line count
 *   -w   print word count
 *   -c   print byte count
 *   -m   print character count (UTF-8 aware)
 *
 * If no flags are given, defaults to -l -w -c (matches GNU wc).
 * Reads from stdin if no files are given. Accepts multiple files
 * and prints a "total" line when more than one file is given.
 *
 * Build:  gcc -Wall -Wextra -o wc wc.c
 * Usage:  ./wc [-lwcm] [file...]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    long lines;
    long words;
    long bytes;
    long chars;
} counts_t;

typedef struct {
    int lines;
    int words;
    int bytes;
    int chars;
} flags_t;

/* Returns 1 if byte is a UTF-8 continuation byte (10xxxxxx), else 0 */
static int is_utf8_continuation(unsigned char c) {
    return (c & 0xC0) == 0x80;
}

/* Reads from fp and fills in counts. Returns 0 on success, -1 on read error. */
static int count_stream(FILE *fp, counts_t *counts) {
    int c;
    int in_word = 0;

    counts->lines = 0;
    counts->words = 0;
    counts->bytes = 0;
    counts->chars = 0;

    while ((c = fgetc(fp)) != EOF) {
        unsigned char byte = (unsigned char)c;

        counts->bytes++;

        if (!is_utf8_continuation(byte)) {
            counts->chars++;
        }

        if (byte == '\n') {
            counts->lines++;
        }

        if (isspace(byte)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            counts->words++;
        }
    }

    if (ferror(fp)) {
        return -1;
    }

    return 0;
}

static void print_counts(const counts_t *counts, const flags_t *flags, const char *label) {
    if (flags->lines) printf("%7ld", counts->lines);
    if (flags->words) printf("%7ld", counts->words);
    if (flags->bytes) printf("%7ld", counts->bytes);
    if (flags->chars) printf("%7ld", counts->chars);

    if (label != NULL) {
        printf(" %s", label);
    }
    printf("\n");
}

static void add_counts(counts_t *total, const counts_t *c) {
    total->lines += c->lines;
    total->words += c->words;
    total->bytes += c->bytes;
    total->chars += c->chars;
}

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-lwcm] [file ...]\n", prog_name);
}

/*
 * Manual argv parser (no getopt dependency, so this builds with any C
 * compiler on any platform, including MSVC/MinGW where unistd.h and
 * getopt aren't available).
 *
 * Any argv entry starting with '-' (and longer than just "-") is treated
 * as a flag cluster, e.g. "-lw" sets both lines and words. Everything
 * else is treated as a filename. Returns the index of the first
 * filename argument, or argc if there are none.
 */
static int parse_args(int argc, char *argv[], flags_t *flags, int *any_flag_set) {
    int i;
    int first_file_index = argc;
    int found_first_file = 0;

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (arg[0] == '-' && arg[1] != '\0') {
            char *p;
            for (p = arg + 1; *p != '\0'; p++) {
                switch (*p) {
                    case 'l':
                        flags->lines = 1;
                        *any_flag_set = 1;
                        break;
                    case 'w':
                        flags->words = 1;
                        *any_flag_set = 1;
                        break;
                    case 'c':
                        flags->bytes = 1;
                        *any_flag_set = 1;
                        break;
                    case 'm':
                        flags->chars = 1;
                        *any_flag_set = 1;
                        break;
                    default:
                        print_usage(argv[0]);
                        exit(1);
                }
            }
        } else {
            if (!found_first_file) {
                first_file_index = i;
                found_first_file = 1;
            }
        }
    }

    return first_file_index;
}

int main(int argc, char *argv[]) {
    flags_t flags = {0, 0, 0, 0};
    int any_flag_set = 0;
    int exit_code = 0;

    int first_file = parse_args(argc, argv, &flags, &any_flag_set);

    /* Default: -l -w -c, matching GNU wc's default behavior */
    if (!any_flag_set) {
        flags.lines = 1;
        flags.words = 1;
        flags.bytes = 1;
    }

    int num_files = argc - first_file;

    /* No files given: read from stdin */
    if (num_files <= 0) {
        counts_t counts;
        if (count_stream(stdin, &counts) != 0) {
            fprintf(stderr, "wc: error reading stdin\n");
            return 1;
        }
        print_counts(&counts, &flags, NULL);
        return 0;
    }

    counts_t total = {0, 0, 0, 0};

    for (int i = first_file; i < argc; i++) {
        const char *filename = argv[i];
        FILE *fp = fopen(filename, "rb");

        if (fp == NULL) {
            fprintf(stderr, "wc: %s: No such file or directory\n", filename);
            exit_code = 1;
            continue;
        }

        counts_t counts;
        if (count_stream(fp, &counts) != 0) {
            fprintf(stderr, "wc: %s: error reading file\n", filename);
            exit_code = 1;
            fclose(fp);
            continue;
        }

        fclose(fp);
        add_counts(&total, &counts);
        print_counts(&counts, &flags, filename);
    }

    if (num_files > 1) {
        print_counts(&total, &flags, "total");
    }

    return exit_code;
}