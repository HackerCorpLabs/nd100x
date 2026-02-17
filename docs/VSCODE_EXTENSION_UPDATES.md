# VS Code Extension Updates for Mixed C/Assembly Debugging

## Overview

This document describes the changes needed to extend the ND-100 Assembly VS Code extension to support mixed-language debugging (C and Assembly). The extension is located at `/mnt/e/Dev/Repos/Ronny/NDGen/output/vscode`.

## Current State (v0.0.3)

The extension currently supports:
- ✅ Syntax highlighting for `.asm` and `.s` files only
- ✅ Debug adapter type: "ND-100 Assembly"
- ✅ Breakpoints in assembly files only
- ✅ Auto-launch of nd100x emulator
- ✅ Single source file debugging

## Required Changes

### 1. Update package.json

Location: `/mnt/e/Dev/Repos/Ronny/NDGen/output/vscode/package.json`

#### 1.1 Add C Language Support

Add C language contribution so breakpoints work in C files:

```json
"languages": [
  {
    "id": "ndasm",
    "aliases": ["ND-100 Assembly", "ndasm"],
    "extensions": [".asm", ".s"],
    "configuration": "./language-configuration.json"
  },
  {
    "id": "c",
    "extensions": [".c", ".h"]
  }
]
```

#### 1.2 Add C Breakpoint Support

```json
"breakpoints": [
  {
    "language": "ndasm"
  },
  {
    "language": "c"
  }
]
```

#### 1.3 Change Debug Type from "ND-100 Assembly" to "ND-100"

**IMPORTANT**: This is a breaking change for existing configurations.

```json
"debuggers": [
  {
    "type": "ND-100",  // Changed from "ND-100 Assembly"
    "label": "ND-100 Debugger",
    "languages": ["ndasm", "c"],  // Support both
    // ... rest of configuration
  }
]
```

#### 1.4 Add `sources` Property

Replace `sourceFile` (single file) with `sources` (array):

```json
"configurationAttributes": {
  "launch": {
    "properties": {
      "program": {
        "type": "string",
        "description": "Path to the assembled/compiled .out file (a.out format)"
      },
      "mapFile": {
        "type": "string",
        "description": "Path to the .map file (optional, for assembly source mapping)"
      },
      "sources": {
        "type": "array",
        "description": "Array of source files (.c, .s, .h) for debugging",
        "items": {
          "type": "string"
        }
      },
      "sourceFile": {
        "type": "string",
        "description": "Path to the primary source file (deprecated, use sources instead)"
      },
      "cwd": {
        "type": "string",
        "description": "Working directory for the program",
        "default": "${workspaceFolder}"
      },
      "stopOnEntry": {
        "type": "boolean",
        "default": true,
        "description": "Automatically stop after launching"
      },
      "autoLaunch": {
        "type": "boolean",
        "default": true,
        "description": "Whether to auto-launch nd100x emulator"
      },
      "port": {
        "type": "number",
        "default": 4711,
        "description": "DAP server port"
      }
    },
    "required": ["program"]
  }
}
```

#### 1.5 Update Initial Configurations

```json
"initialConfigurations": [
  {
    "name": "Debug ND-100 Program",
    "type": "ND-100",
    "request": "launch",
    "program": "${workspaceFolder}/a.out",
    "sources": [
      "${workspaceFolder}/**/*.c",
      "${workspaceFolder}/**/*.s",
      "${workspaceFolder}/**/*.h"
    ],
    "stopOnEntry": true,
    "autoLaunch": true,
    "port": 4711
  }
]
```

#### 1.6 Add Configuration Snippets

Provide easy-to-use snippets for different project types:

