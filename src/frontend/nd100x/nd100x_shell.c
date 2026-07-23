/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the nd100em
 * distribution in the file COPYING); if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "nd100x_shell.h"
#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"
#include "../../machine/machine_types.h"
#include "../../machine/machine_protos.h"
#include "../../cpu/cpu_types.h"
#include "../../cpu/cpu_protos.h"

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define SHELL_PROMPT "@"
#define MAX_CMD_LEN 256
#define MAX_TOKENS 10

typedef struct {
    const char *name;
    const char *abbrev;
    const char *help;
    int (*handler)(const char *nd100Root, int argc, char **argv);
} ShellCommand;

/* Forward declarations */
static int cmd_help(const char *nd100Root, int argc, char **argv);
static int cmd_exit(const char *nd100Root, int argc, char **argv);
static int cmd_list_files(const char *nd100Root, int argc, char **argv);
static int cmd_run_program(const char *nd100Root, int argc, char **argv);
static int cmd_show_regs(const char *nd100Root, int argc, char **argv);

static ShellCommand commands[] = {
    {"HELP",          "HE",   "Show this help message", cmd_help},
    {"LIST-FILES",    "LI-FI", "List BPUN/PROG files", cmd_list_files},
    {"RUN-PROGRAM",   "RU-PR", "Load and run a program", cmd_run_program},
    {"SHOW-REGISTERS","SH-RE", "Display CPU registers", cmd_show_regs},
    {"EXIT",          "EX",   "Exit the shell", cmd_exit},
    {NULL, NULL, NULL, NULL}
};

/**
 * Check if a command name matches a command (supports abbreviation)
 * Abbreviation rules:
 * - Full name match: exact
 * - Abbreviated match: each part separated by '-' must match the prefix
 *   e.g., "LI-FI" matches "LIST-FILES", "LI" matches "LIST-FILES"
 */
static bool cmd_matches(const char *input, const char *full_name, const char *abbrev) {
    if (!input || !full_name) return false;

    /* Uppercase input for case-insensitive comparison */
    char upper_input[MAX_CMD_LEN];
    for (int i = 0; input[i] && i < MAX_CMD_LEN - 1; i++) {
        upper_input[i] = toupper((unsigned char)input[i]);
    }
    upper_input[strlen(input)] = '\0';

    /* Exact match on full name or abbreviation */
    if (strcmp(upper_input, full_name) == 0) return true;
    if (abbrev && strcmp(upper_input, abbrev) == 0) return true;

    /* Prefix match: "LIST" matches "LIST-FILES" */
    size_t len = strlen(upper_input);
    if (strncmp(upper_input, full_name, len) == 0) {
        char next = full_name[len];
        return (next == '\0' || next == '-');
    }

    return false;
}

/**
 * Parse a command line into tokens
 * Returns token count, sets tokens array
 */
static int parse_tokens(char *line, char **tokens, int max_tokens) {
    int count = 0;
    char *ptr = line;

    while (count < max_tokens && *ptr) {
        while (isspace((unsigned char)*ptr)) ptr++;
        if (!*ptr) break;

        tokens[count++] = ptr;
        while (*ptr && !isspace((unsigned char)*ptr)) ptr++;
        if (*ptr) *ptr++ = '\0';
    }

    return count;
}

/**
 * List BPUN/PROG files in the nd100Root directory
 * Supports pattern filtering: *.bpun, *.prog, etc.
 */
static int cmd_list_files(const char *nd100Root, int argc, char **argv) {

    const char *pattern = (argc > 1) ? argv[1] : "*";
    const char *search_dir = nd100Root ? nd100Root : ".";

    DIR *dir = opendir(search_dir);
    if (!dir) {
        fprintf(stderr, "Cannot open directory: %s\n", search_dir);
        return -1;
    }

    printf("Files in %s matching '%s':\n", search_dir, pattern);

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            /* Match pattern */
            bool match = false;
            if (strcmp(pattern, "*") == 0) {
                match = true;
            } else if (strstr(pattern, "*")) {
                /* Simple glob: check extensions */
                const char *ext = strrchr(pattern, '.');
                if (ext) {
                    const char *file_ext = strrchr(entry->d_name, '.');
                    if (file_ext) {
                        match = (strcasecmp(file_ext, ext) == 0);
                    }
                }
            } else {
                match = (strcasecmp(entry->d_name, pattern) == 0);
            }

            if (match) {
                printf("  %s\n", entry->d_name);
                count++;
            }
        }
    }
    closedir(dir);

    if (count == 0) {
        printf("  (no files found)\n");
    }
    return 0;
}

/**
 * Load and run a BPUN program file
 */
static int cmd_run_program(const char *nd100Root, int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: RUN-PROGRAM <filename>\n");
        return -1;
    }

    const char *filename = argv[1];
    const char *search_dir = nd100Root ? nd100Root : ".";

    /* Build full path */
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", search_dir, filename);

    printf("Loading %s...\n", filepath);

    /* Determine file type by extension */
    const char *ext = strrchr(filename, '.');
    int boot_type = BOOT_BPUN;

    if (ext && strcasecmp(ext, ".prog") == 0) {
        printf("Note: PROG files require compilation (not yet implemented)\n");
        boot_type = BOOT_BPUN;
    }

    /* Load the file */
    int boot_addr = program_load(boot_type, 0, filepath, true, 0, false);
    if (boot_addr < 0) {
        fprintf(stderr, "Failed to load program\n");
        return -1;
    }

    printf("Program loaded at entry point: 0o%o\n", boot_addr);
    printf("(CPU execution not yet integrated with shell)\n");

    return 0;
}

