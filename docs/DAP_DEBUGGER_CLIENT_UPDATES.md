# DAP Debugger Client Updates for Mixed-Language Support

## Overview

The `dap_debugger` is a command-line DAP client located in `external/libdap/src/dap_debugger/`. It needs updates to support the new mixed-language debugging features.

## Current State

**Location:** `external/libdap/src/dap_debugger/`

**Files:**
- `dap_debugger_main.c` - Main entry point and command line parsing
- `dap_debugger_commands.c` - Command handlers
- `dap_debugger_commands.h` - Command declarations
- `dap_debugger_help.c` - Help system
- `dap_debugger_ui.c` - UI utilities
- `dap_debugger_types.h` - Type definitions

### Current Launch Parameters

**From `dap_debugger_main.c:410-428`:**

```c
// Current launch request only includes:
cJSON_AddStringToObject(launch_args, "program", program_file);
cJSON_AddBoolToObject(launch_args, "stopOnEntry", stop_at_entry);
cJSON_AddBoolToObject(launch_args, "noDebug", false);

if (program_args) {
    cJSON_AddItemToObject(launch_args, "args", program_args);
}

if (env_vars) {
    cJSON_AddItemToObject(launch_args, "env", env_vars);
}

if (working_dir) {
    cJSON_AddStringToObject(launch_args, "cwd", working_dir);
}
```

### Current Command Line Options

**From `dap_debugger_main.c:238-250`:**

```
-h, --host <host>           Server hostname (default: localhost)
-p, --port <port>           Server port (default: 4711)
-e, --stop-on-entry         Stop at program entry
-f, --program <file>        Program to debug
-a, --args <args>           Program arguments (comma-separated)
-v, --env <vars>            Environment variables (name=value,...)
-w, --cwd <dir>             Working directory
-d, --debug                 Enable debug mode
-?, --help                  Show help
```

## Required Updates

### 1. Add Command Line Options

**File:** `external/libdap/src/dap_debugger/dap_debugger_main.c`

#### Add New Options to `long_options` array (line 239)

```c
struct option long_options[] = {
    {"host", required_argument, 0, 'h'},
    {"port", required_argument, 0, 'p'},
    {"stop-on-entry", no_argument, 0, 'e'},
    {"program", required_argument, 0, 'f'},
    {"args", required_argument, 0, 'a'},
    {"env", required_argument, 0, 'v'},
    {"cwd", required_argument, 0, 'w'},
    {"debug", no_argument, 0, 'd'},
    
    // NEW OPTIONS:
    {"sources", required_argument, 0, 's'},      // Source files
    {"map-file", required_argument, 0, 'm'},     // MAP file
    {"granularity", required_argument, 0, 'g'},  // Step granularity
    
    {"help", no_argument, 0, '?'},
    {0, 0, 0, 0}
};
```

#### Add Variables for New Options (line 236)

```c
const char* program_file = NULL;
const char* host = DEFAULT_HOST;
int port = DEFAULT_PORT;
bool debug_mode = false;
bool stop_at_entry = false;
cJSON* program_args = NULL;
cJSON* env_vars = NULL;
const char* working_dir = NULL;

// NEW VARIABLES:
cJSON* sources = NULL;           // Array of source files
const char* map_file = NULL;     // MAP file path
const char* granularity = "line"; // Default granularity
```

#### Update getopt_long Call (line 255)

```c
while ((opt = getopt_long(argc, argv, "h:p:ef:a:v:d:w:s:m:g:?", long_options, &option_index)) != -1) {
    switch (opt) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'e':
            stop_at_entry = true;
            break;
        case 'f':
            program_file = optarg;
            break;
        case 'a': {
            // Parse program arguments
            // ... existing code ...
            break;
        }
        case 'v': {
            // Parse environment variables
            // ... existing code ...
            break;
        }
        case 'd':
            debug_mode = true;
            break;
        case 'w':
            working_dir = optarg;
            break;
            
        // NEW CASES:
        case 's': {
            // Parse sources - comma-separated list of files
            sources = cJSON_CreateArray();
            char* sources_copy = strdup(optarg);
            char* token = strtok(sources_copy, ",");
            while (token) {
                // Trim whitespace
                while (*token == ' ') token++;
                cJSON_AddItemToArray(sources, cJSON_CreateString(token));
                token = strtok(NULL, ",");
            }
            free(sources_copy);
            break;
        }
        case 'm':
            map_file = optarg;
            break;
        case 'g':
            granularity = optarg;
            break;
            
        case '?':
            print_usage(argv[0]);
            // ... cleanup ...
            return 0;
    }
}
```