```json
"configurationSnippets": [
  {
    "label": "ND-100: Debug Assembly",
    "description": "Debug pure assembly program",
    "body": {
      "name": "Debug ND-100 Assembly",
      "type": "ND-100",
      "request": "launch",
      "program": "^\"\\${fileDirname}/\\${fileBasenameNoExtension}.out\"",
      "sources": ["^\"\\${file}\""],
      "mapFile": "^\"\\${fileDirname}/\\${fileBasenameNoExtension}.map\"",
      "stopOnEntry": true
    }
  },
  {
    "label": "ND-100: Debug C Program",
    "description": "Debug C program with optional assembly",
    "body": {
      "name": "Debug ND-100 C Program",
      "type": "ND-100",
      "request": "launch",
      "program": "^\"\\${workspaceFolder}/a.out\"",
      "sources": [
        "^\"\\${workspaceFolder}/**/*.c\"",
        "^\"\\${workspaceFolder}/**/*.s\""
      ],
      "stopOnEntry": true
    }
  },
  {
    "label": "ND-100: Debug Mixed C/Assembly",
    "description": "Debug mixed C and assembly program",
    "body": {
      "name": "Debug ND-100 Mixed Program",
      "type": "ND-100",
      "request": "launch",
      "program": "^\"\\${workspaceFolder}/a.out\"",
      "sources": [
        "^\"\\${workspaceFolder}/main.c\"",
        "^\"\\${workspaceFolder}/util.s\"",
        "^\"\\${workspaceFolder}/**/*.h\""
      ],
      "stopOnEntry": true
    }
  }
]
```

#### 1.7 Update Activation Events

```json
"activationEvents": [
  "onLanguage:ndasm",
  "onLanguage:c",
  "workspaceContains:**/*.asm",
  "workspaceContains:**/*.s",
  "workspaceContains:**/*.c",
  "workspaceContains:**/*.out",
  "onDebugResolve:ND-100",
  "onDebugDynamicConfigurations:ND-100",
  "onDebugAdapterTracker:ND-100"
]
```

### 2. Update debugger.ts

Location: `/mnt/e/Dev/Repos/Ronny/NDGen/output/vscode/src/debugger.ts`

#### 2.1 Update File Type Validation in `resolveDebugConfiguration`

Change from assembly-only to multi-language support:

```typescript
resolveDebugConfiguration(
    folder: vscode.WorkspaceFolder | undefined,
    config: vscode.DebugConfiguration,
    token?: vscode.CancellationToken
): vscode.ProviderResult<vscode.DebugConfiguration> {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showInformationMessage('No file is open to debug.');
        return undefined;
    }
    
    // Support both C and assembly files
    const activeFileName = editor.document.fileName.toLowerCase();
    const isDebugableFile = 
        activeFileName.endsWith('.asm') || 
        activeFileName.endsWith('.s') ||
        activeFileName.endsWith('.c') ||
        activeFileName.endsWith('.h');
    
    if (!isDebugableFile) {
        vscode.window.showInformationMessage(
            'The ND-100 debugger can only be used with .c, .h, .asm, or .s files.'
        );
        return undefined;
    }
    
    // If launch.json is missing or empty
    if (!config.type && !config.request && !config.name) {
        config.type = "ND-100";
        config.name = "Debug ND-100 Program";
        config.request = "launch";
        
        // Smart defaults based on file type
        if (activeFileName.endsWith('.c') || activeFileName.endsWith('.h')) {
            // C file - assume compiled project
            config.program = "${workspaceFolder}/a.out";
            config.sources = [
                "${workspaceFolder}/**/*.c",
                "${workspaceFolder}/**/*.s",
                "${workspaceFolder}/**/*.h"
            ];
        } else {
            // Assembly file - assume single file or small project
            config.program = "${fileDirname}/${fileBasenameNoExtension}.out";
            config.sources = ["${file}"];
            config.mapFile = "${fileDirname}/${fileBasenameNoExtension}.map";
        }
        config.stopOnEntry = true;
    }

    if (!config.program) {
        vscode.window.showErrorMessage("No program specified for ND-100 debug session.");
        return undefined;
    }
    
    console.log("FINAL DEBUG CONFIG:", JSON.stringify(config, null, 2));
    return config;
}
```