/**
 * Display CPU registers
 */
static int cmd_show_regs(const char *nd100Root, int argc, char **argv) {
    (void)nd100Root;
    (void)argc;
    (void)argv;

    extern struct CpuRegs *gReg;

    printf("CPU Registers:\n");

    if (gReg) {
        printf("  A:     0o%06o\n", gReg->reg[gPIL][_A]);
        printf("  B:     0o%06o\n", gReg->reg[gPIL][_B]);
        printf("  D:     0o%06o\n", gReg->reg[gPIL][_D]);
        printf("  X:     0o%06o\n", gReg->reg[gPIL][_X]);
        printf("  L:     0o%06o\n", gReg->reg[gPIL][_L]);
        printf("  T:     0o%06o\n", gReg->reg[gPIL][_T]);
        printf("  P:     0o%06o\n", gReg->reg[gPIL][_P]);
        printf("  STS:   0o%06o\n", gReg->reg_STS);
    }

    return 0;
}

/**
 * Show help message
 */
static int cmd_help(const char *nd100Root, int argc, char **argv) {
    (void)nd100Root;
    (void)argc;
    (void)argv;

    printf("ND-100 Interactive Shell - Available Commands:\n\n");
    for (int i = 0; commands[i].name; i++) {
        printf("  %-15s  %s\n", commands[i].name, commands[i].help);
        printf("      (abbrev: %s)\n", commands[i].abbrev);
    }
    printf("\nNote: Commands are case-insensitive and support abbreviation.\n");
    printf("      File names use host extensions (.bpun, .prog, etc)\n");

    return 0;
}

/**
 * Exit the shell
 */
static int cmd_exit(const char *nd100Root, int argc, char **argv) {
    (void)nd100Root;
    (void)argc;
    (void)argv;

    printf("Exiting shell.\n");
    return 1;  /* Signal to exit the shell loop */
}

/**
 * Execute a single command
 * Returns: 0 = continue, 1 = exit shell, -1 = error
 */
static int execute_command(const char *nd100Root, char *line) {
    if (!line || *line == '\0') return 0;

    char *tokens[MAX_TOKENS];
    int argc = parse_tokens(line, tokens, MAX_TOKENS);

    if (argc == 0) return 0;

    /* Find matching command */
    for (int i = 0; commands[i].name; i++) {
        if (cmd_matches(tokens[0], commands[i].name, commands[i].abbrev)) {
            int result = commands[i].handler(nd100Root, argc, tokens);
            return result;
        }
    }

    printf("Unknown command: %s\n", tokens[0]);
    printf("Type 'HELP' for available commands.\n");
    return -1;
}

/**
 * Read a line from input (with or without readline)
 */
static char *read_line(void) {
#ifdef HAVE_READLINE
    return readline(SHELL_PROMPT " ");
#else
    static char buffer[MAX_CMD_LEN];
    printf("%s ", SHELL_PROMPT);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return buffer;
    }
    return NULL;
#endif
}

/**
 * Execute commands from a script file
 */
static int execute_script(const char *nd100Root, const char *script_path) {
    FILE *f = fopen(script_path, "r");
    if (!f) {
        fprintf(stderr, "Cannot open script file: %s\n", script_path);
        return -1;
    }

    char line[MAX_CMD_LEN];
    int line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;

        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /* Skip empty lines and comments */
        if (!*line || *line == '#') continue;

        printf("%s %s\n", SHELL_PROMPT, line);
        int result = execute_command(nd100Root, line);
        if (result == 1) break;  /* EXIT command */
        if (result < 0) {
            fprintf(stderr, "Script error at line %d\n", line_num);
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

/**
 * Main shell loop
 */
int nd100x_shell_run(const char *nd100Root, const char *scriptPath) {
    printf("\n");
    printf("ND-100 Interactive Shell\n");
    printf("Type 'HELP' for available commands\n");
    printf("\n");

#ifdef HAVE_READLINE
    using_history();
#endif

    /* Execute script if provided */
    if (scriptPath) {
        printf("Loading script: %s\n\n", scriptPath);
        int result = execute_script(nd100Root, scriptPath);
        if (result < 0) {
            fprintf(stderr, "Script execution failed\n");
            return -1;
        }
    }

    /* Main REPL loop */
    while (1) {
        char *line = read_line();
        if (!line) {
            printf("\n");
            break;  /* EOF */
        }

#ifdef HAVE_READLINE
        if (*line) add_history(line);
#endif

        int result = execute_command(nd100Root, line);
        if (result == 1) {
            break;  /* EXIT command */
        }

#ifdef HAVE_READLINE
        free(line);
#endif
    }

    printf("Shell exited.\n");
    return 0;
}