#### Update Launch Request (line 410-428)

```c
// Add the required program path parameter
cJSON_AddStringToObject(launch_args, "program", program_file);
cJSON_AddBoolToObject(launch_args, "stopOnEntry", stop_at_entry);
cJSON_AddBoolToObject(launch_args, "noDebug", false);

// NEW: Add sources array
if (sources) {
    cJSON_AddItemToObject(launch_args, "sources", sources);
    sources = NULL;  // Ownership transferred
}

// NEW: Add mapFile
if (map_file) {
    cJSON_AddStringToObject(launch_args, "mapFile", map_file);
}

// Add optional parameters if provided
if (program_args) {
    cJSON_AddItemToObject(launch_args, "args", program_args);
}

if (env_vars) {
    cJSON_AddItemToObject(launch_args, "env", env_vars);
}

if (working_dir) {
    cJSON_AddStringToObject(launch_args, "cwd", working_dir);
}
```

### 2. Add print_usage() Function

**File:** `external/libdap/src/dap_debugger/dap_debugger_main.c`

Add before `main()`:

```c
void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Debug Adapter Protocol (DAP) Command-Line Debugger Client\n\n");
    
    printf("Connection Options:\n");
    printf("  -h, --host <host>          Server hostname (default: localhost)\n");
    printf("  -p, --port <port>          Server port (default: 4711)\n\n");
    
    printf("Program Options:\n");
    printf("  -f, --program <file>       Program to debug (required)\n");
    printf("  -s, --sources <files>      Source files (comma-separated)\n");
    printf("                             Examples:\n");
    printf("                               -s main.c,util.c,delay.s\n");
    printf("                               -s \"main.c, util.s, app.h\"\n");
    printf("  -m, --map-file <file>      MAP file for assembly source mapping\n");
    printf("  -a, --args <args>          Program arguments (comma-separated)\n");
    printf("  -v, --env <vars>           Environment variables (name=value,...)\n");
    printf("  -w, --cwd <dir>            Working directory\n\n");
    
    printf("Debug Options:\n");
    printf("  -e, --stop-on-entry        Stop at program entry point\n");
    printf("  -g, --granularity <type>   Step granularity: line|instruction|statement\n");
    printf("                             (default: line)\n");
    printf("  -d, --debug                Enable debug mode (verbose output)\n\n");
    
    printf("Other Options:\n");
    printf("  -?, --help                 Show this help message\n\n");
    
    printf("Examples:\n");
    printf("---------\n");
    printf("Pure Assembly:\n");
    printf("  %s -f program.out -m program.map -s main.s,lib.s\n\n", program_name);
    
    printf("Pure C:\n");
    printf("  %s -f a.out -s \"main.c, utils.c, app.h\"\n\n", program_name);
    
    printf("Mixed C/Assembly:\n");
    printf("  %s -f a.out -s main.c,util.c,delay.s -m program.map\n\n", program_name);
    
    printf("With Arguments:\n");
    printf("  %s -f test.out -a \"arg1,arg2,arg3\" -s test.s\n\n", program_name);
    
    printf("Once connected, type 'help' for debugger commands.\n");
}
```

### 3. Update Step Commands to Support Granularity

**File:** `external/libdap/src/dap_debugger/dap_debugger_commands.c`

#### Update `handle_next_command` (line 59)

```c
int handle_next_command(DAPClient* client, const char* args) {
    if (!client) return 1;
    
    int thread_id = 0;
    bool single_thread = false;
    
    // NEW: Get granularity from global or args
    const char* granularity = "line";  // Default
    
    // Parse args: could be "thread_id" or "thread_id granularity"
    if (args && *args) {
        char args_copy[256];
        strncpy(args_copy, args, sizeof(args_copy) - 1);
        
        char* thread_str = strtok(args_copy, " ");
        char* gran_str = strtok(NULL, " ");
        
        if (thread_str) {
            thread_id = atoi(thread_str);
            single_thread = true;
        }
        
        if (gran_str) {
            granularity = gran_str;  // "line", "instruction", or "statement"
        }
    }
    
    DAPStepResult result = {0};
    int error = dap_client_next(client, thread_id, granularity, single_thread, &result);
    if (error != DAP_ERROR_NONE) {
        fprintf(stderr, "Error stepping over: %d\n", error);
        return 1;
    }
    return 0;
}
```