#### 2.2 Update `createDebugAdapterDescriptor` for Multi-Source Support

```typescript
createDebugAdapterDescriptor(
    session: vscode.DebugSession,
    executable: vscode.DebugAdapterExecutable | undefined
): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
    
    console.log(`ND-100 Debug Adapter: Session configuration:`);
    console.log(JSON.stringify(session.configuration, null, 2));

    // Support both C and assembly files
    if (session.configuration.sources) {
        const sources = Array.isArray(session.configuration.sources) 
            ? session.configuration.sources 
            : [session.configuration.sources];
        
        console.log(`Debug sources: ${sources.join(', ')}`);
        
        // Validate sources contain debuggable file types
        const hasDebugableFiles = sources.some(src => {
            if (typeof src === 'string') {
                const lower = src.toLowerCase();
                return lower.endsWith('.c') || lower.endsWith('.s') || 
                       lower.endsWith('.asm') || lower.endsWith('.h') ||
                       lower.includes('*');  // Allow glob patterns
            }
            return false;
        });
        
        if (!hasDebugableFiles) {
            vscode.window.showErrorMessage(
                "ND-100 Debugger: Sources must include .c, .h, .s, or .asm files"
            );
            return undefined;
        }
    }
    
    // Legacy support for sourceFile (deprecated)
    if (session.configuration.sourceFile && !session.configuration.sources) {
        const sourceFile = session.configuration.sourceFile.toLowerCase();
        const isDebugableFile = sourceFile.endsWith('.asm') || 
                               sourceFile.endsWith('.s') ||
                               sourceFile.endsWith('.c') ||
                               sourceFile.endsWith('.h');
        
        if (!isDebugableFile && !sourceFile.includes('$')) {
            vscode.window.showErrorMessage(
                "ND-100 Debugger: Can only debug .c, .h, .asm, or .s files"
            );
            return undefined;
        }
    }

    // ... rest of implementation
}
```

#### 2.3 Update Registration to Use "ND-100" Type

```typescript
export function registerDebugFeatures(context: vscode.ExtensionContext) {
    const configProvider = new NdasmDebugConfigurationProvider();
    
    // Register for ND-100 debug type (handles both C and assembly)
    context.subscriptions.push(
        vscode.debug.registerDebugConfigurationProvider(
            'ND-100',  // Changed from 'ND-100 Assembly'
            configProvider,
            vscode.DebugConfigurationProviderTriggerKind.Dynamic
        )
    );
    
    // Register debug adapter tracker
    context.subscriptions.push(
        vscode.debug.registerDebugAdapterTrackerFactory('ND-100', {
            createDebugAdapterTracker(session) {
                return {
                    onWillReceiveMessage: m => {
                        console.log(`→ TO ADAPTER: ${JSON.stringify(m)}`);
                    },
                    onDidSendMessage: m => {
                        console.log(`← FROM ADAPTER: ${JSON.stringify(m)}`);
                    }
                };
            }
        })
    );
    
    // Register debug adapter factory
    const factory = new NdasmDebugAdapterDescriptorFactory();
    context.subscriptions.push(
        vscode.debug.registerDebugAdapterDescriptorFactory('ND-100', factory)
    );
    
    // ... rest of event listeners
}
```

## User-Facing Configuration Examples

### Example 1: Pure Assembly Project

**`.vscode/launch.json`:**
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Assembly",
            "type": "ND-100",
            "request": "launch",
            "program": "${workspaceFolder}/program.out",
            "sources": [
                "${workspaceFolder}/main.s",
                "${workspaceFolder}/lib.s"
            ],
            "mapFile": "${workspaceFolder}/program.map",
            "stopOnEntry": true,
            "port": 4711
        }
    ]
}
```

**`.vscode/tasks.json`:**
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Assemble ND-100",
            "type": "shell",
            "command": "ndasm",
            "args": [
                "-o", "program.out",
                "-m", "program.map",
                "main.s", "lib.s"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```

