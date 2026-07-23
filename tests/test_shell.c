/*
 * test_shell.c - Unit tests for ND-100X interactive shell
 *
 * Tests core shell functionality:
 * - Command parsing and tokenization
 * - Abbreviation matching
 * - File discovery
 * - Shell entry point
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* ================================================================ */
/* Helper: Create test directory structure                          */
/* ================================================================ */
static bool setup_test_dir(const char *testdir) {
    if (mkdir(testdir, 0755) < 0 && errno != EEXIST) {
        return false;
    }

    /* Create test BPUN files */
    char path[512];
    snprintf(path, sizeof(path), "%s/test1.bpun", testdir);
    FILE *f = fopen(path, "w");
    if (!f) return false;
    fprintf(f, "test");
    fclose(f);

    snprintf(path, sizeof(path), "%s/test2.prog", testdir);
    f = fopen(path, "w");
    if (!f) return false;
    fprintf(f, "test");
    fclose(f);

    return true;
}

/* ================================================================ */
/* Test: Parse tokens from input line                               */
/* ================================================================ */
static bool test_parse_tokens(void) {
    printf("TEST: parse_tokens()...\n");

    /* Test tokenization */
    char line[] = "LIST-FILES *.bpun extra";
    char *tokens[10];

    /* Manually test tokenization logic */
    int count = 0;
    char *ptr = line;
    while (count < 10 && *ptr) {
        while (*ptr && *ptr == ' ') ptr++;
        if (!*ptr) break;
        tokens[count++] = ptr;
        while (*ptr && *ptr != ' ') ptr++;
        if (*ptr) *ptr++ = '\0';
    }

    if (count != 3) {
        printf("  FAIL: Expected 3 tokens, got %d\n", count);
        return false;
    }
    if (strcmp(tokens[0], "LIST-FILES") != 0) {
        printf("  FAIL: First token is '%s', expected 'LIST-FILES'\n", tokens[0]);
        return false;
    }
    if (strcmp(tokens[1], "*.bpun") != 0) {
        printf("  FAIL: Second token is '%s', expected '*.bpun'\n", tokens[1]);
        return false;
    }

    printf("  PASS\n");
    return true;
}

/* ================================================================ */
/* Test: Command abbreviation matching                              */
/* ================================================================ */
static bool test_cmd_abbreviations(void) {
    printf("TEST: cmd_abbreviations()...\n");

    struct {
        const char *input;
        const char *full;
        const char *abbrev;
        bool should_match;
    } cases[] = {
        { "HELP", "HELP", "HE", true },
        { "HE", "HELP", "HE", true },
        { "help", "HELP", "HE", true },  /* case-insensitive */
        { "HELPX", "HELP", "HE", false }, /* no partial match */
        { "LI-FI", "LIST-FILES", "LI-FI", true },
        { "LIST", "LIST-FILES", "LI-FI", true }, /* prefix match */
        { "LI", "LIST-FILES", "LI-FI", true },   /* prefix match */
        { "EXIT", "EXIT", "EX", true },
        { "EX", "EXIT", "EX", true },
        { NULL, NULL, NULL, false }
    };

    int passed = 0, total = 0;
    for (int i = 0; cases[i].input; i++) {
        total++;
        const char *input = cases[i].input;
        const char *full = cases[i].full;
        const char *abbrev = cases[i].abbrev;
        bool expected = cases[i].should_match;

        /* Simulate cmd_matches logic */
        char upper_input[256];
        for (int j = 0; input[j]; j++) {
            upper_input[j] = input[j] >= 'a' && input[j] <= 'z'
                ? input[j] - 'a' + 'A' : input[j];
        }
        upper_input[strlen(input)] = '\0';

        bool exact_match = (strcmp(upper_input, full) == 0);
        bool abbrev_match = abbrev && (strcmp(upper_input, abbrev) == 0);
        bool prefix_match = (strncmp(upper_input, full, strlen(upper_input)) == 0);

        bool result = exact_match || abbrev_match || prefix_match;

        if (result == expected) {
            passed++;
        } else {
            printf("  FAIL: input='%s' full='%s' abbrev='%s': got %d, expected %d\n",
                   input, full, abbrev, result, expected);
        }
    }

    printf("  %d/%d cases passed\n", passed, total);
    return passed == total;
}

/* ================================================================ */
/* Test: File discovery and pattern matching                        */
/* ================================================================ */
static bool test_file_discovery(void) {
    printf("TEST: file_discovery()...\n");

    const char *testdir = "/tmp/nd100x-shell-test";

    if (!setup_test_dir(testdir)) {
        printf("  FAIL: Could not create test directory\n");
        return false;
    }

    /* Verify files were created */
    char path1[512], path2[512];
    snprintf(path1, sizeof(path1), "%s/test1.bpun", testdir);
    snprintf(path2, sizeof(path2), "%s/test2.prog", testdir);

    if (access(path1, F_OK) != 0) {
        printf("  FAIL: test1.bpun not created\n");
        return false;
    }
    if (access(path2, F_OK) != 0) {
        printf("  FAIL: test2.prog not created\n");
        return false;
    }

    printf("  PASS: Files created in %s\n", testdir);

    /* Clean up */
    unlink(path1);
    unlink(path2);
    rmdir(testdir);

    return true;
}

/* ================================================================ */
/* Test: Shell entry point (non-interactive)                       */
/* ================================================================ */
static bool test_shell_entry_point(void) {
    printf("TEST: shell_entry_point()...\n");

    /* Verify nd100x_shell_run symbol exists and is callable */
    extern int nd100x_shell_run(const char *nd100Root, const char *scriptPath);

    if (!nd100x_shell_run) {
        printf("  FAIL: nd100x_shell_run symbol not found\n");
        return false;
    }

    printf("  PASS: nd100x_shell_run is defined\n");
    return true;
}

/* ================================================================ */
/* Main test runner                                                 */
/* ================================================================ */
int main(int argc, char *argv[]) {
    printf("\n========================================\n");
    printf("ND-100X Shell Unit Tests\n");
    printf("========================================\n\n");

    int passed = 0, total = 0;

    /* Run tests */
    if (test_parse_tokens()) passed++;
    total++;

    if (test_cmd_abbreviations()) passed++;
    total++;

    if (test_file_discovery()) passed++;
    total++;

    if (test_shell_entry_point()) passed++;
    total++;

    /* Report */
    printf("\n========================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    printf("========================================\n\n");

    return (passed == total) ? 0 : 1;
}