#### Update `handle_step_command` (line 79)

Similar changes for step-in.

#### Update `handle_step_out_command`

Similar changes for step-out.

### 4. Update Help Documentation

**File:** `external/libdap/src/dap_debugger/dap_debugger.c`

#### Update "next" command (line 61)

```c
{
    .name = "next",
    .alias = "n",
    .syntax = "next [thread_id] [granularity]",
    .description = "Step over next line or instruction",
    .request_format = "{\"threadId\": number, \"granularity\": string, \"singleThread\": boolean}",
    .response_format = "{\"allThreadsContinued\": boolean}",
    .events = "continued, stopped",
    .has_options = true,
    .option_types = "thread_id|granularity",
    .option_descriptions = "Optional thread ID to step|Step granularity: line, instruction, statement",
    .examples = "next|Step over current line|next 1|Step over in thread 1|next 1 instruction|Step one instruction|next 1 line|Step one source line",
    .category = CATEGORY_EXECUTION_CONTROL,
    .implemented = true,
    .handler = handle_next_command
},
```

#### Update "step" command (line 77)

```c
{
    .name = "step",
    .alias = "s",
    .syntax = "step [thread_id] [granularity]",
    .description = "Step into function call",
    .request_format = "{\"threadId\": number, \"granularity\": string, \"singleThread\": boolean}",
    .response_format = "{\"allThreadsContinued\": boolean}",
    .events = "continued, stopped",
    .has_options = true,
    .option_types = "thread_id|granularity",
    .option_descriptions = "Optional thread ID to step|Step granularity: line, instruction, statement",
    .examples = "step|Step into function|step 1|Step into in thread 1|step 1 instruction|Step one instruction into function",
    .category = CATEGORY_EXECUTION_CONTROL,
    .implemented = true,
    .handler = handle_step_command
},
```

#### Add New Command for Source Request

**After disassemble command:**

```c
{
    .name = "source",
    .alias = "src",
    .syntax = "source <sourceReference>",
    .description = "Show source file content by reference",
    .request_format = "{\"sourceReference\": number}",
    .response_format = "{\"content\": string}",
    .events = NULL,
    .has_options = true,
    .option_types = "sourceReference",
    .option_descriptions = "Source reference ID from stack frame",
    .examples = "source 1001|Show content of sourceReference 1001",
    .category = CATEGORY_SOURCE,
    .implemented = true,
    .handler = handle_source_command
},
```

### 5. Implement Source Command Handler

**File:** `external/libdap/src/dap_debugger/dap_debugger_commands.c`

Add new handler:

```c
int handle_source_command(DAPClient* client, const char* args) {
    if (!client) {
        printf("Error: Debugger not connected\n");
        return -1;
    }
    
    if (!args || !*args) {
        printf("Error: sourceReference required\n");
        printf("Usage: source <sourceReference>\n");
        printf("Example: source 1001\n");
        return -1;
    }
    
    int sourceReference = atoi(args);
    if (sourceReference <= 0) {
        printf("Error: Invalid sourceReference: %s\n", args);
        return -1;
    }
    
    // Create request arguments
    cJSON* request_args = cJSON_CreateObject();
    cJSON_AddNumberToObject(request_args, "sourceReference", sourceReference);
    
    // Send source request
    char* response = NULL;
    int error = dap_client_send_request(client, DAP_CMD_SOURCE, request_args, &response);
    cJSON_Delete(request_args);
    
    if (error != DAP_ERROR_NONE) {
        fprintf(stderr, "Error requesting source: %d\n", error);
        return -1;
    }
    
    // Parse response
    cJSON* response_json = cJSON_Parse(response);
    if (response_json) {
        cJSON* body = cJSON_GetObjectItem(response_json, "body");
        if (body) {
            cJSON* content = cJSON_GetObjectItem(body, "content");
            if (content && content->valuestring) {
                printf("\n--- Source Content ---\n");
                printf("%s\n", content->valuestring);
                printf("--- End Source ---\n\n");
            }
        }
        cJSON_Delete(response_json);
    }
    
    if (response) {
        free(response);
    }
    
    return 0;
}
```

**Add to `dap_debugger_commands.h`:**

```c
int handle_source_command(DAPClient* client, const char* args);
```

## Usage Examples

### Pure Assembly Debugging