### Example 2: Mixed C/Assembly Project

**`.vscode/launch.json`:**
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Mixed C/Assembly",
            "type": "ND-100",
            "request": "launch",
            "program": "${workspaceFolder}/a.out",
            "sources": [
                "${workspaceFolder}/main.c",
                "${workspaceFolder}/utils.c",
                "${workspaceFolder}/delay.s",
                "${workspaceFolder}/**/*.h"
            ],
            "stopOnEntry": true,
            "port": 4711,
            "preLaunchTask": "Build Mixed Project"
        }
    ]
}
```

**`.vscode/tasks.json`:**
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Mixed Project",
            "type": "shell",
            "command": "make",
            "args": ["all"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
```

### Example 3: Pure C Project

**`.vscode/launch.json`:**
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug C Program",
            "type": "ND-100",
            "request": "launch",
            "program": "${workspaceFolder}/a.out",
            "sources": [
                "${workspaceFolder}/**/*.c",
                "${workspaceFolder}/**/*.h"
            ],
            "cwd": "${workspaceFolder}",
            "stopOnEntry": true,
            "port": 4711,
            "preLaunchTask": "Compile C Program"
        }
    ]
}
```

## DAP Adapter Requirements

The nd100x emulator (DAP adapter) must support:

1. **Accept `sources` array** in launch request
   - Parse array of source file paths
   - Support glob patterns (e.g., `**/*.c`)
   - Handle both absolute and relative paths

2. **Load multiple symbol tables**
   - STABS from a.out files (for C programs)
   - MAP files (for assembly programs)
   - Combine and search across all tables

3. **Resolve breakpoints across all source types**
   - Map C source lines to addresses
   - Map assembly source lines to addresses
   - Handle breakpoints in header files

4. **Provide source content via `source` request**
   - Return file content when IDE requests by `sourceReference`
   - Handle missing files gracefully

5. **Return correct stack frames**
   - Include proper `source.path` or `source.sourceReference`
   - Show mixed C/assembly stack traces
   - Map addresses to correct source files

## Migration Path for Existing Users

### Breaking Change: Debug Type Renamed

**Old configuration (will break):**
```json
{
    "type": "ND-100 Assembly",
    "request": "launch",
    "sourceFile": "${file}"
}
```

**New configuration:**
```json
{
    "type": "ND-100",
    "request": "launch",
    "sources": ["${file}"]
}
```

### Backward Compatibility

The extension should support both:
- **New**: `sources` array (preferred)
- **Legacy**: `sourceFile` single string (deprecated but working)

Implementation in `resolveDebugConfiguration`:

```typescript
// Auto-convert legacy sourceFile to sources
if (config.sourceFile && !config.sources) {
    config.sources = [config.sourceFile];
    console.log('Converted legacy sourceFile to sources array');
}
```

## Testing Checklist

- [ ] Pure assembly debugging works
- [ ] Pure C debugging works
- [ ] Mixed C/assembly debugging works
- [ ] Glob patterns in sources work (e.g., `**/*.c`)
- [ ] Breakpoints work in .c files
- [ ] Breakpoints work in .s/.asm files
- [ ] Breakpoints work in .h files
- [ ] Stack traces show correct source files
- [ ] Stepping across C/assembly boundary works
- [ ] Legacy `sourceFile` configurations still work
- [ ] Auto-launch of nd100x emulator works
- [ ] Configuration snippets appear in UI

## Version Update

Update `package.json`:

```json
{
  "name": "nd100-assembly",
  "displayName": "ND-100 Assembly & C Debugger",
  "description": "Comprehensive language support for ND-100 Assembly and C with mixed-language debugging",
  "version": "0.0.4",
  // ...
}
```

Update README.md to document:
- Mixed-language debugging support
- New `sources` configuration property
- Migration guide from v0.0.3
- Examples for different project types