```bash
# Launch with MAP file and assembly sources
dap_debugger -f program.out -m program.map -s main.s,lib.s -e

# Once in debugger:
(dap) break main.s:10      # Set breakpoint
(dap) continue             # Run
(dap) next line            # Step over by line
(dap) next instruction     # Step over by instruction
(dap) backtrace            # Show stack
(dap) disassemble          # Show disassembly
```

### Pure C Debugging

```bash
# Launch C program with sources
dap_debugger -f a.out -s "main.c,utils.c,app.h" -e

# Once in debugger:
(dap) break main.c:15      # Set breakpoint in C file
(dap) continue             # Run
(dap) next line            # Step over by C line
(dap) step line            # Step into C function
(dap) backtrace            # Show C call stack
(dap) variables            # Show variables
(dap) source 1001          # Show source if file not on disk
```

### Mixed C/Assembly Debugging

```bash
# Launch mixed program with both C and assembly sources
dap_debugger -f a.out -s main.c,util.c,delay.s -m program.map -e

# Once in debugger:
(dap) break main.c:10      # Breakpoint in C
(dap) break delay.s:42     # Breakpoint in assembly
(dap) continue             # Run
(dap) next line            # Step in current language
(dap) step                 # Step into (C→assembly or assembly→C)
(dap) step-out             # Step out to caller
(dap) backtrace            # Show mixed stack (C and assembly)
(dap) info sources         # List all source files
```

### Advanced Usage

```bash
# With granularity control
dap_debugger -f a.out -s main.c -g instruction -e

(dap) next 1 instruction   # Step one instruction
(dap) next 1 line          # Step one source line
(dap) next 1 statement     # Step one statement

# With working directory
dap_debugger -f a.out -s ../src/main.c -w /home/user/project

# With program arguments
dap_debugger -f test.out -s test.c -a "arg1,arg2,arg3"
```

## Help System Updates

### Update "next" Command Help

**File:** `external/libdap/src/dap_debugger/dap_debugger_help.c`

The help is auto-generated from the command table, but examples should be enhanced:

```c
.examples = 
    "next|Step over current line|"
    "next 1|Step over in thread 1|"
    "next 1 line|Step one source line|"
    "next 1 instruction|Step one instruction|"
    "next 1 statement|Step one statement"
```

### Add Source Command Help

In command table:

```c
{
    .name = "source",
    .alias = "src",
    .syntax = "source <sourceReference>",
    .description = "Display source file content by reference ID",
    .examples = 
        "source 1001|Show source for reference 1001|"
        "backtrace|First get stack trace to see sourceReferences|"
        "source 1002|Show embedded or missing source file",
    .category = CATEGORY_SOURCE,
    .implemented = true,
    .handler = handle_source_command
}
```

### Enhanced Granularity Help

Add new section to general help:

```
Stepping Granularity:
--------------------
The debugger supports different stepping levels:

  line         Step by source line (default for C/assembly with debug info)
  instruction  Step by CPU instruction (always works)
  statement    Step by statement (similar to line)

Use granularity in step commands:
  next 1 line         Step over next source line
  next 1 instruction  Step over next instruction
  step 1 line         Step into next source line
```

## Complete Example Session

### Mixed C/Assembly Project

**Directory structure:**
```
project/
├── main.c          # C main function
├── util.c          # C utility functions
├── delay.s         # Assembly delay routine
├── program.map     # MAP file from assembler
└── a.out           # Compiled executable
```

**Launch:**
```bash
$ dap_debugger -f a.out \
               -s "main.c,util.c,delay.s" \
               -m program.map \
               -e \
               -g line

Connecting to localhost:4711...
Connected to DAP server
Initializing debug session...
Debug session initialized
Launching program: a.out
Sources: main.c, util.c, delay.s
MAP file: program.map
Stopped at program entry

DAP Debugger Shell
Type 'help' for available commands

(dap) 
```

**Debug session:**
```
(dap) break main.c:15
Breakpoint 1 set at main.c:15 (address 0x0102)

(dap) break delay.s:42
Breakpoint 2 set at delay.s:42 (address 0x0234)

(dap) continue
Continuing execution...
Stopped at main.c:15 (breakpoint)

(dap) backtrace
Frame 0: main at main.c:15 (PC=0x0102)
Frame 1: _start at crt0.s:5 (PC=0x0000)

(dap) next line
Stepping over line...
Stopped at main.c:16 (step)

(dap) step line
Stepping into...
Stopped at delay.s:42 (step) [entering assembly function]

(dap) backtrace
Frame 0: delay at delay.s:42 (PC=0x0234) [assembly]
Frame 1: main at main.c:16 (PC=0x0104) [C]
Frame 2: _start at crt0.s:5 (PC=0x0000) [assembly]

(dap) step-out
Stepping out of function...
Stopped at main.c:17 (step)

(dap) info sources
Source files loaded:
  main.c (from STABS)
  util.c (from STABS)
  delay.s (from MAP)
  crt0.s (from MAP)

(dap) variables
Locals:
  (none - need STABS parser enhancement)

Registers:
  A = 000042
  D = 000100
  P = 000104
  B = 000200
  ...

(dap) disassemble 0x0104 10
0x0104: 060000  LDA 0,0
0x0105: 070000  STA 0,0
0x0106: 134000  JPL delay
...

(dap) quit
Disconnecting...
Goodbye!
```

## Command Reference Updates

### New Commands

| Command | Alias | Description | Example |
|---------|-------|-------------|---------|
| source | src | Show source by reference | `source 1001` |

### Enhanced Commands

| Command | New Options | Description |
|---------|-------------|-------------|
| next | `[granularity]` | Step over with granularity control |
| step | `[granularity]` | Step into with granularity control |
| step-out | `[granularity]` | Step out with granularity control |

### Granularity Values

| Value | Description | Use Case |
|-------|-------------|----------|
| `line` | Step by source line | Default for C/assembly with debug info |
| `instruction` | Step by CPU instruction | Low-level debugging, no source |
| `statement` | Step by statement | Similar to line |

## Implementation Checklist

### dap_debugger Client Updates

- [ ] Add `-s, --sources` command line option
- [ ] Add `-m, --map-file` command line option
- [ ] Add `-g, --granularity` command line option
- [ ] Add `print_usage()` function
- [ ] Update launch request to include `sources` array
- [ ] Update launch request to include `mapFile`
- [ ] Update `handle_next_command()` to support granularity
- [ ] Update `handle_step_command()` to support granularity
- [ ] Update `handle_step_out_command()` to support granularity
- [ ] Add `handle_source_command()` implementation
- [ ] Update command table with new "source" command
- [ ] Update command table with enhanced step commands
- [ ] Update help strings and examples
- [ ] Test all scenarios (assembly, C, mixed)

### Help Documentation Updates

- [ ] Update command syntax strings
- [ ] Update command examples
- [ ] Add granularity explanation
- [ ] Add mixed-language examples
- [ ] Update quick reference

### Testing

- [ ] Test with assembly-only program
- [ ] Test with C-only program
- [ ] Test with mixed C/assembly program
- [ ] Test granularity switching
- [ ] Test source command
- [ ] Test all step variations

## Mock Server Updates (Low Priority)

**File:** `external/libdap/src/dap_mock_server/dap_mock_server.c`

### Updates Needed

1. **Accept new launch parameters:**
   - `sources` array
   - `mapFile` string

2. **Update capabilities:**
   - Add LOG_POINTS
   - Add STEPPING_GRANULARITY
   - Add INSTRUCTION_BREAKPOINTS
   - Remove false claims

3. **Implement source command:**
   - Return mock source content
   - Handle sourceReference

**Priority:** LOW - Mock server is for testing only

## Summary

### Required Changes to dap_debugger Client

1. ✅ **Command line options** - Add `-s`, `-m`, `-g`
2. ✅ **Launch request** - Include `sources` and `mapFile`
3. ✅ **Step commands** - Support granularity parameter
4. ✅ **Source command** - NEW command handler
5. ✅ **Help system** - Update documentation
6. ✅ **Usage function** - Add comprehensive help

### Estimated Effort

- Command line options: 1 hour
- Launch request updates: 30 minutes
- Step command updates: 1 hour
- Source command: 1 hour
- Help/documentation: 1 hour
- Testing: 2 hours

**Total:** 6-7 hours

### Benefits

1. **Complete feature parity** with VS Code extension
2. **Command-line debugging** for C and assembly
3. **Scriptable debugging** for automated testing
4. **Alternative UI** for non-VS Code users

### Priority

**MEDIUM** - dap_debugger client is useful but:
- Most users will use VS Code extension
- Not blocking for core functionality
- Can be done after STABS parser

---

**Recommendation:** Document now (done ✅), implement after STABS parser and testing are complete.

