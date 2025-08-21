# SINTRAN III Commands Reference

This document contains a reference of all available SINTRAN III commands.

## Index

### Commands

- [ABORT](#abort)
- [ABORT-BATCH](#abort-batch)
- [ABORT-JOB](#abort-job)
- [ABORT-PRINT](#abort-print)
- [ABSET](#abset)
- [ALLOCATE-FILE](#allocate-file)
- [ALLOCATE-NEW-VERSION](#allocate-new-version)
- [ALTOFF](#altoff)
- [ALTON](#alton)
- [APPEND-BATCH](#append-batch)
- [APPEND-REMOTE](#append-remote)
- [APPEND-SPOOLING-FILE](#append-spooling-file)
- [BACKSPACE-PRINT](#backspace-print)
- [BATCH](#batch)
- [CC](#cc)
- [CHANGE-BACKGROUND-SEGMENT-SIZE](#change-background-segment-size)
- [CHANGE-BIT-FILE](#change-bit-file)
- [CHANGE-DIRECTORY-ENTRY](#change-directory-entry)
- [CHANGE-OBJECT-ENTRY](#change-object-entry)
- [CHANGE-PAGE](#change-page)
- [CHANGE-PASSWORD](#change-password)
- [CHANGE-USER-ENTRY](#change-user-entry)
- [CLADJ](#cladj)
- [CLEAR-BATCH-QUEUE](#clear-batch-queue)
- [CLEAR-DEFAULT-DIRECTORY](#clear-default-directory)
- [CLEAR-DEVICE](#clear-device)
- [CLEAR-MAIN-DIRECTORY](#clear-main-directory)
- [CLEAR-PASSWORD](#clear-password)
- [CLEAR-REENTRANT-SEGMENT](#clear-reentrant-segment)
- [CLOSE-FILE](#close-file)
- [COLD-START](#cold-start)
- [CONCT](#conct)
- [CONNECT-FILE](#connect-file)
- [COPY](#copy)
- [COPY-DEVICE](#copy-device)
- [COPY-DIRECTORY](#copy-directory)
- [COPY-FILE](#copy-file)
- [CREATE-DIRECTORY](#create-directory)
- [CREATE-FILE](#create-file)
- [CREATE-FRIEND](#create-friend)
- [CREATE-NEW-VERSION](#create-new-version)
- [CREATE-USER](#create-user)
- [DATCL](#datcl)
- [DEFAULT-SUBSYSTEM-DISABLE](#default-subsystem-disable)
- [DEFAULT-SUBSYSTEM-ENABLE](#default-subsystem-enable)
- [DEFINE-DEFAULT-SUBSYSTEM](#define-default-subsystem)
- [DEFINE-ESCAPE-CHARACTER](#define-escape-character)
- [DEFINE-HISTOGRAM](#define-histogram)
- [DEFINE-LOCAL-CHARACTER](#define-local-character)
- [DEFINE-MASS-STORAGE-UNIT](#define-mass-storage-unit)
- [DEFINE-REENTRANT-PROGRAM](#define-reentrant-program)
- [DEFINE-SPOOLING-CONDITIONS](#define-spooling-conditions)
- [DEFINE-SPOOLING-FILE-MESSAGE](#define-spooling-file-message)
- [DEFINE-SYSTEM-HISTOGRAM](#define-system-histogram)
- [DELETE-BATCH-QUEUE-ENTRY](#delete-batch-queue-entry)
- [DELETE-FILE](#delete-file)
- [DELETE-FRIEND](#delete-friend)
- [DELETE-MASS-STORAGE-UNIT](#delete-mass-storage-unit)
- [DELETE-REENTRANT](#delete-reentrant)
- [DELETE-REMOTE-QUEUE-ENTRY](#delete-remote-queue-entry)
- [DELETE-SPOOLING-FILE](#delete-spooling-file)
- [DELETE-USER](#delete-user)
- [DELETE-USERS-FILES](#delete-users-files)
- [DEVICE-FUNCTION](#device-function)
- [DIRECTORY-STATISTICS](#directory-statistics)
- [DISABLE-ESCAPE-FUNCTION](#disable-escape-function)
- [DISABLE-TERMINATION-HANDLING](#disable-termination-handling)
- [DMAC](#dmac)
- [DSCNT](#dscnt)
- [DUMP](#dump)
- [DUMP-BIT-FILE](#dump-bit-file)
- [DUMP-DIRECTORY-ENTRY](#dump-directory-entry)
- [DUMP-OBJECT-ENTRY](#dump-object-entry)
- [DUMP-PAGE](#dump-page)
- [DUMP-PROGRAM-REENTRANT](#dump-program-reentrant)
- [DUMP-REENTRANT](#dump-reentrant)
- [DUMP-USER-ENTRY](#dump-user-entry)
- [ENABLE-ESCAPE-FUNCTION](#enable-escape-function)
- [ENABLE-TERMINATION-HANDLING](#enable-termination-handling)
- [ENTER](#enter)
- [ENTER-DIRECTORY](#enter-directory)
- [ENTSG](#entsg)
- [EXECUTE-IOX](#execute-iox)
- [EXPAND-FILE](#expand-file)
- [FILE-STATISTICS](#file-statistics)
- [FIX](#fix)
- [FIXC](#fixc)
- [FORWARD-SPACE-PRINT](#forward-space-print)
- [GET-ERROR-DEVICE](#get-error-device)
- [GET-RT-NAME](#get-rt-name)
- [GET-TERMINAL-TYPE](#get-terminal-type)
- [GIVE-OBJECT-BLOCKS](#give-object-blocks)
- [GIVE-SPOOLING-PAGES](#give-spooling-pages)
- [GIVE-USER-SPACE](#give-user-space)
- [GOTO-USER](#goto-user)
- [HELP](#help)
- [HOLD](#hold)
- [INIT-ACCOUNTING](#init-accounting)
- [INITIAL-COMMAND](#initial-command)
- [INITIALIZE-BACKGROUND-PROGRAMS](#initialize-background-programs)
- [INITIALIZE-ERROR-LOG](#initialize-error-log)
- [INTV](#intv)
- [IOSET](#ioset)
- [LIST-BATCH-PROCESS](#list-batch-process)
- [LIST-BATCH-QUEUE](#list-batch-queue)
- [LIST-DEFAULT-SUBSYSTEM](#list-default-subsystem)
- [LIST-DEVICE](#list-device)
- [LIST-DEVICE-FUNCTION](#list-device-function)
- [LIST-DIRECTORIES-ENTERED](#list-directories-entered)
- [LIST-EXECUTION-QUEUE](#list-execution-queue)
- [LIST-FILES](#list-files)
- [LIST-FRIENDS](#list-friends)
- [LIST-INITIAL-COMMANDS](#list-initial-commands)
- [LIST-MASS-STORAGE-UNIT](#list-mass-storage-unit)
- [LIST-OPEN-FILES](#list-open-files)
- [LIST-REENTRANT](#list-reentrant)
- [LIST-REMOTE-QUEUE](#list-remote-queue)
- [LIST-RT-ACCOUNT](#list-rt-account)
- [LIST-RT-DESCRIPTION](#list-rt-description)
- [LIST-RTOPEN-FILES](#list-rtopen-files)
- [RTENTER](#rtenter)
- [RTOFF](#rtoff)
- [RTON](#rton)
- [RTOPEN-FILE](#rtopen-file)
- [RTRELEASE-OPEN-FILE-ENTRIES](#rtrelease-open-file-entries)
- [RTRESERVE-OPEN-FILE-ENTRIES](#rtreserve-open-file-entries)
- [SAVE-DIRECTORY](#save-directory)
- [SCHEDULE](#schedule)
- [SCRATCH-OPEN](#scratch-open)
- [SET](#set)
- [SET-AVAILABLE](#set-available)
- [SET-BLOCK-POINTER](#set-block-pointer)
- [SET-BLOCK-SIZE](#set-block-size)
- [SET-BYTE-POINTER](#set-byte-pointer)
- [SET-DEFAULT-DIRECTORY](#set-default-directory)
- [SET-DEFAULT-FILE-ACCESS](#set-default-file-access)
- [SET-DEFAULT-REMOTE-SYSTEM](#set-default-remote-system)
- [SET-ERROR-DEVICE](#set-error-device)
- [SET-FILE-ACCESS](#set-file-access)
- [SET-FRIEND-ACCESS](#set-friend-access)
- [SET-INITIAL-FILE-ACCESS](#set-initial-file-access)
- [SET-INITIAL-FRIEND-ACCESS](#set-initial-friend-access)
- [SET-LOCAL-MODE](#set-local-mode)
- [SET-MAIN-DIRECTORY](#set-main-directory)
- [SET-MEMORY-CONTENTS](#set-memory-contents)
- [SET-NUMBER-OF-PRINT-COPIES](#set-number-of-print-copies)
- [SET-PERIPHERAL-FILE](#set-peripheral-file)
- [SET-PERMANENT-OPEN](#set-permanent-open)
- [SET-REMOTE-MODE](#set-remote-mode)
- [SET-SPOOLING-FORM](#set-spooling-form)
- [SET-TEMPORARY-FILE](#set-temporary-file)
- [SET-TERMINAL-FILE](#set-terminal-file)
- [SET-TERMINAL-TYPE](#set-terminal-type)
- [SET-UNAVAILABLE](#set-unavailable)
- [SET-USER-PARAMETERS](#set-user-parameters)
- [SINTRAN-SERVICE-PROGRAM](#sintran-service-program)
- [SPOOLING-PAGES-LEFT](#spooling-pages-left)
- [START](#start)
- [START-ACCOUNTING](#start-accounting)
- [START-HISTOGRAM](#start-histogram)
- [START-PRINT](#start-print)
- [START-PROGRAM-LOG](#start-program-log)
- [START-RT-ACCOUNT](#start-rt-account)
- [START-SPOOLING](#start-spooling)
- [START-TADADM](#start-tadadm)
- [STATUS](#status)
- [STOP](#stop)
- [STOP-ACCOUNTING](#stop-accounting)
- [STOP-HISTOGRAM](#stop-histogram)
- [STOP-PRINT](#stop-print)
- [STOP-PROGRAM-LOG](#stop-program-log)
- [STOP-RT-ACCOUNT](#stop-rt-account)
- [STOP-SPOOLING](#stop-spooling)
- [STOP-SYSTEM](#stop-system)
- [STOP-TADADM](#stop-tadadm)
- [SUSPEND](#suspend)
- [TERMINATE](#terminate)
- [TEST-COMMAND](#test-command)
- [TIME](#time)
- [TYPE](#type)
- [UNLOAD](#unload)
- [UNLOCK](#unlock)
- [UNMOUNT](#unmount)
- [UNRESERVE](#unreserve)

### Sintran-Service-Program

- [ASCII-DUMP](#ascii-dump)
- [BACKGROUND-ALLOCATION-UTILITIES](#background-allocation-utilities)
- [CC](#cc)
- [CHANGE-BUFFER-SIZE](#change-buffer-size)
- [CHANGE-DATAFIELD](#change-datafield)
- [CHANGE-GPIB-BUFFERSIZE](#change-gpib-buffersize)
- [CHANGE-TABLE](#change-table)
- [CHANGE-VARIABLE](#change-variable)
- [CLEAR-ENTER-COUNT](#clear-enter-count)
- [CPU-LOG](#cpu-log)
- [CREATE-LAMU](#create-lamu)
- [CREATE-SYSTEM-LAMU](#create-system-lamu)
- [DEFINE-BATCH-SUPERVISOR](#define-batch-supervisor)
- [DEFINE-HDLC-BUFFER](#define-hdlc-buffer)
- [DEFINE-PROMPT-STRING](#define-prompt-string)
- [DEFINE-RTCOMMON-SIZE](#define-rtcommon-size)
- [DEFINE-SEGMENT-FILE](#define-segment-file)
- [DEFINE-TIMESLICE](#define-timeslice)
- [DEFINE-TITLE](#define-title)
- [DEFINE-USER-MONITOR-CALL](#define-user-monitor-call)
- [DEFINE-USER-RESTART-PROGRAM](#define-user-restart-program)
- [DEFINE-USER-RESTART-SUBROUTINE](#define-user-restart-subroutine)
- [DEFINE-USER-START-SUBROUTINE](#define-user-start-subroutine)
- [DELETE-LAMU](#delete-lamu)
- [DELETE-SEGMENT-FILE](#delete-segment-file)
- [DISC-ACCESS-LOG](#disc-access-log)
- [DUMP-RT-DESCRIPTION](#dump-rt-description)
- [DUMP-SEGMENT-TABLE-ENTRY](#dump-segment-table-entry)
- [EXIT](#exit)
- [FIND-CPULOOPTIME](#find-cpulooptime)
- [HELP](#help)
- [INITIALIZE-SYSTEM-SEGMENT](#initialize-system-segment)
- [INSERT-IN-BACKGROUND-TABLE](#insert-in-background-table)
- [INSERT-IN-EXTENDED-IDENT-TABLE](#insert-in-extended-ident-table)
- [INSERT-IN-IDENT-TABLE](#insert-in-ident-table)
- [INSERT-IN-IOX-TABLE](#insert-in-iox-table)
- [INSERT-IN-LOGICAL-UNIT-TABLE](#insert-in-logical-unit-table)
- [INSERT-IN-TIME-SLICE](#insert-in-time-slice)
- [INSERT-IN-TIMER-TABLE](#insert-in-timer-table)
- [INSERT-PROGRAM-IN-TIME-SLICE](#insert-program-in-time-slice)
- [INSERT-SPOOLING-HEADER](#insert-spooling-header)
- [LAMU-AREAS](#lamu-areas)
- [LAMU-INFORMATION](#lamu-information)
- [LIST-HDLC-BUFFER](#list-hdlc-buffer)
- [LIST-LAMU-CONSTANTS](#list-lamu-constants)
- [LIST-SERVICE-COMMANDS](#list-service-commands)
- [LIST-TIME-SLICE-CLASS](#list-time-slice-class)
- [LIST-TIME-SLICE-PARAMETERS](#list-time-slice-parameters)
- [LIST-TIME-SLICED-PROGRAMS](#list-time-sliced-programs)
- [LIST-USER-RESTART-PROGRAMS](#list-user-restart-programs)
- [MONCALL-LOG](#moncall-log)
- [NEXT-USER-RESTART-PROGRAM](#next-user-restart-program)
- [OCTAL-DUMP](#octal-dump)
- [PAGES-FROM-LAMU](#pages-from-lamu)
- [PAGES-TO-LAMU](#pages-to-lamu)
- [PROTECT-LAMU](#protect-lamu)
- [READ-BINARY](#read-binary)
- [REINSERT-SINTRAN-COMMAND](#reinsert-sintran-command)
- [REMOVE-FROM-BACKGROUND-TABLE](#remove-from-background-table)
- [REMOVE-FROM-EXTENDED-IDENT-TABLE](#remove-from-extended-ident-table)
- [REMOVE-FROM-IDENT-TABLE](#remove-from-ident-table)
- [REMOVE-FROM-IOX-TABLE](#remove-from-iox-table)
- [REMOVE-FROM-LOGICAL-UNIT-TABLE](#remove-from-logical-unit-table)
- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice)
- [REMOVE-FROM-TIMER-TABLE](#remove-from-timer-table)
- [REMOVE-PROGRAM-FROM-TIME-SLICE](#remove-program-from-time-slice)
- [REMOVE-SINTRAN-COMMAND](#remove-sintran-command)
- [REMOVE-SPOOLING-HEADER](#remove-spooling-header)
- [RESET-COLDSTART-MODE-FILE](#reset-coldstart-mode-file)
- [SEGMENT-WRITE-PERMIT](#segment-write-permit)
- [SEGMENT-WRITE-PROTECT](#segment-write-protect)
- [SET-CLOSED-SCRATCH-FILE-SIZE](#set-closed-scratch-file-size)
- [SET-COLDSTART-MODE-FILE](#set-coldstart-mode-file)
- [SET-COMMAND-PROTECTION](#set-command-protection)
- [SET-LAMU-CONSTANTS](#set-lamu-constants)
- [SET-MAX-ENTER-COUNT](#set-max-enter-count)
- [SET-SPOOLING-DEVICE-NUMBER](#set-spooling-device-number)
- [START-GPIB](#start-gpib)
- [START-XMSG](#start-xmsg)
- [STOP-GPIB](#stop-gpib)
- [STOP-XMSG](#stop-xmsg)
- [SWAP-DIRECTORY-ENTRIES](#swap-directory-entries)
- [SWAPPING-LOG](#swapping-log)
- [TEST-COMMAND](#test-command)

### ND-500 Monitor commands

- [ABORT-BATCH-ON-ERROR](#abort-batch-on-error)
- [ABORT-PROCESS](#abort-process)
- [ATTACH-PROCESS](#attach-process)
- [AUTOMATIC-ERROR-MESSAGE](#automatic-error-message)
- [BRANCH-TRACE](#branch-trace)
- [BREAK](#break)
- [CACHE-MONITOR](#cache-monitor)
- [CALL-TRACE](#call-trace)
- [CC](#cc)
- [CHANGE-CPU](#change-cpu)
- [CLOSE-FILE](#close-file)
- [COMPARE-CONTROL-STORE](#compare-control-store)
- [CONTINUE](#continue)
- [DEBUG-PLACE](#debug-place)
- [DEBUG-STATUS](#debug-status)
- [DEBUG-SWAPPER](#debug-swapper)
- [DEBUGGER](#debugger)
- [DEFINE-MACRO](#define-macro)
- [DEFINE-MEMORY-CONFIGURATION](#define-memory-configuration)
- [DEFINE-STANDARD-DOMAIN](#define-standard-domain)
- [DEFINE-SWAP-FILE](#define-swap-file)
- [DELETE-STANDARD-DOMAIN](#delete-standard-domain)
- [DUMP-MACRO](#dump-macro)
- [DUMP-PHYSICAL-SEGMENT](#dump-physical-segment)
- [DUMP-SWAPPER](#dump-swapper)
- [ENABLED-TRAPS](#enabled-traps)
- [ERASE-MACRO](#erase-macro)
- [EXECUTE-MACRO](#execute-macro)
- [EXHIBIT-ADDRESS](#exhibit-address)
- [EXIT](#exit)
- [EXTRA-FORMAT](#extra-format)
- [FIX-SEGMENT-ABSOLUTE](#fix-segment-absolute)
- [FIX-SEGMENT-CONTIGOUS](#fix-segment-contigous)
- [FIX-SEGMENT-SCATTERED](#fix-segment-scattered)
- [GET-FLAG](#get-flag)
- [GIVE-N500-PAGES](#give-n500-pages)
- [GO](#go)
- [GUARD](#guard)
- [HELP](#help)
- [INSERT-IN-TIME-SLICE](#insert-in-time-slice)
- [INSPECT-DUMP](#inspect-dump)
- [LIST-ACTIVE-PROCESSES](#list-active-processes)
- [LIST-ACTIVE-SEGMENTS](#list-active-segments)
- [LIST-DOMAIN](#list-domain)
- [LIST-EXECUTION-QUEUE](#list-execution-queue)
- [LIST-MACRO](#list-macro)
- [LIST-OPEN-FILES](#list-open-files)
- [LIST-PROCESS-TABLE-ENTRY](#list-process-table-entry)
- [LIST-SEGMENT-TABLE-ENTRY](#list-segment-table-entry)
- [LIST-STANDARD-DOMAINS](#list-standard-domains)
- [LIST-SWAP-FILE-INFO](#list-swap-file-info)
- [LIST-SYSTEM-PARAMETERS](#list-system-parameters)
- [LIST-TABLE](#list-table)
- [LIST-TIME-QUEUE](#list-time-queue)
- [LOAD-CONTROL-STORE](#load-control-store)
- [LOAD-SWAPPER](#load-swapper)
- [LOCAL-TRAP-DISABLE](#local-trap-disable)
- [LOCAL-TRAP-ENABLE](#local-trap-enable)
- [LOGOUT-PROCESS](#logout-process)
- [LOOK-AT](#look-at)
- [LOOK-AT-CONTROL-STORE](#look-at-control-store)
- [LOOK-AT-DATA](#look-at-data)
- [LOOK-AT-FILE](#look-at-file)
- [LOOK-AT-HARDWARE](#look-at-hardware)
- [LOOK-AT-PHYSICAL-SEGMENT](#look-at-physical-segment)
- [LOOK-AT-PROGRAM](#look-at-program)
- [LOOK-AT-REGISTER](#look-at-register)
- [LOOK-AT-RELATIVE](#look-at-relative)
- [LOOK-AT-RESIDENT-MEMORY](#look-at-resident-memory)
- [LOOK-AT-STACK](#look-at-stack)
- [MAIN-FORMAT](#main-format)
- [MASTER-CLEAR](#master-clear)
- [MEMORY-CONFIGURATION](#memory-configuration)
- [MICRO-START](#micro-start)
- [MICRO-STOP](#micro-stop)
- [OPEN-FILE](#open-file)
- [OUTPUT-FILE](#output-file)
- [PLACE-DOMAIN](#place-domain)
- [PRINT-HISTOGRAM](#print-histogram)
- [PRINT-MONCALL-LOG](#print-moncall-log)
- [PRINT-PROCESS-LOG](#print-process-log)
- [PROCESS-LOG-ALL](#process-log-all)
- [PROCESS-LOG-ONE](#process-log-one)
- [PROCESS-STATUS](#process-status)
- [RECOVER-DOMAIN](#recover-domain)
- [RELEASE-HISTOGRAM](#release-histogram)
- [RELEASE-LOG-BUFFER](#release-log-buffer)
- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice)
- [RESET-AUTOMATIC-ERROR-MESSAGE](#reset-automatic-error-message)
- [RESET-BRANCH-TRACE](#reset-branch-trace)
- [RESET-BREAKS](#reset-breaks)
- [RESET-CALL-TRACE](#reset-call-trace)
- [RESET-DEBUG](#reset-debug)
- [RESET-GUARD](#reset-guard)
- [RESET-INSPECT-DUMP](#reset-inspect-dump)
- [RESET-LAST-BREAK](#reset-last-break)
- [RESET-TRACE](#reset-trace)
- [RESIDENT-PLACE](#resident-place)
- [RESTART-PROCESS](#restart-process)
- [RESUME-MACRO](#resume-macro)
- [RUN](#run)
- [SET-BLOCK-SIZE](#set-block-size)
- [SET-FLAG](#set-flag)
- [SET-HISTOGRAM](#set-histogram)
- [SET-MEMORY-CONTENTS](#set-memory-contents)
- [SET-ND-500-AVAILABLE](#set-nd-500-available)
- [SET-ND-500-UNAVAILABLE](#set-nd-500-unavailable)
- [SET-PHYSICAL-SEGMENT-ADDRESS](#set-physical-segment-address)
- [SET-PRIORITY](#set-priority)
- [SET-PROCESS-NAME](#set-process-name)
- [SET-SEGMENT-LIMITS](#set-segment-limits)
- [SET-SYSTEM-PARAMETERS](#set-system-parameters)
- [SPECIAL-DEBUGGER](#special-debugger)
- [START-HISTOGRAM](#start-histogram)
- [START-MONCALL-LOG](#start-moncall-log)
- [START-PROCESS-LOG-ALL](#start-process-log-all)
- [START-PROCESS-LOG-ONE](#start-process-log-one)
- [START-SWAPPER](#start-swapper)
- [STEP](#step)
- [STOP-HISTOGRAM](#stop-histogram)
- [STOP-MONCALL-LOG](#stop-moncall-log)
- [STOP-ND-500](#stop-nd-500)
- [SWAPPING-LOG](#swapping-log)
- [SYSTEM-TRAP-DISABLE](#system-trap-disable)
- [SYSTEM-TRAP-ENABLE](#system-trap-enable)
- [TAKE-N500-PAGES](#take-n500-pages)
- [TEMPORARY-BREAK](#temporary-break)
- [TIME-USED](#time-used)
- [TRACE](#trace)
- [UNFIX-SEGMENT](#unfix-segment)
- [VALUE-ENTRIES](#value-entries)
- [VERSION](#version)
- [WHO-IS-ON](#who-is-on)

### SINTRAN-Appendix

- [APPENDIX A](#appendix-a)
- [APPENDIX B](#appendix-b)
- [APPENDIX C](#appendix-c)
- [APPENDIX D](#appendix-d)
- [APPENDIX E](#appendix-e)

---

## Commands

### ABORT

**Description:** Stops an RT-program by setting it in the passive state. The program is removed from the time and execution queues, all resources are released, and periodic execution is discontinued.

**Format:** `@ABORT <program>`

**Parameters:**

- `program`: RT-program name or RT-description address.

**Rules:**

- Permitted only for users RT and SYSTEM.
- The command has effect only if the program is in the running or wait state.
- Do not abort a background program. If a background program is hanging, the system may have to be restarted. Contact your system supervisor.

**Related Commands:**

- [ABORT-BATCH](#abort-batch) - Aborts a batch process
- [ABORT-JOB](#abort-job) - Aborts the current batch job
- [ABORT-PRINT](#abort-print) - Aborts the current print-out on a spooling device
- [ABORT-PROCESS](#abort-process) - Aborts a process which cannot be stopped in any other way
- [LOGOUT-PROCESS](#logout-process) - Logs out a process from the system

**Monitor Calls:**

- ABORT (MON 105) (105): Monitor call to abort an RT-program
- RTEXT (MON 134) (134): Monitor call to terminate an RT-program

**Example:**

```
@ABORT KLOKK

The RT-program KLOKK is aborted.

```

---

### ABORT-BATCH

**Description:** Stop the batch processor by setting it in the passive state. Any currently running batch job is aborted and the batch queue cleared.

**Format:** `@ABORT-BATCH <batch no.>`

**Parameters:**

- `batch no.`: Batch processor number as returned from @BATCH (decimal value).

**Rules:**

- Permitted only for users RT and SYSTEM.
- Effective only if the batch processor is idle or active.

**Related Commands:**

- [ABORT](#abort) - Related to ABORT
- [ABORT-JOB](#abort-job) - Related to ABORT-JOB
- [BATCH](#batch) - Related to BATCH
- [CLEAR-BATCH-QUEUE](#clear-batch-queue) - Related to CLEAR-BATCH-QUEUE

**Example:**

```
@ABORT-BATCH 1

The batch processor no. 1 is aborted.

```

---

### ABORT-JOB

**Description:** Abort the current batch job being processed. The next batch job in the batch queue (if any) will be initiated.

**Format:** `@ABORT-JOB <batch no.>,<user name>`

**Parameters:**

- `batch no.`: Batch processor number as returned from @BATCH.
- `user name`: Owner of the batch job as specified by @ENTER.

**Rules:**

- Users may abort their own job and user SYSTEM may abort a batch job belonging to any user.
- Only effective if the specified user name is logged in on the batch processor.

**Related Commands:**

- [ABORT-BATCH](#abort-batch) - Related to ABORT-BATCH
- [DELETE-BATCH-QUEUE-ENTRY](#delete-batch-queue-entry) - Related to DELETE-BATCH-QUEUE-ENTRY

**Example:**

```
@ABORT-JOB 2,GUEST

The current batch job for batch processor 2 is aborted if its owner is GUEST.

```

---

### ABORT-PRINT

**Description:** Abort the current print-out on a spooling device and let the spooling program continue with the next file in the queue.

**Format:** `@ABORT-PRINT <peripheral file name>`

**Parameters:**

- `peripheral file name`: Name of the spooling device.

**Rules:**

- Permitted only for user SYSTEM and the user who appended the file.
- Effective only if the spooling program for the peripheral is started and a file is being printed on it.

**Related Commands:**

- [STOP-PRINT](#stop-print) - Related to STOP-PRINT
- [STOP-SPOOLING](#stop-spooling) - Related to STOP-SPOOLING

**Example:**

```
@ABORT-PRINT LINE-PRINTER

The current file on LINE-PRINTER is aborted.

```

---

### ABSET

**Description:** Start an RT-program at a specific time of day. The program is put in the time queue and moved to the execution queue at the specified time.

**Format:** `@ABSET <program name>,<second>,<minute>,<hour>`

**Parameters:**

- `program name`: RT-program name or address (octal value) of RT-description (default = own terminal background program).
- `second`: Decimal value in the range 0-59 (default = 0).
- `minute`: Decimal value in the range 0-59 (default = 0).
- `hour`: Decimal value in the range 0-23 (default = 0).

**Rules:**

- Permitted for users RT and SYSTEM.
- If the time of day has passed, the program is scheduled for the next day.
- If the program is already in the time queue, it is removed and inserted according to the new specifications.
- Programs in the time queue scheduled by @ABSET are rescheduled according to the new clock, if the clock is adjusted (CLADJ).

**Related Commands:**

- [INTV](#intv) - Prepare an RT-program for periodic execution
- [SET](#set) - Enter an RT-program in the time queue to be transferred to the execution queue after a specified period

**Monitor Calls:**

- ABSET (MON 102) (102): Monitor call to start an RT-program at a specific time
- INTV (MON 103) (103): Monitor call for periodic execution
- SET (MON 101) (101): Monitor call to enter program in time queue

**Example:**

```
@ABSET KLOKK,,10,13

KLOKK will be put in the execution queue at 13:10.

```

---

### ALLOCATE-FILE

**Description:** Create and allocate a contiguous file. The file is created on a specified area of a disk.

**Format:** `@ALLOCATE-FILE <file name>,<page address>,<no. of pages>`

**Parameters:**

- `file name`: Optional version number specifies the number of versions allocated (default type = :DATA, default version = 1).
- `page address`: Page number on the device where the file is to start (octal value, 1+).
- `no. of pages`: Size of the contiguous file area (decimal value, 1+).

**Rules:**

- Permitted for all users.
- If several versions are created, they are allocated one after the other. Version 1 will start at the specified page address.
- The allocated area must not be already in use.

**Related Commands:**

- [ALLOCATE-NEW-VERSION](#allocate-new-version) - Create and allocate a new version of a contiguous file
- [CREATE-FILE](#create-file) - Create a file with specified number of pages
- [EXPAND-FILE](#expand-file) - Increase the length of a contiguous file

**Monitor Calls:**

- CRALF (MON 221) (221): Monitor call to create and allocate a file

**Example:**

```
@ALLOCATE-FILE FILE-3,100,8

The contiguous file FILE-3:DATA is allocated in the executing user's default directory starting at address 1003. Its size is 810 pages.

```

---

### ALLOCATE-NEW-VERSION

**Description:** Create and allocate a new version of a contiguous file. The file is created on a specified area of a disk.

**Format:** `@ALLOCATE-NEW-VERSION <file name>,<page address>,<no. of pages>`

**Parameters:**

- `file name`: File name with indication of highest version. For version (v) see rule 3 of @CREATE-NEW-VERSION (default type = :DATA).
- `page address`: Page number on the device where the file is to start (octal value, 1+).
- `no. of pages`: Size of the contiguous file area (decimal value, 1+).

**Rules:**

- Permitted for all users.
- The allocated area must not be already in use.

**Related Commands:**

- [ALLOCATE-FILE](#allocate-file) - Create and allocate a contiguous file
- [CREATE-NEW-VERSION](#create-new-version) - Create a new version of a file

**Monitor Calls:**

- CRALF (MON 221) (221): Monitor call to create and allocate a file

**Example:**

```
@ALLOCATE-NEW-VERSION FILE-3;2,200,8

Version 2 of the contiguous file FILE-3 is created and allocated at address 2008 in default directory. Its size is 810 pages.

```

---

### ALTOFF

**Description:** Reset 2-bank mode.

**Format:** `@ALTOFF`

**Rules:**

- Permitted for all users.

**Related Commands:**

- [ALTON](#alton) - Set 2-bank mode
- [CHANGE-BACKGROUND-SEGMENT-SIZE](#change-background-segment-size) - Change virtual memory size

**Example:**

```
@ALTOFF

Resets 2-bank mode.

```

---

### ALTON

**Description:** Set 2-bank mode.

**Format:** `@ALTON`

**Rules:**

- Permitted for all users.

**Related Commands:**

- [ALTOFF](#altoff) - Reset 2-bank mode
- [CHANGE-BACKGROUND-SEGMENT-SIZE](#change-background-segment-size) - Change virtual memory size

**Example:**

```
@ALTON

Sets 2-bank mode.

```

---

### APPEND-BATCH

**Description:** Append a batch job to the batch queue.

**Format:** `@APPEND-BATCH <batch no.>,<input file>,<output file>`

**Parameters:**

- `batch no.`: Batch processor number as returned from @BATCH or @LIST-BATCH-PROCESS.
- `input file`: Name of a file containing one or more batch jobs (default type = :SYMB).
- `output file`: Name of file to which the output is appended (default type = :SYMB).

**Rules:**

- Permitted for all users.
- Input file must have read access for all users with jobs on it and for the user SYSTEM.
- Output file must have write append access for all users with jobs on the corresponding input file.
- In a batch job, logical device number 1 means the input file/output file pair is in use.
- The command is only valid if the batch processor is started (see @BATCH).
- The batch file must be terminated with double escape.

**Related Commands:**

- [APPEND-REMOTE](#append-remote) - Append a batch input file to the remote batch queue for a remote computer
- [BATCH](#batch) - Start a passive batch processor
- [ENTER](#enter) - Enter a batch job into the batch queue
- MODE - Set terminal mode for batch processing

**Example:**

```
@APPEND-BATCH 1,JOB-1,LINE-PRINTER

The batch input file JOB-1:SYMB is appended to batch processor 1. Output is appended to LINE-PRINTER.

```

---

### APPEND-REMOTE

**Description:** Append a batch input file to the remote batch queue for a remote computer. This is used for remote job entry (RJE) to a host computer.

**Format:** `@APPEND-REMOTE <remote computer>,<input file>`

**Parameters:**

- `remote computer`: Peripheral file name denoting the host computer. Standard names are IBM, CDC, UNIVAC and HONEYWELL-BULL (default type = REM).
- `input file`: Name of the file containing one or more batch jobs.

**Rules:**

- Permitted for all users.
- Input file must have read access for user RT.

**Related Commands:**

- [APPEND-BATCH](#append-batch) - Append a batch job to the batch queue
- [DELETE-REMOTE-QUEUE-ENTRY](#delete-remote-queue-entry) - Delete an entry from the remote batch queue
- [LIST-REMOTE-QUEUE](#list-remote-queue) - List entries in the remote batch queue

**Example:**

```
@APPEND-REMOTE UNIVAC,JOB-1

The batch input file JOB-1 is appended to the batch queue for the remote computer UNIVAC.

```

---

### APPEND-SPOOLING-FILE

**Description:** Append a spooling queue entry to a spooling queue. When this entry is processed by a spooling program, one or more copies of the file will be printed.

**Format:** `@APPEND-SPOOLING-FILE <peripheral file name>,<file name>,<no. of copies>,<text>[,<printing message?>]`

**Parameters:**

- `peripheral file name`: Name of peripheral file associated with spooling program.
- `file name`: The file to be appended.
- `no. of copies`: Number of output copies required (decimal value, default = 1).
- `text`: Any printable characters terminated by an apostrophe '. The text is printed on the error device when file output is started. If no text is given ' alone must be given.
- `printing message?`: YES = wait for a @START-PRINT command after printing text. This specification overrides @DEFINE-SPOOLING-CONDITIONS. NO = the text is printed on the error device only if @DEFINE-SPOOLING-CONDITIONS specifies print (default = NO).

**Rules:**

- Permitted for all users.
- If no text is specified, the last parameter is ignored.
- The parameter 'file name' may specify files on a remote computer. The file specification may contain the following parameters: system(user(password:project)).(directory:user)file:type;version

**Related Commands:**

- [DEFINE-SPOOLING-CONDITIONS](#define-spooling-conditions) - Define conditions for spooling operations
- LIST-SPOOLING-QUEUE - List entries in the spooling queue

**Monitor Calls:**

- APSPF (MON 240) (240): Append file to spooling queue
- SPCLO (MON 40) (40): Close spooling file

**Example:**

```
@APPEND-SPOOLING-FILE PRINTER,REPORT,2,'Printing report'

Appends the file REPORT to the spooling queue for the printer device, requesting 2 copies and displaying the message "Printing report" when printing starts.

```

---

### BACKSPACE-PRINT

**Description:** Causes the spooling program to repeat the printing of the specified pages or lines and then continue to the end of the file.

**Format:** `@BACKSPACE-PRINT <peripheral file name>,<no. of pages>,<no. of lines>`

**Parameters:**

- `peripheral file name`: Spooling device name.
- `no. of pages`: Number of pages to backspace.
- `no. of lines`: Number of lines to backspace (decimal value, default = 0).

**Rules:**

- Permitted only for user SYSTEM and the user who appended the current print file to the queue.
- Valid only when spooling is started and printing has been stopped by @STOP-PRINT.
- The number of lines per page may be changed by @DEFINE-SPOOLING-CONDITIONS.

**Related Commands:**

- [FORWARD-SPACE-PRINT](#forward-space-print) - Skip printing of specified pages and lines
- RESTART-PRINT - Restart printing from the beginning
- [START-PRINT](#start-print) - Resume printing from where it was stopped
- [STOP-PRINT](#stop-print) - Stop the current printout

**Example:**

```
@STOP-PRINT LINE-PRINTER
@BACKSPACE-PRINT LINE-PRINTER,2,,
@START-PRINT LINE-PRINTER

The spooling output for LINE-PRINTER is stopped and output is resumed, starting two pages back.

```

---

### BATCH

**Description:** Start a passive batch processor.

**Format:** `@BATCH [<batch number>]`

**Parameters:**

- `batch number`: Start the batch process indicated. Default value is the first batch process not started.

**Rules:**

- Permitted only for users RT and SYSTEM.
- The response is: BATCH NUMBER = n, where n is the decimal number of this batch processor. The number is used to identify this batch processor in other commands.
- If no passive batch processor is available an error message is given.
- When a batch processor is started, it enters the idle state because the batch queue is empty. It is activated by the first @APPEND-BATCH command.
- If the optional parameter is used, the corresponding batch processor will start. Otherwise the first passive batch processor will be activated.

**Related Commands:**

- [ABORT-BATCH](#abort-batch) - Stop the batch processor
- [APPEND-BATCH](#append-batch) - Append a batch job to the batch queue
- [LIST-BATCH-PROCESS](#list-batch-process) - List batch processes

**Example:**

```
@BATCH
BATCH NUMBER = 2

Batch processor no. 2 is activated. (Batch processor 1 is already active.)

```

---

### CC

**Description:** Comment. The command or text which follows has no effect. Used in batch or mode files.

**Format:** `@CC <text>`

**Parameters:**

- `text`: Any printable characters.

**Rules:**

- Permitted for all users.
- There should be a space between the second C and the text.

**Related Commands:**

- [CC](#cc) - Comment command in ND-500 Monitor

**Example:**

```
@CC This is a comment in a batch file
The above line will be ignored by the system 
```

---

### CHANGE-BACKGROUND-SEGMENT-SIZE

**Description:** Change segment size for a particular background program.

**Format:** `@CHANGE-BACKGROUND-SEGMENT-SIZE <segment>,<no. of pages>`

**Parameters:**

- `segment (name or number)`: Must be a segment identification - segment number or segment name for a background segment.
- `background segment size in pages`: Must be 128 or 64 (decimal value, default is 128).

**Rules:**

- Permitted only for user SYSTEM.
- This command must be executed after @INITIALIZE-BACKGROUND-PROGRAMS.
- A 64K segment is automatically re-allocated if there is not enough space on the segment file for a 128K segment.

**Related Commands:**

- [ALTOFF](#altoff) - Reset 2-bank mode
- [ALTON](#alton) - Set 2-bank mode

**Monitor Calls:**

- ALTOF (MON 34) (34): Monitor call to reset 2-bank mode
- ALTON (MON 33) (33): Monitor call to set 2-bank mode

**Example:**

```
@CHANGE-BACKGROUND-SEGMENT-SIZE 250,64

The background segment number 2508 is set to 64 pages.

```

---

### CHANGE-BIT-FILE

**Description:** Change a block in the bit file of a directory.

**Format:** `@CHANGE-BIT-FILE <directory name>,<block no.> (subcommands)`

**Parameters:**

- `directory name`: Name of the directory containing the bit file.
- `block no.`: Block number in the bit file (decimal value, default = 0).

**Rules:**

- Permitted only for user SYSTEM.
- Should not be used in batch or mode jobs.
- The subcommands must follow rule 3 given in @LOOK-AT. Terminate subcommands with a full stop.
- The bit file is divided into 20-word blocks. <address> in subcommands is relative address within block (0 - 17).

**Related Commands:**

- [DUMP-BIT-FILE](#dump-bit-file) - Display contents of a bit file block

**Example:**

```
@CHANGE-BIT-FILE PACK-ONE,5
5/177777 157777
5600 ;
@

Changes block 5 in the bit file of directory PACK-ONE.

```

---

### CHANGE-DIRECTORY-ENTRY

**Description:** Change the contents of the directory entry on a device unit.

**Format:** `@CHANGE-DIRECTORY-ENTRY <device name>,<unit>[,<'F' or 'R'>,<subunit>] (subcommands)`

**Parameters:**

- `device name`: Device name, see appendix F for a list of legal device names.
- `unit`: Device unit number (0-3).
- `'F' or 'R'`: F (for fixed) or R (for removable) part of a disk. This parameter only applies to disks which have both a fixed and a removable part.
- `subunit`: Subunit number. This parameter only applies to disks which are subdivided into several parts or directories (0-5).

**Rules:**

- Permitted only for user SYSTEM.
- Should not be used in batch or mode jobs.
- The directory must not be entered.
- <'F' or 'R'> is only used for fixed or removable cartridge disk.
- <subunit> is only required on logically sub-divided disks.
- Note that if you give an erroneous disk type as device type (this may happen, for example, if your system has different disk types as different units and you specify the wrong unit), the disk will enter fault state and the system will hang.
- The parameter <unit> is now always required.
- The subcommands must follow the rules given for @LOOK-AT. Terminate subcommands with a full stop. <address> is relative address in the range 0 - 17.

**Related Commands:**

- [DUMP-DIRECTORY-ENTRY](#dump-directory-entry) - Related to DUMP-DIRECTORY-ENTRY

---

### CHANGE-OBJECT-ENTRY

**Description:** Change the specified object entry.

**Format:** `@CHANGE-OBJECT-ENTRY [<directory name>:]<user name>,<object no.> (subcommands)`

**Parameters:**

- `directory name:user name`: The user name (and directory) of the file.
- `object no.`: The object number of the file (decimal value, default = 0).

**Rules:**

- Permitted only for user SYSTEM.
- Should not be used in batch or mode jobs.
- The subcommands must follow rule 3 given for @LOOK-AT. Terminate subcommands with a full stop. <address> is relative address in the range 0 to 37.
- If no directory name is specified, the user's main directory is used.

**Related Commands:**

- [DUMP-OBJECT-ENTRY](#dump-object-entry) - Related to DUMP-OBJECT-ENTRY
- [LIST-FILES](#list-files) - Related to LIST-FILES

---

### CHANGE-PAGE

**Description:** Change a page in a directory.

**Format:** `@CHANGE-PAGE <directory name>,<page address> (subcommands)`

**Parameters:**

- `directory name`: Name of the directory.
- `page address`: Address within the directory (octal value, default = 0).

**Rules:**

- Permitted only for user SYSTEM.
- Should not be used in batch or mode jobs.
- The subcommands must follow rule 3 given for @LOOK-AT. Terminate subcommands with a full stop. <address> is relative address within the page in the range 0 to 1777.

**Related Commands:**

- [DUMP-PAGE](#dump-page) - Related to DUMP-PAGE

---

### CHANGE-PASSWORD

**Description:** Change user password.

**Format:** `@CHANGE-PASSWORD <old password>,<new password>`

**Parameters:**

- `old password`: Current password (mandatory).
- `new password`: New password (default = no password).

**Rules:**

- Permitted for all users.
- The password is only changed if the old one is specified correctly.
- A password may consist of printable and control characters, except carriage return.
- The parameters are not echoed on the terminal if the command is entered on more than one line.
- Only the password of the logged-in user may be changed.

**Related Commands:**

- [CLEAR-PASSWORD](#clear-password) - Related to CLEAR-PASSWORD

---

### CHANGE-USER-ENTRY

**Description:** Change a user entry in a directory.

**Format:** `@CHANGE-USER-ENTRY <directory name>,<user no.> (subcommands)`

**Parameters:**

- `directory name`: Directory name of the directory containing the specific user (default = main directory).
- `user no.`: Number of the user found by @USER-STATISTICS (decimal value, default = 0).

**Rules:**

- Permitted only for user SYSTEM.
- Should not be used in batch or mode jobs.
- The subcommands must follow rule 3 given for @LOOK-AT. Terminate subcommands with a full stop. <address> is relative address in the range 0 to 37.

**Related Commands:**

- [DUMP-USER-ENTRY](#dump-user-entry) - Related to DUMP-USER-ENTRY

---

### CLADJ

**Description:** Adjust internal clock.

**Format:** `@CLADJ <no. of time units>,<time unit>`

**Parameters:**

- `no. of time units`: Time adjustment as a positive or negative number (decimal value).
- `time unit`: Time unit as specified in @SET command.

**Rules:**

- Permitted only for users RT and SYSTEM.
- The software clock and calendar are set forward if the adjustment is positive or stand still for the specified period if it is negative.
- The time queue is checked and the schedule of all programs started by @ABSET is corrected.
- The command adjusts the panel clock.

**Related Commands:**

- [DATCL](#datcl) - Related to DATCL
- UPDAT - Related to UPDAT

**Monitor Calls:**

- CLADJ (MON 112) (0): Monitor call for CLADJ (MON 112)

---

### CLEAR-BATCH-QUEUE

**Description:** Delete all entries in a batch queue.

**Format:** `@CLEAR-BATCH-QUEUE <batch process number>`

**Parameters:**

- `batch process number`: The number of a batch process.

**Rules:**

- Permitted for user SYSTEM only.

**Related Commands:**

- [ABORT-BATCH](#abort-batch) - Related to ABORT-BATCH
- [DELETE-BATCH-QUEUE-ENTRY](#delete-batch-queue-entry) - Related to DELETE-BATCH-QUEUE-ENTRY
- [LIST-BATCH-PROCESS](#list-batch-process) - Related to LIST-BATCH-PROCESS
- [LIST-BATCH-QUEUE](#list-batch-queue) - Related to LIST-BATCH-QUEUE

---

### CLEAR-DEFAULT-DIRECTORY

**Description:** Clear the specified directory from the list of entered directories which are marked as default.

**Format:** `@CLEAR-DEFAULT-DIRECTORY <directory name>`

**Parameters:**

- `directory name`: An entered directory.

**Rules:**

- Only user SYSTEM may clear the default status of a directory on a hard disk. Any user may clear the default status of a floppy disk.
- A default directory can only be cleared if no user is logged in with this directory as default.
- A main directory is always default, and its main status must be cleared before its default status can be cleared.

**Related Commands:**

- [CLEAR-MAIN-DIRECTORY](#clear-main-directory) - Related to CLEAR-MAIN-DIRECTORY
- [SET-DEFAULT-DIRECTORY](#set-default-directory) - Related to SET-DEFAULT-DIRECTORY

---

### CLEAR-DEVICE

**Description:** Execute a clear device operation (IOX instruction). This command can for example be used to stop the line printer if an attempt is made to print non-alphanumeric information, or it can be used to stop a search for a nonexistent EOF mark on a magnetic tape and prevent the tape from winding off.

**Format:** `@CLEAR-DEVICE <logical device no.>`

**Parameters:**

- `logical device no.`: Logical number of a peripheral device (octal value).

**Rules:**

- Permitted only for user SYSTEM.
- @CLEAR-DEVICE on a magnetic tape operates on the magnetic tape controller, that is, if several magnetic tape stations are connected to the same controller, they are all cleared through one @CLEAR-DEVICE command.

**Related Commands:**

- DEVICE-FUNCTION CLEAR-DEVICE - Related to DEVICE-FUNCTION CLEAR-DEVICE
- [EXECUTE-IOX](#execute-iox) - Related to EXECUTE-IOX
- [IOSET](#ioset) - Related to IOSET

---

### CLEAR-MAIN-DIRECTORY

**Description:** Clear the specified directory from the list of entered directories marked as main.

**Format:** `@CLEAR-MAIN-DIRECTORY <directory name>`

**Parameters:**

- `directory name`: An entered directory.

**Rules:**

- Only executable by user SYSTEM.
- After clearing the main status of a directory, it is still left as a default directory.
- No user must be logged in with this directory as main directory.

**Related Commands:**

- [CLEAR-DEFAULT-DIRECTORY](#clear-default-directory) - Related to CLEAR-DEFAULT-DIRECTORY
- [SET-MAIN-DIRECTORY](#set-main-directory) - Related to SET-MAIN-DIRECTORY

---

### CLEAR-PASSWORD

**Description:** Clear the password of a user.

**Format:** `@CLEAR-PASSWORD <user name>`

**Parameters:**

- `user name`: Name of the user.

**Rules:**

- Permitted only for user SYSTEM.
- User SYSTEM should not forget his/her password!

**Related Commands:**

- [CHANGE-PASSWORD](#change-password) - Related to CHANGE-PASSWORD

---

### CLEAR-REENTRANT-SEGMENT

**Description:** Clear a specified segment, that is, release the space on the segment file occupied by that segment. The segment number will then be free again.

**Format:** `@CLEAR-REENTRANT-SEGMENT <segment name/number>`

**Parameters:**

- `segment name/number`: The name or number of the segment.

**Rules:**

- Permitted only for user SYSTEM.

**Related Commands:**

- LOAD-REENTRANT-SEGMENT - Related to LOAD-REENTRANT-SEGMENT
- CLEAR-SEGMENT (RT-Loader) - Related to CLEAR-SEGMENT (RT-Loader)

---

### CLOSE-FILE

**Description:** Close one or more files opened by currently logged-in user.

**Format:** `@CLOSE-FILE <file no.>`

**Parameters:**

- `file no.`: > 0: close the file, = -1: close all user files not permanently open, = -2: close all user files (octal value)

**Rules:**

- Permitted for all users.
- The file with the specified file number must be opened in order for it to be closed.
- CLOSE -1 sets the block size of all permanently opened scratch files to 400.

**Related Commands:**

- [OPEN-FILE](#open-file) - Related to OPEN-FILE
- RTCLOSE-FILE - Related to RTCLOSE-FILE

**Monitor Calls:**

- CLOSE (MON 43) (0): Monitor call for CLOSE (MON 43)
- SPCLO (MON 40) (0): Monitor call for SPCLO (MON 40)

---

### COLD-START

**Description:** Perform a cold start.

**Format:** `@COLD-START [<terminal number>]`

**Parameters:**

- `terminal number`: The terminal indicated will be treated as console terminal during the start-up process.

**Rules:**

- Permitted only for user SYSTEM.
- The contents of the save-areas is transferred to the image-area on SEGFIL0 and to the other "run"-segments, and the system is started. The contents of user segments and reentrant segments must be built up again by running the mode file HENT-MODE, and using the DUMP-REENTRANT and/or DUMP-PROGRAM-REENTRANT commands.
- If the system includes separate processors (ND-500 part, HDLC, etc.) these should be stopped before COLD-START.

**Related Commands:**

- RESTART-SYSTEM - Related to RESTART-SYSTEM
- [STOP-SYSTEM](#stop-system) - Related to STOP-SYSTEM
- RESET-COLDSTART-MODE-FILE (SINTRAN Service Program) - Related to RESET-COLDSTART-MODE-FILE (SINTRAN Service Program)
- SET-COLDSTART-MODE-FILE (SINTRAN Service Program) - Related to SET-COLDSTART-MODE-FILE (SINTRAN Service Program)

---

### CONCT

**Description:** Connect an RT-program to the interrupt from a device. The RT-program is put in the execution queue when the device gives an interrupt.

**Format:** `@CONCT <program name>,<logical device no.>`

**Parameters:**

- `program name`: RT-program name or RT-description address (octal value, default = user's terminal background program).
- `logical device no.`: Logical device number (decimal value).

**Rules:**

- Permitted only for users RT and SYSTEM.
- Several units may be connected to one program.

**Related Commands:**

- [DSCNT](#dscnt) - Related to DSCNT

**Monitor Calls:**

- CONCT (MON 106) (0): Monitor call for CONCT (MON 106)

---

### CONNECT-FILE

**Description:** Open a mass-storage file with a specified number.

**Format:** `@CONNECT-FILE <file name>,<file no.>,<access type>`

**Parameters:**

- `file name`: Name of the file (default type = :SYMB).
- `file no.`: A logical device number (octal value, 100-177).
- `access type`: See @OPEN-FILE.

**Rules:**

- Permitted for all users.
- Valid only if file number has not been opened previously.
- See @OPEN-FILE, rules.

**Related Commands:**

- [OPEN-FILE](#open-file) - Related to OPEN-FILE
- RTCONNECT-FILE - Related to RTCONNECT-FILE

---

### COPY

**Description:** Copy data to a destination file from a source file. The file is copied byte for byte up to the value of the maximum byte pointer.

**Format:** `@COPY <to device>,<from device>`

**Parameters:**

- `to device`: A file to which data is copied. It may be any type of file (default type = :SYMB).
- `from device`: A file from which data is copied (default type = :SYMB).

**Rules:**

- Permitted for all users.
- If the source file is a mass-storage file with a hole, the copying will stop at the hole and the error message NO SUCH PAGE will be given.
- The file is copied byte for byte up to the value of the maximum byte pointer.
- Both destination file and source file may specify files on a remote computer. The file specifications may contain the following parameters: system(user(password:project)).(directory:user)file:type;version

**Related Commands:**

- [COPY-FILE](#copy-file) - Related to COPY-FILE

---

### COPY-DEVICE

**Description:** Copy all pages to the destination device from the source (mass-storage) device.

**Format:** `@COPY-DEVICE <destination device name>,<unit>,[<'F' or 'R'>,] <source device name>,<unit>[,<'F' or 'R'>]`

**Parameters:**

- `destination device name`: Device name, see appendix F for a list of legal device names.
- `unit`: Device unit number (0-3, default 0).
- `F or R`: F (for fixed) or R (for removable) part of a disk. This parameter only apply to disks which have both a fixed and a removable part.
- `source device name`: Device name, see appendix F for a list of legal device names.
- `unit`: Device unit number (0-3, default 0).
- `F or R`: F (for fixed) or R (for removable) part of a disk. This parameter only apply to disks which have both a fixed and a removable part.

**Rules:**

- Permitted only for user SYSTEM.
- Valid only for devices which can contain directories.
- A directory must exist on the source device.
- Destination device must not be an entered directory.
- Note that if you give an erroneous disk type as device type (this may happen, for example, if your system has different disk types as different units and you specify the wrong unit), the disk will enter fault state and the system will hang.
- The parameters unit are now always required.

**Related Commands:**

- [COPY-DIRECTORY](#copy-directory) - Related to COPY-DIRECTORY
- [SAVE-DIRECTORY](#save-directory) - Related to SAVE-DIRECTORY

---

### COPY-DIRECTORY

**Description:** Copy all files onto the destination directory from the source directory.

**Format:** `@COPY-DIRECTORY <destination directory name>,<source directory name>`

**Parameters:**

- `destination directory name`: Name of the destination directory.
- `source directory name`: Name of the source directory.

**Rules:**

- Permitted only for user SYSTEM.
- The users and the file names are identical in both directories after the command is executed.
- All files must be closed during the copying. All spooling processes should be stopped.

**Related Commands:**

- [COPY-DEVICE](#copy-device) - Related to COPY-DEVICE

---

### COPY-FILE

**Description:** Copy data to a destination file from a source file. The file is copied page by page if both files are mass-storage files.

**Format:** `@COPY-FILE <destination file>,<source file>`

**Parameters:**

- `destination file`: A file to which data is copied (default type = :SYMB).
- `source file`: A file from which data is copied (default type = :SYMB).

**Rules:**

- Permitted for all users.
- If the destination file is a peripheral file and the mass-storage source file is an indexed file with a hole, copying stops at the hole and the error message NO SUCH PAGE is given. If both files are mass-storage files, the file is copied including holes.
- All pages allocated to the source file are copied, except if there are zero bytes in the file.
- If the destination file does not exist, it is created by giving the name in quotes. It will be an indexed file.
- Both destination file and source file may specify files on a remote computer. The file specifications may contain the following parameters: system(user(password:project)).(directory:user)file:type;version

**Related Commands:**

- [COPY](#copy) - Related to COPY

---

### CREATE-DIRECTORY

**Description:** Create a new directory on a specified device.

**Format:** `@CREATE-DIRECTORY <directory name>,<device name>,<unit>[,<'F' or 'R'>] [,<subunit>],<bit file address>`

**Parameters:**

- `directory name`: The name to be written onto the new directory. A maximum of 16 alphanumeric characters, including the hyphen -, is legal.
- `device name`: Device name, see appendix F for a list of legal device names.
- `unit`: Device unit number (0-3, default 0).
- `F or R`: F (for fixed) or R (for removable) part of a disk. This parameter only apply to disks which have both a fixed and a removable part.
- `subunit`: Subunit number. This parameter only apply to disks which are subdivided into several parts or directories (0-5).
- `bit file address`: Specified if the user wants to place the bit file in a particular area. (octal value, default = the file system will select a medium dependent optimal value.) The System Supervisor manual contains more information on this parameter.

**Rules:**

- Permitted only for user SYSTEM when hard disk is specified, for all users when floppy disk.
- Fixed or removable is specified only for 10, 30, 60, 90Mb cartridge disks.
- Note that if you give an erroneous disk type as device type (this may happen, for example, if your system has different disk types as different units and you specify the wrong unit), the disk will enter fault state and the system will hang.
- The parameter unit is now always required.

**Related Commands:**

- [ENTER-DIRECTORY](#enter-directory) - Related to ENTER-DIRECTORY
- RENAME-DIRECTORY - Related to RENAME-DIRECTORY

---

### CREATE-FILE

**Description:** Create one or more versions of a file. The file will be contiguous or indexed depending on number of pages.

**Format:** `@CREATE-FILE <file name>,<no. of pages>`

**Parameters:**

- `file name`: Optional version number specifies number of versions to be created (default type = :DATA). File type should be specified, if not: :DATA (default) is assumed.
- `no. of pages`: 0: create empty file, the file will be indexed the first time something is written to it, or be contiguous if expanded with @EXPAND-FILE. 
> 0: create contiguous file with the specified no. of pages (decimal value, default = 0).


**Rules:**

- Permitted for all users.
- If there are not enough pages for all versions, the system creates as many as possible and gives an error message. To find the number created, use @LIST-FILES.
- Contiguous files are positioned in the highest page address range possible on the directory.
- Contiguous files are more efficient than indexed files. If contiguous files can be used, performance may be improved.

**Related Commands:**

- [ALLOCATE-FILE](#allocate-file) - Related to ALLOCATE-FILE
- [CREATE-NEW-VERSION](#create-new-version) - Related to CREATE-NEW-VERSION
- [OPEN-FILE](#open-file) - Related to OPEN-FILE
- RENAME-FILE - Related to RENAME-FILE

---

### CREATE-FRIEND

**Description:** Declare a user as a friend to the current user. This friendship is not reciprocal, that is, you cannot create yourself as a friend of another user.

**Format:** `@CREATE-FRIEND <user name>`

**Parameters:**

- `user name`: Name of a user in the same main directory as you.

**Rules:**

- Permitted for all users.
- A user can have a maximum of eight friends, and all must belong to the same main directory as him/her.
- When the friend is created, his/her general default file access to the terminal user's files is read, write and append (RWA). This can be changed by @SET-FRIEND-ACCESS. Friends can be given restricted access to a specific file by @SET-FILE-ACCESS.
- Access given to a friend overrides public access, even if it is more limited than public access.

**Related Commands:**

- [DELETE-FRIEND](#delete-friend) - Related to DELETE-FRIEND
- [LIST-FRIENDS](#list-friends) - Related to LIST-FRIENDS
- [SET-FRIEND-ACCESS](#set-friend-access) - Related to SET-FRIEND-ACCESS
- [SET-INITIAL-FRIEND-ACCESS](#set-initial-friend-access) - Related to SET-INITIAL-FRIEND-ACCESS

---

### CREATE-NEW-VERSION

**Description:** Create one or more new versions of an existing file.

**Format:** `@CREATE-NEW-VERSION <file name>,<no. of pages>`

**Parameters:**

- `file name`: For version (v) see rule 3 below (default type = :DATA).
- `no. of pages`: See @CREATE-FILE.

**Rules:**

- Permitted for all users with directory access (D) to the user's file space.
- See @CREATE-FILE, rule 2.
- If version v already exists, one new version is created and inserted as this version. The old version is renumbered as v + 1 and so on. If v is higher than the highest version existing (w) the versions w + 1, w + 2, ...., v are created. Default version is w + 1.
- A new version can be created by other commands (for example @OPEN-FILE). The file version number is then enclosed in quotes.

**Related Commands:**

- [ALLOCATE-NEW-VERSION](#allocate-new-version) - Related to ALLOCATE-NEW-VERSION
- [CREATE-FILE](#create-file) - Related to CREATE-FILE

---

### CREATE-USER

**Description:** Create a new user in a directory, or define a new user name in a directory.

**Format:** `@CREATE-USER [<directory name> ]<user name>`

**Parameters:**

- `directory name`: Optional directory name where the user will be created.
- `user name`: Name of the user to be created.

**Rules:**

- Permitted only for user SYSTEM.
- The user name must be unique within the directory.
- The user will be created with default access permissions.

**Related Commands:**

- [DELETE-USER](#delete-user) - Related to DELETE-USER
- [GIVE-USER-SPACE](#give-user-space) - Related to GIVE-USER-SPACE
- RENAME-USER - Related to RENAME-USER
- TAKE-USER-SPACE - Related to TAKE-USER-SPACE

---

### DATCL

**Description:** Shows current time and date

**Format:** `@DATCL
`

**Rules:**

- No parameters are required.
- The command displays the current system time and date.
- Available to all users.

**Example:**

```
@DATCL 
```

---

### DEFAULT-SUBSYSTEM-DISABLE

**Description:** Disables execution of default subsystem

**Format:** `@DEFAULT-SUBSYSTEM-DISABLE terminal_number
`

**Parameters:**

- `terminal_number`: The terminal number to disable default subsystem for

**Rules:**

- Permitted only for user SYSTEM.
- The terminal must be active.
- The command affects only the specified terminal.

**Related Commands:**

- [DEFAULT-SUBSYSTEM-ENABLE](#default-subsystem-enable) - Enables execution of default subsystem

**Example:**

```
@DEFAULT-SUBSYSTEM-DISABLE 5 
```

---

### DEFAULT-SUBSYSTEM-ENABLE

**Description:** Enables execution of default subsystem

**Format:** `@DEFAULT-SUBSYSTEM-ENABLE terminal_number
`

**Parameters:**

- `terminal_number`: The terminal number to enable default subsystem for

**Rules:**

- Permitted only for user SYSTEM.
- The terminal must be active.
- The command affects only the specified terminal.

**Related Commands:**

- [DEFAULT-SUBSYSTEM-DISABLE](#default-subsystem-disable) - Disables execution of default subsystem

**Example:**

```
@DEFAULT-SUBSYSTEM-ENABLE 5 
```

---

### DEFINE-DEFAULT-SUBSYSTEM

**Description:** Define the default subsystem for a user.

**Format:** `@DEFINE-DEFAULT-SUBSYSTEM <subsystem name>`

**Parameters:**

- `subsystem name`: Name of the subsystem to be set as default.

**Rules:**

- Permitted only for user SYSTEM.
- The subsystem must exist in the system.
- This command sets the default subsystem that will be used when a user logs in.

**Related Commands:**

- DEFINE-SUBSYSTEM - Related to DEFINE-SUBSYSTEM

---

### DEFINE-ESCAPE-CHARACTER

**Description:** Define the escape character for terminal input.

**Format:** `@DEFINE-ESCAPE-CHARACTER <character>`

**Parameters:**

- `character`: The character to be used as the escape character.

**Rules:**

- Permitted only for user SYSTEM.
- The character must be a printable ASCII character.
- The escape character is used to interrupt command execution.

**Related Commands:**

- [DEFINE-LOCAL-CHARACTER](#define-local-character) - Related to DEFINE-LOCAL-CHARACTER

---

### DEFINE-HISTOGRAM

**Description:** Define a histogram for system performance monitoring.

**Format:** `@DEFINE-HISTOGRAM <histogram name>,<number of bins>,<bin size>`

**Parameters:**

- `histogram name`: Name of the histogram to be defined.
- `number of bins`: Number of bins in the histogram (decimal value).
- `bin size`: Size of each bin in the histogram (decimal value).

**Rules:**

- Permitted only for user SYSTEM.
- The histogram name must be unique.
- The number of bins and bin size must be positive values.
- The histogram can be used to monitor system performance metrics.

**Related Commands:**

- [START-HISTOGRAM](#start-histogram) - Related to START-HISTOGRAM
- [STOP-HISTOGRAM](#stop-histogram) - Related to STOP-HISTOGRAM
- [PRINT-HISTOGRAM](#print-histogram) - Related to PRINT-HISTOGRAM

---

### DEFINE-LOCAL-CHARACTER

**Description:** Define a local character for terminal input.

**Format:** `@DEFINE-LOCAL-CHARACTER <character>`

**Parameters:**

- `character`: The character to be defined as a local character.

**Rules:**

- Permitted only for user SYSTEM.
- The character must be a printable ASCII character.
- Local characters are used for special terminal functions.

**Related Commands:**

- [DEFINE-ESCAPE-CHARACTER](#define-escape-character) - Related to DEFINE-ESCAPE-CHARACTER

---

### DEFINE-MASS-STORAGE-UNIT

**Description:** Define a mass storage unit in the system.

**Format:** `@DEFINE-MASS-STORAGE-UNIT <unit number>,<device type>,<parameters>`

**Parameters:**

- `unit number`: The unit number to be assigned to the mass storage device (0-3).
- `device type`: Type of mass storage device (e.g., disk, tape).
- `parameters`: Additional parameters specific to the device type.

**Rules:**

- Permitted only for user SYSTEM.
- The unit number must be unique.
- The device type must be supported by the system.
- The parameters must be valid for the specified device type.

**Related Commands:**

- [CLEAR-DEVICE](#clear-device) - Related to CLEAR-DEVICE

---

### DEFINE-REENTRANT-PROGRAM

**Description:** Define a program as reentrant in the system.

**Format:** `@DEFINE-REENTRANT-PROGRAM <program name>`

**Parameters:**

- `program name`: Name of the program to be defined as reentrant.

**Rules:**

- Permitted only for user SYSTEM.
- The program must exist in the system.
- A reentrant program can be executed by multiple users simultaneously.
- The program must be designed to be reentrant.

**Related Commands:**

- [CLEAR-REENTRANT-SEGMENT](#clear-reentrant-segment) - Related to CLEAR-REENTRANT-SEGMENT

---

### DEFINE-SPOOLING-CONDITIONS

**Description:** Define conditions for spooling operations.

**Format:** `@DEFINE-SPOOLING-CONDITIONS <conditions>`

**Parameters:**

- `conditions`: Spooling conditions to be defined.

**Rules:**

- Permitted only for user SYSTEM.
- The conditions must be valid for the system configuration.
- Spooling conditions affect how print jobs are processed.

**Related Commands:**

- [START-SPOOLING](#start-spooling) - Related to START-SPOOLING
- [STOP-SPOOLING](#stop-spooling) - Related to STOP-SPOOLING

---

### DEFINE-SPOOLING-FILE-MESSAGE

**Description:** Define a message to be displayed when a spooling file is processed.

**Format:** `@DEFINE-SPOOLING-FILE-MESSAGE <message>`

**Parameters:**

- `message`: The message to be displayed during spooling file processing.

**Rules:**

- Permitted only for user SYSTEM.
- The message can contain system variables and formatting.
- The message is displayed to users when their spooling files are processed.

**Related Commands:**

- [DEFINE-SPOOLING-CONDITIONS](#define-spooling-conditions) - Related to DEFINE-SPOOLING-CONDITIONS

---

### DEFINE-SYSTEM-HISTOGRAM

**Description:** Define a system-wide histogram for performance monitoring.

**Format:** `@DEFINE-SYSTEM-HISTOGRAM <histogram name>,<number of bins>,<bin size>`

**Parameters:**

- `histogram name`: Name of the system histogram to be defined.
- `number of bins`: Number of bins in the histogram (decimal value).
- `bin size`: Size of each bin in the histogram (decimal value).

**Rules:**

- Permitted only for user SYSTEM.
- The histogram name must be unique.
- The number of bins and bin size must be positive values.
- System histograms monitor overall system performance metrics.

**Related Commands:**

- [START-HISTOGRAM](#start-histogram) - Related to START-HISTOGRAM
- [STOP-HISTOGRAM](#stop-histogram) - Related to STOP-HISTOGRAM
- [PRINT-HISTOGRAM](#print-histogram) - Related to PRINT-HISTOGRAM

---

### DELETE-BATCH-QUEUE-ENTRY

**Description:** Deletes an entry from the batch queue

**Format:** `@DELETE-BATCH-QUEUE-ENTRY batch_no, input_file, output_file
`

**Parameters:**

- `batch_no`: The batch process number
- `input_file`: The input file name
- `output_file`: The output file name

**Rules:**

- Permitted only for user SYSTEM.
- The batch queue entry must exist.
- The command removes the specified entry from the batch queue.

**Related Commands:**

- [APPEND-BATCH](#append-batch) - Appends batch files to batch queue
- [LIST-BATCH-QUEUE](#list-batch-queue) - Lists batch queue

**Example:**

```
@DELETE-BATCH-QUEUE-ENTRY 1, INPUT, OUTPUT 
```

---

### DELETE-FILE

**Description:** Deletes a file from the system

**Format:** `@DELETE-FILE file_name
`

**Parameters:**

- `file_name`: The name of the file to delete

**Rules:**

- The user must have delete access to the file.
- The file must exist.
- The command permanently removes the file from the system.

**Related Commands:**

- [CREATE-FILE](#create-file) - Creates a new file
- [COPY-FILE](#copy-file) - Copies a file

**Example:**

```
@DELETE-FILE TEST.DAT 
```

---

### DELETE-FRIEND

**Description:** Deletes a friend from the system

**Format:** `@DELETE-FRIEND friend_name
`

**Parameters:**

- `friend_name`: The name of the friend to delete

**Rules:**

- Permitted only for user SYSTEM.
- The friend must exist.
- The command removes the friend's entry from the system.

**Related Commands:**

- [CREATE-FRIEND](#create-friend) - Creates a new friend
- [SET-FRIEND-ACCESS](#set-friend-access) - Sets friend access rights

**Example:**

```
@DELETE-FRIEND FRIEND1 
```

---

### DELETE-MASS-STORAGE-UNIT

**Description:** Deletes a mass storage unit from the system

**Format:** `@DELETE-MASS-STORAGE-UNIT unit_number
`

**Parameters:**

- `unit_number`: The number of the mass storage unit to delete

**Rules:**

- Permitted only for user SYSTEM.
- The mass storage unit must exist.
- The command removes the mass storage unit from the system.

**Related Commands:**

- [DEFINE-MASS-STORAGE-UNIT](#define-mass-storage-unit) - Defines a mass storage unit
- [LIST-MASS-STORAGE-UNIT](#list-mass-storage-unit) - Lists mass storage units

**Example:**

```
@DELETE-MASS-STORAGE-UNIT 1 
```

---

### DELETE-REENTRANT

**Description:** Deletes a reentrant program from the system

**Format:** `@DELETE-REENTRANT program_name
`

**Parameters:**

- `program_name`: The name of the reentrant program to delete

**Rules:**

- Permitted only for user SYSTEM.
- The reentrant program must exist.
- The command removes the reentrant program from the system.

**Related Commands:**

- [DEFINE-REENTRANT-PROGRAM](#define-reentrant-program) - Defines a reentrant program
- [LIST-REENTRANT](#list-reentrant) - Lists reentrant programs

**Example:**

```
@DELETE-REENTRANT PROG1 
```

---

### DELETE-REMOTE-QUEUE-ENTRY

**Description:** Deletes an entry from the remote queue

**Format:** `@DELETE-REMOTE-QUEUE-ENTRY queue_number, input_file, output_file
`

**Parameters:**

- `queue_number`: The queue number
- `input_file`: The input file name
- `output_file`: The output file name

**Rules:**

- Permitted only for user SYSTEM.
- The remote queue entry must exist.
- The command removes the specified entry from the remote queue.

**Related Commands:**

- [APPEND-REMOTE](#append-remote) - Appends remote files to remote queue
- [LIST-REMOTE-QUEUE](#list-remote-queue) - Lists remote queue

**Example:**

```
@DELETE-REMOTE-QUEUE-ENTRY 1, INPUT, OUTPUT 
```

---

### DELETE-SPOOLING-FILE

**Description:** Deletes a spooling file from the system

**Format:** `@DELETE-SPOOLING-FILE file_name
`

**Parameters:**

- `file_name`: The name of the spooling file to delete

**Rules:**

- Permitted only for user SYSTEM.
- The spooling file must exist.
- The command removes the spooling file from the system.

**Related Commands:**

- [APPEND-SPOOLING-FILE](#append-spooling-file) - Appends a file to the spooling queue
- LIST-SPOOLING-QUEUE - Lists spooling queue

**Example:**

```
@DELETE-SPOOLING-FILE SPOOL1 
```

---

### DELETE-USER

**Description:** Deletes a user from the system

**Format:** `@DELETE-USER user_name
`

**Parameters:**

- `user_name`: The name of the user to delete

**Rules:**

- Permitted only for user SYSTEM.
- The user must exist.
- The command removes the user's entry from the system.

**Related Commands:**

- [CREATE-USER](#create-user) - Creates a new user
- MODIFY-USER - Modifies user parameters

**Example:**

```
@DELETE-USER JOHN 
```

---

### DELETE-USERS-FILES

**Description:** Deletes all files owned by a user

**Format:** `@DELETE-USERS-FILES user_name
`

**Parameters:**

- `user_name`: The name of the user whose files are to be deleted

**Rules:**

- Permitted only for user SYSTEM.
- The user must exist.
- The command permanently removes all files owned by the specified user.

**Related Commands:**

- [DELETE-USER](#delete-user) - Deletes a user from the system
- [LIST-FILES](#list-files) - Lists files

**Example:**

```
@DELETE-USERS-FILES JOHN 
```

---

### DEVICE-FUNCTION

**Description:** Performs a function on a device

**Format:** `@DEVICE-FUNCTION device_number, function_code
`

**Parameters:**

- `device_number`: The number of the device
- `function_code`: The function code to perform

**Rules:**

- Permitted only for user SYSTEM.
- The device must exist.
- The function code must be valid for the device.

**Related Commands:**

- [LIST-DEVICE](#list-device) - Lists devices
- [LIST-DEVICE-FUNCTION](#list-device-function) - Lists device functions

**Example:**

```
@DEVICE-FUNCTION 1, 2 
```

---

### DIRECTORY-STATISTICS

**Description:** Displays statistics for a directory

**Format:** `@DIRECTORY-STATISTICS directory_name
`

**Parameters:**

- `directory_name`: The name of the directory

**Rules:**

- The user must have access to the directory.
- The directory must exist.
- The command displays statistics for the specified directory.

**Related Commands:**

- [LIST-DIRECTORIES-ENTERED](#list-directories-entered) - Lists directories entered
- [ENTER-DIRECTORY](#enter-directory) - Enters a directory

**Example:**

```
@DIRECTORY-STATISTICS DIR1 
```

---

### DISABLE-ESCAPE-FUNCTION

**Description:** Disables the escape function

**Format:** `@DISABLE-ESCAPE-FUNCTION
`

**Rules:**

- Permitted only for user SYSTEM.
- The command disables the escape function for the current terminal.

**Related Commands:**

- [ENABLE-ESCAPE-FUNCTION](#enable-escape-function) - Enables the escape function
- [DEFINE-ESCAPE-CHARACTER](#define-escape-character) - Defines the escape character

**Example:**

```
@DISABLE-ESCAPE-FUNCTION 
```

---

### DISABLE-TERMINATION-HANDLING

**Description:** Disables termination handling

**Format:** `@DISABLE-TERMINATION-HANDLING
`

**Rules:**

- Permitted only for user SYSTEM.
- The command disables termination handling for the current terminal.

**Related Commands:**

- [ENABLE-TERMINATION-HANDLING](#enable-termination-handling) - Enables termination handling
- LIST-TERMINATION-HANDLING - Lists termination handling

**Example:**

```
@DISABLE-TERMINATION-HANDLING 
```

---

### DMAC

**Description:** Displays memory access control

**Format:** `@DMAC
`

**Rules:**

- Permitted only for user SYSTEM.
- The command displays memory access control information.

**Related Commands:**

- MEMORY - Displays memory information
- MEMORY-LIMITS - Displays memory limits

**Example:**

```
@DMAC 
```

---

### DSCNT

**Description:** Disconnects a terminal

**Format:** `@DSCNT terminal_number
`

**Parameters:**

- `terminal_number`: The number of the terminal to disconnect

**Rules:**

- Permitted only for user SYSTEM.
- The terminal must be active.
- The command disconnects the specified terminal.

**Related Commands:**

- [CONCT](#conct) - Connects a terminal
- LIST-USERS - Lists users

**Example:**

```
@DSCNT 5 
```

---

### DUMP

**Description:** Dumps memory contents

**Format:** `@DUMP start_address, end_address
`

**Parameters:**

- `start_address`: The starting memory address
- `end_address`: The ending memory address

**Rules:**

- Permitted only for user SYSTEM.
- The addresses must be valid.
- The command dumps the contents of the specified memory range.

**Related Commands:**

- [DUMP-BIT-FILE](#dump-bit-file) - Dumps a bit file
- [DUMP-PAGE](#dump-page) - Dumps a page

**Example:**

```
@DUMP 0, 1000 
```

---

### DUMP-BIT-FILE

**Description:** Dumps a bit file

**Format:** `@DUMP-BIT-FILE file_name
`

**Parameters:**

- `file_name`: The name of the bit file to dump

**Rules:**

- Permitted only for user SYSTEM.
- The bit file must exist.
- The command dumps the contents of the specified bit file.

**Related Commands:**

- [DUMP](#dump) - Dumps memory contents
- [DUMP-PAGE](#dump-page) - Dumps a page

**Example:**

```
@DUMP-BIT-FILE BIT1 
```

---

### DUMP-DIRECTORY-ENTRY

**Description:** Dumps a directory entry

**Format:** `@DUMP-DIRECTORY-ENTRY directory_name
`

**Parameters:**

- `directory_name`: The name of the directory

**Rules:**

- Permitted only for user SYSTEM.
- The directory must exist.
- The command dumps the contents of the specified directory entry.

**Related Commands:**

- [DUMP](#dump) - Dumps memory contents
- [DUMP-PAGE](#dump-page) - Dumps a page

**Example:**

```
@DUMP-DIRECTORY-ENTRY DIR1 
```

---

### DUMP-OBJECT-ENTRY

**Description:** Dumps an object entry

**Format:** `@DUMP-OBJECT-ENTRY object_name
`

**Parameters:**

- `object_name`: The name of the object

**Rules:**

- Permitted only for user SYSTEM.
- The object must exist.
- The command dumps the contents of the specified object entry.

**Related Commands:**

- [DUMP](#dump) - Dumps memory contents
- [DUMP-PAGE](#dump-page) - Dumps a page

**Example:**

```
@DUMP-OBJECT-ENTRY OBJ1 
```

---

### DUMP-PAGE

**Description:** Dumps a page

**Format:** `@DUMP-PAGE page_number
`

**Parameters:**

- `page_number`: The number of the page to dump

**Rules:**

- Permitted only for user SYSTEM.
- The page must exist.
- The command dumps the contents of the specified page.

**Related Commands:**

- [DUMP](#dump) - Dumps memory contents
- [DUMP-BIT-FILE](#dump-bit-file) - Dumps a bit file

**Example:**

```
@DUMP-PAGE 1 
```

---

### DUMP-PROGRAM-REENTRANT

**Description:** Dumps a program reentrant

**Format:** `@DUMP-PROGRAM-REENTRANT program_name
`

**Parameters:**

- `program_name`: The name of the program

**Rules:**

- Permitted only for user SYSTEM.
- The program must exist.
- The command dumps the contents of the specified program reentrant.

**Related Commands:**

- [DUMP](#dump) - Dumps memory contents
- [DUMP-PAGE](#dump-page) - Dumps a page

**Example:**

```
@DUMP-PROGRAM-REENTRANT PROG1 
```

---

### DUMP-REENTRANT

**Description:** Dumps a reentrant

**Format:** `@DUMP-REENTRANT reentrant_name
`

**Parameters:**

- `reentrant_name`: The name of the reentrant

**Rules:**

- Permitted only for user SYSTEM.
- The reentrant must exist.
- The command dumps the contents of the specified reentrant.

**Related Commands:**

- [DUMP](#dump) - Dumps memory contents
- [DUMP-PAGE](#dump-page) - Dumps a page

**Example:**

```
@DUMP-REENTRANT REENT1 
```

---

### DUMP-USER-ENTRY

**Description:** Dump, in octal, a user entry onto an output file.

**Format:** `@DUMP-USER-ENTRY <directory name>,<user no.>,<output file>`

**Parameters:**

- `directory name`: Directory name of the directory containing the specific user (default = main directory).
- `user no.`: Number of the user found by @USER-STATISTICS (decimal value, default = 0).
- `output file`: Destination of the octal dump (default = TERMINAL).

**Rules:**

- Permitted only for user SYSTEM.

**Related Commands:**

- [CHANGE-USER-ENTRY](#change-user-entry) - Related to CHANGE-USER-ENTRY

**Monitor Calls:**

- RUSER (44): Monitor call for RUSER (MON 44)

**Example:**

```
@DUMP-USER-ENTRY P-O, 4,,

User entry number 4 on directory P-O is dumped to the output file.

```

---

### ENABLE-ESCAPE-FUNCTION

**Description:** Enables the escape function

**Format:** `@ENABLE-ESCAPE-FUNCTION
`

**Rules:**

- Permitted only for user SYSTEM.
- The command enables the escape function for the current terminal.

**Related Commands:**

- [DISABLE-ESCAPE-FUNCTION](#disable-escape-function) - Disables the escape function
- [DEFINE-ESCAPE-CHARACTER](#define-escape-character) - Defines the escape character

**Example:**

```
@ENABLE-ESCAPE-FUNCTION 
```

---

### ENABLE-TERMINATION-HANDLING

**Description:** Enables termination handling

**Format:** `@ENABLE-TERMINATION-HANDLING
`

**Rules:**

- Permitted only for user SYSTEM.
- The command enables termination handling for the current terminal.

**Related Commands:**

- [DISABLE-TERMINATION-HANDLING](#disable-termination-handling) - Disables termination handling
- LIST-TERMINATION-HANDLING - Lists termination handling

**Example:**

```
@ENABLE-TERMINATION-HANDLING 
```

---

### ENTER

**Description:** Enters a program

**Format:** `@ENTER program_name
`

**Parameters:**

- `program_name`: The name of the program to enter

**Rules:**

- The program must exist.
- The user must have access to the program.
- The command enters the specified program.

**Related Commands:**

- [ENTER-DIRECTORY](#enter-directory) - Enters a directory
- LIST-RT-PROGRAMS - Lists RT programs

**Example:**

```
@ENTER PROG1 
```

---

### ENTER-DIRECTORY

**Description:** Enters a directory

**Format:** `@ENTER-DIRECTORY directory_name
`

**Parameters:**

- `directory_name`: The name of the directory to enter

**Rules:**

- The directory must exist.
- The user must have access to the directory.
- The command enters the specified directory.

**Related Commands:**

- [ENTER](#enter) - Enters a program
- [LIST-DIRECTORIES-ENTERED](#list-directories-entered) - Lists directories entered

**Example:**

```
@ENTER-DIRECTORY DIR1 
```

---

### ENTSG

**Description:** Enters a segment

**Format:** `@ENTSG segment_name
`

**Parameters:**

- `segment_name`: The name of the segment to enter

**Rules:**

- The segment must exist.
- The user must have access to the segment.
- The command enters the specified segment.

**Related Commands:**

- [ENTER](#enter) - Enters a program
- LIST-SEGMENT - Lists segments

**Example:**

```
@ENTSG SEG1 
```

---

### EXECUTE-IOX

**Description:** Execute an IOX instruction on a device.

**Format:** `@EXECUTE-IOX <value>,<device register address>`

**Parameters:**

- `value`: The IOX command to be executed.
- `device register address`: Address of the device register (must be listed in the internal SINTRAN III IOX table).

**Rules:**

- Permitted only for users RT and SYSTEM.
- The contents of the A register are displayed after execution.
- The device register address must be listed in the internal SINTRAN III IOX table.
- Can be set by *INSERT-IN-IOX-TABLE in @SINTRAN-SERVICE-PROGRAM.

**Related Commands:**

- [INSERT-IN-IOX-TABLE](#insert-in-iox-table) - Related to INSERT-IN-IOX-TABLE
- [REMOVE-FROM-IOX-TABLE](#remove-from-iox-table) - Related to REMOVE-FROM-IOX-TABLE

---

### EXPAND-FILE

**Description:** Expands a file

**Format:** `@EXPAND-FILE file_name, new_size
`

**Parameters:**

- `file_name`: The name of the file to expand
- `new_size`: The new size of the file

**Rules:**

- The file must exist.
- The user must have access to the file.
- The command expands the specified file to the new size.

**Related Commands:**

- [CREATE-FILE](#create-file) - Creates a new file
- [COPY-FILE](#copy-file) - Copies a file

**Example:**

```
@EXPAND-FILE TEST.DAT, 1000 
```

---

### FILE-STATISTICS

**Description:** Displays statistics for a file

**Format:** `@FILE-STATISTICS file_name
`

**Parameters:**

- `file_name`: The name of the file

**Rules:**

- The file must exist.
- The user must have access to the file.
- The command displays statistics for the specified file.

**Related Commands:**

- [LIST-FILES](#list-files) - Lists files
- [DIRECTORY-STATISTICS](#directory-statistics) - Displays statistics for a directory

**Example:**

```
@FILE-STATISTICS TEST.DAT 
```

---

### FIX

**Description:** Fixes a file

**Format:** `@FIX file_name
`

**Parameters:**

- `file_name`: The name of the file to fix

**Rules:**

- The file must exist.
- The user must have access to the file.
- The command fixes the specified file.

**Related Commands:**

- [FIXC](#fixc) - Fixes a file with a specific code
- [LIST-FILES](#list-files) - Lists files

**Example:**

```
@FIX TEST.DAT 
```

---

### FIXC

**Description:** Fixes a file with a specific code

**Format:** `@FIXC file_name, code
`

**Parameters:**

- `file_name`: The name of the file to fix
- `code`: The code to use for fixing

**Rules:**

- The file must exist.
- The user must have access to the file.
- The command fixes the specified file using the provided code.

**Related Commands:**

- [FIX](#fix) - Fixes a file
- [LIST-FILES](#list-files) - Lists files

**Example:**

```
@FIXC TEST.DAT, 123 
```

---

### FORWARD-SPACE-PRINT

**Description:** Forward spaces print

**Format:** `@FORWARD-SPACE-PRINT
`

**Rules:**

- The command forward spaces print.

**Related Commands:**

- [BACKSPACE-PRINT](#backspace-print) - Backspaces print
- [START-PRINT](#start-print) - Starts print

**Example:**

```
@FORWARD-SPACE-PRINT 
```

---

### GET-ERROR-DEVICE

**Description:** Gets the error device

**Format:** `@GET-ERROR-DEVICE
`

**Rules:**

- The command gets the error device.

**Related Commands:**

- [SET-ERROR-DEVICE](#set-error-device) - Sets the error device
- [LIST-DEVICE](#list-device) - Lists devices

**Example:**

```
@GET-ERROR-DEVICE 
```

---

### GET-RT-NAME

**Description:** Gets the RT name

**Format:** `@GET-RT-NAME
`

**Rules:**

- The command gets the RT name.

**Related Commands:**

- LIST-RT-PROGRAMS - Lists RT programs
- [LIST-RT-DESCRIPTION](#list-rt-description) - Lists RT description

**Example:**

```
@GET-RT-NAME 
```

---

### GET-TERMINAL-TYPE

**Description:** Gets the terminal type

**Format:** `@GET-TERMINAL-TYPE
`

**Rules:**

- The command gets the terminal type.

**Related Commands:**

- [SET-TERMINAL-TYPE](#set-terminal-type) - Sets the terminal type
- [LIST-DEVICE](#list-device) - Lists devices

**Example:**

```
@GET-TERMINAL-TYPE 
```

---

### GIVE-OBJECT-BLOCKS

**Description:** Gives object blocks

**Format:** `@GIVE-OBJECT-BLOCKS object_name, blocks
`

**Parameters:**

- `object_name`: The name of the object
- `blocks`: The number of blocks to give

**Rules:**

- The object must exist.
- The user must have access to the object.
- The command gives the specified number of blocks to the object.

**Related Commands:**

- LIST-OBJECT-ENTRY - Lists object entry
- [DUMP-OBJECT-ENTRY](#dump-object-entry) - Dumps an object entry

**Example:**

```
@GIVE-OBJECT-BLOCKS OBJ1, 10 
```

---

### GIVE-SPOOLING-PAGES

**Description:** Gives spooling pages

**Format:** `@GIVE-SPOOLING-PAGES pages
`

**Parameters:**

- `pages`: The number of pages to give

**Rules:**

- The command gives the specified number of spooling pages.

**Related Commands:**

- LIST-SPOOLING-QUEUE - Lists spooling queue
- [SPOOLING-PAGES-LEFT](#spooling-pages-left) - Displays spooling pages left

**Example:**

```
@GIVE-SPOOLING-PAGES 5 
```

---

### GIVE-USER-SPACE

**Description:** Gives user space

**Format:** `@GIVE-USER-SPACE user_name, space
`

**Parameters:**

- `user_name`: The name of the user
- `space`: The amount of space to give

**Rules:**

- The user must exist.
- The command gives the specified amount of space to the user.

**Related Commands:**

- LIST-USERS - Lists users
- MEMORY-LIMITS - Displays memory limits

**Example:**

```
@GIVE-USER-SPACE JOHN, 1000 
```

---

### GOTO-USER

**Description:** Goes to a user

**Format:** `@GOTO-USER user_name
`

**Parameters:**

- `user_name`: The name of the user to go to

**Rules:**

- The user must exist.
- The command goes to the specified user.

**Related Commands:**

- LIST-USERS - Lists users
- LOGOUT - Logs out

**Example:**

```
@GOTO-USER JOHN 
```

---

### HELP

**Description:** Displays help

**Format:** `@HELP
`

**Rules:**

- The command displays help.

**Related Commands:**

- LIST-COMMANDS - Lists commands
- LIST-COMMANDS-SECTION - Lists commands in a section

**Example:**

```
@HELP 
```

---

### HOLD

**Description:** Holds a job

**Format:** `@HOLD job_name
`

**Parameters:**

- `job_name`: The name of the job to hold

**Rules:**

- The job must exist.
- The command holds the specified job.

**Related Commands:**

- [ABORT-JOB](#abort-job) - Aborts a job
- LIST-JOBS - Lists jobs

**Example:**

```
@HOLD JOB1 
```

---

### INIT-ACCOUNTING

**Description:** Initializes accounting

**Format:** `@INIT-ACCOUNTING
`

**Rules:**

- Permitted only for user SYSTEM.
- The command initializes accounting.

**Related Commands:**

- [START-ACCOUNTING](#start-accounting) - Starts accounting
- [STOP-ACCOUNTING](#stop-accounting) - Stops accounting

**Example:**

```
@INIT-ACCOUNTING 
```

---

### INITIAL-COMMAND

**Description:** Sets initial command

**Format:** `@INITIAL-COMMAND command
`

**Parameters:**

- `command`: The command to set as initial

**Rules:**

- Permitted only for user SYSTEM.
- The command sets the initial command.

**Related Commands:**

- [LIST-INITIAL-COMMANDS](#list-initial-commands) - Lists initial commands
- NEXT-INITIAL-COMMAND - Sets next initial command

**Example:**

```
@INITIAL-COMMAND @HELP 
```

---

### INITIALIZE-BACKGROUND-PROGRAMS

**Description:** Initializes background programs

**Format:** `@INITIALIZE-BACKGROUND-PROGRAMS
`

**Rules:**

- Permitted only for user SYSTEM.
- The command initializes background programs.

**Related Commands:**

- [CHANGE-BACKGROUND-SEGMENT-SIZE](#change-background-segment-size) - Changes background segment size
- LIST-BACKGROUND-PROGRAMS - Lists background programs

**Example:**

```
@INITIALIZE-BACKGROUND-PROGRAMS 
```

---

### INITIALIZE-ERROR-LOG

**Description:** Initializes error log

**Format:** `@INITIALIZE-ERROR-LOG
`

**Rules:**

- Permitted only for user SYSTEM.
- The command initializes the error log.

**Related Commands:**

- [START-PROGRAM-LOG](#start-program-log) - Starts program log
- [STOP-PROGRAM-LOG](#stop-program-log) - Stops program log

**Example:**

```
@INITIALIZE-ERROR-LOG 
```

---

### INTV

**Description:** Sets interrupt vector

**Format:** `@INTV vector_number, address
`

**Parameters:**

- `vector_number`: The number of the interrupt vector
- `address`: The address to set

**Rules:**

- Permitted only for user SYSTEM.
- The command sets the specified interrupt vector.

**Related Commands:**

- [IOSET](#ioset) - Sets I/O
- MEMORY - Displays memory information

**Example:**

```
@INTV 1, 1000 
```

---

### IOSET

**Description:** Sets I/O

**Format:** `@IOSET device_number, function_code
`

**Parameters:**

- `device_number`: The number of the device
- `function_code`: The function code to set

**Rules:**

- Permitted only for user SYSTEM.
- The device must exist.
- The command sets the specified I/O function.

**Related Commands:**

- [INTV](#intv) - Sets interrupt vector
- [LIST-DEVICE](#list-device) - Lists devices

**Example:**

```
@IOSET 1, 2 
```

---

### LIST-BATCH-PROCESS

**Description:** Lists batch process

**Format:** `@LIST-BATCH-PROCESS
`

**Rules:**

- The command lists the batch process.

**Related Commands:**

- [LIST-BATCH-QUEUE](#list-batch-queue) - Lists batch queue
- [ABORT-BATCH](#abort-batch) - Aborts batch

**Example:**

```
@LIST-BATCH-PROCESS 
```

---

### LIST-BATCH-QUEUE

**Description:** Lists batch queue

**Format:** `@LIST-BATCH-QUEUE
`

**Rules:**

- The command lists the batch queue.

**Related Commands:**

- [LIST-BATCH-PROCESS](#list-batch-process) - Lists batch process
- [APPEND-BATCH](#append-batch) - Appends batch files to batch queue

**Example:**

```
@LIST-BATCH-QUEUE 
```

---

### LIST-DEFAULT-SUBSYSTEM

**Description:** Lists default subsystem

**Format:** `@LIST-DEFAULT-SUBSYSTEM
`

**Rules:**

- The command lists the default subsystem.

**Related Commands:**

- [DEFINE-DEFAULT-SUBSYSTEM](#define-default-subsystem) - Defines default subsystem
- [DEFAULT-SUBSYSTEM-DISABLE](#default-subsystem-disable) - Disables default subsystem

**Example:**

```
@LIST-DEFAULT-SUBSYSTEM 
```

---

### LIST-DEVICE

**Description:** Lists devices

**Format:** `@LIST-DEVICE
`

**Rules:**

- The command lists devices.

**Related Commands:**

- [LIST-DEVICE-FUNCTION](#list-device-function) - Lists device functions
- SET-DEVICE - Sets device

**Example:**

```
@LIST-DEVICE 
```

---

### LIST-DEVICE-FUNCTION

**Description:** Lists device functions

**Format:** `@LIST-DEVICE-FUNCTION
`

**Rules:**

- The command lists device functions.

**Related Commands:**

- [LIST-DEVICE](#list-device) - Lists devices
- [DEVICE-FUNCTION](#device-function) - Performs a function on a device

**Example:**

```
@LIST-DEVICE-FUNCTION 
```

---

### LIST-DIRECTORIES-ENTERED

**Description:** Lists directories entered

**Format:** `@LIST-DIRECTORIES-ENTERED
`

**Rules:**

- The command lists directories entered.

**Related Commands:**

- [ENTER-DIRECTORY](#enter-directory) - Enters a directory
- [DIRECTORY-STATISTICS](#directory-statistics) - Displays statistics for a directory

**Example:**

```
@LIST-DIRECTORIES-ENTERED 
```

---

### LIST-EXECUTION-QUEUE

**Description:** Lists execution queue

**Format:** `@LIST-EXECUTION-QUEUE
`

**Rules:**

- The command lists the execution queue.

**Related Commands:**

- [LIST-BATCH-QUEUE](#list-batch-queue) - Lists batch queue
- [LIST-REMOTE-QUEUE](#list-remote-queue) - Lists remote queue

**Example:**

```
@LIST-EXECUTION-QUEUE 
```

---

### LIST-FILES

**Description:** Lists files

**Format:** `@LIST-FILES
`

**Rules:**

- The command lists files.

**Related Commands:**

- [FILE-STATISTICS](#file-statistics) - Displays statistics for a file
- [LIST-DIRECTORIES-ENTERED](#list-directories-entered) - Lists directories entered

**Example:**

```
@LIST-FILES 
```

---

### LIST-FRIENDS

**Description:** Lists friends

**Format:** `@LIST-FRIENDS
`

**Rules:**

- The command lists friends.

**Related Commands:**

- [CREATE-FRIEND](#create-friend) - Creates a new friend
- [DELETE-FRIEND](#delete-friend) - Deletes a friend from the system

**Example:**

```
@LIST-FRIENDS 
```

---

### LIST-INITIAL-COMMANDS

**Description:** Lists initial commands

**Format:** `@LIST-INITIAL-COMMANDS
`

**Rules:**

- The command lists initial commands.

**Related Commands:**

- [INITIAL-COMMAND](#initial-command) - Sets initial command
- NEXT-INITIAL-COMMAND - Sets next initial command

**Example:**

```
@LIST-INITIAL-COMMANDS 
```

---

### LIST-MASS-STORAGE-UNIT

**Description:** Lists mass storage units

**Format:** `@LIST-MASS-STORAGE-UNIT
`

**Rules:**

- The command lists mass storage units.

**Related Commands:**

- [DEFINE-MASS-STORAGE-UNIT](#define-mass-storage-unit) - Defines a mass storage unit
- [DELETE-MASS-STORAGE-UNIT](#delete-mass-storage-unit) - Deletes a mass storage unit from the system

**Example:**

```
@LIST-MASS-STORAGE-UNIT 
```

---

### LIST-OPEN-FILES

**Description:** Lists open files

**Format:** `@LIST-OPEN-FILES
`

**Rules:**

- The command lists open files.

**Related Commands:**

- [LIST-FILES](#list-files) - Lists files
- [CLOSE-FILE](#close-file) - Closes a file

**Example:**

```
@LIST-OPEN-FILES 
```

---

### LIST-REENTRANT

**Description:** Lists reentrant programs

**Format:** `@LIST-REENTRANT
`

**Rules:**

- The command lists reentrant programs.

**Related Commands:**

- [DEFINE-REENTRANT-PROGRAM](#define-reentrant-program) - Defines a reentrant program
- [DELETE-REENTRANT](#delete-reentrant) - Deletes a reentrant program from the system

**Example:**

```
@LIST-REENTRANT 
```

---

### LIST-REMOTE-QUEUE

**Description:** Lists remote queue

**Format:** `@LIST-REMOTE-QUEUE
`

**Rules:**

- The command lists the remote queue.

**Related Commands:**

- [LIST-BATCH-QUEUE](#list-batch-queue) - Lists batch queue
- [LIST-EXECUTION-QUEUE](#list-execution-queue) - Lists execution queue

**Example:**

```
@LIST-REMOTE-QUEUE 
```

---

### LIST-RT-ACCOUNT

**Description:** Lists RT account

**Format:** `@LIST-RT-ACCOUNT
`

**Rules:**

- The command lists the RT account.

**Related Commands:**

- [START-RT-ACCOUNT](#start-rt-account) - Starts RT account
- [STOP-RT-ACCOUNT](#stop-rt-account) - Stops RT account

**Example:**

```
@LIST-RT-ACCOUNT 
```

---

### LIST-RT-DESCRIPTION

**Description:** Lists RT description

**Format:** `@LIST-RT-DESCRIPTION
`

**Rules:**

- The command lists the RT description.

**Related Commands:**

- LIST-RT-PROGRAMS - Lists RT programs
- [GET-RT-NAME](#get-rt-name) - Gets the RT name

**Example:**

```
@LIST-RT-DESCRIPTION 
```

---

### LIST-RTOPEN-FILES

**Description:** Lists RTOPEN files

**Format:** `@LIST-RTOPEN-FILES
`

**Rules:**

- The command lists RTOPEN files.

**Related Commands:**

- [RTOPEN-FILE](#rtopen-file) - Opens a file for RT
- [RTRELEASE-OPEN-FILE-ENTRIES](#rtrelease-open-file-entries) - Releases open file entries for RT

**Example:**

```
@LIST-RTOPEN-FILES 
```

---

### RTENTER

**Description:** Enter real-time mode for a program.

**Format:** `@RTENTER <program name>`

**Parameters:**

- `program name`: Name of the program to be executed in real-time mode.

**Rules:**

- Permitted only for users with real-time privileges.
- The program must be designed for real-time execution.
- Real-time mode provides immediate response to external events.

**Related Commands:**

- [RTOFF](#rtoff) - Related to RTOFF
- [RTON](#rton) - Related to RTON

---

### RTOFF

**Description:** Disable real-time mode for a program.

**Format:** `@RTOFF`

**Rules:**

- Permitted only for users with real-time privileges.
- The command disables real-time execution mode.
- All real-time programs must be stopped before using this command.

**Related Commands:**

- [RTENTER](#rtenter) - Related to RTENTER
- [RTON](#rton) - Related to RTON

---

### RTON

**Description:** Enable real-time mode for a program.

**Format:** `@RTON`

**Rules:**

- Permitted only for users with real-time privileges.
- The command enables real-time execution mode.
- Real-time programs can be started after this command is executed.

**Related Commands:**

- [RTENTER](#rtenter) - Related to RTENTER
- [RTOFF](#rtoff) - Related to RTOFF

---

### RTOPEN-FILE

**Description:** Open a file in real-time mode.

**Format:** `@RTOPEN-FILE <file name>`

**Parameters:**

- `file name`: Name of the file to be opened in real-time mode.

**Rules:**

- Permitted only for users with real-time privileges.
- The file must exist and be accessible.
- Real-time file operations provide immediate access to file data.

**Related Commands:**

- [RTENTER](#rtenter) - Related to RTENTER
- [CLOSE-FILE](#close-file) - Related to CLOSE-FILE

---

### RTRELEASE-OPEN-FILE-ENTRIES

**Description:** Release open file entries in real-time mode.

**Format:** `@RTRELEASE-OPEN-FILE-ENTRIES`

**Rules:**

- Permitted only for users with real-time privileges.
- The command releases all open file entries for the current real-time program.
- This should be done before terminating a real-time program.

**Related Commands:**

- [RTENTER](#rtenter) - Related to RTENTER
- [RTRESERVE-OPEN-FILE-ENTRIES](#rtreserve-open-file-entries) - Related to RTRESERVE-OPEN-FILE-ENTRIES

---

### RTRESERVE-OPEN-FILE-ENTRIES

**Description:** Reserve open file entries for real-time mode.

**Format:** `@RTRESERVE-OPEN-FILE-ENTRIES <number of entries>`

**Parameters:**

- `number of entries`: Number of file entries to reserve for real-time operations.

**Rules:**

- Permitted only for users with real-time privileges.
- The number of entries must be sufficient for the real-time program's needs.
- Reserved entries must be released when no longer needed.

**Related Commands:**

- [RTENTER](#rtenter) - Related to RTENTER
- [RTRELEASE-OPEN-FILE-ENTRIES](#rtrelease-open-file-entries) - Related to RTRELEASE-OPEN-FILE-ENTRIES

---

### SAVE-DIRECTORY

**Description:** Save the current directory to a backup device.

**Format:** `@SAVE-DIRECTORY <backup device>`

**Parameters:**

- `backup device`: Device to which the directory will be saved.

**Rules:**

- Permitted only for user SYSTEM.
- The backup device must be properly configured.
- All files in the directory must be closed before saving.

**Related Commands:**

- [COPY-DIRECTORY](#copy-directory) - Related to COPY-DIRECTORY

---

### SCHEDULE

**Description:** Schedule a program or command for execution at a specified time.

**Format:** `@SCHEDULE <time>,<command>`

**Parameters:**

- `time`: Time at which the command should be executed.
- `command`: Command or program to be executed at the specified time.

**Rules:**

- Permitted only for user SYSTEM.
- The time must be in a valid format.
- The command must be valid and accessible.

**Related Commands:**

- [BATCH](#batch) - Related to BATCH

---

### SCRATCH-OPEN

**Description:** Open a file in scratch mode, discarding any existing contents.

**Format:** `@SCRATCH-OPEN <file name>`

**Parameters:**

- `file name`: Name of the file to be opened in scratch mode.

**Rules:**

- Permitted for all users.
- Any existing contents of the file will be discarded.
- The file will be created if it does not exist.

**Related Commands:**

- [OPEN-FILE](#open-file) - Related to OPEN-FILE
- [CLOSE-FILE](#close-file) - Related to CLOSE-FILE

---

### SET

**Description:** Set a system parameter or variable to a specified value.

**Format:** `@SET <parameter>,<value>`

**Parameters:**

- `parameter`: Name of the parameter to be set.
- `value`: Value to which the parameter should be set.

**Rules:**

- Permitted only for user SYSTEM.
- The parameter must be a valid system parameter.
- The value must be appropriate for the parameter type.

**Related Commands:**

- [SET-AVAILABLE](#set-available) - Related to SET-AVAILABLE
- [SET-UNAVAILABLE](#set-unavailable) - Related to SET-UNAVAILABLE

---

### SET-AVAILABLE

**Description:** Mark a device or resource as available for use.

**Format:** `@SET-AVAILABLE <resource>`

**Parameters:**

- `resource`: Name or identifier of the resource to be marked as available.

**Rules:**

- Permitted only for user SYSTEM.
- The resource must exist in the system.
- The resource must be in a state that allows it to be marked as available.

**Related Commands:**

- [SET](#set) - Related to SET
- [SET-UNAVAILABLE](#set-unavailable) - Related to SET-UNAVAILABLE

---

### SET-BLOCK-POINTER

**Description:** Set the block pointer for a file to a specified position.

**Format:** `@SET-BLOCK-POINTER <file number>,<block number>`

**Parameters:**

- `file number`: Number of the file whose block pointer is to be set.
- `block number`: Block number to which the pointer should be set.

**Rules:**

- Permitted for all users.
- The file must be open.
- The block number must be valid for the file.

**Related Commands:**

- [SET-BYTE-POINTER](#set-byte-pointer) - Related to SET-BYTE-POINTER

---

### SET-BLOCK-SIZE

**Description:** Set the block size for a file.

**Format:** `@SET-BLOCK-SIZE <file number>,<block size>`

**Parameters:**

- `file number`: Number of the file whose block size is to be set.
- `block size`: Size of blocks in bytes.

**Rules:**

- Permitted for all users.
- The file must be open.
- The block size must be a power of 2.

**Related Commands:**

- [SET-BLOCK-POINTER](#set-block-pointer) - Related to SET-BLOCK-POINTER

---

### SET-BYTE-POINTER

**Description:** Set the byte pointer for a file to a specified position.

**Format:** `@SET-BYTE-POINTER <file number>,<byte position>`

**Parameters:**

- `file number`: Number of the file whose byte pointer is to be set.
- `byte position`: Position in bytes to which the pointer should be set.

**Rules:**

- Permitted for all users.
- The file must be open.
- The byte position must be valid for the file.

**Related Commands:**

- [SET-BLOCK-POINTER](#set-block-pointer) - Related to SET-BLOCK-POINTER

---

### SET-DEFAULT-DIRECTORY

**Description:** Define a directory as a default directory.


**Format:** `@SET-DEFAULT-DIRECTORY <directory name>
`

**Parameters:**

- `directory name`: Directory to be set as default.

**Rules:**

- Permitted only for user SYSTEM when hard disk is specified, for any user when floppy disk.
- Main directory is always default directory.
- Several directories can be default directory, but a user should not have space in more than one default directory. If so, the directory name should always be specified when accessing a file.
- The directory specified must be entered.

**Related Commands:**

- [CLEAR-DEFAULT-DIRECTORY](#clear-default-directory) - Related to CLEAR-DEFAULT-DIRECTORY
- [ENTER-DIRECTORY](#enter-directory) - Related to ENTER-DIRECTORY

**Example:**

```
@SET-DEFAULT-DIRECTORY PACK-TWO

PACK-TWO is defined as default directory.

```

---

### SET-DEFAULT-FILE-ACCESS

**Description:** Set the default access permissions for files.

**Format:** `@SET-DEFAULT-FILE-ACCESS <access type>`

**Parameters:**

- `access type`: Type of access to be set as default (e.g., read, write, append).

**Rules:**

- Permitted only for user SYSTEM.
- The access type must be valid.
- This setting applies to all new files created by users.

**Related Commands:**

- [SET-FILE-ACCESS](#set-file-access) - Related to SET-FILE-ACCESS

---

### SET-DEFAULT-REMOTE-SYSTEM

**Description:** Set the default remote system for file operations.

**Format:** `@SET-DEFAULT-REMOTE-SYSTEM <system name>`

**Parameters:**

- `system name`: Name of the remote system to be set as default.

**Rules:**

- Permitted only for user SYSTEM.
- The remote system must be properly configured.
- This setting affects all remote file operations.

**Related Commands:**

- [SET-REMOTE-MODE](#set-remote-mode) - Related to SET-REMOTE-MODE

---

### SET-ERROR-DEVICE

**Description:** Set the device to which error messages will be sent.

**Format:** `@SET-ERROR-DEVICE <device name>`

**Parameters:**

- `device name`: Name of the device to receive error messages.

**Rules:**

- Permitted only for user SYSTEM.
- The device must be properly configured.
- Error messages will be redirected to this device.

**Related Commands:**

- [SET-TERMINAL-FILE](#set-terminal-file) - Related to SET-TERMINAL-FILE

---

### SET-FILE-ACCESS

**Description:** Set public, friend and owner access permissions for a specified file.

**Format:** `@SET-FILE-ACCESS <file name>,<public access>,<friend access>,<own access>`

**Parameters:**

- `file name`: Name of the file (default type = :SYMB).
- `public access`: N or any combination of R, W, A, C and D (see rules below, default = no change).
- `friend access`: N or any combination of R, W, A, C and D (see rules below, default = no change).
- `own access`: N or any combination of R, W, A, C and D (see rules below, default = no change).

**Rules:**

- Permitted for all users with directory access to the file.
- Access permissions include read (R), write (W), append (A), common (C), directory (D), and none (N).
- The N permission must be used alone and cannot be combined with other permissions.
- Directory access (D) allows creating, deleting, changing access mode, and creating new versions.
- FILE-STATISTICS can be used to check the access types for the file.

**Related Commands:**

- [SET-DEFAULT-FILE-ACCESS](#set-default-file-access) - Sets default file access permissions for new files
- [SET-FRIEND-ACCESS](#set-friend-access) - Sets friend access permissions for files
- [FILE-STATISTICS](#file-statistics) - Displays file access permissions and other file information

**Monitor Calls:**

- RACCESS (MON 45) (45): Monitor call to change file access permissions

**Example:**

```
@SET—FILE-ACCESS F—l DATA N,N,RWACD

Public and friends have no access to the file F—1:DATA. Owner has total access.

```

---

### SET-FRIEND-ACCESS

**Description:** Set access permissions for a friend user.

**Format:** `@SET-FRIEND-ACCESS <friend name>,<access type>`

**Parameters:**

- `friend name`: Name of the friend user.
- `access type`: Type of access to be granted (e.g., read, write, append).

**Rules:**

- Permitted for all users.
- The friend must exist in the system.
- The access type must be valid.

**Related Commands:**

- [CREATE-FRIEND](#create-friend) - Related to CREATE-FRIEND
- [SET-FILE-ACCESS](#set-file-access) - Related to SET-FILE-ACCESS

---

### SET-INITIAL-FILE-ACCESS

**Description:** Set the initial access permissions for a file.

**Format:** `@SET-INITIAL-FILE-ACCESS <file name>,<access type>`

**Parameters:**

- `file name`: Name of the file whose initial access is to be set.
- `access type`: Type of access to be set initially (e.g., read, write, append).

**Rules:**

- Permitted for all users.
- The file must exist.
- The access type must be valid.

**Related Commands:**

- [SET-FILE-ACCESS](#set-file-access) - Related to SET-FILE-ACCESS

---

### SET-INITIAL-FRIEND-ACCESS

**Description:** Set the initial access permissions for friend users.

**Format:** `@SET-INITIAL-FRIEND-ACCESS <access type>`

**Parameters:**

- `access type`: Type of access to be set initially for friends (e.g., read, write, append).

**Rules:**

- Permitted only for user SYSTEM.
- The access type must be valid.
- This setting applies to all new friend relationships.

**Related Commands:**

- [CREATE-FRIEND](#create-friend) - Related to CREATE-FRIEND
- [SET-FRIEND-ACCESS](#set-friend-access) - Related to SET-FRIEND-ACCESS

---

### SET-LOCAL-MODE

**Description:** Set the system to operate in local mode.

**Format:** `@SET-LOCAL-MODE`

**Rules:**

- Permitted only for user SYSTEM.
- The system will operate only with local resources.
- Remote operations will be disabled.

**Related Commands:**

- [SET-REMOTE-MODE](#set-remote-mode) - Related to SET-REMOTE-MODE

---

### SET-MAIN-DIRECTORY

**Description:** Set the main directory for the system.

**Format:** `@SET-MAIN-DIRECTORY <directory name>`

**Parameters:**

- `directory name`: Name of the directory to be set as main directory.

**Rules:**

- Permitted only for user SYSTEM.
- The directory must exist.
- This directory will be the root for all file operations.

**Related Commands:**

- [CLEAR-MAIN-DIRECTORY](#clear-main-directory) - Related to CLEAR-MAIN-DIRECTORY

---

### SET-MEMORY-CONTENTS

**Description:** Set the contents of a memory location.

**Format:** `@SET-MEMORY-CONTENTS <address>,<value>`

**Parameters:**

- `address`: Memory address to be modified.
- `value`: Value to be written to the memory location.

**Rules:**

- Permitted only for user SYSTEM.
- The address must be valid.
- The value must be appropriate for the memory location.

**Related Commands:**

- LOOK-AT-MEMORY - Related to LOOK-AT-MEMORY

**Example:**

```
@SET-MEMORY-CONTENTS 124000,,177777

Set entire 64K to 1240008. 
```

---

### SET-NUMBER-OF-PRINT-COPIES

**Description:** Set the number of copies to be printed for a file.

**Format:** `@SET-NUMBER-OF-PRINT-COPIES <number of copies>`

**Parameters:**

- `number of copies`: Number of copies to be printed (default = 1).

**Rules:**

- Permitted for all users.
- The number of copies must be positive.
- This setting applies to the next print operation.

**Related Commands:**

- [START-PRINT](#start-print) - Related to START-PRINT

**Example:**

```
@SET-NUMBER-OF-PRINT-COPIES 8

Change the desired number of print copies to 8 for the next print operation. 
```

---

### SET-PERIPHERAL-FILE

**Description:** Set a peripheral device file for a program.

**Format:** `@SET-PERIPHERAL-FILE <file name>,<device name>`

**Parameters:**

- `file name`: Name of the file to be associated with the peripheral device.
- `device name`: Name of the peripheral device.

**Rules:**

- Permitted only for user SYSTEM.
- The device must be properly configured.
- The file must exist and be accessible.

**Related Commands:**

- [SET-TERMINAL-FILE](#set-terminal-file) - Related to SET-TERMINAL-FILE

**Example:**

```
@SET-PERIPHERAL-FILE "LINE-PRINTER",5

The file LINE-PRINTER is created and associated with logical device number 5. 
```

---

### SET-PERMANENT-OPEN

**Description:** Set a file to be permanently open.

**Format:** `@SET-PERMANENT-OPEN <file name>`

**Parameters:**

- `file name`: Name of the file to be set as permanently open.

**Rules:**

- Permitted only for user SYSTEM.
- The file must exist and be accessible.
- The file will remain open until explicitly closed.

**Related Commands:**

- [CLOSE-FILE](#close-file) - Related to CLOSE-FILE

**Example:**

```
@OPEN-FILE OLE:DATA,BW
FILE NUMBER IS: 101
@SET-PERMANENT-OPEN 101
@CLOSE -1

The file OLE DATA is still open. 
```

---

### SET-REMOTE-MODE

**Description:** Set the system to operate in remote mode.

**Format:** `@SET-REMOTE-MODE`

**Rules:**

- Permitted only for user SYSTEM.
- The system will operate with both local and remote resources.
- Remote operations will be enabled.

**Related Commands:**

- [SET-LOCAL-MODE](#set-local-mode) - Related to SET-LOCAL-MODE

---

### SET-SPOOLING-FORM

**Description:** Set the form type for spooling operations.

**Format:** `@SET-SPOOLING-FORM <form type>`

**Parameters:**

- `form type`: Type of form to be used for spooling (e.g., standard, wide, special).

**Rules:**

- Permitted only for user SYSTEM.
- The form type must be valid.
- This setting applies to all spooling operations.

**Related Commands:**

- [START-SPOOLING](#start-spooling) - Related to START-SPOOLING

---

### SET-TEMPORARY-FILE

**Description:** Set a file as temporary, to be deleted when closed.

**Format:** `@SET-TEMPORARY-FILE <file name>`

**Parameters:**

- `file name`: Name of the file to be set as temporary.

**Rules:**

- Permitted for all users.
- The file must exist and be accessible.
- The file will be deleted when closed.

**Related Commands:**

- [CLOSE-FILE](#close-file) - Related to CLOSE-FILE

---

### SET-TERMINAL-FILE

**Description:** Set the terminal file for a program.

**Format:** `@SET-TERMINAL-FILE <file name>`

**Parameters:**

- `file name`: Name of the file to be used as terminal output.

**Rules:**

- Permitted only for user SYSTEM.
- The file must exist and be accessible.
- Terminal output will be redirected to this file.

**Related Commands:**

- [SET-ERROR-DEVICE](#set-error-device) - Related to SET-ERROR-DEVICE

**Example:**

```
@SET-TERMINAL-FILE "TERMINAL"

The new name is TERMINAL.

```

---

### SET-TERMINAL-TYPE

**Description:** Set the type of terminal for a user.

**Format:** `@SET-TERMINAL-TYPE <terminal type>`

**Parameters:**

- `terminal type`: Type of terminal to be used (e.g., VT100, ANSI, TTY).

**Rules:**

- Permitted only for user SYSTEM.
- The terminal type must be valid.
- This setting affects terminal behavior and capabilities.

**Related Commands:**

- [SET-TERMINAL-FILE](#set-terminal-file) - Related to SET-TERMINAL-FILE

---

### SET-UNAVAILABLE

**Description:** Mark a device or resource as unavailable for use.

**Format:** `@SET-UNAVAILABLE <resource>`

**Parameters:**

- `resource`: Name or identifier of the resource to be marked as unavailable.

**Rules:**

- Permitted only for user SYSTEM.
- The resource must exist in the system.
- The resource must be in a state that allows it to be marked as unavailable.

**Related Commands:**

- [SET](#set) - Related to SET
- [SET-AVAILABLE](#set-available) - Related to SET-AVAILABLE

**Example:**

```
@SET-UNAVAILABLE DOWN FOR MAINT $ AVAILABLE 11:30$

When anyone tries to log in on a terminal other than terminal 1, the following message is output:

SYSTEM UNAVAILABLE
DOWN FOR MAINT.
AVAILABLE 11:30

```

---

### SET-USER-PARAMETERS

**Description:** Set parameters for a user account.

**Format:** `@SET-USER-PARAMETERS <user name>,<parameter>,<value>`

**Parameters:**

- `user name`: Name of the user whose parameters are to be set.
- `parameter`: Name of the parameter to be set.
- `value`: Value to which the parameter should be set.

**Rules:**

- Permitted only for user SYSTEM.
- The user must exist in the system.
- The parameter must be valid for the user.

**Related Commands:**

- [CHANGE-USER-ENTRY](#change-user-entry) - Related to CHANGE-USER-ENTRY

---

### SINTRAN-SERVICE-PROGRAM

**Description:** Start the SINTRAN service program.

**Format:** `@SINTRAN-SERVICE-PROGRAM`

**Rules:**

- Permitted only for user SYSTEM.
- The service program provides system maintenance and debugging capabilities.
- The program must be run from the system console.

**Related Commands:**

- [SET](#set) - Related to SET

---

### SPOOLING-PAGES-LEFT

**Description:** List the remaining number of pages that can be used by the spooling files.


**Format:** `@SPOOLING-PAGES-LEFT
`

**Rules:**

- Permitted for all users.
- User system should have at least as many unused pages as there are spooling pages left.

**Related Commands:**

- [GIVE-SPOOLING-PAGES](#give-spooling-pages) - Related to GIVE-SPOOLING-PAGES
- TAKE-SPOOLING-PAGES - Related to TAKE-SPOOLING-PAGES

**Example:**

```
@SPOOLING-PAGES-LEFT
500 SPOOLING PAGES LEFT
@

```

---

### START

**Description:** Start execution of a program.

**Format:** `@START <program name>`

**Parameters:**

- `program name`: Name of the program to be started.

**Rules:**

- Permitted for all users.
- The program must exist and be executable.
- The user must have permission to execute the program.

**Related Commands:**

- [RUN](#run) - Related to RUN

---

### START-ACCOUNTING

**Description:** Start the accounting system, but do not initiate the accounting file; if the file does not exist, the command executes as for @INIT-ACCOUNTING.


**Format:** `@START-ACCOUNTING <background>[,<RT>,<clear logged information>,<logging interval>][,<ND-500>][,<spooling>]
`

**Parameters:**

- `background`: Are background programs to be accounted.
- `RT`: Are user RT-programs to be accounted.
- `clear logged information`: Whether information already logged in the RT accounting table should be cleared.
- `logging interval`: Number of seconds between dumps of RT accounting table on the file ACCOUNTS:DATA.
- `ND-500`: Are ND-500 programs to be accounted?
- `spooling`: Is spooling to be accounted?

**Rules:**

- Permitted only for user SYSTEM.
- Accounting on an ND-500 or spooling cannot be started unless background accounting is running. It can be stopped independently but is stopped automatically if background accounting is stopped.

**Related Commands:**

- [INIT-ACCOUNTING](#init-accounting) - Related to INIT-ACCOUNTING
- [LIST-RT-ACCOUNT](#list-rt-account) - Related to LIST-RT-ACCOUNT
- [START-RT-ACCOUNT](#start-rt-account) - Related to START-RT-ACCOUNT
- [STOP-ACCOUNTING](#stop-accounting) - Related to STOP-ACCOUNTING
- [STOP-RT-ACCOUNT](#stop-rt-account) - Related to STOP-RT-ACCOUNT

---

### START-HISTOGRAM

**Description:** Turn on sampling for the histogram.


**Format:** `@START-HISTOGRAM
`

**Rules:**

- Permitted for all users.
- The histogram must be defined (@DEFINE-HISTOGRAM).
- The command is normally given before starting the program to be sampled.

**Related Commands:**

- [DEFINE-HISTOGRAM](#define-histogram) - Related to DEFINE-HISTOGRAM
- [DEFINE-SYSTEM-HISTOGRAM](#define-system-histogram) - Related to DEFINE-SYSTEM-HISTOGRAM
- [PRINT-HISTOGRAM](#print-histogram) - Related to PRINT-HISTOGRAM
- [STOP-HISTOGRAM](#stop-histogram) - Related to STOP-HISTOGRAM

---

### START-PRINT

**Description:** Resume printout of the current spooling file from the point where it stopped.


**Format:** `@START-PRINT <peripheral file name>
`

**Parameters:**

- `peripheral file name`: The spooling device.

**Rules:**

- Permitted only for user SYSTEM and the user who appended the file to be printed.
- Print can be stopped by a) @STOP-PRINT, b) An automatic stop print defined by @DEFINE-SPOOLING-CONDITIONS, c) Closing a file with SPCLO (MON 40) specifying stop print, d) Specifying stop print in @APPEND-SPOOLING.

**Related Commands:**

- [APPEND-SPOOLING-FILE](#append-spooling-file) - Related to APPEND-SPOOLING-FILE
- [STOP-PRINT](#stop-print) - Related to STOP-PRINT

**Monitor Calls:**

- SPCLO (40): Monitor call related to START-PRINT

---

### START-PROGRAM-LOG

**Description:** Start the logging of RT and background programs. When used with @DEFINE-SYSTEM-HISTOGRAM, a sampling distribution on various interrupt levels is produced.


**Format:** `@START-PROGRAM-LOG <interrupts/sample>
`

**Parameters:**

- `interrupts/sample`: Number of terminal interrupts per sample.

**Rules:**

- Permitted only for user SYSTEM.

**Related Commands:**

- RT-PROGRAM-LOG - Related to RT-PROGRAM-LOG
- [STOP-PROGRAM-LOG](#stop-program-log) - Related to STOP-PROGRAM-LOG

**Example:**

```
@START-PROGRAM-LOG 100

Samples are taken at a rate of approximately 10 samples/second.

```

---

### START-RT-ACCOUNT

**Description:** Start accounting for RT-programs.


**Format:** `@START-RT-ACCOUNT <RT-program>
`

**Parameters:**

- `RT-program`: The name of an RT-program which will be logged.

**Rules:**

- Each user RT-program is associated with a project password.
- Available for users SYSTEM and RT.
- System RT-programs cannot be logged.

**Related Commands:**

- [INIT-ACCOUNTING](#init-accounting) - Related to INIT-ACCOUNTING
- [LIST-RT-ACCOUNT](#list-rt-account) - Related to LIST-RT-ACCOUNT
- [START-ACCOUNTING](#start-accounting) - Related to START-ACCOUNTING
- [STOP-ACCOUNTING](#stop-accounting) - Related to STOP-ACCOUNTING
- [STOP-RT-ACCOUNT](#stop-rt-account) - Related to STOP-RT-ACCOUNT

---

### START-SPOOLING

**Description:** Start the spooling program for a peripheral, which is now reserved by the spooling program. Print the files already in the spooling queue and those put in later.


**Format:** `@START-SPOOLING <peripheral file name>
`

**Parameters:**

- `peripheral file name`: The spooling device.

**Rules:**

- Permitted only for user SYSTEM.
- The number of pages given to spooling files is compared to the number of unused pages belonging to user SYSTEM. If the latter number is smaller, the number of pages given to spooling will be reduced accordingly (see also SPOOLING-PAGES-LEFT).
- If more than one version of the file is a peripheral file, the spooling programs for all peripheral versions of the file are started. One specific peripheral file can be selected by including a version number in the file name.
- An error message appears if the <peripheral file name> is not the name of a peripheral or if no spooling program exists for the peripheral.
- RTENTER must be given before this command is executed.
- If there are files in the spooling queue, START-SPOOLING causes immediate output at the spooling device.

**Related Commands:**

- [STOP-SPOOLING](#stop-spooling) - Related to STOP-SPOOLING

---

### START-TADADM

**Description:** COSMOS command. Starts the TADADM RT-program, opens the *TADADM port and links to SINTRAN. See manual COSMOS System Supervisor, VD-30.025.


**Format:** `@START-TADADM
`

**Rules:**

- Permitted for user SYSTEM only.

**Related Commands:**

- [STOP-TADADM](#stop-tadadm) - Related to STOP-TADADM
- TADADM - Related to TADADM

---

### STATUS

**Description:** Print the register contents of the background program.


**Format:** `@STATUS
`

**Rules:**

- Permitted for all users.
- The printout is as follows - program counter, post-index register, temporary register, accumulator, double accumulator, subroutine link address register, status register, pre-index (base) register.

**Related Commands:**

- [LOOK-AT](#look-at) - Related to LOOK-AT

---

### STOP

**Description:** Stop execution of a program.

**Format:** `@STOP <program name>`

**Parameters:**

- `program name`: Name of the program to be stopped.

**Rules:**

- Permitted only for user SYSTEM.
- The program must be running.
- The program will be terminated immediately.

**Related Commands:**

- [START](#start) - Related to START

---

### STOP-ACCOUNTING

**Description:** Stop the accounting of system resources.


**Format:** `@STOP-ACCOUNTING <background>[,<RT>][,<ND-500>][,<spooling>]
`

**Parameters:**

- `background`: Stop accounting of background programs (if started).
- `RT`: Stop accounting of RT-programs (if started).
- `ND-500`: Stop accounting of ND-500 programs (if started).
- `spooling`: Stop accounting of spooling usage (if started).

**Rules:**

- Permitted only for user SYSTEM.
- The accounting file is not affected.

**Related Commands:**

- [INIT-ACCOUNTING](#init-accounting) - Related to INIT-ACCOUNTING
- [START-ACCOUNTING](#start-accounting) - Related to START-ACCOUNTING

---

### STOP-HISTOGRAM

**Description:** Turn off sampling for the histogram.


**Format:** `@STOP-HISTOGRAM
`

**Rules:**

- Permitted for all users.
- STOP-HISTOGRAM is performed as part of PRINT-HISTOGRAM.

**Related Commands:**

- [DEFINE-HISTOGRAM](#define-histogram) - Related to DEFINE-HISTOGRAM
- [PRINT-HISTOGRAM](#print-histogram) - Related to PRINT-HISTOGRAM
- [START-HISTOGRAM](#start-histogram) - Related to START-HISTOGRAM

---

### STOP-PRINT

**Description:** Stop the current printout and await further commands.


**Format:** `@STOP-PRINT <peripheral file name>
`

**Parameters:**

- `peripheral file name`: The spooling device.

**Rules:**

- Permitted only for user SYSTEM and the user who appended the file being printed.
- The current print buffer is finished before the printing stops (@ABORT-PRINT causes immediate stop).

**Related Commands:**

- [ABORT-PRINT](#abort-print) - Related to ABORT-PRINT
- [BACKSPACE-PRINT](#backspace-print) - Related to BACKSPACE-PRINT
- [FORWARD-SPACE-PRINT](#forward-space-print) - Related to FORWARD-SPACE-PRINT
- RESTART-PRINT - Related to RESTART-PRINT
- [START-PRINT](#start-print) - Related to START-PRINT

---

### STOP-PROGRAM-LOG

**Description:** Stop logging programs and print report on a file. The program names are printed and the percentage of the measured time during which the program has been active. If combined with @DEFINE-SYSTEM-HISTOGRAM, it produces a list of sampling distribution on various interrupt levels.


**Format:** `@STOP-PROGRAM-LOG <output file>
`

**Parameters:**

- `output file`: Destination of the report (default = TERMINAL).

**Rules:**

- Permitted only for user SYSTEM.

**Related Commands:**

- [START-PROGRAM-LOG](#start-program-log) - Related to START-PROGRAM-LOG

**Example:**

```
@STOP-PROGRAM-LOG,

DUMMY:        63        10258
STSIN:        00        0
RTERR:        00        0
RTSLI:        01        114
RWRTI:        00        0
RWRT2:        00        0
RWRT3:        00        0
SCOM1:        00        0
RCOM1:        00        2
SCOM2:        00        0
RCOM2:        00        0
BAK01:        00        0
BAK02:        00        0
BAK03:        14        2341
BAK04:        00        0
BAK05:        01        132
BAK06:        02        253
BAK07:        02        258
BAK08:        18        2949
BAK09:        00        7
BAK10:        00        0
BAK11:        00        0

```

---

### STOP-RT-ACCOUNT

**Description:** Stop accounting for individual RT-programs.


**Format:** `@STOP-RT-ACCOUNT <RT-program>
`

**Parameters:**

- `RT-program`: The name of an RT-program which will no longer be logged.

**Rules:**

- Each RT-program is associated with a project password.
- Available for users SYSTEM and RT.

**Related Commands:**

- [INIT-ACCOUNTING](#init-accounting) - Related to INIT-ACCOUNTING
- [START-ACCOUNTING](#start-accounting) - Related to START-ACCOUNTING
- [START-RT-ACCOUNT](#start-rt-account) - Related to START-RT-ACCOUNT

---

### STOP-SPOOLING

**Description:** Stop the spooling program for a peripheral and release the peripheral from the spooling program.


**Format:** `@STOP-SPOOLING <peripheral file name>
`

**Parameters:**

- `peripheral file name`: The spooling device.

**Rules:**

- Permitted only for user SYSTEM.
- The spooling program will abort after the current printing is finished, or, immediately if the spooling queue is empty.
- Files can still be appended to the queue. The spooling program resumes printing the files in the queue when START-SPOOLING is given.

**Related Commands:**

- [ABORT-PRINT](#abort-print) - Related to ABORT-PRINT
- [START-SPOOLING](#start-spooling) - Related to START-SPOOLING
- [STOP-PRINT](#stop-print) - Related to STOP-PRINT

---

### STOP-SYSTEM

**Description:** Stop the system.


**Format:** `@STOP-SYSTEM
`

**Rules:**

- Permitted only for user SYSTEM.
- The command name (STOP-SYSTEM) may not be abbreviated.
- All hardware registers are saved before the system goes into stop mode.
- The system can be restarted by typing 20!. On restart, logged-in users can continue their programs. No restart-up procedure is necessary and no information is lost.
- To turn off the system for a longer period - a) Log out all users, b) Press STOP and MASTER CLEAR on operator's panel, c) Stop the disk only if temperature, energy, and/or noise conditions require it. The system should normally be left running, even over night. It should only be turned off for hardware maintenance, etc.

**Related Commands:**

- [COLD-START](#cold-start) - Related to COLD-START
- OPCOM - Related to OPCOM
- RESTART-SYSTEM - Related to RESTART-SYSTEM

---

### STOP-TADADM

**Description:** COSMOS command. Closes *TADADM port. For further details see the manual COSMOS System Supervisor, ND-30.025.


**Format:** `@STOP-TADADM
`

**Rules:**

- Permitted for user SYSTEM only.

**Related Commands:**

- [START-TADADM](#start-tadadm) - Related to START-TADADM
- TADADM - Related to TADADM

---

### SUSPEND

**Description:** Suspend execution of a program temporarily.

**Format:** `@SUSPEND <program name>`

**Parameters:**

- `program name`: Name of the program to be suspended.

**Rules:**

- Permitted only for user SYSTEM.
- The program must be running.
- The program can be resumed later.

**Related Commands:**

- RESUME - Related to RESUME

---

### TERMINATE

**Description:** Terminate execution of a program.

**Format:** `@TERMINATE <program name>`

**Parameters:**

- `program name`: Name of the program to be terminated.

**Rules:**

- Permitted only for user SYSTEM.
- The program must be running.
- The program will be terminated gracefully.

**Related Commands:**

- [STOP](#stop) - Related to STOP

---

### TEST-COMMAND

**Description:** A test command to verify schema validation

**Format:** `@TEST-COMMAND <parameter>`

**Parameters:**

- `parameter`: Test parameter description

**Rules:**

- Must have proper permissions

**Privileges:** Supervisor

**Related Commands:**

- ANOTHER-COMMAND - Another test command
- SPECIAL-COMMAND - Special test command

**Monitor Calls:**

- TEST-MON (99): Test monitor call

**Notes:**

This is a test command for schema validation.


**Aliases:**

TEST

---

### TIME

**Description:** Display the current system time.

**Format:** `@TIME`

**Rules:**

- Permitted for all users.
- The time is displayed in the system time format.

---

### TYPE

**Description:** Display the contents of a file.

**Format:** `@TYPE <file name>`

**Parameters:**

- `file name`: Name of the file to be displayed.

**Rules:**

- Permitted for all users.
- The file must exist.
- The user must have read permission for the file.

**Related Commands:**

- LIST - Related to LIST

---

### UNLOAD

**Description:** Unload a program from memory.

**Format:** `@UNLOAD <program name>`

**Parameters:**

- `program name`: Name of the program to be unloaded.

**Rules:**

- Permitted only for user SYSTEM.
- The program must be loaded in memory.
- The program will be removed from memory.

**Related Commands:**

- LOAD - Related to LOAD

---

### UNLOCK

**Description:** Unlock a file that has been locked.

**Format:** `@UNLOCK <file name>`

**Parameters:**

- `file name`: Name of the file to be unlocked.

**Rules:**

- Permitted for all users.
- The file must exist.
- The file must be locked.
- The user must have permission to unlock the file.

**Related Commands:**

- LOCK - Related to LOCK

---

### UNMOUNT

**Description:** Unmount a device from the system.

**Format:** `@UNMOUNT <device name>`

**Parameters:**

- `device name`: Name of the device to be unmounted.

**Rules:**

- Permitted only for user SYSTEM.
- The device must be mounted.
- The device must not be in use.

**Related Commands:**

- MOUNT - Related to MOUNT

---

### UNRESERVE

**Description:** Unreserve a resource that has been reserved.

**Format:** `@UNRESERVE <resource>`

**Parameters:**

- `resource`: Name of the resource to be unreserved.

**Rules:**

- Permitted for all users.
- The resource must exist.
- The resource must be reserved.
- The user must have permission to unreserve the resource.

**Related Commands:**

- RESERVE - Related to RESERVE

---

## Sintran-Service-Program

### ASCII-DUMP

**Description:** Dump each byte of an area as ASCII characters. Use OCTAL-DUMP to have the area dumped as octal numbers.

**Format:** `*ASCII-DUMP <area>[,<segment number>],<lower address>,<upper address>,<output file>
`

**Parameters:**

- `area`: In SINTRAN III/VSE, legal parameters are MEMORY, IMAGE, SAVE AREA or SEGMENT. In SINTRAN III/VSX, legal parameters are SEGMENT or ALT-SEGMENT.
- `segment number`: If SEGMENT (or ALT-SEGMENT) is specified as <area>, the octal segment number should be entered.
- `lower address`: The lower address of the area to dump.
- `upper address`: The upper address of the area to dump.
- `output file`: The file where the dump should be written. Default is your terminal.

**Related Commands:**

- [OCTAL-DUMP](#octal-dump) - Dump each byte of an area as octal numbers

---

### BACKGROUND-ALLOCATION-UTILITIES

**Description:** The background allocation system makes it possible to run SINTRAN III with a larger number of terminals/TADS than the number of background processes. When a background process is requested (user presses ESCAPE), the first free background process, if any, will be allocated.

**Format:** `*BACKGROUND-ALLOCATION-UTILITIES
`

---

### CC

**Description:** This command has no effect. It is normally used to comment mode and batch files.

**Format:** `*CC <text>
`

**Parameters:**

- `text`: The text can be any printable characters. It is terminated by carriage return.

**Related Commands:**

- [CC](#cc) - Comment command in SINTRAN III
- [CC](#cc) - Comment command in ND-500 Monitor

---

### CHANGE-BUFFER-SIZE

**Description:** Change the length of the ring buffer of a device. The total buffer area is limited depending on system configuration. If a substantial increase in size is desired, calculations should be made.

**Format:** `*CHANGE-BUFFER-SIZE <logical device number>,<input/output>,<buffer size>,<image?>,<save area?>
`

**Parameters:**

- `logical device number`: The octal logical device number identifying the device.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `buffer size`: The octal length of the ring buffer in words or bytes.
- `image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- For SINTRAN III/VSX, the following rule applies to terminals: The size of the data fields (input+output), plus the buffers (input+output), must fit inside one page. This gives the maximum size of the sum of input and output buffers to 34248 bytes. The default buffer size for terminals are 1348 bytes in input and 2703 bytes in output.

**Related Commands:**

- [CHANGE-GPIB-BUFFERSIZE](#change-gpib-buffersize) - Change GPIB buffer size

---

### CHANGE-DATAFIELD

**Description:** Change the contents of variables in the data fields connected to devices. The displacements of items in each data field can be entered as symbolic names.


**Format:** `*CHANGE-DATAFIELD <logical device number>,<input/output>,<memory?>,<image?>,<save area?> (subcommands for patching)
`

**Parameters:**

- `logical device number`: The octal logical device number identifying the device.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.
- `subcommands for patching`: These subcommands follow rules 3, 4 and 6 of the @LOOK-AT command. The address specified must be a relative octal address within the data field or the symbolic name of an item within the data field.

**Related Commands:**

- [CHANGE-TABLE](#change-table) - Change values in internal tables
- [CHANGE-VARIABLE](#change-variable) - Examine and change system variables
- [LOOK-AT](#look-at) - Look at memory contents

---

### CHANGE-GPIB-BUFFERSIZE

**Description:** Change the size of buffers used on a GPIB controller.

**Format:** `*CHANGE-GPIB-BUFFERSIZE <controller no.>,<user buffer size (oct)>,<DMA buffer size (oct)>
`

**Parameters:**

- `controller no.`: GPIB controller.
- `user buffer size (oct)`: User buffer size in (octal) words.
- `DMA buffer size (oct)`: DMA buffer size in (octal) words.

**Related Commands:**

- [CHANGE-BUFFER-SIZE](#change-buffer-size) - Change the length of the ring buffer of a device

---

### CHANGE-TABLE

**Description:** Change values in one of four internal tables: 
  USER-RESERVED-DEVICE-NUMBERS: A table of hardware device numbers which should not be accessed at system start-up time. An element in this table is defined by the first and last device number of an interval.
  USER-RESERVED-MEMORY-AREA: This table specifies memory pages which should not be used for system tables (memory map, device buffers, etc.). An element in this table is defined by the first and last physical page of an area not to be used for system tables.
  MEMORY-AREA-UNAVAILABLE-FOR-SWAPPING: This table specifies memory areas which should not be used for swapping. An element in this table is defined by the first and last physical pages of an area not to be used for swapping.
  MEMORY-AREA-INVISIBLE-FOR-THIS-SYSTEM: This table specifies memory areas invisible for this system.

Available subcommands: 
  LIST-TABLE: List the contents of the table.
  CHANGE-TABLE: Change the name of the table.
  CHANGE-ELEMENT: Change the value of an element in the table.
  DELETE-ELEMENT: Delete an element from the table.
  INSERT-ELEMENT: Insert a new element in the table.
  CLEAR-TABLE: Clear the table.
  EXIT: Exit the command.
  HELP: Display help information.


**Format:** `*CHANGE-TABLE <table>`

**Parameters:**

- `table`: The name of one of the tables listed above.

**Related Commands:**

- [CHANGE-DATAFIELD](#change-datafield) - Change contents of variables in data fields
- [CHANGE-VARIABLE](#change-variable) - Examine and change system variables

---

### CHANGE-VARIABLE

**Description:** Examine and change system variables. These may be single variables or arrays. For some variables, the IMAGE AREA and/or SAVE AREA may be affected. Some variables may also affect SINTRAN III RESIDENT.
 
VARIABLE and MEANING:
----------------------
BUFERASE: Erase buffer option on/off
BYPINITC: Flag to bypass initial commands.
*CCFPAGE: First legal logical page for RT common.
*CCLPAGE: Last legal logical page for RT common.
*CNVRT: Address of logical device tables. SINTRAN III RESIDENT is affected for index values from 0-24.
CPULOOPTIME: Number of runs of the idle loop per second.
DVBFPAGE: First physical page of memory legal for device buffer.
*ENDCOR: Upper address of SINTRAN III RESIDENT.
*EXECURITY: Flags to show which security features to be used.
*EXTDS: Address of the extended ident code tables. SINTRAN III RESIDENT is affected for index values from 0-3.
FIXMAX: Maximum number of pages which can be fixed in physical memory simultaneously. Affects SINTRAN III RESIDENT.
*IDNTS: Address of the ident code tables. Affects SINTRAN III RESIDENT for index values from 0-3.
IMASK: Value is a mask to be used for enabling internal interrupts, that is, the TRR IIE instruction.
LCACHLIM: Change the lower limit of the CACHE-INHIBIT-LIMIT.
LOADI: Set to 0 if the RT-Loader shall initialize RTFIL. SINTRAN III RESIDENT is affected.
MAXP: Maximum number of pages in physical memory for a demand segment. SINTRAN III RESIDENT is affected.
MINSWPAGES: Minimum number of pages of memory for swapping.
MXDVBUF: Maximum number of device buffers in this system.
NMATP: Number of RFA attempts before forced logout.
*RTPPAGE: First legal logical page number for RT-programs on page table 1.
*RTLPAGE: Last legal logical page number for RT-programs on page table 1.
SWPFLAG: Swapping and disk reservation as in the H-version.
*TABLES: Address of timer, background, batch and RT common table (CCTAB). SINTRAN III RESIDENT is affected for index from 0-3.
*TMCTAB: Monitor call types. See DEFINE-USER-MONITOR-CALL. The TMCTAB array is a byte array. The parameter <index> is a word index. Thus two bytes are changed by one command. SINTRAN III RESIDENT is affected for index values from 0-107.
UCACHLIM: Change the upper limit of the CACHE-INHIBIT-LIMIT.
UNAFLAG: Flag set if system is unavailable after the command @SET-UNAVAILABLE. SINTRAN III RESIDENT is affected.
USEGADR: Address of the first free entry in the segment table.

Symbolic names marked (*) are not available in the VSX-version.


**Format:** `*CHANGE-VARIABLE <variable name>[,<index>],<value>[,<memory?>],<image?>,<save area?>
`

**Parameters:**

- `variable name`: The system variables shown in the following table can be examined and modified.
- `index`: An octal index will be requested if the <variable name> is an array.
- `value`: New octal value. The old value is default.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [CHANGE-DATAFIELD](#change-datafield) - Change contents of variables in data fields
- [CHANGE-TABLE](#change-table) - Change values in internal tables
- [LOOK-AT](#look-at) - Look at memory contents

---

### CLEAR-ENTER-COUNT

**Description:** Clear the enter count for a terminal and allow attempts to log in on that terminal again.

**Format:** `*CLEAR-ENTER-COUNT <logical device number>,<memory?>
`

**Parameters:**

- `logical device number`: The decimal logical device number of the terminal.
- `memory?`: Answer YES or NO to whether memory should be modified. The default answer is NO.

**Related Commands:**

- [SET-MAX-ENTER-COUNT](#set-max-enter-count) - Set the maximum number of consecutive unsuccessful attempts a user can make to log in on a terminal.

---

### CPU-LOG

**Description:** Report CPU activity.


**Format:** `*CPU-LOG <interval in seconds>,<output file>
`

**Parameters:**

- `interval in seconds`: Interval. The default is 30 seconds.
- `output file`: The default is terminal.

---

### CREATE-LAMU

**Description:** Creates a LAMU with an entry in the LAMU table.


**Format:** `*CREATE-LAMU <LAMU id>,<size>,<physical address>
`

**Parameters:**

- `LAMU id`: A number to identify the LAMU in the LAMU table. The number will be accepted if it is unused and inside the legal range starting from 1. If 0 is entered, the system selects a LAMU id.
- `size`: Octal number of pages in the range 1-2003.
- `physical address`: Specify the first physical page for the LAMU. Legal values are all existing physical pages currently used as LAMU areas. The system selects the first LAMU area large enough for the LAMU if 0 is entered.

**Related Commands:**

- [CREATE-SYSTEM-LAMU](#create-system-lamu) - Related to CREATE-SYSTEM-LAMU
- [DELETE-LAMU](#delete-lamu) - Related to DELETE-LAMU
- [LAMU-INFORMATION](#lamu-information) - Related to LAMU-INFORMATION
- [PAGES-TO-LAMU](#pages-to-lamu) - Related to PAGES-TO-LAMU
- [PROTECT-LAMU](#protect-lamu) - Related to PROTECT-LAMU

---

### CREATE-SYSTEM-LAMU

**Description:** Creates a system LAMU with an entry in the LAMU table.


**Format:** `*CREATE-SYSTEM-LAMU <LAMU id>,<size>,<physical start page>
`

**Parameters:**

- `LAMU id`: A number to identify the LAMU in the LAMU table. The number will be accepted if it is unused and inside the legal range starting from 1. If 0 is entered, the system selects a LAMU identifier.
- `size`: Octal number of pages in the range 1-2003.
- `physical start page`: Specify the first physical page for the LAMU. Memory for system LAMUs are allocated from the swapping area. The system selects the first free area large enough for the LAMU if 0 is entered.

**Related Commands:**

- [CREATE-LAMU](#create-lamu) - Related to CREATE-LAMU
- [DELETE-LAMU](#delete-lamu) - Related to DELETE-LAMU
- [LAMU-INFORMATION](#lamu-information) - Related to LAMU-INFORMATION
- [PAGES-TO-LAMU](#pages-to-lamu) - Related to PAGES-TO-LAMU
- [PROTECT-LAMU](#protect-lamu) - Related to PROTECT-LAMU

---

### DEFINE-BATCH-SUPERVISOR

**Description:** Define an RT-program to be started each time a batch job is terminated.


**Format:** `*DEFINE-BATCH-SUPERVISOR <RT-program>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `RT-program`: An octal RT-description address or an RT-program name.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- DEFINE-TERMINATION-HANDLING - Related to DEFINE-TERMINATION-HANDLING

---

### DEFINE-HDLC-BUFFER

**Description:** Allocate buffer for specified HDLC interface.


**Format:** `*DEFINE-HDLC-BUFFER <logical device number>,<buffer size>
`

**Parameters:**

- `logical device number`: Logical device number for HDLC interface.
- `buffer size`: Buffer size.

**Related Commands:**

- [LIST-HDLC-BUFFER](#list-hdlc-buffer) - Related to LIST-HDLC-BUFFER

---

### DEFINE-PROMPT-STRING

**Description:** Define a prompt string to be printed instead of an @ in SINTRAN III remote mode. The system name may, for example, precede the @. The string may also be declared to replace the default prompt on the local system.


**Format:** `*DEFINE-PROMPT-STRING <string>,<local mode?>,<memory>,<save area?>
`

**Parameters:**

- `string`: The string to replace the old one. The text is terminated with an apostrophe (').
- `local mode?`: Answer YES or NO to whether the string also should be used in local mode.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- [DEFINE-TITLE](#define-title) - Related to DEFINE-TITLE

---

### DEFINE-RTCOMMON-SIZE

**Description:** Define the size of the RT-common area in pages.


**Format:** `*DEFINE-RTCOMMON-SIZE <number of pages>,<first physical page>,<image?>,<save area?>
`

**Parameters:**

- `number of pages`: The new octal size of the RT-common area.
- `first physical page`: The page in physical memory where RT-common should start. In addition to the pages generated for the system, only 8 pages can be added. The default value is the upper end of physical memory.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Rules:**

- The size can be increased by up to 10B pages in addition to the RT-common size defined at system generation.

---

### DEFINE-SEGMENT-FILE

**Description:** Connect a segment file number to a segment file. This is needed for the RT-Loader to access the file.


**Format:** `*DEFINE-SEGMENT-FILE <memory?>,<save area?>,<segment file number>,<segment file name>[,<redefine segment file?>]
`

**Parameters:**

- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.
- `segment file number`: Specify a segment file number in the range 0-3.
- `segment file name`: Any existing contiguous file.
- `redefine segment file?`: Only requested if the <segment file number> is already defined. Specify YES or NO. The default answer is NO.

**Rules:**

- User SYSTEM and user RT must have read and write access to to the segment file.
- The page address of all the pages in the segment file must be less than 77777B.
- The command must not be used in batch jobs.

**Related Commands:**

- [DELETE-SEGMENT-FILE](#delete-segment-file) - Related to DELETE-SEGMENT-FILE

---

### DEFINE-TIMESLICE

**Description:** Define the length of the time slices of the background processes which control the terminal and batch processors. The unit used in the parameters is 10 basic time units, that is, 20 milliseconds.


**Format:** `*DEFINE-TIME-SLICE <image?>,<save area?>,<change timeslice parameters (yes/no?)>
`

**Parameters:**

- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.
- `change timeslice parameters (yes/no?)`: If you answer yes, you will be asked for new values for the time slice parameters.

**Example:**

```
@SINTRAN-SERVICE-PROGRAM
*DEFINE-TIME-SLICE
IMAGE? YES
SAVE-AREA? NO
CHANGE TIMESLICE PARAMETERS [YES/NO] [DEFAULT IS NO]: YES
                                                                 IMAGE
PRIORITY FOR OWNER OF SYSTEM RESOURCES WHICH
         ARE WAITED FOR BY OTHER PROGRAMS        [1B - 77B]      /     67/:
NO. OF BASIC TIME UNITS IN ONE TIMESLICE UNIT [1B - 400B]        /     14/:
LOWEST PRIORITY BEFORE GETTING RAISED ON BREAK [1B - 70B]        /     40/:
LOWEST TIME COUNT BEFORE GETTING HASHED        [1B - 400B]       /     22/:
           BIT MASK USED WHEN HASHING          [1B - 177B]       /     17/:
CHANGE TIMESLICE ELEMENTS [YES/NO] [DEFAULT IS NO]? YES
TIMESLICE CLASS                [0B -   7B] /       0/:"
                                             IMAGE     "
ESCAPE ELEMENT FOR THIS CLASS [0B - 37B] / 30/: 30
BREAK ELEMENT FOR THIS CLASS [0B - 37B] / 30/: 36
TIMESLICE ELEMENT TO CHANGE    [0B - 37B] / 30/: 36
PRIORITY FOR THIS ELEMENT      [1B - 77B] / 10/: 10
TIME COUNT FOR THIS ELEMENT    [1B - 400B] /    2/: 2
POINTER TO NEXT ELEMENT        [0B - 37B] / 31/: 31
CHANGE NEXT ELEMENT [YES/NO] [DEFAULT IS NO]: YES
TIMESLICE ELEMENT NO.     31                             
PRIORITY FOR THIS ELEMENT            [1B - 77B] /    6/: 6
TIME COUNT FOR THIS ELEMENT          [1B - 400B] /   4/: 4
POINTER TO NEXT ELEMENT              [0B - 37B] / 32/: 32
CHANGE NEXT ELEMENT [YES/NO] [DEFAULT IS NO]: YES
TIMESLICE ELEMENT NO.     32                             
PRIORITY FOR THIS ELEMENT            [1B - 77B] /     1/: 1
TIME COUNT FOR THIS ELEMENT          [1B - 400B] /   30/: 30
POINTER TO NEXT ELEMENT              [0B - 37B] /   31/: 31
CHANGE NEXT ELEMENT [YES/NO] [DEFAULT IS NO]:
MORE CLASSES (YES/NO) [DEFAULT IS NO]: NO
*EXIT
@ 
```

---

### DEFINE-TITLE

**Description:** Define the string to be output in addition to system version string when a user logs in on a terminal. The string is also output as part of the spooling header and as response to the command @LIST-TITLE.


**Format:** `*DEFINE-TITLE <text>,<memory?>,<save area?>
`

**Parameters:**

- `text`: Any printable characters terminated by an apostrophe ('). The character $ will start a new line on output. Carriage return on input is ignored. An apostrophe only specifies no text.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Rules:**

- The maximum is 120 characters including the apostrophe.
- The command must not be used in batch jobs.

**Related Commands:**

- [DEFINE-PROMPT-STRING](#define-prompt-string) - Related to DEFINE-PROMPT-STRING
- LIST-TITLE - Related to LIST-TITLE

**Example:**

```
@SINTRAN-SEVICE-PROGRAM
*DEFINE-TITLE $ND-570.2323 $SYSTEM NUMBER 6323$'
*EXIT
@LIST-TITLE
SINTRAN III - VSX/500 K

ND-570.2323
SYSTEM NUMBER 6323

REVISION:   101300B
CPU (SYSTEM NUMBER):       6323
GENERATED:   16.39.00       15 MAY        1986
@ 
```

---

### DEFINE-USER-MONITOR-CALL

**Description:** Define number, entry point address and type of user-defined monitor calls. The code should be assembled using FMAC or DMAC.

**Format:** `*DEFINE-USER-MONITOR-CALL <monitor call number>,<start address>,<type>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `monitor call number`: The octal number of the monitor call. User defined monitor calls may be in the range 170-177.
- `start address`: The octal entry point of the monitor routine.
- `type`: Specify 1 if the monitor call should be available to RT-programs only and 16 if it also should be available to background programs. The default value is 1.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- DEFINE-USER-MONITOR-CALL is not available in SINTRAN III/VSX.
- The subroutine must reside in SINTRAN III RESIDENT.
- If TPS is installed, monitor call numbers 170-177 are occupied.
- In SINTRAN III/VSX, user-defined monitor calls are still available, but the user must update the monitor call tables himself.

---

### DEFINE-USER-RESTART-PROGRAM

**Description:** Define the first RT-program to be started by SINTRAN III after each power fail. The RT-program may, for example, request the clock to be updated.


**Format:** `*DEFINE-USER-RESTART-PROGRAM <program>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `program`: An octal RT-description address or an RT-program name.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- [LIST-USER-RESTART-PROGRAMS](#list-user-restart-programs) - Related to LIST-USER-RESTART-PROGRAMS
- [NEXT-USER-RESTART-PROGRAM](#next-user-restart-program) - Related to NEXT-USER-RESTART-PROGRAM

---

### DEFINE-USER-RESTART-SUBROUTINE

**Description:** Define the start address of a subroutine to be called when SINTRAN III is restarted after a power fail.


**Format:** `*DEFINE-USER-RESTART-SUBROUTINE <restart address>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `restart address`: The octal entry point of the subroutine.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Rules:**

- The subroutine must reside in SINTRAN III RESIDENT.

**Related Commands:**

- [DEFINE-USER-RESTART-PROGRAM](#define-user-restart-program) - Define the first RT-program to be started by SINTRAN III after each power fail.
- [DEFINE-USER-START-SUBROUTINE](#define-user-start-subroutine) - Define the address of a subroutine to be executed when the system is started.

---

### DEFINE-USER-START-SUBROUTINE

**Description:** Define the address of a subroutine to be executed when the system is started.


**Format:** `*DEFINE-USER-START-SUBROUTINE <start address>,<image?>,<save area?>
`

**Parameters:**

- `start address`: The octal entry point of the subroutine.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Rules:**

- The subroutine must reside in SINTRAN III RESIDENT or on the operator communication segment, that is, segment 3.

**Related Commands:**

- [DEFINE-USER-RESTART-SUBROUTINE](#define-user-restart-subroutine) - Define the start address of a subroutine to be called when SINTRAN III is restarted after a power fail.

---

### DELETE-LAMU

**Description:** Deletes a selected LAMU from the LAMU table.


**Format:** `*DELETE-LAMU <LAMU id>
`

**Parameters:**

- `LAMU id`: The number identifying the LAMU in the LAMU table.

**Rules:**

- The LAMU cannot be in use by RT-programs.
- The physical pages of the LAMU will remain in the LAMU area.

**Related Commands:**

- [CREATE-LAMU](#create-lamu) - Creates a LAMU with an entry in the LAMU table.
- [CREATE-SYSTEM-LAMU](#create-system-lamu) - Creates a system LAMU with an entry in the LAMU table.
- [LAMU-INFORMATION](#lamu-information) - Provides information about LAMUs in the system.
- [PAGES-FROM-LAMU](#pages-from-lamu) - Move pages from the LAMU area to the swapping area.
- [PROTECT-LAMU](#protect-lamu) - Change LAMU protection.

---

### DELETE-SEGMENT-FILE

**Description:** Set a segment file unavailable for the RT-Loader. The file is not deleted from the directory. Use the @DELETE-FILE command for this purpose.


**Format:** `*DELETE-SEGMENT-FILE <memory?>,<save area?>,<segment file number>
`

**Parameters:**

- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.
- `segment file number`: The number of one of the four segment files, that is, a number from 0-3.

**Rules:**

- A defined segment file must not be deleted without previously having been undefined with DELETE-SEGMENT-FILE.

**Related Commands:**

- [DEFINE-SEGMENT-FILE](#define-segment-file) - Define a segment file to be used by the RT-Loader.

---

### DISC-ACCESS-LOG

**Description:** Log all or a selection of disk accesses.


**Format:** `*DISC-ACCESS-LOG
`

**Rules:**

- The Disk Access Log is an option to be ordered on the SINTRAN III order form.

**Example:**

```
The command *DISC-ACCESS-LOG has several subcommands:

DEFINE-DISC-ACCESS-LOG <disc access log file>,
                       <small or big record size on disc log file
                          (default is big)>,
                         <log all disc accesses (default is yes)>,
                        [<count disc accesses to one controller only
                         (default is no)>],
                       [<logical device number of disc to log (octal)>],
                        [<count disc accesses to one disc unit only
                         (default is no)>],
                        [<drive number of drive to log>],
                        [<log only write accesses (default is no)>],
                        [<log only read accesses (default is no)>],
                       [<log only accesses to a limited part of the disc
                         (default is no)>},
                        [<first disc address in the disc part to log
                         (octal only)>],
                        [<last disc address in the disc part to log
                         (octal onlyl>l

      Define the parameters for the disk log. Logfile must be a
      contiguous file. Record size is either 4 (small) or 8 (big) words.

START-DISC-ACCESS-LOG
    Start logging of disk accesses if a log has been defined.

STOP-DISC-ACCESS-LOG
      Stop logging of disk accesses.

START-DISC-ACCESS-COUNTER <count all disc accesses (default is yes)>,
                           [<count disc accesses to one controller only
                            (default is no)>},
                       [<logical device number of disc to log (octal)>l,
                           [<count disc accesses to one disc unit only
                            (default is no)>},
                           [<drive number of drive to log>]

      Define and start a simple disk access log which only counts the
      number of read and write accesses.

STOP-DISC-ACCESS-COUNTER
    Stop the simple disk access log.

CLEAR-DISC-ACCESS-COUNTER
       Clear the counters used by the simple disk access 109.

DISC-ACCESS-COUNTER
     Report the values of the counters used by the simple disk access
     109.

DISC-DRIVER-ERROR-INFORMATION <logical device number>

       Report the values of certain error variables in the disk driver.

DISC-ERROR-STATUS <logical device number>,<unit number>

       Report the error status in the disk data field.

LOG-DISC-ACCESS-COUNTER <interval in seconds (default is 60 secs)>

       Report the values of the counters used by the simple disk access
       log at specified intervals. This command must be terminated by
       pressing <escape>.

EXIT

HELP 
```

---

### DUMP-RT-DESCRIPTION

**Description:** Dump the symbolic names and the contents of locations in an RT-description.

**Format:** `*DUMP-RT-DESCRIPTION <program>,<area>,<output file>
`

**Parameters:**

- `program`: An octal RT-description address or an RT-program name.
- `area`: Legal parameters are MEMORY, IMAGE, and SAVE AREA.
- `output file`: The file where the information should be dumped. Default is your terminal.

**Related Commands:**

- [DUMP-SEGMENT-TABLE-ENTRY](#dump-segment-table-entry) - Related to DUMP-SEGMENT-TABLE-ENTRY
- [LIST-RT-DESCRIPTION](#list-rt-description) - Related to LIST-RT-DESCRIPTION

---

### DUMP-SEGMENT-TABLE-ENTRY

**Description:** Dump the symbolic name and contents of locations in a segment table entry.

**Format:** `*DUMP-SEGMENT-TABLE-ENTRY <segment number>,<area>,<output file>
`

**Parameters:**

- `segment number`: The octal number of the segment to be dumped.
- `area`: Legal parameters are MEMORY, IMAGE or SAVE AREA.
- `output file`: The file where the information should be dumped. Default is your terminal.

**Related Commands:**

- [DUMP-RT-DESCRIPTION](#dump-rt-description) - Related to DUMP-RT-DESCRIPTION
- LIST-SEGMENT - Related to LIST-SEGMENT

**Example:**

```
*DUMP-SEGMENT-TABLE-ENTRY 30,MEMORY,,
SEGLINK:                      O
BPAGELINK:                    O
LOGADR:                    1074
MADE:                       325
FLAG:                    162003 
```

---

### EXIT

**Description:** Leave the SINTRAN SERVICE PROGRAM and return to SINTRAN III.

**Format:** `*EXIT
`

**Related Commands:**

- [EXIT](#exit) - Related to EXIT

---

### FIND-CPULOOPTIME

**Description:** Find number of "CPU loops" per second and store it in the SINTRAN III variable CPULOOPTIME.

**Format:** `*FIND-CPULOOPTIME
`

**Rules:**

- This command should only be issued when there is no activity on the system.

---

### HELP

**Description:** List all SINTRAN SERVICE PROGRAM commands. The command is identical to LIST-SERVICE-COMMANDS.

**Format:** `*HELP <command>,<output file>
`

**Parameters:**

- `command`: List commands matching command name given.
- `output file`: The file where the commands should be listed. Default is your terminal.

**Rules:**

- The order of the parameters is changed from previous versions.

**Related Commands:**

- [LIST-SERVICE-COMMANDS](#list-service-commands) - Related to LIST-SERVICE-COMMANDS
- [HELP](#help) - Related to HELP
- [HELP](#help) - Related to HELP

---

### INITIALIZE-SYSTEM-SEGMENT

**Description:** Fetch a new copy of the system segment for a terminal. The segment is transferred from the save area to the segment file.

**Format:** `*INITIALIZE-SYSTEM-SEGMENT <segment (name or number(oct))>
`

**Parameters:**

- `segment (name or number(oct))`: The segment name or (octal) number of a system segment.

**Rules:**

- This command will only be executed if the corresponding background process is passive. Otherwise, the message BACKGROUND PROGRAM ACTIVE will be given.

---

### INSERT-IN-BACKGROUND-TABLE

**Description:** Insert a device into the background table.


**Format:** `*INSERT-IN-BACKGROUND-TABLE <logical device number>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `logical device number`: An octal logical device number.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Rules:**

- There must be a free entry in the background table.

**Related Commands:**

- [REMOVE-FROM-BACKGROUND-TABLE](#remove-from-background-table) - Remove a device from the background table.

---

### INSERT-IN-EXTENDED-IDENT-TABLE

**Description:** Insert an entry in the extended ident code table of a hardware interrupt level.


**Format:** `*INSERT-IN-EXTENDED-IDENT-TABLE <level>,<logical device number>,<input/output>,<ident code>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `level`: The octal interrupt level.
- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `ident code`: The new octal ident code.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-IDENT-TABLE](#insert-in-ident-table) - Insert an entry in the ident code table of a hardware interrupt level.
- [REMOVE-FROM-EXTENDED-IDENT-TABLE](#remove-from-extended-ident-table) - Remove an entry from the extended ident code table of a hardware interrupt level.

---

### INSERT-IN-IDENT-TABLE

**Description:** Insert an entry in the ident code table of a hardware interrupt level.


**Format:** `*INSERT-IN-IDENT-TABLE <level>,<logical device number>,<input/output>,<ident code>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `level`: The octal interrupt level.
- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `ident code`: The new octal ident code.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-EXTENDED-IDENT-TABLE](#insert-in-extended-ident-table) - Insert an entry in the extended ident code table of a hardware interrupt level.
- [REMOVE-FROM-IDENT-TABLE](#remove-from-ident-table) - Remove an entry from the ident code table of a hardware interrupt level.

**Example:**

```
@SINTRAN-SERVICE-PROGRAM
*INSERT-IN-IDENT-TABLE 12, 5, OUTPUT, 3
*REMOVE-FROM-IDENT-TABLE 12, 5, OUTPUT 
```

---

### INSERT-IN-IOX-TABLE

**Description:** Make a hardware device number available for @EXECUTE-IOX and EXIOX (MON 31).


**Format:** `*INSERT-IN-IOX-TABLE <hardware device number>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `hardware device number`: An octal physical device number.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `image?`: Select whether image should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- [REMOVE-FROM-IOX-TABLE](#remove-from-iox-table) - Remove a hardware device number from the IOX table.

**Monitor Calls:**

- EXIOX (31): Execute input/output specified by input/output register contents.

---

### INSERT-IN-LOGICAL-UNIT-TABLE

**Description:** Insert a logical device number in the logical device table.


**Format:** `*INSERT-IN-LOGICAL-UNIT-TABLE <logical device number>,<input/output>,<data field>
`

**Parameters:**

- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `data field`: The octal address of the data field of the device.

**Rules:**

- There must be a free entry in the logical device table for the logical device number.

**Related Commands:**

- [REMOVE-FROM-LOGICAL-UNIT-TABLE](#remove-from-logical-unit-table) - Remove a device from the logical device table.

**Example:**

```
@SINTRAN-SERVICE-PROGRAM
*INSERT-IN-LOGICAL-UNIT-TABLE 5, INPUT, 14341
*REMOVE-FROM-LOGICAL-UNIT-TABLE 6, OUTPUT 
```

---

### INSERT-IN-TIME-SLICE

**Description:** Cause the background process priority to be changed dynamically. If a background process is not in the time slicer, it will run at a fixed priority.

**Format:** `*INSERT-IN-TIME-SLICE <logical device number>,<time slice class>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `logical device number`: The octal logical device number of a terminal, batch processor or communication device.
- `time slice class`: The number of time slice class to be used.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- A background process not in the time slicer will run on a fixed priority. This priority is defined by @PRIOR.

**Related Commands:**

- [INSERT-PROGRAM-IN-TIME-SLICE](#insert-program-in-time-slice) - Related to INSERT-PROGRAM-IN-TIME-SLICE
- [LIST-TIME-SLICED-PROGRAMS](#list-time-sliced-programs) - Related to LIST-TIME-SLICED-PROGRAMS
- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice) - Related to REMOVE-FROM-TIME-SLICE
- [REMOVE-PROGRAM-FROM-TIME-SLICE](#remove-program-from-time-slice) - Related to REMOVE-PROGRAM-FROM-TIME-SLICE
- PRIOR - Related to PRIOR

---

### INSERT-IN-TIMER-TABLE

**Description:** Insert a logical device number in the timer table.

**Format:** `*INSERT-IN-TIMER-TABLE <logical device number>,<input/output>
`

**Parameters:**

- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.

**Rules:**

- There must be a free entry in the timer table.

**Related Commands:**

- [REMOVE-FROM-TIMER-TABLE](#remove-from-timer-table) - Related to REMOVE-FROM-TIMER-TABLE

---

### INSERT-PROGRAM-IN-TIME-SLICE

**Description:** Set the specified RT-program to be time sliced.

**Format:** `*INSERT-PROGRAM-IN-TIME-SLICE <RT name>,<memory?>,<image>,<save area?>,<timeslice class>
`

**Parameters:**

- `RT name`: An RT-description address or an RT-program name.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.
- `timeslice class`: Which time slice class is to be used. Default = 0

**Related Commands:**

- [INSERT-IN-TIME-SLICE](#insert-in-time-slice) - Related to INSERT-IN-TIME-SLICE
- [LIST-TIME-SLICED-PROGRAMS](#list-time-sliced-programs) - Related to LIST-TIME-SLICED-PROGRAMS
- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice) - Related to REMOVE-FROM-TIME-SLICE
- [REMOVE-PROGRAM-FROM-TIME-SLICE](#remove-program-from-time-slice) - Related to REMOVE-PROGRAM-FROM-TIME-SLICE

---

### INSERT-SPOOLING-HEADER

**Description:** Cause the spooling program to print the spooling header and trailer between each file output by the spooling system.

**Format:** `*INSERT-SPOOLING-HEADER <spooling index>,<memory?>,<save area?>
`

**Parameters:**

- `spooling index`: The index of the spooling device as defined by the command SET-SPOOLING-DEVICE-NUMBER.
- `memory? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO. The area parameters apply only to the VSE-version; in the VSX-version, only the MEMORY area of SINTRAN III is affected.

**Related Commands:**

- [REMOVE-SPOOLING-HEADER](#remove-spooling-header) - Related to REMOVE-SPOOLING-HEADER

---

### LAMU-AREAS

**Description:** List the memory areas reserved for LAMUS.

**Format:** `*LAMU-AREAS <output file>
`

**Parameters:**

- `output file`: The file where the information should be dumped. Default is your terminal.

**Related Commands:**

- [LAMU-INFORMATION](#lamu-information) - Related to LAMU-INFORMATION

---

### LAMU-INFORMATION

**Description:** List all the relevant information of one or more LAMUs.

**Format:** `*LAMU-INFORMATION <output file>
`

**Parameters:**

- `output file`: The file where the information should be dumped. Default is your terminal.

**Related Commands:**

- [LAMU-AREAS](#lamu-areas) - Related to LAMU-AREAS

---

### LIST-HDLC-BUFFER

**Description:** List buffer for specified HDLC interface.

**Format:** `*LIST-HDLC-BUFFER <logical device number>
`

**Parameters:**

- `logical device number`: Logical device number for HDLC interface.

**Related Commands:**

- [DEFINE-HDLC-BUFFER](#define-hdlc-buffer) - Related to DEFINE-HDLC-BUFFER

---

### LIST-LAMU-CONSTANTS

**Description:** List the total number of LAMUS and the number of LAMUs per RT-program.

**Format:** `*LIST-LAMU-CONSTANTS
`

**Related Commands:**

- [SET-LAMU-CONSTANTS](#set-lamu-constants) - Related to SET-LAMU-CONSTANTS

---

### LIST-SERVICE-COMMANDS

**Description:** List all SINTRAN SERVICE PROGRAM commands. The command is identical to *HELP.

**Format:** `*LIST-SERVICE-COMMANDS <command>,<output file>
`

**Parameters:**

- `command`: List commands matching command name given.
- `output file`: The file where the commands should be listed. Default is your terminal.

**Rules:**

- The order of the parameters is changed from previous versions.

**Related Commands:**

- [HELP](#help) - Related to HELP

---

### LIST-TIME-SLICE-CLASS

**Description:** List information about a specific time slice class.

**Format:** `*LIST-TIME-SLICE-CLASS <timeslice class>,<image or save-area>
`

**Parameters:**

- `timeslice class`: Time slice class.
- `image or save-area`: Select area.

**Related Commands:**

- DEFINE-TIME-SLICE - Related to DEFINE-TIME-SLICE
- [LIST-TIME-SLICE-PARAMETERS](#list-time-slice-parameters) - Related to LIST-TIME-SLICE-PARAMETERS

---

### LIST-TIME-SLICE-PARAMETERS

**Description:** List time slicer information.

**Format:** `*LIST-TIME-SLICE-PARAMETERS <image?>,<save-area?>
`

**Parameters:**

- `image? or save-area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- DEFINE-TIME-SLICE - Related to DEFINE-TIME-SLICE

---

### LIST-TIME-SLICED-PROGRAMS

**Description:** List information about time sliced programs.

**Format:** `*LIST-TIME-SLICED-PROGRAMS
`

**Related Commands:**

- [INSERT-IN-TIME-SLICE](#insert-in-time-slice) - Related to INSERT-IN-TIME-SLICE
- [INSERT-PROGRAM-IN-TIME-SLICE](#insert-program-in-time-slice) - Related to INSERT-PROGRAM-IN-TIME-SLICE
- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice) - Related to REMOVE-FROM-TIME-SLICE
- [REMOVE-PROGRAM-FROM-TIME-SLICE](#remove-program-from-time-slice) - Related to REMOVE-PROGRAM-FROM-TIME-SLICE

---

### LIST-USER-RESTART-PROGRAMS

**Description:** List the RT-programs to be started by SINTRAN III after each power fail.

**Format:** `*LIST-USER-RESTART-PROGRAM <output file>
`

**Parameters:**

- `output file`: Default is your terminal.

**Related Commands:**

- [DEFINE-USER-RESTART-PROGRAM](#define-user-restart-program) - Related to DEFINE-USER-RESTART-PROGRAM
- [NEXT-USER-RESTART-PROGRAM](#next-user-restart-program) - Related to NEXT-USER-RESTART-PROGRAM

---

### MONCALL-LOG

**Description:** Inform on monitor call activity for the system.

The following subcommands are available:
 - START-MONCALL-LOG: Start the log procedure for one or all programs. Format: <log moncalls for only one program (default is yes)>[,<RT name>]
 - STOP-MONCALL-LOG: Stop the log procedure. The log may be restarted by RESTART-MONCALL-LOG.
 - RESTART-MONCALL-LOG: Restart the log procedure.
 - PRINT-MONCALL-LOG: Print the current contents of the log to an output file. This command may be given both during the log (before STOP-MONCALL-LOG), and after.
 - HELP: Display help information.
 - EXIT: Exit the MONCALL-LOG command.


**Format:** `*MONCALL-LOG
`

**Rules:**

- The Monitor Call Log is an option to be ordered on the SINTRAN III order form.

**Example:**

```
The following subcommands are available:

START-MONCALL-LOG
      <log moncalls for only one program (default is yes)>[,<RT name>]>

      start the log procedure for one or all programs.

STOP-MONCALL-LOG
      stop the log procedure. the log may be restarted by:

RESTART-MONCALL-LOG

PRINT-MONCALL-LOG
    <output file>

      print the current contents of the log. this command may be given
      both during the log (before stop-moncall-log), and after.

HELP

EXIT 
```

---

### NEXT-USER-RESTART-PROGRAM

**Description:** Define further RT-programs to be started by SINTRAN III after each power fail.

**Format:** `*NEXT-USER-RESTART-PROGRAM <program>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `program`: An octal RT-description address or an RT-program name.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [DEFINE-USER-RESTART-PROGRAM](#define-user-restart-program) - Related to DEFINE-USER-RESTART-PROGRAM
- [LIST-USER-RESTART-PROGRAMS](#list-user-restart-programs) - Related to LIST-USER-RESTART-PROGRAMS

---

### OCTAL-DUMP

**Description:** Dump each byte of an area as octal numbers. Use ASCII-DUMP dump to have the area dumped as characters.

**Format:** `*OCTAL-DUMP <area>[,<segment number>],<lower address>,<upper address>,<output file>
`

**Parameters:**

- `area`: In SINTRAN III/VSE, legal parameters are MEMORY, IMAGE, SAVE AREA or SEGMENT. In SINTRAN III/VSX, legal parameters are SEGMENT or ALT-SEGMENT.
- `segment number`: If SEGMENT (or ALT-SEGMENT) is specified as <area>, the octal segment number should be entered.
- `lower address`: The lower address of the area to be dumped.
- `upper address`: The upper address of the area to be dumped.
- `output file`: The file where the information should be dumped. Default is your terminal.

**Related Commands:**

- [ASCII-DUMP](#ascii-dump) - Related to ASCII-DUMP

---

### PAGES-FROM-LAMU

**Description:** Transfer pages from a LAMU to memory.

**Format:** `*PAGES-FROM-LAMU <LAMU id>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `LAMU id`: The number that identifies the LAMU in the LAMU table.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [LAMU-AREAS](#lamu-areas) - Related to LAMU-AREAS
- [LAMU-INFORMATION](#lamu-information) - Related to LAMU-INFORMATION
- [PAGES-TO-LAMU](#pages-to-lamu) - Related to PAGES-TO-LAMU

---

### PAGES-TO-LAMU

**Description:** Transfer pages from memory to a LAMU.

**Format:** `*PAGES-TO-LAMU <LAMU id>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `LAMU id`: The number that identifies the LAMU in the LAMU table.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [LAMU-AREAS](#lamu-areas) - Related to LAMU-AREAS
- [LAMU-INFORMATION](#lamu-information) - Related to LAMU-INFORMATION
- [PAGES-FROM-LAMU](#pages-from-lamu) - Related to PAGES-FROM-LAMU

---

### PROTECT-LAMU

**Description:** Protect a LAMU from being overwritten.

**Format:** `*PROTECT-LAMU <LAMU id>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `LAMU id`: The number that identifies the LAMU in the LAMU table.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [LAMU-AREAS](#lamu-areas) - Related to LAMU-AREAS
- [LAMU-INFORMATION](#lamu-information) - Related to LAMU-INFORMATION

---

### READ-BINARY

**Description:** Read a binary file into memory.

**Format:** `*READ-BINARY <file>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `file`: The binary file to be read.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

---

### REINSERT-SINTRAN-COMMAND

**Description:** Reinsert a SINTRAN III command that has been removed.

**Format:** `*REINSERT-SINTRAN-COMMAND <command>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `command`: The SINTRAN III command to be reinserted.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [REMOVE-SINTRAN-COMMAND](#remove-sintran-command) - Related to REMOVE-SINTRAN-COMMAND

---

### REMOVE-FROM-BACKGROUND-TABLE

**Description:** Remove a device from the background table.

**Format:** `*REMOVE-FROM-BACKGROUND-TABLE <logical device number>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `logical device number`: An octal logical device number.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-BACKGROUND-TABLE](#insert-in-background-table) - Related to INSERT-IN-BACKGROUND-TABLE

---

### REMOVE-FROM-EXTENDED-IDENT-TABLE

**Description:** Remove an entry from the extended ident code table of a hardware interrupt level.

**Format:** `*REMOVE-FROM-EXTENDED-IDENT-TABLE <level>,<logical device number>,<input/output>,<ident code>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `level`: The octal interrupt level.
- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `ident code`: The octal ident code.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-EXTENDED-IDENT-TABLE](#insert-in-extended-ident-table) - Related to INSERT-IN-EXTENDED-IDENT-TABLE
- [REMOVE-FROM-IDENT-TABLE](#remove-from-ident-table) - Related to REMOVE-FROM-IDENT-TABLE

---

### REMOVE-FROM-IDENT-TABLE

**Description:** Remove an entry from the ident code table of a hardware interrupt level.

**Format:** `*REMOVE-FROM-IDENT-TABLE <level>,<logical device number>,<input/output>,<ident code>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `level`: The octal interrupt level.
- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `ident code`: The octal ident code.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-IDENT-TABLE](#insert-in-ident-table) - Related to INSERT-IN-IDENT-TABLE
- [REMOVE-FROM-EXTENDED-IDENT-TABLE](#remove-from-extended-ident-table) - Related to REMOVE-FROM-EXTENDED-IDENT-TABLE

---

### REMOVE-FROM-IOX-TABLE

**Description:** Make a hardware device number unavailable for @EXECUTE-IOX and EXIOX (MON 31).

**Format:** `*REMOVE-FROM-IOX-TABLE <hardware device number>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `hardware device number`: An octal physical device number.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-IOX-TABLE](#insert-in-iox-table) - Related to INSERT-IN-IOX-TABLE

---

### REMOVE-FROM-LOGICAL-UNIT-TABLE

**Description:** Remove a logical device number from the logical device table.

**Format:** `*REMOVE-FROM-LOGICAL-UNIT-TABLE <logical device number>,<input/output>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [INSERT-IN-LOGICAL-UNIT-TABLE](#insert-in-logical-unit-table) - Related to INSERT-IN-LOGICAL-UNIT-TABLE

---

### REMOVE-FROM-TIME-SLICE

**Description:** Cause the background process to run on a fixed priority. If a background process is in the time slicer, its priority will be modified dynamically.

**Format:** `*REMOVE-FROM-TIME-SLICE <logical device number>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `logical device number`: The octal logical device number of a terminal, batch processor or communication device.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- A background process not in the time slicer will run on a fixed priority. This priority is defined by @PRIOR.

**Related Commands:**

- [INSERT-IN-TIME-SLICE](#insert-in-time-slice) - Related to INSERT-IN-TIME-SLICE
- [INSERT-PROGRAM-IN-TIME-SLICE](#insert-program-in-time-slice) - Related to INSERT-PROGRAM-IN-TIME-SLICE
- [LIST-TIME-SLICED-PROGRAMS](#list-time-sliced-programs) - Related to LIST-TIME-SLICED-PROGRAMS
- [REMOVE-PROGRAM-FROM-TIME-SLICE](#remove-program-from-time-slice) - Related to REMOVE-PROGRAM-FROM-TIME-SLICE
- PRIOR - Related to PRIOR

---

### REMOVE-FROM-TIMER-TABLE

**Description:** Remove a logical device number from the timer table.

**Format:** `*REMOVE-FROM-TIMER-TABLE <logical device number>,<input/output>
`

**Parameters:**

- `logical device number`: An octal logical device number in the range 1-77 or greater than 200.
- `input/output`: Select the INPUT or OUTPUT part of the device.

**Related Commands:**

- [INSERT-IN-TIMER-TABLE](#insert-in-timer-table) - Related to INSERT-IN-TIMER-TABLE

---

### REMOVE-PROGRAM-FROM-TIME-SLICE

**Description:** Set the specified RT-program to run on fixed priority.

**Format:** `*REMOVE-PROGRAM-FROM-TIME-SLICE <RT name>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `RT name`: An RT-description address or an RT-program name.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- The program will continue to run on the priority it had when it was removed from time slice. This may be changed with the @PRIOR command.

**Related Commands:**

- [INSERT-IN-TIME-SLICE](#insert-in-time-slice) - Related to INSERT-IN-TIME-SLICE
- [INSERT-PROGRAM-IN-TIME-SLICE](#insert-program-in-time-slice) - Related to INSERT-PROGRAM-IN-TIME-SLICE
- [LIST-TIME-SLICED-PROGRAMS](#list-time-sliced-programs) - Related to LIST-TIME-SLICED-PROGRAMS
- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice) - Related to REMOVE-FROM-TIME-SLICE

---

### REMOVE-SINTRAN-COMMAND

**Description:** Remove a SINTRAN III command.

**Format:** `*REMOVE-SINTRAN-COMMAND <command>,<memory?>,<image?>,<save area?>
`

**Parameters:**

- `command`: The SINTRAN III command to be removed.
- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Related Commands:**

- [REINSERT-SINTRAN-COMMAND](#reinsert-sintran-command) - Related to REINSERT-SINTRAN-COMMAND

---

### REMOVE-SPOOLING-HEADER

**Description:** Cause the spooling program to not print the spooling header and trailer between each file output by the spooling system.

**Format:** `*REMOVE-SPOOLING-HEADER <spooling index>,<memory?>,<save area?>
`

**Parameters:**

- `spooling index`: The index of the spooling device as defined by the command SET-SPOOLING-DEVICE-NUMBER.
- `memory? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO. The area parameters apply only to the VSE-version; in the VSX-version, only the MEMORY area of SINTRAN III is affected.

**Related Commands:**

- [INSERT-SPOOLING-HEADER](#insert-spooling-header) - Related to INSERT-SPOOLING-HEADER

---

### RESET-COLDSTART-MODE-FILE

**Description:** Reset the coldstart mode file.

**Format:** `*RESET-COLDSTART-MODE-FILE <memory?>,<image?>,<save area?>
`

**Parameters:**

- `memory?, image? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

---

### SEGMENT-WRITE-PERMIT

**Description:** Allows write access to a segment.

**Format:** `*SEGMENT-WRITE-PERMIT <segment number>`

**Parameters:**

- `segment number`: The octal segment number of the segment to be write permitted.

**Rules:**

- The segment must have been loaded to.

**Related Commands:**

- [SEGMENT-WRITE-PROTECT](#segment-write-protect) - Protect a segment against writing

---

### SEGMENT-WRITE-PROTECT

**Description:** Protect a segment against writing.

**Format:** `*SEGMENT-WRITE-PROTECT <segment number>`

**Parameters:**

- `segment number`: The octal segment number of the segment to be write protected.

**Rules:**

- The segment must have been loaded to.

**Related Commands:**

- [SEGMENT-WRITE-PERMIT](#segment-write-permit) - Allow write access to a segment

---

### SET-CLOSED-SCRATCH-FILE-SIZE

**Description:** Set the maximum number of pages to remain in a scratch file when it is closed.

**Format:** `*SET-CLOSED-SCRATCH-FILE-SIZE <logical device number>,<number of pages>,<memory?>,<save area?>`

**Parameters:**

- `logical device number`: The octal logical device number of a terminal owning the scratch file.
- `number of pages`: The octal maximum number of pages to remain in the scratch file.
- `memory? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- The parameter <number of pages> may be a negative number. This will be taken to mean that pages will only be deleted from a scratch file if the user owning the scratch file has less free pages than the absolute value of this number.

**Related Commands:**

- [SCRATCH-OPEN](#scratch-open) - Open a file in scratch mode

---

### SET-COLDSTART-MODE-FILE

**Description:** Set parameters for commands to be executed at cold start.


**Format:** `*SET-COLDSTART-MODE-FILE <parameters to the enter-directory command when entering main directory>,<coldstart input file>,<coldstart output file>
`

**Parameters:**

- `parameters to the enter-directory command when entering main directory`: This parameter contains directory name, disk name and unit and/or subunit numbers.
- `coldstart input file`: Name of mode file to be run at cold start.
- `coldstart output file`: Name of output file for mode file to be run at cold start.

**Related Commands:**

- [RESET-COLDSTART-MODE-FILE](#reset-coldstart-mode-file) - Remove definition of parameters for commands to be executed at cold start.
- [COLD-START](#cold-start) - Initialize the system and restart it.

---

### SET-COMMAND-PROTECTION

**Description:** Change the category of users who are allowed to use a SINTRAN III command. It is also possible to set protection on reentrant subsystems and ND-500 standard domains.

**Format:** `*SET-COMMAND-PROTECTION <command>,<protection>,<memory?>,<save area?>`

**Parameters:**

- `command`: Any command or reentrant subsystem name or ND-500 standard domain name.
- `protection`: Select a user category. PUBLIC permits the command for all users. RT permits the command for user RT and user SYSTEM. SYSTEM permits the command for user SYSTEM only.
- `memory? and save area?`: Select the areas to be modified by answering each question with YES or NO. The default answer is NO.

**Rules:**

- File system commands can only be changed to be restricted for users RT and/or SYSTEM.

**Related Commands:**

- [REINSERT-SINTRAN-COMMAND](#reinsert-sintran-command) - Reinsert a SINTRAN command
- [REMOVE-SINTRAN-COMMAND](#remove-sintran-command) - Remove a SINTRAN command

---

### SET-LAMU-CONSTANTS

**Description:** Set the system constants of the LAMU system, that is, the number of LAMUs per RT-program and the total number of LAMUs.


**Format:** `*SET-LAMU-CONSTANTS <number of LAMUs per RT-program>,<total number of LAMUs>
`

**Parameters:**

- `number of LAMUs per RT-program`: The number of LAMUs available for each RT-program.
- `total number of LAMUs`: The maximum number of LAMUs in the system.

**Rules:**

- The system needs to be restarted for the command to take effect.

**Related Commands:**

- [LIST-LAMU-CONSTANTS](#list-lamu-constants) - List the total number of LAMUs and the number of LAMUs per RT-program.

---

### SET-MAX-ENTER-COUNT

**Description:** Set the maximum number of consecutive unsuccessful attempts a user can make to log in on a terminal. If the number is exceeded, the terminal will be disabled until the command CLEAR-ENTER-COUNT is given.


**Format:** `*SET-MAX-ENTER-COUNT <logical device number>,<enter count wanted?>[,<max enter count>],<memory?>,<save area?>
`

**Parameters:**

- `logical device number`: The logical device number of the terminal, batch processor or communication device.
- `enter count wanted?`: Answer YES or NO to whether you want to use the enter count facility.
- `max enter count`: The maximum number of consecutive unsuccessful attempts to enter that can be made on the specified terminal.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

**Related Commands:**

- [CLEAR-ENTER-COUNT](#clear-enter-count) - Clear the enter count and re-enable a terminal that has been disabled due to exceeding the maximum enter count.

---

### SET-SPOOLING-DEVICE-NUMBER

**Description:** Connect a spooling index to a logical device number.


**Format:** `*SET-SPOOLING-DEVICE-NUMBER <spooling index>,<logical device number>,<memory?>,<save area?>
`

**Parameters:**

- `spooling index`: The octal index of the spooling device in the system. The range is determined at system generation time.
- `logical device number`: The octal logical device number of the printer to be used for spooling.
- `memory?`: Select whether memory should be modified by answering YES or NO. The default answer is NO.
- `save area?`: Select whether save area should be modified by answering YES or NO. The default answer is NO.

---

### START-GPIB

**Description:** Start a GPIB controller.


**Format:** `*START-GPIB <controller no.>
`

**Parameters:**

- `controller no.`: GPIB controller.

**Related Commands:**

- [STOP-GPIB](#stop-gpib) - Stop a GPIB controller.

---

### START-XMSG

**Description:** Fix the XMSG PAGING OFF and BUFFER AREA segments in physical memory and start the routine XROUT. See the SINTRAN III Communication Guide (ND-60.134).


**Format:** `*START-XMSG
`

**Rules:**

- The command should be performed before starting any COSMOS products, preferably in the batch file LOAD-MODE28YMB.

**Related Commands:**

- [STOP-XMSG](#stop-xmsg) - Disable the XMSG monitor call and release the physical memory space.

---

### STOP-GPIB

**Description:** Stop a GPIB controller.


**Format:** `*STOP-GPIB <controller no.>
`

**Parameters:**

- `controller no.`: GPIB controller.

**Related Commands:**

- [START-GPIB](#start-gpib) - Start a GPIB controller.

---

### STOP-XMSG

**Description:** Disable the XMSG monitor call and release the physical memory space.


**Format:** `*STOP-XMSG
`

**Related Commands:**

- [START-XMSG](#start-xmsg) - Fix the XMSG PAGING OFF and BUFFER AREA segments in physical memory and start the routine XROUT.

---

### SWAP-DIRECTORY-ENTRIES

**Description:** Exchange two directories in the directory table.

**Format:** `*SWAP-DIRECTORY-ENTRIES <directory index 1>,<directory index 2>[,<save area?>]
`

**Parameters:**

- `directory index 1`: The index of the first directory, to be exchanged with
- `directory index 2`: The index of the second directory. Directory indices are listed by the command @LIST-DIRECTORIES-ENTERED
- `save area?`: Answer YES or NO to whether the SAVE AREA should be affected. Default is NO

**Rules:**

- This command has no effect on SINTRAN III/VSX version K standard systems. On these systems, the directory index is determined from the sequence the directories are defined (either by @ENTER-DIRECTORY or by @DEFINE-MASS-STORAGE-UNIT)

**Related Commands:**

- [LIST-DIRECTORIES-ENTERED](#list-directories-entered) - Lists directory indices that can be used with this command

**Example:**

```
*SWAP-DIRECTORY-ENTRIES 1,2,NO 
```

---

### SWAPPING-LOG

**Description:** Inform on swapping activity for the system.

The following subcommands are available:
  - START-SWAPPING-LOG: Start swapping-log for one specific program, or for all programs. Format: <log swapping for specific program (default: yes)>[,<RT name>]
  - STOP-SWAPPING-LOG: Stop the current logging. The log may be restarted by RESTART-SWAPPING-LOG.
  - RESTART-SWAPPING-LOG: Restart the log procedure.
  - READ-SWAPPING-LOG: Print the current contents of the log. This command may be given both during the log (before STOP-SWAPPING-LOG), and after.
  - SWAPPING-LOG: Print swapping log information at specified intervals, default 60 seconds. Format: <log swapping for a specific program (default is yes)>[,<rt name>], <interval in seconds (default is 60 secs)>. This command must be terminated by pressing <escape>. If either of the commands START-SWAPPING-LOG or SWAPPING-LOG has been used, they cannot be used again before the command STOP-SWAPPING-LOG is given.
  - HELP: Display help information.
  - EXIT: Exit the SWAPPING-LOG command.


**Format:** `*SWAPPING-LOG
`

**Rules:**

- The Swapping Log is an option to be ordered on the SINTRAN III order form

**Example:**

```
*SWAPPING-LOG
START-SWAPPING-LOG yes,RT1
READ-SWAPPING-LOG
STOP-SWAPPING-LOG 
```

---

### TEST-COMMAND

**Description:** This is a test command to verify that the generators are working correctly.

**Format:** `@TEST-COMMAND <param1> <param2>`

**Parameters:**

- `param1`: First test parameter
- `param2`: Second test parameter

**Rules:**

- This is a test rule
- Another test rule

**Privileges:** Operator

**Related Commands:**

- [SET](#set) - Related to SET command

**Monitor Calls:**

- TEST-MON (999): Test monitor call

**Notes:**

This is a test command to verify that:
1. The generators correctly read YAML files
2. The section is properly assigned
3. All fields are correctly formatted in the output


**Aliases:**

TEST, TESTCMD

---

## ND-500 Monitor commands

### ABORT-BATCH-ON-ERROR

**Description:** Toggle flag to tell if a batch or mode job should be terminated when an error occurs.


**Format:** `ABORT-BATCH-ON-ERROR <ON/OFF>
`

**Parameters:**

- `ON/OFF`: ON if batch jobs should terminate if an error occurs, OFF if only the current command should be terminated.

**Rules:**

- If an error occurs in a batch or mode job and this command has been executed with the parameter OFF, only the current command is aborted and the next command in the batch input file is executed. If the command has not been executed or executed with the parameter ON, the entire job is terminated. The error message will be written on the batch output file.
- This command may be specified several times, switching the batch termination on and off before and after critical sequences.

---

### ABORT-PROCESS

**Description:** Abort a process which cannot be stopped in any other way.

**Format:** `ABORT-PROCESS <process number>`

**Parameters:**

- `process number`: The number of a currently running process.

**Rules:**

- The process specified will be aborted and its reserved resources released. The user will be forced to leave the monitor.
- This command should be used with care, as no clean-up of the system tables and queues is performed. It should be employed only in case of a system hangup, where there is no other way to stop a process.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [LOGOUT-PROCESS](#logout-process) - Logs out a process from the system
- [ABORT](#abort) - Stops an RT-program
- STOP-TERMINAL - Terminates a user from a terminal

---

### ATTACH-PROCESS

**Description:** Attach to a specific process to communicate with it.

**Format:** `ATTACH-PROCESS <process number>`

**Parameters:**

- `process number`: The number of the process with which communication is desired. Default is the current process connected to the terminal.

**Rules:**

- Subsequent commands LOOK-AT, RUN etc will be routed to the specified process. The process should not be connected to any other terminal.
- This command is currently used for debugging purposes, attaching to the swapper process.
- This command is restricted to user SYSTEM.

---

### AUTOMATIC-ERROR-MESSAGE

**Description:** Force output of error messages from monitor calls.

**Format:** `AUTOMATIC-ERROR-MESSAGE`

**Related Commands:**

- [RESET-AUTOMATIC-ERROR-MESSAGE](#reset-automatic-error-message) - Suppress output of error messages from monitor calls

---

### BRANCH-TRACE

**Description:** Initiate tracing of the program counter upon branch trap conditions. The tracing is written to the output device.

**Format:** `BRANCH-TRACE <start address>,<min. trace>,<max. trace>`

**Parameters:**

- `start address`: The start address of the program to be traced.
- `min. trace`: The lower limit of program area to be traced.
- `max. trace`: The upper limit of program area to be traced.

**Rules:**

- If <min. trace> and <max. trace> are specified, tracing is performed only within the specified area. Branches executed outside this area are not listed. If the parameters are omitted, all branches will be listed.

**Related Commands:**

- [CALL-TRACE](#call-trace) - Initiates tracing of the program counter upon call trap conditions
- [RESET-BRANCH-TRACE](#reset-branch-trace) - Discontinues branch tracing

---

### BREAK

**Description:** Set a breakpoint at the specified address.

**Format:** `BREAK <address>,[<count>],[<command>]`

**Parameters:**

- `address`: The program address where a breakpoint is to be set.
- `count`: One plus the number of times the breakpoint should be ignored before a break occurs. Default value is 1.
- `command`: Command to be executed on a break.

**Rules:**

- When a breakpoint is encountered during execution, the program pauses and control is returned to the monitor.

**Related Commands:**

- [RESET-BREAKS](#reset-breaks) - Removes breakpoints
- [RESET-LAST-BREAK](#reset-last-break) - Removes the last breakpoint encountered

---

### CACHE-MONITOR

**Description:** Change the way instructions and/or data are fetched from memory.

**Format:** `CACHE-MODE <program cache mode>,<data cache mode>`

**Parameters:**

- `program cache mode`: Select cache mode for instruction fetch.
- `data cache mode`: Select cache mode for data. The following options are available for both parameters - NORMAL (use cache when possible, otherwise memory), CACHE-ONLY (use cache only), MEMORY-ONLY (bypass cache).

**Rules:**

- When using this command, the micro-program will stop and must be restarted.
- This command is restricted for user SYSTEM only.

---

### CALL-TRACE

**Description:** Initiate tracing of the program counter upon call trap conditions. The tracing is written to the output device.

**Format:** `CALL-TRACE <start address>,<min. trace>,<max. trace>`

**Parameters:**

- `start address`: The start address of the program to be traced.
- `min. trace`: The lower limit of program area to be traced.
- `max. trace`: The upper limit of program area to be traced.

**Rules:**

- If <min. trace> and <max. trace> are specified, tracing is performed only within the specified area. Calls executed outside this area are not listed. If the parameters are omitted, all calls will be listed.
- All routine calls within the specified area, including run-time library routines, are traced.

**Related Commands:**

- [BRANCH-TRACE](#branch-trace) - Initiates tracing of the program counter upon branch trap conditions
- [RESET-CALL-TRACE](#reset-call-trace) - Discontinues routine call tracing

---

### CC

**Description:** Comment. The command or text which follows has no effect. Used in batch or mode files.

**Format:** `@CC <text>`

**Parameters:**

- `text`: Any printable characters.

**Rules:**

- Permitted for all users.
- There should be a space between the second C and the text.

**Privileges:** Operator

**Related Commands:**

- [CC](#cc) - Comment command in ND-500 Monitor

---

### CHANGE-CPU

**Description:** Change current CPU in a multi-CPU system (ND-580).

**Format:** `CHANGE-CPU <CPU number>`

**Parameters:**

- `CPU number`: Number of the ND-500 CPU in a multi-CPU system (ND-580) which will execute the commands (programs) following.

---

### CLOSE-FILE

**Description:** Closes a file and disconnects the file number.

**Format:** `CLOSE-FILE <connect number>`

**Parameters:**

- `connect number`: The connect number of a file open from a ND-500 program or through the OPEN-FILE command. Possible values: >0 (close the file open with the given number), -1 (close all files temporarily open), -2 (close all open files), -3 (close all files open from the ND-500 program or by the OPEN-FILE command in the Monitor).

**Related Commands:**

- [OPEN-FILE](#open-file) - Open a file for access from an ND-500 program
- [CLOSE-FILE](#close-file) - Close one or more files opened by currently logged-in user

---

### COMPARE-CONTROL-STORE

**Description:** Compare micro program currently loaded in the control store with the micro program stored on a file.

**Format:** `COMPARE-CONTROL-STORE <file name>,<start address>,<number of words>,<max. number of faults>`

**Parameters:**

- `file name`: The name where the micro program is stored. Default is CONTROL-STORE:DATA.
- `start address`: The octal address where the comparison should start. Default is 0.
- `number of words`: The number of words to be compared. Default is 20000B (entire control store).
- `max. number of faults`: The maximum number of differences accepted between the file contents and the loaded micro program before the comparison is aborted. Default is 7 (the number of messages that will fit on a VDU screen).

**Rules:**

- This command is restricted to user SYSTEM.
- The current ND-500 micro program is compared to the micro program residing on the specified file. The comparison starts at the specified micro-program address. This word is compared to the first word on the file, etc. Four words will be modified after the microcode is loaded and will always be different.
- Upon difference, the address and the two differing control-store words are written to the output device. The comparison lasts until the specified number of words are compared or the maximum number of faults are found.

**Related Commands:**

- [LOAD-CONTROL-STORE](#load-control-store) - Load the micro code from disk

---

### CONTINUE

**Description:** Continue execution of a program.

**Format:** `CONTINUE`

**Rules:**

- The execution is restarted at the current program counter. There is one exception - if a program has stopped normally (by MON 0 or a stack underflow trap) the execution is started at the original start address.
- If the execution has stopped because of a breakpoint, the original instruction will be restored. If the breakpoint is a permanent breakpoint, a single instruction is performed, and the original instruction is replaced by a breakpoint instruction before the execution is started.
- If the execution has stopped because an escape character was typed, the execution will be restarted where it stopped. Files will remain opened after an escape, and the program will continue as if nothing had happened.

---

### DEBUG-PLACE

**Description:** Place a domain and allow temporary patches to be made before execution.

**Format:** `DEBUG-PLACE <domain name>`

**Parameters:**

- `domain name`: The name of an existing domain.

**Rules:**

- The program segments as well as the data segments will be copied to the swap file. This allows patches to be done to the program segment. Patches are not permanent. In order to do permanent patches, LOOK-AT-PROGRAM must be used. Otherwise, this command works exactly like PLACE-DOMAIN.

**Related Commands:**

- [PLACE-DOMAIN](#place-domain) - Place a domain and start execution

---

### DEBUG-STATUS

**Description:** List information about previously-used debug commands. Enabled traps, breakpoints, and the use of the LL and HL registers are listed.

**Format:** `DEBUG-STATUS`

---

### DEBUG-SWAPPER

**Description:** Debug the ND-500 Swapper.

**Format:** `DEBUG-SWAPPER <ON/OFF>`

**Parameters:**

- `ON/OFF`: Set debugging mode on or turn it off.

**Rules:**

- This command is restricted to user SYSTEM.
- It is intended for internal use by ND.

---

### DEBUGGER

**Description:** Start the Symbolic Debugger.

**Format:** `DEBUGGER [<domain name>]`

**Parameters:**

- `domain name`: The name of the domain to be debugged. Default is the domain currently in memory.

**Rules:**

- The symbolic debugger is started with the specified or current domain as the system to be debugged. The commands of the symbolic debugger are documented fully in the manual Symbolic Debugger User Guide, ND-60.158.
- For symbolic names to be available the program must have been compiled with the DEBUG-MODE option in the compiler turned ON. If the DEBUG-MODE option was off, the symbolic debugger may be used, but no symbolic references can be made.
- The debugger is located on the files (SYSTEM)DEBUGGER:PSEG and (SYSTEM)DEBUGGER:DSEG. When started, it will execute as segment number 26D in the user domain; this segment number must not be used by the domain to be debugged.
- The DEBUGGER command may be issued at any time during execution. The normal execution may be interrupted by pressing the "escape" key, after which the debugger is started and execution resumed from the interrupt point, now in debug mode.

---

### DEFINE-MACRO

**Description:** Define a macro (a collection of commands executed in a specified sequence and called as if it was a single, user-defined, command).

**Format:** `DEFINE-MACRO <macro name>
<macro body>
END-MACRO
`

**Parameters:**

- `macro name`: The name of the macro to be defined.
- `macro body`: The series of commands that make up the macro. May include special macro subcommands.

**Rules:**

- Macros defined by this command are temporary. Permanent macros may be prepared by a text editor on a file. The file must be of type :MACR.
- Every line following the DEFINE-MACRO command is taken as the macro body until the END-MACRO is encountered. END-MACRO must be written on a new line.
- It is possible within the macro body to define parameters that are replaced by the actual parameters when the macro is called. A parameter is defined by the parameter command in the macro body. If spaces or commas should be part of the parameter name, default value or prompting text, they may be enclosed in apostrophes. Otherwise, apostrophes are permitted but not required.
- The first actual parameter supplied in the macro call line replaces the parameter name used in the first PARAMETER definition; the second actual parameter replaces the parameter name used in the next PARAMETER definition and so on. Excessive parameters are ignored.
- When the macro is called, the parameters which are not specified are asked for by typing the prompting text on the communication device. If the actual parameter is empty, the default value is used when expanding the macro.
- A monitor call, MACROE (MON 400), for signalling error return from a program to the Monitor is implemented. There is a flag which is raised when the executing program is terminated by this monitor call or by a trap. The error flag is set to zero when a program is terminated normally.
- Macro subcommands may not be abbreviated.

**Related Commands:**

- [ERASE-MACRO](#erase-macro) - Erase a temporarily defined macro
- [EXECUTE-MACRO](#execute-macro) - Execute a macro
- [LIST-MACRO](#list-macro) - List a macro or all macros

**Monitor Calls:**

- MACROE (MON 400) (400): Monitor call for signalling error return from a program to the Monitor

---

### DEFINE-MEMORY-CONFIGURATION

**Description:** Define the physical memory configuration to the operating system.

**Format:** `DEFINE-MEMORY-CONFIGURATION <ND-100 page no. for ND-500 phys. addr. 0>`

**Parameters:**

- `ND-100 page no. for ND-500 phys. addr. 0`: The ND-100 page number for which the ND-500 physical address is zero, that is, the difference between the ND-500 and ND-100 physical addresses for the same physical cell in common memory.

**Rules:**

- Normally, the DEFINE-MEMORY-CONFIGURATION command is not required unless it is necessary to define the memory configuration differently from the actual physical configuration. This may be the case on multi-CPU systems if parts of memory is to be reserved for one particular CPU and regarded as invisible to other CPUs.
- The DEFINE-MEMORY-CONFIGURATION does not survive a warm start.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [MEMORY-CONFIGURATION](#memory-configuration) - List information about memory configuration

---

### DEFINE-STANDARD-DOMAIN

**Description:** Define a domain in the reentrant subsystem table, which means that it may be started without going through the ND-500 monitor.

**Format:** `DEFINE-STANDARD-DOMAIN <standard domain name>,<domain name>`

**Parameters:**

- `standard domain name`: The name to be used when calling the domain. May be the same as the domain name, but may not include user name. It should not be a legal abbreviation of a monitor command.
- `domain name`: Name of an already loaded domain, belonging to any user.

**Rules:**

- When a user issues the standard domain name, or an unambiguous abbreviation of it, as a command, the domain will be started. If the user has a private domain that would otherwise have been started, the name must include the user name in parentheses.
- The files comprising the domain name should have public read access.
- DEFINE-STANDARD-DOMAIN is permitted for user SYSTEM only.

**Related Commands:**

- [DELETE-STANDARD-DOMAIN](#delete-standard-domain) - Remove the definition of a domain from the reentrant subsystem table
- [LIST-STANDARD-DOMAINS](#list-standard-domains) - List all domains defined as standard domains and their segments
- [LIST-REENTRANT](#list-reentrant) - List all reentrant subsystems

---

### DEFINE-SWAP-FILE

**Description:** Define a file as a swap file for ND-500 segments.

**Format:** `DEFINE-SWAP-FILE <file name>`

**Parameters:**

- `file name`: The name of an existing contiguous file.

**Rules:**

- The file specified is defined as a swap file for ND-500 segments. The file must be a contiguous file, and must be created before this command is used. The file may belong to any user, but user SYSTEM must have at least read and write access (RW) to it.
- There may be several swap files in the system; the Monitor will assign a swap area to a process on whichever file has sufficient free space left.
- Definition of swap files will survive a warm start, but not a cold start.
- This command is restricted to user SYSTEM.

**Related Commands:**

- DELETE-SWAP-FILE - Remove the definition of a file as swap file for ND-500 segments
- [LIST-SWAP-FILE-INFO](#list-swap-file-info) - List information on one or all ND-500 swap files

---

### DELETE-STANDARD-DOMAIN

**Description:** Remove the definition of a domain from the reentrant subsystem table.

**Format:** `DELETE-STANDARD-DOMAIN <name>`

**Parameters:**

- `name`: Name of an existing standard domain.

**Rules:**

- The specified standard domain is deleted from the name table of standard domains. The domain will not be deleted, but will no longer be a standard domain.
- DELETE-STANDARD-DOMAIN may not be issued while the standard domain is in use.
- DELETE-STANDARD-DOMAIN is permitted for user SYSTEM.

**Related Commands:**

- [DEFINE-STANDARD-DOMAIN](#define-standard-domain) - Define a domain in the reentrant subsystem table
- [LIST-STANDARD-DOMAINS](#list-standard-domains) - List all domains defined as standard domains and their segments
- [LIST-REENTRANT](#list-reentrant) - List all reentrant subsystems

---

### DUMP-MACRO

**Description:** Write the definition of a temporary macro (defined by DEFINE-MACRO) to a file.

**Format:** `DUMP-MACRO <macro name>`

**Parameters:**

- `macro name`: The name of an existing temporary macro.

**Rules:**

- The named temporary macro will be written to a file with the name of the macro, that is, the macro is made permanent and can at a later time be executed by using the macro name as a command. If the file does not exist, it will be created. The default type of the file is :MACR.

**Related Commands:**

- [DEFINE-MACRO](#define-macro) - Define a macro (a collection of commands executed in a specified sequence)
- [ERASE-MACRO](#erase-macro) - Erase a temporarily defined macro
- [LIST-MACRO](#list-macro) - List a macro or all macros

---

### DUMP-PHYSICAL-SEGMENT

**Description:** Write a copy of a segment to a file.

**Format:** `DUMP-PHYSICAL-SEGMENT <file name>,<physical segment number>`

**Parameters:**

- `file name`: The name of the file to receive the dump.
- `physical segment number`: Physical segment number of the segment to be dumped.

**Rules:**

- This command is restricted to user SYSTEM.
- It is intended for debugging purposes.

**Related Commands:**

- [LOOK-AT-PHYSICAL-SEGMENT](#look-at-physical-segment) - View the contents of a physical segment in memory

---

### DUMP-SWAPPER

**Description:** Write a copy of the ND-500 Swapper to a file.

**Format:** `DUMP-SWAPPER <file name>`

**Parameters:**

- `file name`: The name of the file to receive the dump.

**Rules:**

- The ND-500 Swapper's data segment will be dumped on the specified file.
- This command is restricted to user SYSTEM.
- It is intended for debugging purposes.

**Related Commands:**

- [LOAD-SWAPPER](#load-swapper) - Load a new ND-500 Swapper from a file

---

### ENABLED-TRAPS

**Description:** Lists the contents of the own trap-enable register (OTE) of the current domain and the mother trap-enable register.

**Format:** `ENABLED-TRAPS`

**Rules:**

- Enabled traps, either in the current domain or in ND-100, are listed on the output device.

**Related Commands:**

- [SYSTEM-TRAP-DISABLE](#system-trap-disable) - Disable a system trap
- [SYSTEM-TRAP-ENABLE](#system-trap-enable) - Enable a system trap
- [LOCAL-TRAP-DISABLE](#local-trap-disable) - Disable a local trap
- [LOCAL-TRAP-ENABLE](#local-trap-enable) - Enable a local trap

---

### ERASE-MACRO

**Description:** Erase a temporary macro.

**Format:** `ERASE-MACRO <macro name>...`

**Parameters:**

- `macro name`: The name of an existing temporary macro.

**Rules:**

- The named temporary macros are erased. Permanent macros are erased through the SINTRAN III command @DELETE-FILE <macro name>:MACR.

**Related Commands:**

- [DEFINE-MACRO](#define-macro) - Define a macro (a collection of commands executed in a specified sequence)
- [LIST-MACRO](#list-macro) - List a macro or all macros
- [DUMP-MACRO](#dump-macro) - Write the definition of a temporary macro to a file

---

### EXECUTE-MACRO

**Description:** Execute a macro.

**Format:** `EXECUTE-MACRO <macro name>,[<parameters>]...`

**Parameters:**

- `macro name`: The name of an existing (temporary or permanent) macro.
- `parameters`: Actual parameters to replace formal parameters in the macro. If several parameters are supplied they are separated by comma or space. The parameter may contain any character except space or comma.

**Rules:**

- The macro with the specified name is processed. Formal parameters are substituted with actual parameters. If the actual parameters are not supplied, they are prompted for with <leading text> specified in the PARAMETER definition (see the DEFINE-MACRO command).
- The words EXECUTE-MACRO can be left out. The procedure used for looking up a command or macro follows established command lookup patterns.

**Related Commands:**

- [DEFINE-MACRO](#define-macro) - Define a macro (a collection of commands executed in a specified sequence)
- [LIST-MACRO](#list-macro) - List a macro or all macros

---

### EXHIBIT-ADDRESS

**Description:** Define a breakpoint in a program.

**Format:** `EXHIBIT-ADDRESS <program address>,<data address>,<data type>`

**Parameters:**

- `program address`: The instruction that causes the specified variable to be displayed when executed.
- `data address`: The address of the variable to be displayed.
- `data type`: BYTE, HALFWORD, WORD, FLOAT or DOUBLEFLOAT, indicating the size of the variable to be displayed. Default is WORD.

**Rules:**

- With this command a breakpoint is set in the specified <program address>. When the execution reaches this breakpoint, the <data address> and its contents are written to the output device. The data type of the variable may be specified.
- Several variables may be traced simultaneously with this command, as long as the Monitor has room for information about the breakpoints.

**Related Commands:**

- [BREAK](#break) - Set a breakpoint at the specified address
- [RESET-BREAKS](#reset-breaks) - Removes breakpoints

---

### EXIT

**Description:** Exit from the ND-500 Monitor.

**Format:** `EXIT`

**Rules:**

- Returns to the SINTRAN III command processor.
- In the Monitor this command releases the allocated ND-500 resources. If the buffer used by the histogram and logging commands was reserved, it will be released.
- This command is also used to return from the LOOK-AT commands.

---

### EXTRA-FORMAT

**Description:** Select format on output from commands.

**Format:** `EXTRA-FORMAT <format>`

**Parameters:**

- `format`: One of the supported formats or an unambiguous abbreviation of one of them - BYTE, HALFWORD, WORD, FLOAT.

**Rules:**

- Set format of output from commands displaying memory or segment contents. The locations may be displayed in various formats in addition to the format specified in the MAIN-FORMAT command. Data and instructions are then displayed in both the format(s) specified in this command as well as the main format.
- The alternatives are BYTE (The displayed location is divided into bytes and they are displayed in the main format), HALFWORD (Similar to BYTE, except halfwords are displayed. This is effective only when displaying words or doublewords as main format), WORD (Similar to BYTE, except words are displayed. This is effective only when displaying doublewords as main format), and FLOAT (Single precision floating point format).

**Related Commands:**

- [MAIN-FORMAT](#main-format) - Select main format on output from commands

---

### FIX-SEGMENT-ABSOLUTE

**Description:** Fix a segment in a particular part of physical memory.

**Format:** `FIX-SEGMENT-ABSOLUTE <segment number>,<type(P or D)>,<lower address>,<upper address>,<physical address>`

**Parameters:**

- `segment number`: The number of an existing segment.
- `type(P or D)`: P or D, signifying program or data segment.
- `lower address`: The lower boundary of the area to be fixed. Default is the lowest address on the segment.
- `upper address`: The upper boundary of the area to be fixed. Default is the uppermost address of the segment.
- `physical address`: The address in physical memory where the segment should start.

**Rules:**

- <lower address> will be rounded down, <upper address> will be rounded up to the nearest page boundary.
- The specified segment or part of segment is declared to be allocated in a contiguous area in memory, starting at the physical address specified. It will remain in memory until explicitly released through the Monitor command UNFIX-SEGMENT.

**Related Commands:**

- [FIX-SEGMENT-CONTIGOUS](#fix-segment-contigous) - Fix a segment contiguously in physical memory
- [FIX-SEGMENT-SCATTERED](#fix-segment-scattered) - Fix a segment in physical memory
- [UNFIX-SEGMENT](#unfix-segment) - Release a segment previously fixed in memory

---

### FIX-SEGMENT-CONTIGOUS

**Description:** Fix a segment contiguously in physical memory.

**Format:** `FIX-SEGMENT-CONTIGOUS <segment number>,<type(P or D)>,<lower address>,<upper address>`

**Parameters:**

- `segment number`: The number of an existing segment.
- `type(P or D)`: P or D, signifying program or data segment.
- `lower address`: The lower boundary of the area to be fixed. Default is the lowest address on the segment.
- `upper address`: The upper boundary of the area to be fixed. Default is the uppermost address of the segment.

**Rules:**

- <lower address> will be rounded down, <upper address> will be rounded up to the nearest page boundary.
- The segment or part of segment specified is declared to be allocated in a contiguous area of memory, and to be retained in memory until it is explicitly released through the Monitor command UNFIX-SEGMENT.

**Related Commands:**

- [FIX-SEGMENT-ABSOLUTE](#fix-segment-absolute) - Fix a segment in a particular part of physical memory
- [FIX-SEGMENT-SCATTERED](#fix-segment-scattered) - Fix a segment in physical memory
- [UNFIX-SEGMENT](#unfix-segment) - Release a segment previously fixed in memory

---

### FIX-SEGMENT-SCATTERED

**Description:** Fix a segment in physical memory.

**Format:** `FIX-SEGMENT-SCATTERED <segment number>,<type(P or D)>,<lower address>,<upper address>`

**Parameters:**

- `segment number`: The number of an existing segment.
- `type(P or D)`: P or D, signifying program or data segment.
- `lower address`: The lower boundary of the area to be fixed. Default is the lowest address on the segment.
- `upper address`: The upper boundary of the area to be fixed. Default is the uppermost address of the segment.

**Rules:**

- <lower address> will be rounded down, <upper address> will be rounded up to the nearest page boundary.
- The specified segment or part of segment is declared to be allocated in memory, and to be retained in memory until it is explicitly released through the Monitor command UNFIX-SEGMENT.

**Related Commands:**

- [FIX-SEGMENT-ABSOLUTE](#fix-segment-absolute) - Fix a segment in a particular part of physical memory
- [FIX-SEGMENT-CONTIGOUS](#fix-segment-contigous) - Fix a segment contiguously in physical memory
- [UNFIX-SEGMENT](#unfix-segment) - Release a segment previously fixed in memory

---

### GET-FLAG

**Description:** Read the output flag (32-bit word) of a process.

**Format:** `GET-FLAG <process number>`

**Parameters:**

- `process number`: Process number of a process.

**Rules:**

- The output flag (32-bit word) of the specified process is written on the output device in the current main format.
- If the specified process is connected to a terminal, this command must be given from another terminal.

**Related Commands:**

- [SET-FLAG](#set-flag) - Set the flag word of a specified process

---

### GIVE-N500-PAGES

**Description:** Reserve part of common memory for use by the ND-500.

**Format:** `GIVE-N500-PAGES <no. of pages>`

**Parameters:**

- `no. of pages`: The number of pages to be used by ND-500.

**Rules:**

- This command is restricted to user SYSTEM.
- The specified number of pages are taken from the ND-100 and released to the ND-500. If ND-500 already has pages, the specified number of pages is added to those ND-500 had previously.
- All system tables are located in memory belonging to the ND-100. Thus, the number of pages specified will all be available for user processes.

**Related Commands:**

- [TAKE-N500-PAGES](#take-n500-pages) - Release part of common memory previously reserved for exclusive use by the ND-500

---

### GO

**Description:** Starts the execution of an ND-500 program at the specified address.

**Format:** `GO <address>`

**Parameters:**

- `address`: An address within the domain.

**Related Commands:**

- [RUN](#run) - Start an already placed domain
- [CONTINUE](#continue) - Continue execution of a program

---

### GUARD

**Description:** Define a breakpoint as break on reference to a location.

**Format:** `GUARD <address>,<datatype>,[<lower limit>,[<upper limit>]]`

**Parameters:**

- `address`: The address of the variable to be guarded (lowermost byte).
- `datatype`: BYTE, HALFWORD, WORD, FLOAT or DOUBLEFLOAT or abbreviation of one of these, indicating the size of the data element to be traced.
- `lower limit`: The lower limit of the legal value range or upper limit of prohibited range.
- `upper limit`: The upper limit of the legal value range or lower limit of prohibited range.

**Related Commands:**

- [BREAK](#break) - Set a breakpoint at the specified address
- [RESET-GUARD](#reset-guard) - Remove guard breakpoint

---

### HELP

**Description:** Show available commands in the ND-500 Monitor.

**Format:** `HELP <command name>`

**Parameters:**

- `command name`: Any command abbreviation, ambiguous or unambiguous. Default is all commands available.

**Rules:**

- All commands matching <command name> are written with their parameters on the output device. Parameters enclosed in brackets [] are optional parameters that will not be prompted for if not supplied.

**Related Commands:**

- [HELP](#help) - Show available SINTRAN III commands

---

### INSERT-IN-TIME-SLICE

**Description:** Set a process to be time sliced.

**Format:** `INSERT-IN-TIME-SLICE <process number>,<time slice class>`

**Parameters:**

- `process number`: Process number of a process.
- `time slice class`: The selected time slice class.

**Rules:**

- This command is restricted to user SYSTEM.

**Related Commands:**

- [REMOVE-FROM-TIME-SLICE](#remove-from-time-slice) - Remove a process from time slicing

---

### INSPECT-DUMP

**Description:** Set the monitor in inspect mode to look at a dump of the ND-500 Swapper.

**Format:** `INSPECT-DUMP <file name>`

**Parameters:**

- `file name`: File name of a file containing a dump of the ND-500 Swapper's data segment.

**Rules:**

- In inspect mode, the commands LOOK-AT-DATA, LOOK-AT-STACK, LOOK-AT-RELATIVE, LOOK-AT-REGISTER and LIST-TABLE will relate to the file specified.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [RESET-INSPECT-DUMP](#reset-inspect-dump) - Reset inspect mode
- [DUMP-SWAPPER](#dump-swapper) - Write a copy of the ND-500 Swapper to a file

---

### LIST-ACTIVE-PROCESSES

**Description:** List names of all active processes.

**Format:** `LIST-ACTIVE-PROCESSES`

**Rules:**

- The active processes and their process names are written to the output device.
- This command will also list processes not owned by a terminal background program.

**Related Commands:**

- [PROCESS-STATUS](#process-status) - List a summary of the status of all active processes

---

### LIST-ACTIVE-SEGMENTS

**Description:** List all segments used by a process.

**Format:** `LIST-ACTIVE-SEGMENTS <process number>`

**Parameters:**

- `process number`: The number of an active process.

**Rules:**

- This command will list all the segments currently in use by a process, the correspondence between logical and physical segments and the name of the process.
- The <process number> parameter may also take the values OWN or -1, indicating the user's own process, ALL or -2 indicating all active processes.

---

### LIST-DOMAIN

**Description:** List existing domains.

**Format:** `LIST-DOMAIN <domain name>`

**Parameters:**

- `domain name`: The name or abbreviation of names to be listed. Default is all domains belonging to the current user.

**Rules:**

- Writes all domains with names matching <domain name> and their start addresses (if any) on the output device.
- If no domain name is specified, all domains belonging to the current user are listed.
- The domain name can be specified with a user name prefix in parentheses to list domains belonging to other users.

**Related Commands:**

- DOMAIN-STATUS - List detailed status of a domain
- [LIST-STANDARD-DOMAINS](#list-standard-domains) - List all domains defined as standard domains and their segments

---

### LIST-EXECUTION-QUEUE

**Description:** List processes in the ND-500 execution queue.

**Format:** `LIST-EXECUTION-QUEUE <interval>`

**Parameters:**

- `interval`: Time in seconds between each report.

**Rules:**

- The currently executing program, its priority, the queue of jobs for the ND-500 and their priorities are listed on the output device every <interval> seconds.
- The output includes the process number, process name, and priority for each process in the queue.
- The listing continues until interrupted by pressing the escape key.

**Related Commands:**

- [LIST-TIME-QUEUE](#list-time-queue) - List processes in the time queue on the ND-500
- [LIST-ACTIVE-PROCESSES](#list-active-processes) - List names of all active processes

---

### LIST-MACRO

**Description:** List the definition of a macro.

**Format:** `LIST-MACRO <macro name>`

**Parameters:**

- `macro name`: The name of the macro to be listed.

**Rules:**

- The definition of the specified macro is displayed on the output device.
- If the macro name is not found, an error message is displayed.
- This command is useful for reviewing macro definitions before using them.

**Related Commands:**

- [DEFINE-MACRO](#define-macro) - Define a macro for use in the monitor
- [DUMP-MACRO](#dump-macro) - Write the definition of a macro to a file
- [ERASE-MACRO](#erase-macro) - Remove a macro definition

---

### LIST-OPEN-FILES

**Description:** List files open from the ND-500.

**Format:** `LIST-OPEN-FILES`

**Rules:**

- Lists all files opened from a ND-500 program or by the OPEN-FILE command in the Monitor. The list will appear on the output device.
- Files opened locally in the ND-100 will not be listed.
- The output includes the file number, file name, and access mode for each open file.

**Related Commands:**

- [OPEN-FILE](#open-file) - Open a file for access from an ND-500 program
- [CLOSE-FILE](#close-file) - Close a file and disconnect the file number

---

### LIST-PROCESS-TABLE-ENTRY

**Description:** List the process table entry for a process.

**Format:** `LIST-PROCESS-TABLE-ENTRY <process number>`

**Parameters:**

- `process number`: The number of an active process.

**Rules:**

- The process description of the specified process is printed on the specified file. OWN or -1 indicates the user's own process, ALL or -2 indicates all active processes.
- The returned information includes the process segment, the program and data capabilities.

**Related Commands:**

- [LIST-ACTIVE-PROCESSES](#list-active-processes) - List names of all active processes

---

### LIST-SEGMENT-TABLE-ENTRY

**Description:** List the segment table entry for a segment.

**Format:** `LIST-SEGMENT-TABLE-ENTRY <segment number>`

**Parameters:**

- `segment number`: A physical segment number.

**Rules:**

- The information in the physical segment table will be printed on the output device. This information includes the segment name and type, the owner process, the size of the segment, the segment attributes and allocation in the swap file, and the current use of the segment by the active processes.
- <segment number> equal to ALL or -1 indicates all segments.

**Related Commands:**

- [LIST-ACTIVE-SEGMENTS](#list-active-segments) - List all segments used by a process

---

### LIST-STANDARD-DOMAINS

**Description:** List all domains defined as standard domains and their segments.

**Format:** `LIST-STANDARD-DOMAINS`

**Rules:**

- The names of all standard domains and the segments comprising them are listed on the output device.
- This command is permitted for all users.

**Related Commands:**

- [DEFINE-STANDARD-DOMAIN](#define-standard-domain) - Define a domain in the reentrant subsystem table
- [DELETE-STANDARD-DOMAIN](#delete-standard-domain) - Remove the definition of a domain from the reentrant subsystem table

---

### LIST-SWAP-FILE-INFO

**Description:** List information on one or all ND-500 swap files.

**Format:** `LIST-SWAP-FILE-INFO <swap file number>`

**Parameters:**

- `swap file number`: The number of the swap file, starting at 0, or ALL.

**Rules:**

- Information about the swap file is printed on the output device. This information includes both file system statistics and the current usage of the file. If the parameter is given as ALL, information about all swap files defined is printed.

**Related Commands:**

- [DEFINE-SWAP-FILE](#define-swap-file) - Define a file as a swap file for ND-500 segments
- DELETE-SWAP-FILE - Remove the definition of a file as swap file for ND-500 segments

---

### LIST-SYSTEM-PARAMETERS

**Description:** List values of some system parameters.

**Format:** `LIST-SYSTEM-PARAMETERS`

**Rules:**

- System parameters are listed on the output device.

**Related Commands:**

- [SET-SYSTEM-PARAMETERS](#set-system-parameters) - Change the values of some system parameters

---

### LIST-TABLE

**Description:** List all or one element of one of the system tables. Detailed system knowledge is required in order to utilize the information obtained through this command.

**Format:** `LIST-TABLE <table name>,<index>`

**Parameters:**

- `table name`: The name of one of the system tables.
- `index`: Number of one particular element in this table.

**Rules:**

- The table names available are FOLLOW-LINK (Follow the link to the next element in the table), FOLLOW-TABLE (List the next element in the table), PH-SEGM-TAB (List the physical segment table), LAST-N100-MSG (List the ring buffer containing the last 64 messages to ND-500), LAST-N500-MSG (List the messages from ND-100 to ND-500, currently the last 256 messages, provided that a ND-500 Swapper with message-log capabilities is used), MEMORY-MAP (List the memory map), N500-MSG (List the messages to ND-500 from a specified process), PROC-TAB (List process table entries), SW-SEGM-TAB (List the segment table used by software), <octal value> (List the specified entry in the current table), CR (List the next element in the current table), EXIT (Return to the command processor).

---

### LIST-TIME-QUEUE

**Description:** List processes in the time queue on the ND-500.

**Format:** `LIST-TIME-QUEUE <interval>`

**Parameters:**

- `interval`: Interval between each sample in seconds.

**Rules:**

- The processes waiting in the time queue are listed on the output device.

**Related Commands:**

- [LIST-EXECUTION-QUEUE](#list-execution-queue) - List processes in the ND-500 execution queue

---

### LOAD-CONTROL-STORE

**Description:** Load the micro code from disk.

**Format:** `LOAD-CONTROL-STORE <file name>,<start address>,<number of words>`

**Parameters:**

- `file name`: The name of the file from which the micro program is read. Default is CONTROL-STORE:DATA.
- `start address`: The octal address where the first micro-program word should be loaded in control store. Default is 0.
- `number of words`: The number of words to be compared with the file contents after loading. Default is 20000B (entire control store).

**Rules:**

- This command loads microcode into the ND-500 control store.

**Related Commands:**

- [COMPARE-CONTROL-STORE](#compare-control-store) - Compare the contents of the control store with a file

---

### LOAD-SWAPPER

**Description:** Load the ND-500 Swapper into ND-500 memory.

**Format:** `LOAD-SWAPPER <file name>`

**Parameters:**

- `file name`: Name of binary file where the swapper is located. Default file name is (SYSTEM)SWAPPER.

**Rules:**

- This command is restricted to user SYSTEM.
- The swapper process is loaded into ND-500 memory. Normally, this is done automatically when the first ND-500 process is initiated by the monitor, but this command may be useful to load a new copy if there are reasons to believe that the existing one is corrupted, or to load a non-standard version of the swapper process.
- The file type may not be specified but will always be :PSEG and :DSEG.
- The swapper will always run as process number zero.

**Related Commands:**

- [DUMP-SWAPPER](#dump-swapper) - Write a copy of the ND-500 Swapper to a file

---

### LOCAL-TRAP-DISABLE

**Description:** Disable one or all trap conditions.

**Format:** `LOCAL-TRAP-DISABLE <trap condition>`

**Parameters:**

- `trap condition`: One of the trap names or an unambiguous abbreviation or ALL.

**Rules:**

- Bit in the OTE register corresponding to the specified <trap condition> is cleared, thereby disabling trap handling for that trap condition. If ALL is specified, all traps will be locally disabled. This is mainly used in order to override the default setting before a new selection of traps is enabled.
- The routine defined in the exception handler table is not cleared. If the OTE bit is later set (by program or by using the LOCAL-TRAP-ENABLE command in the monitor before execution is started), the routine defined in the LOCAL-TRAP-ENABLE command acts as the default exception handler.

**Related Commands:**

- [LOCAL-TRAP-ENABLE](#local-trap-enable) - Specify an exception handler to handle a specific trap condition
- [ENABLED-TRAPS](#enabled-traps) - Lists the contents of the own trap-enable register (OTE) of the current domain and the mother trap-enable register

---

### LOCAL-TRAP-ENABLE

**Description:** Specify an exception handler to handle a specific trap condition.

**Format:** `LOCAL-TRAP-ENABLE <label>,<trap condition>`

**Parameters:**

- `label`: The name of a user written or library exception handler routine. Default is the standard handler in the library for the specified trap condition.
- `trap condition`: One of the trap names listed or an unambiguous abbreviation.

**Rules:**

- The bit in the OTE register corresponding to the specified trap condition will be set, thereby causing the trap condition to be reacted upon if it occurs. The trap condition parameter must be one or more of the trap names. Abbreviations are legal as long as they are unambiguous.
- Trap condition names include OVERFLOW, INVALID-OPERATION, DIVISION-BY-ZERO, FLOATING-UNDERFLOW, FLOATING-OVERFLOW, BCD-OVERFLOW, ILLEGAL-OPERAND-VALUE, SINGLE-INSTRUCTION-TRAP, BRANCH-TRAP, CALL-TRAP, BREAKPOINT-INSTRUCTION-TRAP, ADDRESS-TRAP-FETCH, ADDRESS-TRAP-READ, ADDRESS-TRAP-WRITE, ADDRESS-ZERO-ACCESS, DESCRIPTOR-RANGE, ILLEGAL-INDEX, STACK-OVERFLOW, STACK-UNDERFLOW, PROGRAMMED-TRAP, DISABLE-PROCESS-SWITCH-TIMEOUT, DISABLE-PROCESS-SWITCH-ERROR, INDEX-SCALING-ERROR, ILLEGAL-INSTRUCTION-CODE, ILLEGAL-OPERAND-SPECIFIER, INSTRUCTION-SEQUENCE-ERROR, PROTECT-VIOLATION.

**Related Commands:**

- [LOCAL-TRAP-DISABLE](#local-trap-disable) - Disable one or all trap conditions
- [ENABLED-TRAPS](#enabled-traps) - Lists the contents of the own trap-enable register (OTE) of the current domain and the mother trap-enable register

---

### LOGOUT-PROCESS

**Description:** Stop a process.

**Format:** `LOGOUT-PROCESS <process>`

**Parameters:**

- `process`: The number of a currently running process.

**Rules:**

- The ND-500 process specified will be aborted and its reserved resources released. Also, the user will be forced to leave the ND-500-MONITOR.
- This is the normal command to remove a user from the ND-500 system. A proper clean-up of the area used by the logged out process is done; it is therefore safer than ABORT-PROCESS. LOGOUT-PROCESS resembles the SINTRAN III command @STOP-TERMINAL for ND-100 processes.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [ABORT-PROCESS](#abort-process) - Abort a process which cannot be stopped in any other way
- STOP-TERMINAL - Terminates a user from a terminal

---

### LOOK-AT

**Description:** By this set of commands it is possible to display and modify register and locations in program and data memory.

**Format:** `LOOK-AT-area`

**Rules:**

- An address in the current segment is specified by its 27 bit segment relative address. An address in an arbitrary segment may be specified as <segment no>'<segment relative address>.
- Generally, modification of program or data is not permanent.

**Related Commands:**

- [LOOK-AT-CONTROL-STORE](#look-at-control-store) - Look at the contents of the control store
- [LOOK-AT-DATA](#look-at-data) - Look at the contents of data memory
- [LOOK-AT-FILE](#look-at-file) - Look at the contents of a file
- [LOOK-AT-HARDWARE](#look-at-hardware) - Look at hardware registers
- [LOOK-AT-PHYSICAL-SEGMENT](#look-at-physical-segment) - Look at the contents of a physical segment
- [LOOK-AT-PROGRAM](#look-at-program) - Look at the contents of program memory
- [LOOK-AT-REGISTER](#look-at-register) - Look at the contents of registers
- [LOOK-AT-RELATIVE](#look-at-relative) - Look at memory relative to a register
- [LOOK-AT-RESIDENT-MEMORY](#look-at-resident-memory) - Look at the contents of resident memory
- [LOOK-AT-STACK](#look-at-stack) - Look at the contents of the stack

---

### LOOK-AT-CONTROL-STORE

**Description:** Command to examine and modify the control store.

**Format:** `LOOK-AT-CONTROL-STORE <address>`

**Parameters:**

- `address`: An octal address in control store, range 0-20000. Default is 0.

**Rules:**

- This command is restricted to user SYSTEM.
- The display is started at the specified address. One control store word and the corresponding address are displayed on one line. On carriage return, the next control-store word is displayed. A control-store word consists of 144 bits which are grouped into nine 16-bit words.
- The next control-store word to be displayed may be specified by typing its address followed by a slash and carriage return.
- By default, the control store is disassembled and displayed symbolically. Symbolic modifying of the control store is performed by either the subcommand EDIT or ORIN. By EDIT the current control-store word is cleared and the disassembled string is then put into the terminal input buffer. It is then possible to modify the disassembled string by the SINTRAN III line editing features. At carriage return the modified string is assembled and written into the control store. By ORIN the next terminal input is assembled and a logical OR of the entered instruction and the old contents is stored into the current control-store word.
- By typing OCTAL the control store is displayed in octal format. The display is returned to the symbolic mode by typing the command SYMBOLIC.
- By typing GROUP only one 16-bit word is displayed. On carriage return the next 16-bit word is displayed. Within GROUP mode it is possible to modify the displayed 16-bit word by typing the new octal value followed by a carriage return. By typing WORD the display of nine 16-bit words continues.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory

---

### LOOK-AT-DATA

**Description:** Command to examine and modify a data segment of a domain.

**Format:** `LOOK-AT-DATA <address>,[<domain>]`

**Parameters:**

- `address`: The segment address from where inspection should start.
- `domain`: The name of an existing domain. Default is inspection of the domain currently in memory.

**Rules:**

- This command is similar to LOOK-AT-PROGRAM except that the data memory or data segment is involved. Modification is always permitted.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory
- [LOOK-AT-PROGRAM](#look-at-program) - Command to examine and modify a program segment of a domain

---

### LOOK-AT-FILE

**Description:** Used to examine or modify files that are used as segments on the ND-500.

**Format:** `LOOK-AT-FILE <address>,<file name>`

**Parameters:**

- `address`: The segment address from where inspection should start.
- `file name`: The name of the file containing the program segment to be used. Default file type is :PSEG.

**Rules:**

- This command is similar to LOOK-AT-PROGRAM except that the segment is always found on a file. Modification is always permitted.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory
- [LOOK-AT-PROGRAM](#look-at-program) - Command to examine and modify a program segment of a domain

---

### LOOK-AT-HARDWARE

**Description:** Command to examine and modify internal ND-500 CPU registers or ND-100/ND-500 interface register.

**Format:** `LOOK-AT-HARDWARE <register name>`

**Parameters:**

- `register name`: The name of an ND-500 CPU register or ND-100/ND-500 interface register. The registers available are listed below.

**Rules:**

- This command is restricted to user SYSTEM.
- Display the contents of the specified internal ND-500 CPU register or ND-100/ND-500 interface register.
- The hardware register name may be one of: INTERFACE (Display the interface registers), Carriage Return (Display the hardware registers, approximately 80 registers), A,XD (Display the registers starting with name A,XD), Register name (Display the specified register), or HMS (Display the 40 Memory management registers).
- Note that after this command the microprogram needs to be restarted (MICRO-START <address>).

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory
- [MICRO-START](#micro-start) - Start the microprogram at the specified address

---

### LOOK-AT-PHYSICAL-SEGMENT

**Description:** Command to examine and modify a physical segment.

**Format:** `LOOK-AT-PHYSICAL-SEGMENT <address>,<segment number>`

**Parameters:**

- `address`: The octal segment relative address to be inspected.
- `segment number`: The number of the physical segment to be inspected.

**Rules:**

- This command is restricted to user SYSTEM.
- Equal to LOOK-AT-PROGRAM or LOOK-AT-DATA, except that a physical segment is inspected and modified directly.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory
- [LOOK-AT-PROGRAM](#look-at-program) - Command to examine and modify a program segment of a domain
- [LOOK-AT-DATA](#look-at-data) - Command to examine and modify a data segment of a domain

---

### LOOK-AT-PROGRAM

**Description:** Command to examine and modify a program segment of a domain.

**Format:** `LOOK-AT-PROGRAM <address>,[<domain>]`

**Parameters:**

- `address`: The segment address from where inspection should start.
- `domain`: The name of an existing domain. Default is inspection of the domain currently in memory.

**Rules:**

- Displays and modifies program memory or program segments. The display is started at the specified <address>.
- If <domain> is specified, the program segment file is displayed and may be modified. Only one segment may be displayed and modified at a time.
- Within the LOOK-AT-PROGRAM command the subcommand BREAK may be specified, setting a breakpoint at the current address.
- If <domain> is not specified, the default is the domain currently in memory. The memory image is inspected, rather than the original segment from which it was loaded. If any modifications are made, the domain must have been placed in memory by the DEBUG-PLACE command, otherwise no modification is legal.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory
- [DEBUG-PLACE](#debug-place) - Place a domain and allow temporary patches to be made before execution

---

### LOOK-AT-REGISTER

**Description:** Command to examine and modify the registers.

**Format:** `LOOK-AT-REGISTER [<register name>]`

**Parameters:**

- `register name`: The name of one of the registers. Default is P.

**Rules:**

- The specified register is displayed in current main format. When carriage return is typed, the next register in the sequence below is displayed. Registers identified as MIC are used by the microprogram and are not available to the user.
- Register sequence: P, L, B, R, I1, I2, I3, I4, A1, A2, A3, A4, E1, E2, E3, E4, 3T1, 3T2, PS, TOS, LL, HL, THA, CED, CAD, MIC, MIC, MIC, MIC, OTE1, OTE2, CTE1, CTE2, MTE1, MTE2, TEMM1, TEMM2

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory

---

### LOOK-AT-RELATIVE

**Description:** Command to examine and modify the memory relative to the contents of a base register.

**Format:** `LOOK-AT-RELATIVE <relative to>`

**Parameters:**

- `relative to`: B, R, I1, I2, I3, I4 or a numeric address. Default is R.

**Rules:**

- Start listing of data memory relative to either the contents of the R, B, I1, I2, I3 or I4 register or an address. Both global and relative address are displayed.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory

---

### LOOK-AT-RESIDENT-MEMORY

**Description:** Command to examine and modify locations in memory.

**Format:** `LOOK-AT-RESIDENT-MEMORY <address>`

**Parameters:**

- `address`: The octal physical address to be inspected.

**Rules:**

- This command is restricted to user SYSTEM.
- Equal to LOOK-AT-DATA except that physical memory is examined and modified.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory
- [LOOK-AT-DATA](#look-at-data) - Command to examine and modify a data segment of a domain

---

### LOOK-AT-STACK

**Description:** Command to examine and modify the program stack.

**Format:** `LOOK-AT-STACK`

**Rules:**

- Examines and modifies the program stack.

**Related Commands:**

- [LOOK-AT](#look-at) - By this set of commands it is possible to display and modify register and locations in program and data memory

---

### MAIN-FORMAT

**Description:** Select format on output from commands.

**Format:** `MAIN-FORMAT <format>`

**Parameters:**

- `format`: OCTAL, HEXADECIMAL or DECIMAL or abbreviation of one of these.

**Rules:**

- When displaying registers, memory contents, or segments with the LOOK-AT commands, the specified <format> is used. When the Monitor is started, octal is used as the main format.
- The default MAIN-FORMAT may be modified by using the MAIN-FORMAT command, then leaving the Monitor by the EXIT command. The memory image can then be copied to a file by using the SINTRAN III command @DUMP. The :PROG file created by the @DUMP command will be equivalent to the existing monitor, but the default MAIN-FORMAT is as specified before the @DUMP.

**Related Commands:**

- [EXTRA-FORMAT](#extra-format) - Select format on output from commands

---

### MASTER-CLEAR

**Description:** Stop the ND-500.

**Format:** `MASTER-CLEAR`

**Rules:**

- ND-500: Brings the ND-500 out of any hang-up state by sending a hardware master-clear signal to the ND-500 interface. This will cause the ND-500 to stop immediately and reset the interface. This is equivalent to pressing the MCL button on the ND-500 front panel.
- This command is used before a complete restart of the ND-500, and the contents of registers are unpredictable.
- This command is restricted to user SYSTEM.

---

### MEMORY-CONFIGURATION

**Description:** List information about memory configuration.

**Format:** `MEMORY-CONFIGURATION`

**Rules:**

- Information about the current memory configuration is printed on the output device.
- When the ND-500 is started the first time, every page in ND-100/ND-500 shared memory belongs to ND-100. Memory is administered through the commands GIVE-N500-PAGES and TAKE-N500-PAGES.

**Related Commands:**

- [DEFINE-MEMORY-CONFIGURATION](#define-memory-configuration) - Define the physical memory configuration to the operating system

---

### MICRO-START

**Description:** Start the ND-500 micro program.

**Format:** `MICRO-START <address>`

**Parameters:**

- `address`: The octal control-store address where execution of the micro program should start.

**Rules:**

- The execution of the ND-500 micro program is started at the specified address.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [MICRO-STOP](#micro-stop) - Stop the ND-500 micro program

---

### MICRO-STOP

**Description:** Stop the ND-500 micro program.

**Format:** `MICRO-STOP`

**Related Commands:**

- [MICRO-START](#micro-start) - Start the ND-500 micro program

---

### OPEN-FILE

**Description:** Open a file for access from an ND-500 program.

**Format:** `OPEN-FILE <file name>,<connect file number>,<access mode>`

**Parameters:**

- `file name`: The name of a file to be used by a program. Default file type is :DATA.
- `connect file number`: The file number used in the program.
- `access mode`: One of the following access modes:
- WE (0): sequential write (OUTBT,OUTST)
- R (1): sequential read (INBT)
- WX (2): random read/write (RFILE/WFILE)
- RX (3): random read (RFILE)
- RW (4): sequential read/write (INBT/OUTBT,OUTST)
- WA (5): sequential write append
- WC (6): random read/write with read/write access allowed from other users (contiguous files only)
- RC (7): random read with read access allowed from other users (contiguous files only)
- D (8): direct transfer
- DC (9): direct transfer with the file closed, mode 9
- WRITE (10): The system will select the most optimal access mode (Rw, WX or D) based on the file/device type


**Related Commands:**

- [CLOSE-FILE](#close-file) - Close a file and disconnect the file number

---

### OUTPUT-FILE

**Description:** Redirect output from the Monitor to a file.

**Format:** `OUTPUT-FILE <file name>`

**Parameters:**

- `file name`: The name of the file to which output is redirected. Default file type is :DATA.

---

### PLACE-DOMAIN

**Description:** Prepare a domain for execution.

**Format:** `PLACE-DOMAIN <domain name>`

**Parameters:**

- `domain name`: The name of a domain in the description file of the current user or the user specified in parentheses as a prefix to <domain name>.

**Rules:**

- An executable ND-500 domain is made ready for execution. The specified <domain name> is searched for on the description file of the current user. If no match is found, the description file of user SYSTEM is scanned. A user name prefixing <domain name> is valid. The syntax is equal to the file system syntax.

**Related Commands:**

- [DEBUG-PLACE](#debug-place) - Place a domain and allow temporary patches to be made before execution

---

### PRINT-HISTOGRAM

**Description:** Print a histogram on the output file.

**Format:** `PRINT-HISTOGRAM`

**Rules:**

- This command prints the histogram on the output device. If sampling has been started and stopped several times, the histogram will represent the sum of all samples since SET-HISTOGRAM. The histogram buffer is not cleared by PRINT-HISTOGRAM.

**Related Commands:**

- [SET-HISTOGRAM](#set-histogram) - Reserve and clear the histogram buffer
- [START-HISTOGRAM](#start-histogram) - Start sampling for a histogram
- [STOP-HISTOGRAM](#stop-histogram) - Stop the histogram sampling

---

### PRINT-MONCALL-LOG

**Description:** Print a count of monitor calls executed on the output file.

**Format:** `PRINT-MONCALL-LOG`

**Rules:**

- A count of monitor calls executed since START-MONCALL-LOG is printed on the output device. Each monitor call number up to 7778 is listed with an individual count. Parts of this range are not valid as monitor call numbers, and will always appear with a count of zero.
- This command does not release the buffer, nor does it clear it. Further monitor calls will add to the count already in the buffer.

**Related Commands:**

- [START-MONCALL-LOG](#start-moncall-log) - Start logging of monitor calls
- [STOP-MONCALL-LOG](#stop-moncall-log) - Stop sampling for the monitor call log

---

### PRINT-PROCESS-LOG

**Description:** Print the accumulated measurements of a process log on the output file.

**Format:** `PRINT-PROCESS-LOG <first process>`

**Parameters:**

- `first process`: The first process to be printed. Default is 0.

**Rules:**

- The accumulated measurements from the last START-PROCESS-LOG-ALL or START-PROCESS-LOG-ONE are printed on the output device. The buffer is not cleared, and the logging is continued, adding subsequent measurements to the printed values. In order to clear the buffer, the START-PROCESS-LOG-ALL or START-PROCESS-LOG-ONE should be used to start the next logging period.
- This command is allowed for user SYSTEM only.

**Related Commands:**

- [START-PROCESS-LOG-ALL](#start-process-log-all) - Start accumulating measurements for the process log (all processes)
- [START-PROCESS-LOG-ONE](#start-process-log-one) - Start accumulating measurements for the process log (one process)

---

### PROCESS-LOG-ALL

**Description:** Process log files for all active processes.

**Format:** `PROCESS-LOG-ALL`

**Rules:**

- Processes all log files for active processes and writes the results to the output device.
- The log files must have been created by START-PROCESS-LOG-ALL.
- This command is useful for analyzing the execution history of all active processes.

**Related Commands:**

- [START-PROCESS-LOG-ALL](#start-process-log-all) - Start logging for all active processes
- [PROCESS-LOG-ONE](#process-log-one) - Process log file for a specific process

---

### PROCESS-LOG-ONE

**Description:** Process log file for a specific process.

**Format:** `PROCESS-LOG-ONE <process number>`

**Parameters:**

- `process number`: The number of the process whose log file is to be processed.

**Rules:**

- Processes the log file for the specified process and writes the results to the output device.
- The log file must have been created by START-PROCESS-LOG-ONE.
- This command is useful for analyzing the execution history of a specific process.

**Related Commands:**

- [START-PROCESS-LOG-ONE](#start-process-log-one) - Start logging for a specific process
- [PROCESS-LOG-ALL](#process-log-all) - Process log files for all active processes

---

### PROCESS-STATUS

**Description:** Display status information for a specific process.

**Format:** `PROCESS-STATUS <process number>`

**Parameters:**

- `process number`: The number of the process whose status is to be displayed.

**Rules:**

- Displays detailed status information for the specified process on the output device.
- The output includes the process name, priority, state, and other relevant information.
- This command is useful for monitoring and debugging process execution.

**Related Commands:**

- [LIST-ACTIVE-PROCESSES](#list-active-processes) - List names of all active processes
- [WHO-IS-ON](#who-is-on) - List all active processes and their users

---

### RECOVER-DOMAIN

**Description:** Recover a domain that has been marked as unrecoverable.

**Format:** `RECOVER-DOMAIN <domain name>`

**Parameters:**

- `domain name`: The name of the domain to be recovered.

**Rules:**

- Attempts to recover a domain that has been marked as unrecoverable.
- The domain must exist and must have been previously marked as unrecoverable.
- This command is restricted to user SYSTEM.
- If recovery is successful, the domain will be available for normal use.

**Related Commands:**

- [LIST-DOMAIN](#list-domain) - List existing domains
- DOMAIN-STATUS - List detailed status of a domain

---

### RELEASE-HISTOGRAM

**Description:** Release the histogram buffer and stop histogram collection.

**Format:** `RELEASE-HISTOGRAM`

**Rules:**

- Releases the histogram buffer and stops histogram collection.
- This command should be used after PRINT-HISTOGRAM to free system resources.
- The histogram data will be lost after this command is executed.

**Related Commands:**

- [START-HISTOGRAM](#start-histogram) - Start histogram collection
- [PRINT-HISTOGRAM](#print-histogram) - Print the collected histogram data

---

### RELEASE-LOG-BUFFER

**Description:** Release the log buffer and stop log collection.

**Format:** `RELEASE-LOG-BUFFER`

**Rules:**

- Releases the log buffer and stops log collection.
- This command should be used after printing log data to free system resources.
- The log data will be lost after this command is executed.
- This command affects all types of logs (process, moncall, and swapping).

**Related Commands:**

- [START-PROCESS-LOG-ALL](#start-process-log-all) - Start logging for all active processes
- [START-MONCALL-LOG](#start-moncall-log) - Start logging of monitor calls

---

### REMOVE-FROM-TIME-SLICE

**Description:** Remove a process from time slicing.

**Format:** `REMOVE-FROM-TIME-SLICE <process number>`

**Parameters:**

- `process number`: The number of the process to be removed from time slicing.

**Rules:**

- Removes the specified process from time slicing.
- The process will continue to run but will no longer be subject to time slice limits.
- This command is restricted to user SYSTEM.
- The process must be currently active and time-sliced.

**Related Commands:**

- [INSERT-IN-TIME-SLICE](#insert-in-time-slice) - Add a process to time slicing
- [LIST-TIME-SLICED-PROGRAMS](#list-time-sliced-programs) - List all time-sliced programs

---

### RESET-AUTOMATIC-ERROR-MESSAGE

**Description:** Reset the automatic error message flag.

**Format:** `RESET-AUTOMATIC-ERROR-MESSAGE`

**Rules:**

- Resets the automatic error message flag to its default state.
- This command is used to disable automatic error message display.
- Error messages will no longer be automatically displayed when errors occur.

**Related Commands:**

- [AUTOMATIC-ERROR-MESSAGE](#automatic-error-message) - Enable automatic error message display

---

### RESET-BRANCH-TRACE

**Description:** Reset branch tracing for a process.

**Format:** `RESET-BRANCH-TRACE <process number>`

**Parameters:**

- `process number`: The number of the process for which branch tracing is to be reset.

**Rules:**

- Disables branch tracing for the specified process.
- This command is used to stop branch tracing that was started with BRANCH-TRACE.
- The process must be currently active.

**Related Commands:**

- [BRANCH-TRACE](#branch-trace) - Start branch tracing for a process
- [RESET-TRACE](#reset-trace) - Reset all tracing for a process

---

### RESET-BREAKS

**Description:** Remove all breakpoints from a process.

**Format:** `RESET-BREAKS <process number>`

**Parameters:**

- `process number`: The number of the process from which breakpoints are to be removed.

**Rules:**

- Removes all breakpoints that were set for the specified process.
- This command is used to clear all breakpoints set by BREAK or TEMPORARY-BREAK.
- The process must be currently active.

**Related Commands:**

- [BREAK](#break) - Set a breakpoint in a process
- [TEMPORARY-BREAK](#temporary-break) - Set a temporary breakpoint in a process

---

### RESET-CALL-TRACE

**Description:** Reset call tracing for a process.

**Format:** `RESET-CALL-TRACE <process number>`

**Parameters:**

- `process number`: The number of the process for which call tracing is to be reset.

**Rules:**

- Disables call tracing for the specified process.
- This command is used to stop call tracing that was started with CALL-TRACE.
- The process must be currently active.

**Related Commands:**

- [CALL-TRACE](#call-trace) - Start call tracing for a process
- [RESET-TRACE](#reset-trace) - Reset all tracing for a process

---

### RESET-DEBUG

**Description:** Reset debug state for a process.

**Format:** `RESET-DEBUG <process number>`

**Parameters:**

- `process number`: The number of the process for which debug state is to be reset.

**Rules:**

- Resets all debug settings for the specified process.
- This command clears all breakpoints, traces, and other debug-related settings.
- The process must be currently active.

**Related Commands:**

- [DEBUGGER](#debugger) - Enter the debugger for a process
- [RESET-BREAKS](#reset-breaks) - Remove all breakpoints from a process

---

### RESET-GUARD

**Description:** Reset guard settings for a process.

**Format:** `RESET-GUARD <process number>`

**Parameters:**

- `process number`: The number of the process for which guard settings are to be reset.

**Rules:**

- Removes all guard settings for the specified process.
- This command is used to disable memory access protection.
- The process must be currently active.

**Related Commands:**

- [GUARD](#guard) - Set guard for memory access protection

---

### RESET-INSPECT-DUMP

**Description:** Reset inspect dump settings for a process.

**Format:** `RESET-INSPECT-DUMP <process number>`

**Parameters:**

- `process number`: The number of the process for which inspect dump settings are to be reset.

**Rules:**

- Removes all inspect dump settings for the specified process.
- This command is used to disable automatic dump inspection.
- The process must be currently active.

**Related Commands:**

- [INSPECT-DUMP](#inspect-dump) - Inspect a process dump

---

### RESET-LAST-BREAK

**Description:** Reset the last break for a process.

**Format:** `RESET-LAST-BREAK <process number>`

**Parameters:**

- `process number`: The number of the process for which the last break is to be reset.

**Rules:**

- Removes the last break point for the specified process.
- This command is used to clear the most recently set breakpoint.
- The process must be currently active.

**Related Commands:**

- [BREAK](#break) - Set a breakpoint in a process
- [RESET-BREAKS](#reset-breaks) - Remove all breakpoints from a process

---

### RESET-TRACE

**Description:** Reset all tracing for a process.

**Format:** `RESET-TRACE <process number>`

**Parameters:**

- `process number`: The number of the process for which all tracing is to be reset.

**Rules:**

- Disables all tracing for the specified process.
- This command is used to stop all types of tracing (branch, call, etc.).
- The process must be currently active.

**Related Commands:**

- [BRANCH-TRACE](#branch-trace) - Start branch tracing for a process
- [CALL-TRACE](#call-trace) - Start call tracing for a process

---

### RESIDENT-PLACE

**Description:** Place a domain in resident memory.

**Format:** `RESIDENT-PLACE <domain name>`

**Parameters:**

- `domain name`: The name of the domain to be placed in resident memory.

**Rules:**

- Places the specified domain in resident memory.
- The domain will remain in memory and not be swapped out.
- This command is restricted to user SYSTEM.
- The domain must exist and be valid.

**Related Commands:**

- [PLACE-DOMAIN](#place-domain) - Place a domain in memory
- [LIST-ACTIVE-SEGMENTS](#list-active-segments) - List all active segments in memory

---

### RESTART-PROCESS

**Description:** Restart a process from its initial state.

**Format:** `RESTART-PROCESS <process number>`

**Parameters:**

- `process number`: The number of the process to be restarted.

**Rules:**

- Restarts the specified process from its initial state.
- All resources used by the process are released and reallocated.
- The process must be currently active.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [ABORT-PROCESS](#abort-process) - Abort a process
- [PROCESS-STATUS](#process-status) - Display status information for a process

---

### RESUME-MACRO

**Description:** Resume execution of a suspended macro.

**Format:** `RESUME-MACRO <macro name>`

**Parameters:**

- `macro name`: The name of the macro to be resumed.

**Rules:**

- Resumes execution of a previously suspended macro.
- The macro must have been suspended by a breakpoint or other interruption.
- The macro must exist and be valid.

**Related Commands:**

- [EXECUTE-MACRO](#execute-macro) - Execute a macro
- [LIST-MACRO](#list-macro) - List the definition of a macro

---

### RUN

**Description:** Start execution of a process.

**Format:** `RUN <process number>`

**Parameters:**

- `process number`: The number of the process to be started.

**Rules:**

- Starts execution of the specified process.
- The process must be in a ready state.
- The process must be currently active.

**Related Commands:**

- [GO](#go) - Continue execution of a process
- [PROCESS-STATUS](#process-status) - Display status information for a process

---

### SET-BLOCK-SIZE

**Description:** Set the block size for a file.

**Format:** `SET-BLOCK-SIZE <file number>,<block size>`

**Parameters:**

- `file number`: The number of the file for which the block size is to be set.
- `block size`: The new block size in words.

**Rules:**

- Sets the block size for the specified file.
- The file must be currently open.
- The block size must be a valid value.

**Related Commands:**

- [OPEN-FILE](#open-file) - Open a file for access
- [CLOSE-FILE](#close-file) - Close a file

---

### SET-FLAG

**Description:** Set a flag for a process.

**Format:** `SET-FLAG <process number>,<flag number>`

**Parameters:**

- `process number`: The number of the process for which the flag is to be set.
- `flag number`: The number of the flag to be set.

**Rules:**

- Sets the specified flag for the process.
- The process must be currently active.
- The flag number must be valid.

**Related Commands:**

- [GET-FLAG](#get-flag) - Get the value of a flag
- [PROCESS-STATUS](#process-status) - Display status information for a process

---

### SET-HISTOGRAM

**Description:** Set histogram parameters.

**Format:** `SET-HISTOGRAM <interval>,<first address>,<last address>`

**Parameters:**

- `interval`: The time interval between samples in milliseconds.
- `first address`: The first address to be monitored.
- `last address`: The last address to be monitored.

**Rules:**

- Sets the parameters for histogram collection.
- The interval must be a valid time value.
- The address range must be valid.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [START-HISTOGRAM](#start-histogram) - Start histogram collection
- [PRINT-HISTOGRAM](#print-histogram) - Print the collected histogram data

---

### SET-MEMORY-CONTENTS

**Description:** Set the contents of memory locations.

**Format:** `SET-MEMORY-CONTENTS <address>,<value>`

**Parameters:**

- `address`: The memory address to be modified.
- `value`: The new value to be stored at the address.

**Rules:**

- Sets the contents of the specified memory location.
- The address must be valid and accessible.
- This command is restricted to user SYSTEM.
- Use with caution as it can affect system stability.

**Related Commands:**

- [LOOK-AT](#look-at) - Display the contents of memory locations
- [LOOK-AT-DATA](#look-at-data) - Display the contents of data memory

---

### SET-ND-500-AVAILABLE

**Description:** Make the ND-500 available for use.

**Format:** `SET-ND-500-AVAILABLE`

**Rules:**

- Makes the ND-500 available for use by processes.
- This command is restricted to user SYSTEM.
- The ND-500 must be in a valid state.

**Related Commands:**

- [SET-ND-500-UNAVAILABLE](#set-nd-500-unavailable) - Make the ND-500 unavailable for use
- [STATUS](#status) - Display system status

---

### SET-ND-500-UNAVAILABLE

**Description:** Make the ND-500 unavailable for use.

**Format:** `SET-ND-500-UNAVAILABLE`

**Rules:**

- Makes the ND-500 unavailable for use by processes.
- This command is restricted to user SYSTEM.
- The ND-500 must be in a valid state.
- No processes should be actively using the ND-500.

**Related Commands:**

- [SET-ND-500-AVAILABLE](#set-nd-500-available) - Make the ND-500 available for use
- [STATUS](#status) - Display system status

---

### SET-PHYSICAL-SEGMENT-ADDRESS

**Description:** Set the physical address of a segment.

**Format:** `SET-PHYSICAL-SEGMENT-ADDRESS <segment number>,<address>`

**Parameters:**

- `segment number`: The number of the segment to be modified.
- `address`: The new physical address for the segment.

**Rules:**

- Sets the physical address of the specified segment.
- The segment must exist and be valid.
- The address must be valid and accessible.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [LIST-ACTIVE-SEGMENTS](#list-active-segments) - List all active segments in memory
- [LOOK-AT-PHYSICAL-SEGMENT](#look-at-physical-segment) - Display the contents of a physical segment

---

### SET-PRIORITY

**Description:** Set the priority of a process.

**Format:** `SET-PRIORITY <process number>,<priority>`

**Parameters:**

- `process number`: The number of the process whose priority is to be set.
- `priority`: The new priority value (0-255).

**Rules:**

- Sets the priority of the specified process.
- The process must be currently active.
- The priority must be a valid value (0-255).
- Higher numbers indicate higher priority.

**Related Commands:**

- [PROCESS-STATUS](#process-status) - Display status information for a process
- [LIST-EXECUTION-QUEUE](#list-execution-queue) - List processes in the execution queue

---

### SET-PROCESS-NAME

**Description:** Set the name of a process.

**Format:** `SET-PROCESS-NAME <process number>,<name>`

**Parameters:**

- `process number`: The number of the process whose name is to be set.
- `name`: The new name for the process.

**Rules:**

- Sets the name of the specified process.
- The process must be currently active.
- The name must be a valid string.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [PROCESS-STATUS](#process-status) - Display status information for a process
- [WHO-IS-ON](#who-is-on) - List all active processes and their users

---

### SET-SEGMENT-LIMITS

**Description:** Set the limits of a segment.

**Format:** `SET-SEGMENT-LIMITS <segment number>,<lower limit>,<upper limit>`

**Parameters:**

- `segment number`: The number of the segment whose limits are to be set.
- `lower limit`: The new lower limit for the segment.
- `upper limit`: The new upper limit for the segment.

**Rules:**

- Sets the lower and upper limits of the specified segment.
- The segment must exist and be valid.
- The limits must be valid addresses.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [LIST-ACTIVE-SEGMENTS](#list-active-segments) - List all active segments in memory
- [LOOK-AT-PHYSICAL-SEGMENT](#look-at-physical-segment) - Display the contents of a physical segment

---

### SET-SYSTEM-PARAMETERS

**Description:** Set system-wide parameters.

**Format:** `SET-SYSTEM-PARAMETERS <parameter name>,<value>`

**Parameters:**

- `parameter name`: The name of the parameter to be set.
- `value`: The new value for the parameter.

**Rules:**

- Sets the value of a system-wide parameter.
- The parameter must be a valid system parameter.
- The value must be appropriate for the parameter.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [LIST-SYSTEM-PARAMETERS](#list-system-parameters) - List all system parameters
- [STATUS](#status) - Display system status

---

### SPECIAL-DEBUGGER

**Description:** Enter the special debugger for a process.

**Format:** `SPECIAL-DEBUGGER <process number>`

**Parameters:**

- `process number`: The number of the process to be debugged.

**Rules:**

- Enters the special debugger for the specified process.
- The process must be currently active.
- This command is restricted to user SYSTEM.
- The special debugger provides advanced debugging capabilities.

**Related Commands:**

- [DEBUGGER](#debugger) - Enter the standard debugger for a process
- [RESET-DEBUG](#reset-debug) - Reset debug state for a process

---

### START-HISTOGRAM

**Description:** Start histogram collection.

**Format:** `START-HISTOGRAM`

**Rules:**

- Starts collecting histogram data.
- The histogram parameters must have been set using SET-HISTOGRAM.
- This command is restricted to user SYSTEM.
- The histogram data will be collected until stopped.

**Related Commands:**

- [SET-HISTOGRAM](#set-histogram) - Set histogram parameters
- [PRINT-HISTOGRAM](#print-histogram) - Print the collected histogram data

---

### START-MONCALL-LOG

**Description:** Start logging of monitor calls.

**Format:** `START-MONCALL-LOG`

**Rules:**

- Starts logging all monitor calls.
- The log data will be collected until stopped.
- This command is restricted to user SYSTEM.
- The log buffer must be available.

**Related Commands:**

- [PRINT-MONCALL-LOG](#print-moncall-log) - Print the collected monitor call log
- [STOP-MONCALL-LOG](#stop-moncall-log) - Stop logging of monitor calls

---

### START-PROCESS-LOG-ALL

**Description:** Start logging for all active processes.

**Format:** `START-PROCESS-LOG-ALL`

**Rules:**

- Starts logging for all currently active processes.
- The log data will be collected until stopped.
- This command is restricted to user SYSTEM.
- The log buffer must be available.

**Related Commands:**

- [PROCESS-LOG-ALL](#process-log-all) - Process log files for all active processes
- STOP-PROCESS-LOG-ALL - Stop logging for all active processes

---

### START-PROCESS-LOG-ONE

**Description:** Start logging for a specific process.

**Format:** `START-PROCESS-LOG-ONE <process number>`

**Parameters:**

- `process number`: The number of the process to be logged.

**Rules:**

- Starts logging for the specified process.
- The process must be currently active.
- The log data will be collected until stopped.
- This command is restricted to user SYSTEM.

**Related Commands:**

- [PROCESS-LOG-ONE](#process-log-one) - Process log file for a specific process
- STOP-PROCESS-LOG-ONE - Stop logging for a specific process

---

### START-SWAPPER

**Description:** Start the memory swapper.

**Format:** `START-SWAPPER`

**Rules:**

- Starts the memory swapper process.
- This command is restricted to user SYSTEM.
- The swapper must be properly configured.
- The system must be in a valid state.

**Related Commands:**

- STOP-SWAPPER - Stop the memory swapper
- [SWAPPING-LOG](#swapping-log) - Display swapper activity log

---

### STEP

**Description:** Execute a single instruction in the program.

**Format:** `STEP [<address>]`

**Parameters:**

- `address`: Optional program address where execution should start. If not specified, the next instruction in sequence will be executed.

**Rules:**

- This command may be used immediately after a domain has been placed in memory by the PLACE-DOMAIN (or DEBUG-PLACE) command.
- More commonly it is used when the program is in a temporary halt state after a breakpoint has been detected.
- A break is then inserted immediately before the program address where the tracing should start.
- From this point on, single instruction execution is started.
- If desired, the contents of any register or data location may be inspected after each instruction executed.
- Any intermediate command (other than CR) will require that STEP be re-specified in order to continue single step execution.
- Default parameters to the STEP command will cause the next instruction in sequence to be executed.

**Related Commands:**

- [DEBUG-PLACE](#debug-place) - Place a domain in memory for debugging
- [PLACE-DOMAIN](#place-domain) - Place a domain in memory

---

### STOP-HISTOGRAM

**Description:** Stop the histogram sampling.

**Format:** `STOP-HISTOGRAM`

**Rules:**

- Permitted for all users.
- STOP-HISTOGRAM is performed as part of PRINT-HISTOGRAM.

**Related Commands:**

- [DEFINE-HISTOGRAM](#define-histogram) - Define intervals for measuring CPU time
- [PRINT-HISTOGRAM](#print-histogram) - Print a histogram on the output file
- [START-HISTOGRAM](#start-histogram) - Start histogram sampling

---

### STOP-MONCALL-LOG

**Description:** Stop sampling for the monitor call log.

**Format:** `STOP-MONCALL-LOG`

**Rules:**

- The log buffer is released, and no further logging of monitor calls will be done.
- Other users may use the HISTOGRAM, the PROCESS-LOG, the MONCALL-LOG and the SWAPPING-LOG commands.
- If the buffer is not released through this command, it will automatically be released when the user leaves the Monitor.

**Related Commands:**

- [START-MONCALL-LOG](#start-moncall-log) - Start sampling for the monitor call log
- [PRINT-MONCALL-LOG](#print-moncall-log) - Print the monitor call log

---

### STOP-ND-500

**Description:** Stop the ND-500 Monitor and return control to ND-100.

**Format:** `STOP-ND-500`

**Rules:**

- This command is restricted to user SYSTEM.
- All resources are released when the command is executed.
- All processes are terminated.
- All files are closed.
- All memory is released.

**Related Commands:**

- [SET-ND-500-AVAILABLE](#set-nd-500-available) - Make ND-500 available for use
- [SET-ND-500-UNAVAILABLE](#set-nd-500-unavailable) - Make ND-500 unavailable for use

---

### SWAPPING-LOG

**Description:** Start sampling for the swapping log.

**Format:** `SWAPPING-LOG <interval>
`

**Parameters:**

- `interval`: The period in seconds between each report

**Rules:**

- This command will clear the log buffer and reserve it for the user issuing the command
- The buffer is the same as the one used in the PROCESS-LOG, MONCALL-LOG and HISTOGRAM commands, which means that only one user at a time can use any of these commands
- Logging of swapping is started, and will be written to the output device every <interval> seconds
- The logging is stopped by pressing the escape key
- Each report will include values for the last interval, the average per interval since logging was started and the total
- For each of these, a count of page faults, transfers, the total free space etc. will be listed
- This command is allowed for user SYSTEM only

**Example:**

```
SWAPPING-LOG 60 
```

---

### SYSTEM-TRAP-DISABLE

**Description:** Disable handling of the trap conditions specified.

**Format:** `SYSTEM-TRAP-DISABLE <trap condition> ...`

**Parameters:**

- `trap condition`: One of the trap names listed below or an unambiguous abbreviation.

**Rules:**

- The trap conditions specified will not be reacted upon by the system when the condition occurs.
- A number of trap conditions may not be system disabled. If a modification of these traps is attempted, an error message is issued and the command ignored.

**Related Commands:**

- [SYSTEM-TRAP-ENABLE](#system-trap-enable) - Enable handling of trap conditions

---

### SYSTEM-TRAP-ENABLE

**Description:** Enable handling of the trap conditions specified.

**Format:** `SYSTEM-TRAP-ENABLE <trap condition>`

**Parameters:**

- `trap condition`: One of the trap names listed below or an unambiguous abbreviation. Trap condition names include OVERFLOW, INVALID-OPERATION, DIVISION-BY-ZERO, FLOATING-UNDERFLOW, FLOATING-OVERFLOW, BCD-OVERFLOW, ILLEGAL-OPERAND-VALUE, SINGLE-INSTRUCTION-TRAP, BRANCH-TRAP, CALL-TRAP, BREAKPOINT-INSTRUCTION-TRAP, ADDRESS-TRAP-FETCH, ADDRESS-TRAP-READ, ADDRESS-TRAP-WRITE, ADDRESS-ZERO-ACCESS, DESCRIPTOR-RANGE, ILLEGAL-INDEX, STACK-OVERFLOW, STACK-UNDERFLOW, PROGRAMMED-TRAP, DISABLE-PROCESS-SWITCH-TIMEOUT, DISABLE-PROCESS-SWITCH-ERROR, INDEX-SCALING-ERROR, ILLEGAL-INSTRUCTION-CODE, ILLEGAL-OPERAND-SPECIFIER, INSTRUCTION-SEQUENCE-ERROR, PROTECT-VIOLATION.

**Rules:**

- The trap condition specified will be handled by the Monitor residing in the ND-100 when the condition occurs.
- It will be given a standard treatment, which varies with the kind of trap.
- If a local trap handler is defined and the local trap enabled, it will be used rather than the system trap handler.
- System trap handling is used only for those trap conditions that are locally disabled or have no local trap handling defined.

**Related Commands:**

- [SYSTEM-TRAP-DISABLE](#system-trap-disable) - Disable handling of trap conditions

---

### TAKE-N500-PAGES

**Description:** Release part of common memory previously reserved for exclusive use by the ND-500.

**Format:** `TAKE-N500-PAGES <no. of pages>`

**Parameters:**

- `no. of pages`: The number of pages to be returned to ND-100.

**Rules:**

- This command is restricted to user SYSTEM.
- The specified number of pages are taken from the ND-500 and given to the ND-100.
- The number specified should be less than or equal to the number given to ND-500 previously with the GIVE-N500-PAGES command, otherwise the number of pages actually released is returned.

**Related Commands:**

- [GIVE-N500-PAGES](#give-n500-pages) - Reserve part of common memory for exclusive use by the ND-500

---

### TEMPORARY-BREAK

**Description:** Define a temporary breakpoint.

**Format:** `TEMPORARY-BREAK <address>,[<count>],[<command>]`

**Parameters:**

- `address`: The program address where the breakpoint is to be set.
- `count`: One plus the number of times the breakpoint should be ignored before a break is performed. Default is 1.
- `command`: Command to be executed on a break.

**Rules:**

- Similar to BREAK except that when the breakpoint is reached, the original instruction is permanently restored, and will not cause a break next time the instruction is executed.

**Related Commands:**

- [BREAK](#break) - Define a breakpoint

---

### TIME-USED

**Description:** Report CPU time used by current process.

**Format:** `TIME-USED`

**Rules:**

- This command prints the ND-500 and ND-100 CPU time and clock time elapsed from the moment that the ND-500 Monitor was entered.

---

### TRACE

**Description:** Define a trace condition.

**Format:** `TRACE <address>,[<datatype>]`

**Parameters:**

- `address`: The address of the variable to be traced (lowermost byte).
- `datatype`: BYTE, HALFWORD, WORD, FLOAT or DOUBLEFLOAT or abbreviation of one of these, indicating the size of the data element to be traced.

**Rules:**

- Whenever the location starting at the specified address is modified during program execution, its new value is displayed on the output device.
- This command uses the low and high limit registers, LL and HL of the ND-500 exclusively, that is, the previous command using these registers (GUARD or TRACE) will be discontinued.

**Related Commands:**

- [GUARD](#guard) - Guard a memory area
- [RESET-TRACE](#reset-trace) - Reset trace condition

---

### UNFIX-SEGMENT

**Description:** Release a segment previously fixed in memory.

**Format:** `UNFIX-SEGMENT <segment number>,<type (P or D)>`

**Parameters:**

- `segment number`: The name of a segment which has previously entirely or in part been fixed in memory through one of the above commands.
- `type (P or D)`: P or D, indicating program or data segment, respectively.

**Rules:**

- The area occupied by a segment, or part of segment, previously specified as fixed in memory, is unfixed.
- The freed space may be used by other segments.
- The command has no effect before every process that has fixed the segment has released or unfixed it.

**Related Commands:**

- [FIX-SEGMENT-ABSOLUTE](#fix-segment-absolute) - Fix a segment in memory at an absolute address
- [FIX-SEGMENT-CONTIGOUS](#fix-segment-contigous) - Fix a segment in memory contiguously
- [FIX-SEGMENT-SCATTERED](#fix-segment-scattered) - Fix a segment in memory in a scattered manner

---

### VALUE-ENTRIES

**Description:** Print the value of a defined symbol.

**Format:** `VALUE-ENTRIES <entries>`

**Parameters:**

- `entries`: The name of a defined symbol.

**Rules:**

- Prints the values of the labels specified on the output device.
- The value is printed in octal format.
- The label will also be identified as a program or as a data segment label.

---

### VERSION

**Description:** Print the version of the current running ND-500 Monitor and Swapper.

**Format:** `VERSION`

---

### WHO-IS-ON

**Description:** List all users currently logged on to the system.

**Format:** `WHO-IS-ON`

**Related Commands:**

- [PROCESS-STATUS](#process-status) - Display status of a process

---

## SINTRAN-Appendix

# APPENDIX A
## ND Terminal Types

The list shows the current types defined by Norsk Data A.S. New
numbers are allocated as ND adds new terminal types to systems.

To implement a non-standard terminal type see the VTM-compound
program, explained in the SINTRAN III Utilities Manual, ND-60.151.

## Terminal Type Number Calculation

The terminal type number is calculated as follows:

| Bit | Description |
|-----|-------------|
| 15  | Reserved |
| 14  | Set to one if the terminal is a VDU (not hard copy) |
| 13  | Set to one if the terminal handles the ASCII backspace character (BS) properly |
| 12  | Set to one if ASCII form feed (FF) gives new page or clear screen |
| 11  | Set to one if the VDU has cursor positioning (either directly or by use of cursor arrows) |
| 10  | Set if the terminal utilises ASCII escape (ESC) within input sequences |
| 9-8 | Not used |
| 7-0 | Terminal model number. See table below |

## Example

The number for TANDBERG TDV-2115-STD on logical device number 49 is
set as follows:

```
@SET-TERMINAL-TYPE 49,164003B
```

or more permanently

```
@SINTRAN-SERVICE-PROGRAM
*CHANGE-DATAFIELD <terminal number> INPUT YES YES YES
CTTYP/164003
.
EXIT
```

## Terminal Types Table

| Model Name                       | VDU | BS | FF | Model No. | ND St. | Comments              |
| :------------------------------- | :-- | :-- | :-- | :-------- | :----- | :-------------------- |
| DUMMY                            |     |    |    | 0         |        | Terminal type not set |
| VISTAR-OLD                       | X   |    | X  | 1         |        |                       |
| TELETYPE-ASR-33                  | X   | X  |    | 2         | X      |                       |
| TANDBERG TDV-2115                | X   | X  | X  | 3         | X      |                       |
| INFOTON-200-1                    | X   | X  | X  | 4         |        |                       |
| INFOTON-400                      | X   | X  | X  | 5         |        |                       |
| DEC-VT100                        | X   | X  |    | 6         |        | 80-col. mode          |
| TANDBERG TDV-2000                | X   | X  |    | 7         |        |                       |
| BEEHIVE-100                      | X   | X  |    | 8         |        |                       |
| ND-NCT                           | X   | X  | X  | 9         |        |                       |
| HAZELTINE-1520                   | X   | X  |    | 10        |        |                       |
| DEC-LA36                         |     | X  |    | 11        | X      | Decwriter-II          |
| VISTAR-GTX                       | X   | X  |    | 12        |        |                       |
| DEC-VT52                         | X   | X  |    | 29        |        |                       |
| TEC-501/502                      | X   | X  |    | 30        |        |                       |
| DACOLL-242                       | X   | X  |    | 31        |        |                       |
| NEWBURY-7000/3                   | X   | X  |    | 32        |        |                       |
| TELEVIDEO-912/920                | X   | X  |    | 33        |        |                       |
| VISUAL-200                       | X   | X  |    | 34        |        |                       |
| LEAR-SIEGLER-ADM-3A              | X   | X  |    | 35        |        |                       |
| TANDBERG TDV-2215-EXTENDED       | X   | X  |    | 36        | X      |                       |
| VOLKER-CRAIG-VC404               | X   | X  |    | 37        |        |                       |
| VOLKER-CRAIG-VC410               | X   | X  | X  | 38        |        |                       |
| VOLKER-CRAIG-VC414               | X   | X  |    | 39        |        |                       |
| HEWLETT-PACKARD-2621A            | X   | X  |    | 40        |        |                       |
| DATA-MEDIA-ELITE-3045            | X   | X  |    | 41        |        |                       |
| BEEHIVE-MINIBEE                  | X   |    |    | 42        |        |                       |
| PERICOM-6800                     | X   | X  | X  | 43        |        | 80-col.mode           |
| LEAR-SIEGLER-ADM-31              | X   | X  |    | 44        |        |                       |
| BEEHIVE-DM5A                     | X   | X  |    | 45        |        |                       |
| FACIT-4420                       | X   | X  | X  | 46        |        | VT52-mode             |
| ADDS-VIEWPOINT                   | X   | X  | X  | 47        |        |                       |
| HAZELTINE-EXECUTIVE-80           | X   | X  |    | 48        |        |                       |
| AMPEX-DIALOGUE-80                | X   | X  |    | 49        |        |                       |
| VOLKER-CRAIG-VC4404              | X   | X  |    | 50        |        | ADM-3A                |
| DATA-MEDIA-ELITE-1520/1521       | X   | X  | X  | 51        |        |                       |
| TANDBERG TDV-2215-SDS-V2         | X   | X  |    | 52        | X      |                       |
| TANDBERG TDV-2200/9-ND NOTIS     | X   | X  |    | 53        | X      |                       |
| TANDBERG TDV-2220                | X   | X  |    | 54        |        |                       |
| TANDBERG TDV-2200/9-ND-NET       | X   | X  |    | 55        |        |                       |
| FACIT-4420-ND NOTIS              | X   | X  | X  | 57        | X      |                       |
| NOKIA-VDU210                     | X   | X  |    | 58        |        |                       |
| LEAR-SIEGLER-ADM-42              | X   | X  |    | 66        |        |                       |
| LEAR-SIEGLER-ADM-32              | X   | X  |    | 70        |        |                       |
| GENERAL-TERMINAL-CORP.-100/101   | X   | X  | X  | 73        |        |                       |
| TEKTRONIX-4105                   | X   | X  |    | 78        |        |                       |
| IBM-PC                           | X   | X  |    | 79        |        |                       |
| TANDBERG TDV-2200/9-V2-ND-NOTIS  | X   | X  |    | 83        | X      |                       |
| FACIT TWIST (24-LINE MODE)       | X   | X  | X  | 91        | X      |                       |
| FACIT TWIST (72-LINE MODE)       | X   | X  | X  | 92        | X      |                       |
| TANDBERG TDV-2200/9S-ND NOTIS    | X   | X  | X  | 93        | X      |                       |
| SIEMENS 3975                     | X   |    |    | 94        |        |                       |
| COLORTREND 210 ND                | X   |    |    | 99        | X      |                       |

**Note:** ND St.: Those types automatically provided, when ordering NOTIS.


---

# APPENDIX B
## LOGICAL DEVICE NUMBERS USED IN SINTRAN III

## Overview of Logical Device Number Ranges

| Octal Logical Device No. | Decimal Logical Device No. | Device Name                               |
|--------------------------|----------------------------|-------------------------------------------|
| 0-77                     | 0-63                       | Character devices                         |
| 100-177                  | 64-127                     | Mass storage files                        |
| 200-277                  | 128-191                    | Non-internal devices                      |
| 300-377                  | 192-255                    | User Semaphores                           |
| 400-477                  | 256-319                    | Process Control Devices/Connect Devices   |
| 500-577                  | 320-383                    | System devices                            |
| 600-677                  | 384-447                    | Previously used by ND-Net.                |
| 700-777                  |                            | NORDCOM devices and other special devices |
| 1000-1077                |                            | Extension of character devices            |
| 1100-1177                |                            | System devices                            |
| 1200-1277                |                            | System devices                            |
| 1300-1377                |                            | System devices                            |
| 1400-1477                |                            | System devices                            |
| 1500-1577                |                            | System devices                            |
| 1600-1677                |                            | System devices                            |
| 1700-1777                |                            | System devices                            |
| 2000-2077                |                            | Terminal nos. 65-125                      |

## Detailed Logical Device Numbers

### Character Devices (0-77)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 0     | 0       | INBT; INCH (background): edited input, else: dummy |
| 1     | 1       | Background: "own terminal" RT: Terminal 1       |
| 2     | 2       | Tape reader 1 (Console) / error device          |
| 3     | 3       | Tape punch 1                                    |
| 4     | 4       | Card reader 1                                   |
| 5     | 5       | Line printer 1                                  |
| 6     | 6       | Synchronous modem 1                             |
| 7     | 7       | Terminal 17                                     |
| 10    | 8       | Plotter 1                                       |
| 11    | 9       | Terminal 2                                      |
| 12    | 10      | Tape reader 2                                   |
| 13    | 11      | Tape punch 2                                    |
| 14    | 12      | Bus switch device                               |
| 15    | 13      | Line printer 2                                  |
| 16    | 14      | Synchronous modem 2                             |
| 17    | 15      | Terminal 18                                     |
| 20    | 16      | Cassette drive 1                                |
| 21    | 17      | Cassette drive 2                                |
| 22    | 18      | Versatec on DMA printer/plotter 1               |
| 23    | 19      | Versatec on DMA printer/plotter 2               |
| 24    | 20      | Tektronix display                               |
| 25    | 21      | Magnetic tape 1 unit 2                          |
| 26    | 22      | Synchronous modem 5                             |
| 27    | 23      | Synchronous modem 6                             |
| 30    | 24      | Synchronous modem 3                             |
| 31    | 25      | Synchronous modem 4                             |
| 32    | 26      | Magnetic tape 2 unit 0                          |
| 33    | 27      | Magnetic tape 1 unit 3                          |
| 34    | 28      | Magnetic tape 2 unit 1                          |
| 35    | 29      | Card Punch 3                                    |
| 36    | 30      | CDC link                                        |
| 37    | 31      | Not used                                        |
| 40    | 32      | Magnetic tape 1 unit 0                          |
| 41    | 33      | Magnetic tape 1 unit 1                          |
| 42-47 | 34-39   | Terminals 3-8                                   |
| 50    | 40      | Card punch 1                                    |
| 51    | 41      | Card punch 2                                    |
| 52-57 | 42-47   | Terminals 19-24                                 |
| 60-67 | 48-55   | Terminals 9-16                                  |
| 70-77 | 56-63   | Terminals 25-32                                 |

### Process Control Devices/Connect Devices (400-477)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 400-437 | 256-287 | CAMAC interrupts or special process interface |
| 440   | 288     | Direct task level 6                             |
| 441   | 289     | Direct task level 7                             |
| 442   | 290     | Direct task level 8                             |
| 443   | 291     | Direct task level 9                             |
| 450-467 | 296-311 | CONNECT devices                                |
| 470   | 312     | ND 23 - programmed clock                        |

### System Devices (500-577)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 500   | 320     | Internal device for error message RT-program   |
| 501   | 321     | Semaphore for segment transfer                 |
| 502   | 322     | Disk 10Mb 1 data field                          |
| 503   | 323     | RT-Loader command lock                          |
| 504   | 324     | General lock for file system                   |
| 505   | 325     | User-file-buffer lock                           |
| 506   | 326     | Object-file-buffer lock                         |
| 507   | 327     | RT-open-file-table lock                         |
| 511   | 329     | Disk 10Mb 1, unit 0, R-bit-file-buffer lock    |
| 512   | 330     | Disk 10Mb 1, unit 0, F-bit-file-buffer lock    |
| 513   | 331     | Disk 10Mb 1, unit 0, R-directory lock          |
| 514   | 332     | Disk 10Mb 1, unit 0, F-directory lock          |
| 515   | 333     | DF1, file-transfer for RT lock for disk 1, disk 2, disk3 and disk 4 |
| 516   | 334     | DF2, for open-file monitor call from RT-program data field |
| 517   | 335     | RTFIL semaphore                                |
| 520   | 336     | NOTIS-IR semaphore 2                            |
| 521   | 337     | Device buffer allocation lock                   |
| 522   | 338     | Disk 10Mb 1, unit 1, R-directory lock          |
| 523   | 339     | Disk 10Mb 1, unit 1, F-directory lock          |
| 524   | 340     | Disk 10Mb 1, unit 1, R-bit-file-buffer lock    |
| 525   | 341     | Disk 10Mb 1, unit 1, F-bit-file-buffer lock    |
| 526   | 342     | DF3, transfer lock for magnetic tape 1          |
| 527   | 343     | Spooling queue semaphore                        |
| 530   | 344     | Accounting semaphore                            |
| 531   | 345     | CDC link monitor call data field               |
| 532   | 346     | Spooling device 4, queue semaphore              |
| 533   | 347     | Spooling device 4, queue I/O semaphore         |
| 534   | 348     | Spooling device 5, queue semaphore              |
| 535   | 349     | Spooling device 5, queue I/O semaphore         |
| 536   | 350     | Spooling device 6, queue semaphore              |
| 537   | 351     | Spooling device 6, queue I/O semaphore         |
| 540   | 352     | Internal Device Remote Batch IBM                |
| 541   | 353     | Internal Device Remote Batch UNIVAC             |
| 542   | 354     | Internal Device Remote Batch Honeywell Bull     |
| 543   | 355     | Internal Device Remote Batch CDC                |
| 544   | 356     | Big disk 3, Unit 0 directory lock               |
| 545   | 357     | Big disk 3, Unit 0 bit-file buffer lock        |
| 546   | 358     | Unit 1                                         |
| 550   | 360     | Unit 1                                         |
| 551   | 361     | Unit 2                                         |
| 552   | 362     | Unit 2                                         |
| 554   | 364     | Disk 10Mb 1, unit 3, R-bit-file-buffer lock    |
| 555   | 365     | Disk 10Mb 1, unit 3, F-bit-file-buffer lock    |
| 556   | 366     | Disk 10Mb 1, unit 3, R-directory lock          |
| 557   | 367     | Disk 10Mb 1, unit 3, F-directory lock          |
| 560   | 368     | Magnetic tape 1, data field                     |
| 561   | 369     | All magnetic tapes, directory lock              |
| 562   | 370     | Spooling device 11, queue semaphore             |
| 563   | 371     | Magtape 2, unit 2, I/O data field              |
| 564   | 372     | Magtape 2, unit 3, I/O data field              |
| 565   | 373     | Big disk 3, data field                          |
| 567   | 375     | CDC link data field                             |
| 570   | 376     | Disk 10Mb 1, unit 2, R-directory lock          |
| 571   | 377     | Disk 10Mb 1, unit 2, F-directory lock          |
| 572   | 378     | Disk 10Mb 1, unit 2, R-bit-file-buffer lock    |
| 573   | 379     | Disk 10Mb 1, unit 2, F-bit-file-buffer lock    |
| 574   | 380     | Monitor call data field for cassette            |
| 575   | 381     | Cassette data field                             |
| 576   | 382     | DF5, monitor call data field for Versatec 1     |
| 577   | 383     | Versatec data field                             |

### Extension of Character Devices (1000-1077)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 1000  | 512     | Floppy disk 1, unit 0, I/O data field          |
| 1001  | 513     | Floppy disk 1, unit 1, I/O data field          |
| 1002  | 514     | Floppy disk 1, unit 2, I/O data field          |
| 1003  | 515     | Floppy disk 2, unit 0, I/O data field          |
| 1004  | 516     | Floppy disk 2, unit 1, I/O data field          |
| 1005  | 517     | Floppy disk 2, unit 2, I/O data field          |
| 1006  | 518     | Hasp DMA 1, I/O data field                     |
| 1007  | 519     | Hasp DMA 2, I/O data field                     |
| 1010  | 520     | Hasp DMA 3, I/O data field                     |
| 1011  | 521     | Hasp DMA 4, I/O data field                     |
| 1012  | 522     | Hasp DMA 5, I/O data field                     |
| 1013  | 523     | Hasp DMA 6, I/O data field                     |
| 1014  | 524     | Line printer 3, I/O data field                 |
| 1015  | 525     | Line printer 4, I/O data field                 |
| 1040-1077 | 544-575 | Terminals 33 - 64                              |

### System Devices (1100-1177)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 1100  | 576     | Big disk/big cartridge disk 1, data field      |
| 1101  | 577     | Big disk/big cartridge disk 1, unit 0, directory lock |
| 1102  | 578     | Big disk/big cartridge disk 1, unit 0, bit-file-buffer lock |
| 1103  | 579     | NORD-50 data field                             |
| 1104  | 580     | Disk 10Mb 2, data field                        |
| 1105  | 581     | Disk 10Mb 2, unit 0, R-directory lock         |
| 1106  | 582     | Disk 10Mb 2, unit 0, F-directory lock         |
| 1107  | 583     | Disk 10Mb 2, unit 0, R-bit-file-buffer lock   |
| 1110  | 584     | Disk 10Mb 2, unit 0, F-bit-file-buffer lock   |
| 1111  | 585     | Magnetic tape 2 data field                     |
| 1112  | 586     | Big disk 4, unit 0, directory lock            |
| 1113  | 587     | Floppy disk 1, unit 3, I/O data field         |
| 1114  | 588     | Big disk 4, unit 0, bit-file-buffer lock      |
| 1115  | 589     | Floppy disk 2, unit 3, I/O data field         |
| 1116  | 590     | DF7, transfer lock for magnetic tape 2         |
| 1117  | 591     | Big disk/big cartridge disk 1, unit 1, directory lock |
| 1120  | 592     | Big disk/big cartridge disk 1, unit 1, bit-file-buffer lock |
| 1121  | 593     | Big disk/big cartridge disk 1, unit 2, directory lock |
| 1122  | 594     | Big disk/big cartridge disk 1, unit 2, bit-file-buffer lock |
| 1123  | 595     | Big disk/big cartridge disk 1, unit 3, directory lock |
| 1124  | 596     | Big disk/big cartridge disk 1, unit 3, bit-file-buffer lock |
| 1125  | 597     | Versatec controller 2                           |
| 1126  | 598     | Monitor call data field for Versatec controller 2 |
| 1127  | 599     | DF 39, magnetic tape 3 monitor call data field |
| 1130  | 600     | Disk 10Mb 2, unit 1, R-directory lock         |
| 1131  | 601     | Disk 10Mb 2, unit 1, F-directory lock         |
| 1132  | 602     | Disk 10Mb 2, unit 1, R-bit-file lock          |
| 1133  | 603     | Disk 10Mb 2, unit 1, F-bit-file lock          |
| 1134  | 604     | Floppy disk 1, unit 3, directory table lock   |
| 1135  | 605     | Floppy disk 1, unit 3, bit-file-buffer lock   |
| 1136  | 606     | Spooling device 1, queue semaphore             |
| 1137  | 607     | Spooling device 1, queue I/O semaphore        |
| 1140  | 608     | Spooling device 2, queue semaphore             |
| 1141  | 609     | Spooling device 2, queue I/O semaphore        |
| 1142  | 610     | Spooling system general semaphore              |
| 1143  | 611     | Spooling system wait for used pages semaphore |
| 1144  | 612     | Spooling system wait for free pages semaphore |
| 1145  | 613     | Floppy disk 1, data field                      |
| 1146  | 614     | Monitor call data field for floppy disk 1      |
| 1147  | 615     | Floppy disk 2, unit 3, directory table lock   |
| 1150  | 616     | Floppy disk 1, unit 0, directory table lock   |
| 1151  | 617     | Floppy disk 1, unit 0, bit-file-buffer lock   |
| 1152  | 618     | Floppy disk 1, unit 1, directory table lock   |
| 1153  | 619     | Floppy disk 1, unit 1, bit-file-buffer lock   |
| 1154  | 620     | Floppy disk 1, unit 2, directory table lock   |
| 1155  | 621     | Floppy disk 1, unit 2, bit-file-buffer lock   |
| 1156  | 622     | Floppy disk 2, data field                      |
| 1157  | 623     | Monitor call data field for floppy disk 2      |
| 1160  | 624     | Floppy disk 2, unit 3, bit-file-buffer lock   |
| 1161  | 625     | Floppy disk 2, unit 0, directory table lock   |
| 1162  | 626     | Floppy disk 2, unit 0, bit-file-buffer lock   |
| 1163  | 627     | Floppy disk 2, unit 1, directory table lock   |
| 1164  | 628     | Floppy disk 2, unit 1, bit-file-buffer lock   |
| 1165  | 629     | Floppy disk 2, unit 2, directory table lock   |
| 1166  | 630     | Floppy disk 2, unit 2, bit-file-buffer lock   |
| 1167  | 631     | Line printer 1, data field                     |
| 1170  | 632     | Monitor call data field for line printer 1     |
| 1171  | 633     | Big disk 4, unit 2, directory lock            |
| 1172  | 634     | Big disk 4, unit 2, bit-file buffer lock      |
| 1173  | 635     | Spooling device 3, queue semaphore             |
| 1174  | 636     | Spooling device 3, queue I/O semaphore        |
| 1175  | 637     | Line printer 2, data field                     |
| 1176  | 638     | Monitor call data field for line printer 2     |
| 1177  | 639     | Spooling semaphore, id data buffer lock        |

### System Devices (1200-1277)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 1200  | 640     | ND TPS system semaphore                        |
| 1201  | 641     | DMAC command lock                              |
| 1202  | 642     | RT-PROGRAM-LOG semaphore                       |
| 1203  | 643     | Histogram commands lock                        |
| 1204  | 644     | SINTRAN-SERVICE-PROGRAM command lock           |
| 1205  | 645     | Mail system lock                               |
| 1206  | 646     | Accounting semaphore                           |
| 1207  | 647     | NOTIS-IR semaphore                             |
| 1210  | 648     | Terminal 1, data field                         |
| 1211  | 649     | Big disk/big cartridge disk 2, data field     |
| 1212  | 650     | Internal device 1, data field                 |
| 1213  | 651     | Internal device 1, Monitor call data field   |
| 1214  | 652     | Internal device 2, data field                 |
| 1215  | 653     | Internal device 2, Monitor call data field   |
| 1216  | 654     | Internal device 3, data field                 |
| 1217  | 655     | Internal device 3, Monitor call data field   |
| 1220  | 656     | Internal device 4, data field                 |
| 1221  | 657     | Internal device 4, Monitor call data field   |
| 1222  | 658     | Internal device 5, data field                 |
| 1223  | 659     | Internal device 5, Monitor call data field   |
| 1224  | 660     | Winchester disk, data field or STC magtape controller 4 |
| 1225  | 661     | Winchester disk 1, unit 0, directory table lock |
| 1226  | 662     | Winchester disk 1, unit 0, bit-file-buffer lock |
| 1227  | 663     | Winchester disk 1, unit 1, directory table lock |
| 1230  | 664     | Winchester disk 1, unit 1, bit-file-buffer lock |
| 1231  | 665     | Winchester disk 2, data field or STC magtape controller 3 |
| 1232  | 666     | Winchester disk 2, unit 0, directory table lock |
| 1233  | 667     | Winchester disk 2, unit 0, bit-file-buffer lock |
| 1234  | 668     | Winchester disk 2, unit 1, directory table lock |
| 1235  | 669     | Winchester disk 2, unit 1, bit-file-buffer lock |
| 1236  | 670     | Batch process 1, data field                    |
| 1237  | 671     | Batch process 1, internal device               |
| 1240  | 672     | Batch process 2, data field                    |
| 1241  | 673     | Batch process 2, internal device               |
| 1242  | 674     | Batch process 3, data field                    |
| 1243  | 675     | Batch process 3, internal device               |
| 1244  | 676     | Batch process 4, data field                    |
| 1245  | 677     | Batch process 4, internal device               |
| 1246  | 678     | Batch process 5, data field                    |
| 1247  | 679     | Batch process 5, internal device               |
| 1250  | 680     | Batch process 6, data field                    |
| 1251  | 681     | Batch process 6, internal device               |
| 1252  | 682     | Batch process 7, data field                    |
| 1253  | 683     | Batch process 7, internal device               |
| 1254  | 684     | Batch process 8, data field                    |
| 1255  | 685     | Batch process 8, internal device               |
| 1256  | 686     | Batch process 9, data field                    |
| 1257  | 687     | Batch process 9, internal device               |
| 1260  | 688     | Batch process 10, data field                   |
| 1261  | 689     | Batch process 10, internal device              |
| 1262  | 690     | Spooling device 7, queue semaphore             |
| 1263  | 691     | Spooling device 7, queue I/O semaphore        |
| 1264  | 692     | Spooling device 8, queue semaphore             |
| 1265  | 693     | Spooling device 8, queue I/O semaphore        |
| 1266  | 694     | Spooling device 9, queue semaphore             |
| 1267  | 695     | Spooling device 9, queue I/O semaphore        |
| 1270  | 696     | Spooling device 10, queue semaphore            |
| 1271  | 697     | Spooling device 10, queue I/O semaphore       |
| 1272  | 698     | Monitor call data field for internal device 1  |
| 1273  | 699     | Monitor call data field for internal device 2  |
| 1274  | 700     | Monitor call data field for internal device 3  |
| 1275  | 701     | Monitor call data field for internal device 4  |
| 1276  | 702     | Monitor call data field for internal device 5  |
| 1277  | 703     | DF 40, magnetic tape 4, monitor call data field |

### System Devices (1300-1377)

| Octal | Decimal | Device Name                                     |
|-------|---------|------------------------------------------------|
| 1300  | 704     | Big disk 4, unit 3, directory lock            |
| 1301  | 705     | Big disk 4, unit 3, bit file buffer lock      |
| 1302  | 706     | Device buffer lock                             |
| 1303  | 707     | Hasp DMA 1, data field                         |
| 1304  | 708     | Hasp DMA 1, data field                         |
| 1305  | 709     | Monitor call data field for Hasp DMA 1        |
| 1306  | 710     | Monitor call data field for Hasp DMA 1        |
| 1307  | 711     | Hasp DMA 2, data field                         |
| 1310  | 712     | Hasp DMA 2, data field                         |
| 1311  | 713     | Monitor call data field for Hasp DMA 2        |
| 1312  | 714     | Monitor call data field for Hasp DMA 2        |
| 1313  | 715     | Hasp DMA 3, data field                         |
| 1314  | 716     | Hasp DMA 3, data field                         |
| 1315  | 717     | Monitor call data field for Hasp DMA 3        |
| 1316  | 718     | Monitor call data field for Hasp DMA 3        |
| 1317  | 719     | Hasp DMA 4, data field                         |
| 1320  | 720     | Hasp DMA 4, data field                         |
| 1321  | 721     | Monitor call data field for Hasp DMA 4        |
| 1322  | 722     | Monitor call data field for Hasp DMA 4        |
| 1323  | 723     | Hasp DMA 5, data field                         |
| 1324  | 724     | Hasp DMA 5, data field                         |
| 1325  | 725     | Monitor call data field for Hasp DMA 5        |
| 1326  | 726     | Monitor call data field for Hasp DMA 5        |
| 1327  | 727     | Hasp DMA 6, data field                         |
| 1330  | 728     | Hasp DMA 6, data field                         |
| 1331  | 729     | Monitor call data field for Hasp DMA 6        |
| 1332  | 730     | Monitor call data field for Hasp DMA 6        |
| 1333  | 731     | Big disk/big cartridge disk 2, unit 0, directory table lock |
| 1334  | 732     | Big disk/big cartridge disk 2, unit 0, bit-file-buffer lock |
| 1335  | 733     | Big disk/big cartridge disk 2, unit 1, directory table lock |
| 1336  | 734     | Big disk/big cartridge disk 2, unit 1, bit-file-buffer lock |
| 1337  | 735     | Big disk/big cartridge disk 2, unit 2, directory table lock |
| 1340  | 736     | Big disk/big cartridge disk 2, unit 2, bit-file-buffer lock |
| 1341  | 737     | Big disk/big cartridge disk 2, unit 3, directory table lock |
| 1342  | 738     | Big disk/big cartridge disk 2, unit 3, bit-file-buffer lock |
| 1343  | 739     | Line printer 3, data field                     |
| 1344  | 740     | Monitor call data field for line printer 3    |
| 1345  | 741     | Line printer 4, data field                     |
| 1346  | 742     | Monitor call data field for line printer 4    |
| 1347  | 743     | Spooling device 11, queue I/O semaphore        |
| 1350  | 744     | Spooling device 12, queue semaphore            |
| 1351  | 745     | Spooling device 12, queue I/O semaphore        |
| 1352  | 746     | RT-PROGRAM-LOG command lock                     |
| 1360  | 752     | HDLC DMA, link 1, input; synchronous modem 1 for HDLC interface input/output |
| 1361  | 753     | HDLC DMA, link 1, output                       |
| 1362  | 754     | HDLC DMA, link 2, input; synchronous modem 2 for HDLC interface input/output |
| 1364  | 756     | HDLC DMA, link 3, input; synchronous modem 3 for HDLC interface input/output |
| 1366  | 758     | HDLC DMA, link 4, input; synchronous modem 4 for HDLC interface input/output |
| 1373  | 763     | HDLC DMA, link 6, output; synchronous modem 6 for HDLC interface output |
| 1374  | 764     | X.21 logical number 1                           |
| 1375  | 765     | X.21 logical number 2                           |
| 1376  | 766     | X.21 logical number 3                           |
| 1377  | 767     | X.21 logical number 4                           |

### Additional Device Ranges

| Range | Description                                     |
|-------|------------------------------------------------|
| 1400-1537 | Terminal access device (TAD) 1-96 (decimal)   |
| 1600-1677 | DMA device buffer, header locks for header numbers 0-77 (octal) |
| 1722  | Spooling device 13, queue semaphore            |
| 1723  | Spooling device 13, queue I/O semaphore       |
| 1724  | Spooling device 14, queue semaphore            |
| 1725  | Spooling device 14, queue I/O semaphore       |
| 1726  | Spooling device 15, queue semaphore            |
| 1727  | Spooling device 15, queue I/O semaphore       |
| 1730  | COSMOS file access, DF data field             |
| 1731  | COSMOS Spooling, peripheral device number     |
| 2000-2077 | Terminal nos. 65-127                           |
| 2130  | Spooling device 16, queue semaphore            |
| 2131  | Spooling device 16, I/O semaphore             |
| 2132  | Spooling device 17, queue semaphore            |
| 2133  | Spooling device 17, I/O semaphore             |
| 2134  | Spooling device 18, queue semaphore            |
| 2135  | Spooling device 18, I/O semaphore             |
| 2136  | Spooling device 19, queue semaphore            |
| 2137  | Spooling device 19, I/O semaphore             |
| 2140  | Spooling device 20, queue semaphore            |
| 2141  | Spooling device 20, I/O semaphore             |
| 2142  | Spooling device 21, queue semaphore            |
| 2143  | Spooling device 21, I/O semaphore             |
| 2144  | Spooling device 22, queue semaphore            |
| 2145  | Spooling device 22, I/O semaphore             |
| 2146  | Spooling device 23, queue semaphore            |
| 2147  | Spooling device 23, I/O semaphore             |
| 2150  | Spooling device 24, queue semaphore            |
| 2151  | Spooling device 24, I/O semaphore             |
| 2152  | Spooling device 25, queue semaphore            |
| 2153  | Spooling device 25, I/O semaphore             |
| 2154  | Spooling device 26, queue semaphore            |
| 2155  | Spooling device 26, I/O semaphore             |
| 2156  | Spooling device 27, queue semaphore            |
| 2157  | Spooling device 27, I/O semaphore             |
| 2160  | Spooling device 28, queue semaphore            |
| 2161  | Spooling device 28, I/O semaphore             |
| 2162  | Spooling device 29, queue semaphore            |
| 2163  | Spooling device 29, I/O semaphore             |
| 2164  | Spooling device 30, queue semaphore            |
| 2165  | Spooling device 30, I/O semaphore             |
| 2166  | COSMOS Spooling, queue semaphore              |
| 2167  | COSMOS Spooling, I/O semaphore               |

---

# APPENDIX C
## ERROR MESSAGES

This appendix documents SINTRAN run-time and file system errors. Error messages originating in subsystems are documented in their respective subsystem documentation. This includes FORTRAN and BASIC run-time errors.

## C.1 SINTRAN III Monitor

### C.1.1 Run-Time Errors

Most run-time errors cause the current RT-program to be aborted and an error message to be printed.

**Error Message Format:**
```
aa.bb.cc ERROR nn IN rr AT ll ; tttt xx yy
```

**Error Message Output Location:**
- If an error occurs in a background program, the message is written to the corresponding terminal.
- For RT-programs, the message is written to the error message terminal (usually terminal 1).

**Meaning of Parameters:**
- `aa.bb.cc`: Time when the error message was printed.
  - `aa`: hours
  - `bb`: minutes
  - `cc`: seconds
- `nn`: Error number (refers to following pages).
- `rr`: Name of RT-program.
- `ll`: Octal address where the error occurred.
- `tttt`: Explanatory text.
- `xx, yy`: Numbers giving additional information about the error. One or both numbers can be omitted (refers to following pages).

**Example:**
```
01.43.32 ERROR 14 IN RTP1 AT 114721;
OUTSIDE SEGMENT BOUNDS
```

**Note:** In the case of a segment transfer error, an additional message "TRANSF!" is given.

### C.1.2 Run-Time Error Codes

| Error Code | Meaning | xx | yy | Program Aborted |
|------------|---------|----|----|-----------------|
| 00 | Illegal monitor call | | | yes |
| 01 | Bad RT-program address | | | yes |
| 02 | Wrong priority in PRIOR | | | yes |
| 03 | Bad memory page | page no. | | yes |
| 04 | Internal interrupt on direct task level | level | bit no. | |
| 06 | Batch input error | error no. | | yes |
| 07 | Batch output error | error no. | | yes |
| 08 | Batch system error | error no. | L-reg. | yes |
| 09 | Illegal parameter in CLOCK | | | yes |
| 10 | Illegal parameter in ABSET | | | yes |
| 11 | Illegal parameter in UPDAT | | | yes |
| 12 | Illegal time parameters | | | yes |
| 13 | Page fault for non-demand | page no. | | yes |
| 14 | Outside segment bounds | page no. | | yes |
| 15 | Illegal segment number | segment no. | | yes |
| 16 | Segment not loaded | segment no. | | yes |
| 17 | Fixing demand | segment no. | | yes |
| 18 | Too many fixed pages | segment no. | | yes |
| 19 | Too big segment | segment no. | | yes |
| 20 | Disk transfer error | hardware device no. | unit | no (yes if segment transfer) |
| 21 | Disk transfer error | last 16 bit of sector address | hardware status | no |
| 22 | False interrupt | level | IDENTcode | no |
| 23 | Device error | hardware device no. | hardware status | no |
| 25 | Already fixed | segment no. | | yes |
| 26 | Device timeout | hardware device | unit no. | no |
| 27 | Illegal parameter in CONCT | | | yes |
| 28 | Space not available | segment no. | | yes |
| 29 | MON 64 and MON 65 (File system error) | error no. | (see NORD File Syst ND-60.122) | yes |
| 30 | Divide by zero | | | yes |
| 31 | Permit violation | | | yes |
| 32 | Ring violation | | | yes |
| 33 | HDLC driver, fatal error | | | yes |
| 34 | Illegal instruction | | | yes |
| 35 | Reentrant-FTN stack error | | | yes |
| 36 | Privileged instruction | | | yes |
| 37 | IOX error address level | | | no |
| 38 | Memory parity error | PEA reg. | PES reg. | yes |
| 39 | Memory out of range | PEA reg. | PES reg. | yes |
| 40 | Power fail | | | no |
| 41 | Illegal error code in ERMON | | | yes |
| 42 | Overlapping segments | segments | | yes |
| 44 | Corrected memory error | PEA reg. | PES reg. | no |
| 45 | Not demand segments | | | yes |
| 46 | XMSG fatal error, internal error or inconsistency | XMSG error code | physical address | yes |
| 47 | XMSG user error | calling level | | yes |
| 48 | False BEX interrupt | | | |
| 49 | Remote power fail interrupt | | | |
| 50-69 | User defined error (MON 142) | error no. | suberror number | no |
| 70 | BEX parity error | | | |
| 71 | False MPM4 interrupt | busc no. | hardware status | no |
| 72 | MPM4 power fail interrupt | busc no. | | no |
| 73 | MPM4 memory out of range | busc no. | lower limit | no |
| 74 | MPM4 memory error | local PES | local PEA | no |
| 75 | MPM4 parity error | busc no. | lower limit | no |
| 76 | MPM4 write parity error | busc no. | port code number | |
| 90 | FORTRAN run-time error | error no. | | no |
| 91 | FORTRAN I/O error | error no. | | no |
| 92 | Fatal error in GPIB driver. Controller stopped. | | | |
| 93 | GPIB error | | | no |
| 94 | Illegal page index block | | | |
| 95 | Illegal function code | | | |
| 96 | Segment is fixed in page index table | | | |
| 97 | Trying to start uninitialised background program | | | |
| 98 | No background process available | | | |
| 99 | Octobus error | | | |
| 100 | FTN library error | | | |

## C.2 SINTRAN III File System

### C.2.1 Error Codes Returned from Monitor Calls - Numeric List

| Error Code (Octal) | Error Code (Decimal) | Message |
|-------------------|---------------------|---------|
| 000 | 000 | not used |
| 001 | 001 | not used |
| 002 | 002 | Bad file number |
| 003 | 003 | End of file |
| 004 | 004 | Card reader error (card read) |
| 005 | 005 | Device not reserved |
| 006 | 006 | not used |
| 007 | 007 | Card reader error (card not read) |
| 010 | 008 | not used |
| 011 | 009 | not used |
| 012 | 010 | End of device (timeout) |
| 013 | 011 | not used |
| 014 | 012 | not used |
| 015 | 013 | not used |
| 016 | 014 | not used |
| 017 | 015 | not used |
| 020 | 016 | not used |
| 021 | 017 | Illegal character in parameter |
| 022 | 018 | No such page |
| 023 | 019 | Not decimal number |
| 024 | 020 | Not octal number |
| 025 | 021 | You are not authorised to do this |
| 026 | 022 | Directory not entered |
| 027 | 023 | Ambiguous directory name |
| 030 | 024 | No such device name |
| 031 | 025 | Ambiguous device name |
| 032 | 026 | Directory entered |
| 033 | 027 | No such logical unit |
| 034 | 028 | Unit occupied |
| 035 | 029 | Master block transfer error |
| 036 | 030 | Bit-file transfer error |
| 037 | 031 | No more tracks available |
| 040 | 032 | Directory not on specified unit |
| 041 | 033 | Files open on this directory |
| 042 | 034 | Main directory not last one released |
| 043 | 035 | No main directory |
| 044 | 036 | Too long parameter |
| 045 | 037 | Ambiguous user name |
| 046 | 038 | No such user name |
| 047 | 039 | No such user name in main directory |
| 050 | 040 | Attempt to create too many users |
| 051 | 041 | User already exists |
| 052 | 042 | User has files |
| 053 | 043 | User is entered |
| 054 | 044 | Not so much space unreserved in directory |
| 055 | 045 | Reserved space already used |
| 056 | 046 | No such file name |
| 057 | 047 | Ambiguous file name |
| 060 | 048 | Wrong password |
| 061 | 049 | User already entered |
| 062 | 050 | No user entered |
| 063 | 051 | Friend already exists |
| 064 | 052 | No such friend |
| 065 | 053 | Attempt to create too many friends |
| 066 | 054 | Attempt to create yourself as friend |
| 067 | 055 | Contiguous space not available |
| 070 | 056 | Not directory access |
| 071 | 057 | Space not available to expand file |
| 072 | 058 | Space already allocated |
| 073 | 059 | No space in default directories |
| 074 | 060 | No such file version |
| 075 | 061 | No more pages available for this user |
| 076 | 062 | File already exists |
| 077 | 063 | Attempt to create too many files |
| 100 | 064 | Outside device limits |
| 101 | 065 | No previous version |
| 102 | 066 | File not contiguous |
| 103 | 067 | File type already defined |
| 104 | 068 | No such access code |
| 105 | 069 | File already open |
| 106 | 070 | Not write access |
| 107 | 071 | Attempt to open too many files |
| 110 | 072 | Not write and append access |
| 111 | 073 | Not read access |
| 112 | 074 | Not read, write and common access |
| 113 | 075 | Not read and write access |
| 114 | 076 | Not read and common access |
| 115 | 077 | File reserved by another user |
| 116 | 078 | File already opened for write by you |
| 117 | 079 | No such user index |
| 120 | 080 | Not append access |
| 121 | 081 | Attempt to open too many mass storage files |
| 122 | 082 | Attempt to open too many files |
| 123 | 083 | Not open for sequential write |
| 124 | 084 | Not open for sequential read |
| 125 | 085 | Not open for random write |
| 126 | 086 | Not open for random read |
| 127 | 087 | File number out of range |
| 130 | 088 | File number already used |
| 131 | 089 | No more buffer space |
| 132 | 090 | No file open with this number |
| 133 | 091 | Not mass storage file |
| 134 | 092 | File used for write |
| 135 | 093 | File used for read |
| 136 | 094 | File only open for sequential read or write |
| 137 | 095 | No scratch file open |
| 140 | 096 | File not reserved by you |
| 141 | 097 | Transfer error |
| 142 | 098 | File already reserved |
| 143 | 099 | No such block |
| 144 | 100 | Source and destination equal |
| 145 | 101 | Illegal on tape device |
| 146 | 102 | End of tape |
| 147 | 103 | Device unit reserved for special use |
| 150 | 104 | Main directory must be default |
| 151 | 105 | Not last file on tape |
| 152 | 106 | Not tape device |
| 153 | 107 | Illegal address reference in monitor call |
| 154 | 108 | Source empty |
| 155 | 109 | File already opened by another user |
| 156 | 110 | File already opened for write by another user |
| 157 | 111 | Missing parameter |
| 160 | 112 | Two pages must be left unreserved |
| 161 | 113 | not used |
| 162 | 114 | Device cannot be reserved |
| 163 | 115 | Overflow in read |
| 164 | 116 | DMA error |
| 165 | 117 | Bad data block |
| 166 | 118 | Control/modus word error |
| 167 | 119 | Parity error |
| 170 | 120 | LRC error |
| 171 | 121 | Device error (device-function read-last-status to get status) |
| 172 | 122 | Device buffer of requested size not available |
| 173 | 123 | Illegal mass storage unit number |
| 174 | 124 | Illegal parameter |
| 175 | 125 | Write-protect violation |
| 176 | 126 | Error detected by read after write |
| 177 | 127 | No EOF mark found |
| 200 | 128 | not used |
| 201 | 129 | Illegal function code |
| 202 | 130 | Timeout (no data block found) |
| 203 | 131 | Paper fault |
| 204 | 132 | Device not ready |
| 205 | 133 | Device already reserved |
| 206 | 134 | Not peripheral file |
| 207 | 135 | No such queue entry |
| 210 | 136 | Not so much space left |
| 211 | 137 | No spooling for this device |
| 212 | 138 | No such queue |
| 213 | 139 | Queue empty |
| 214 | 140 | Queue full |
| 215 | 141 | Not last used by you |
| 216 | 142 | not used |
| 217 | 143 | not used |
| 220 | 144 | not used |
| 221 | 145 | not used |
| 222 | 146 | not used |
| 223 | 147 | Formatting error |
| 224 | 148 | Incompatible device sizes |
| 225 | 149 | not used |
| 226 | 150 | Tape format error |
| 227 | 151 | Block count error |
| 230 | 152 | Volume not on specified unit |
| 231 | 153 | Not deleted record |
| 232 | 154 | Device error |
| 233 | 155 | Error in object entry |
| 234 | 156 | Odd number of bytes (right byte in last word insignificant) |
| 235 | 157 | Error in backspace/forward-space print |
| 236 | 158 | Block format error |
| 237 | 159 | Overflow in write |
| 240 | 160 | Illegal device type |
| 241 | 161 | Segment not contiguously fixed |
| 242 | 162 | Segment not fixed |
| 243 | 163 | Approaching end of accounting file |
| 244 | 164 | Accounting file full |
| 245 | 165 | No more unused spooling files available |
| 246 | 166 | Inconsistent directory |
| 247 | 167 | Object entry not used |
| 250 | 168 | User does not exist |
| 251 | 169 | Directory not reserved |
| 252 | 170 | Not a multiple of hardware block size |
| 253 | 171 | Not indexed file |
| 254 | 172 | Illegal floppy format |
| 255 | 173 | File not open |
| 256 | 174 | File already opened for read or write by you |
| 257 | 175 | User does not exist in the same main directory as you |
| 260 | 176 | File-access reentrant segments are not loaded |
| 261 | 177 | Illegal access code for remote file |
| 262 | 178 | File-access connection aborted by file server |
| 263 | 179 | File-access connection aborted by file server administrator |
| 264 | 180 | No answer from remote system; file-access connection aborted |
| 265 | 181 | File-access initialize failed |
| 266 | 182 | Unknown remote system name |
| 267 | 183 | File-access protocol error; connection aborted |
| 270 | 184 | File-access internal error; call not valid in current state |
| 271 | 185 | Illegal range of LAMU identifier |
| 272 | 186 | LAMU in use |
| 273 | 187 | LAMU table full |
| 274 | 188 | Illegal RT-program |
| 275 | 189 | Maximum number of LAMUs per RT-program reached |
| 276 | 190 | LAMU not connected |
| 277 | 191 | No LAMU area big enough |
| 300 | 192 | LAMU not defined |
| 301 | 193 | Illegal logical page number |
| 302 | 194 | Logical LAMU overlap |
| 303 | 195 | You can only log in on a main directory |
| 304 | 196 | Directory index too large |
| 305 | 197 | Object index too large |
| 306 | 198 | not used |
| 307 | 199 | Warning; 2-bank prog. file, but segment is only 1-bank |
| 310 | 200 | Warning; no such page in data bank, program starts as 1-bank |
| 311 | 201 | The specified device is not a terminal |
| 312 | 202 | No termination handling defined |
| 313 | 203 | No more remote file-access data segments available |
| 314 | 204 | Input while escape/local off is illegal |
| 315 | 205 | TAD protocol error, illegal or inconsistent message |
| 316 | 206 | Terminal line not connected |
| 317 | 207 | Illegal combination of DENTE and DTUSE bits |
| 320 | 208 | Directory not reserved by you |
| 321 | 209 | Wrong project password |
| 322 | 210 | File-access transport layer error; all connections aborted |
| 323 | 211 | File-access internal error; invalid parameter value |
| 324 | 212 | File-access not running or crashed; all connections aborted |
| 325 | 213 | Wrong format in file |
| 326 | 214 | Directory already reserved for special use |
| 327 | 215 | Unknown user-control code |
| 330 | 216 | No service is available for this code |
| 331 | 217 | Please terminate current service before requesting new service |
| 332 | 218 | This code is only legal within a service |
| 333 | 219 | Remote file server is not available |
| 334 | 220 | ND-100 panel clock incorrect |
| 335 | 221 | Buffer size too big for buffer previously obtained |
| 336 | 222 | Illegal segment name |
| 337 | 223 | Not octal number |
| 340 | 224 | Not contiguous file |
| 341 | 225 | Ambiguous command |
| 342 | 226 | Protected command |
| 343 | 227 | Ambiguous subsystem |
| 344 | 228 | No more spooling pages left |
| 345 | 229 | No more versions can be created in this object block |
| 346 | 230 | Illegal baud rate specified |
| 347 | 231 | Illegal character length |
| 350 | 232 | Illegal parity specified |
| 351 | 233 | Not legal from RT-program |
| 352 | 234 | Illegal when not originally own terminal |
| 353 | 235 | Illegal break/echo strategy |
| 354 | 236 | Illegal size of parameter array |
| 355 | 237 | Illegal area specified |
| 356 | 238 | Not 8 bits character length |
| 357 | 239 | Terminal already in a display table |
| 360 | 240 | Not a master terminal |
| 361 | 241 | Not connected to specified master terminal |
| 362 | 242 | Error in display table |
| 363 | 243 | Fatal error occurred during read/write in segments |
| 364 | 244 | Baud rate not set by software |
| 365 | 245 | Illegal baud rate in data field |
| 366 | 246 | Program is active |
| 367 | 247 | No more physical memory available |
| 370 | 248 | No more allocate-memory-table indexes available |
| 371 | 249 | Illegal segment |
| 372 | 250 | Illegal address |
| 373 | 251 | Illegal program LAMU identifier |
| 374 | 252 | Program LAMU already exists |
| 375 | 253 | No such program LAMU |
| 376 | 254 | Illegal program LAMU size |
| 377 | 255 | Program LAMU not connected |
| 3200 | 1664 | Not allowed now |
| 3201 | 1665 | Illegal index |
| 3202 | 1666 | Disc access log file is full |

### C.2.2 Error Codes Returned from Monitor Calls - Alphabetic List

**Accounting file full (244 oct)**: The file used to store accounting is full. The file must be reset by the accounting service program.

**Ambiguous command (341 oct)**: The given command is an abbreviated form that can be expanded in more than one way, or its expansion is one of the few commands that must be written in full.

**Ambiguous device name (031 oct)**: The given device specification matches the name of more than one device name.

**Ambiguous directory name (027 oct)**: The given directory specification matches more than one directory name.

**Ambiguous file name (057 oct)**: The given file specification matches more than one file name in the current directory.

**Ambiguous subsystem (343 oct)**: The command given is an abbreviated form that matches the name of more than one subsystem.

**Ambiguous user name (045 oct)**: The user specification matches more than one user name.

**Approaching end of accounting file (243 oct)**: The file used to store accounting information is nearly full. The file must be reset by the accounting service program.

**Attempt to create too many files (077 oct)**: The user already has the maximum number of files. Either delete files or increase the maximum number of files.

**Attempt to create too many friends (065 oct)**: The maximum number of friends (8) has already been defined.

**Attempt to create too many users (050 oct)**: The maximum number of users have already been created.

**Attempt to create yourself as friend (066 oct)**: You are logged in as the user you specify as the new friend.

**Attempt to open too many files (107 oct)**: You have already opened as many files as you are allowed to.

**Attempt to open too many files (122 oct)**: You have already opened as many files as you are allowed to.

**Attempt to open too many mass storage files (121 oct)**: You have already opened the maximum number of mass storage files.

**Bad data block (165 oct)**: Read error - possible device malfunction.

**Bad file number (002 oct)**: Internal error - the internal open-file-number is out of range.

**Baud rate not set by software (364 oct)**: Terminal speed should have been set by software, but was only set by hardware.

**Bit-file transfer error (036 oct)**: Error on a directory device. Unable to read system information on the device.

**Block count error (227 oct)**: Self-explanatory.

**Block format error (236 oct)**: Self-explanatory.

**Buffer size too big for buffer previously obtained (335 oct)**: Attempt to change buffer size.

**Card reader error (card not read) (007 oct)**: Error while reading cards. There may be a jammed card, or mispunch.

**Card reader error (card read) (004 oct)**: Error while reading cards. There may be a jammed card, or mispunch.

**Contiguous space not available (067 oct)**: Your file cannot be expanded.

**Control/modus word error (166 oct)**: Error in control word.

**DMA error (164 oct)**: Error in DMA transfer.

**Device already reserved (205 oct)**: Self-explanatory.

**Device buffer of requested size not available (172 oct)**: Self-explanatory.

**Device cannot be reserved (162 oct)**: Self-explanatory.

**Device error (232 oct)**: Device error detected.

**Device error (device-function read-last-status to get status) (171 oct)**: Device error detected. Use the SINTRAN III command DEVICE-FUNCTION device,READ-LAST-STATUS to get status information.

**Device not ready (204 oct)**: Possibly a floppy disk (or a disk pack) not properly inserted, a printer is in offline (unselect) mode etc. when trying to access the device.

**Device not reserved (005 oct)**: Attempt to access a device which must be reserved without reserving it.

**Device unit reserved for special use (147 oct)**: Device must be reserved for special use to be accessed the way you tried.

**Directory already reserved for special use (326 oct)**: Devices reserved for special use may not be accessed by other users at the same time.

**Directory entered (032 oct)**: Attempt to access a directory erroneously while it is entered.

**Directory index too large (304 oct)**: Attempt to specify a directory index outside range.

**Directory not entered (026 oct)**: Attempt to access a directory that has not been previously entered. Probably due to bad spelling of the directory name.

**Directory not on specified unit (040 oct)**: Directory name misspelt or wrong unit specified or wrong removable pack inserted.

**Directory not reserved (251 oct)**: The directory must be reserved for this function.

**Directory not reserved by you (320 oct)**: The directory is already reserved, but not by you.

**Disc access log file is full (3202 oct)**: The file used to store data for the disk access log facility is full.

**End of device (timeout) (012 oct)**: Unable to get input from the device.

**End of file (003 oct)**: Attempt to read past the end of the file.

**End of tape (146 oct)**: Attempt to read past the last data written on tape, or write past physical end of tape.

**Error detected by read after write (176 oct)**: The written data were read back in order to check that the write was successful, but showed that it was not.

**Error in backspace/forward-space print (235 oct)**: Error while attempting backspace or forward-space print.

**Error in display table (362 oct)**: Internal error in the table of terminals used for the display facility.

**Error in object entry (233 oct)**: Internal file system error.

**Fatal error occurred during read/write in segments (363 oct)**: Self-explanatory.

**File-access internal error; call not valid in current state (270 oct)**: Self-explanatory.

**File-access internal error; invalid parameter value (323 oct)**: Self-explanatory.

**File-access not running or crashed; all connections aborted (324 oct)**: Self-explanatory.

**File-access protocol error; connection aborted (267 oct)**: Internal error in remote file access.

**File already exists (076 oct)**: Attempt to create a file with the same name as an existing file.

**File already open (105 oct)**: Attempt to open a file that is already open, possibly by another user.

**File already opened by another user (155 oct)**: Self-explanatory.

**File already opened for read or write by you (256 oct)**: Attempt to open a file that is already open.

**File already opened for write by another user (156 oct)**: Self-explanatory.

**File already opened for write by you (116 oct)**: Attempt to open a file a second time without first closing it.

**File already reserved (142 oct)**: Attempt to reserve a file that is already reserved for you.

**File not contiguous (102 oct)**: Possibly attempt to define a segment file that is not contiguous.

**File not open (255 oct)**: Attempt to read or write to a file that is not open. Possibly specifying wrong file number.

**File not reserved by you (140 oct)**: File is reserved by and for some other user, or the same user but at an other terminal.

**File number already used (130 oct)**: Attempt to connect a file with a specific file number, when an other file has that number.

**File number out of range (127 oct)**: An open-file-number was specified, but no such open file exists.

**File only open for sequential read or write (136 oct)**: Self-explanatory.

**File reserved by another user (115 oct)**: Attempt to open a file that has been reserved by another user.

**File type already defined (103 oct)**: Attempt to redefine file type (terminal file/peripheral file/spooling file, etc).

**File used for read (135 oct)**: Self-explanatory.

**File used for write (134 oct)**: Self-explanatory.

**File-access connection aborted by file server (262 oct)**: Self-explanatory.

**File-access connection aborted by file server administrator (263 oct)**: Self-explanatory.

**File-access initialize failed (265 oct)**: Self-explanatory.

**File-access reentrant segments are not loaded (260 oct)**: The reentrant segments used for remote file access is not loaded.

**File-access transport layer error; all connections aborted (322 oct)**: Internal error in remote file access.

**Files open on this directory (041 oct)**: The directory cannot be released until all files on it are closed.

**Formatting error (223 oct)**: Error while attempting to format a disk or a floppy disk.

**Friend already exists (063 oct)**: The specified user is already a friend.

**Illegal RT-program (274 oct)**: No legal RT-program address specified.

**Illegal access code for remote file (261 oct)**: Attempt to access a remote file with illegal access code(s).

**Illegal address (372 oct)**: The address specified is illegal.

**Illegal address reference in monitor call (153 oct)**: Probably internal error in the program.

**Illegal area specified (355 oct)**: Attempt to specify illegal area in monitor call.

**Illegal baud rate in data field (365 oct)**: Terminal speed out of range.

**Illegal baud rate specified (346 oct)**: Self-explanatory.

**Illegal break/echo strategy (353 oct)**: Attempt to specify illegal echo and/or break strategy.

**Illegal character in parameter (021 oct)**: Wrong syntax in the parameter. Probably due to bad spelling, missing punctuation, unbalanced parenthesis or extraneous characters.

**Illegal character length (347 oct)**: Attempt to set character length less then 5 bits or greater than 8 bits.

**Illegal combination of DENTE and DTUSE bits (317 oct)**: Internal inconsistency - directory is marked both as entered and reserved for special use.

**Illegal device type (240 oct)**: Self-explanatory.

**Illegal floppy format (254 oct)**: Attempt to set or use an unsupported floppy format.

**Illegal function code (201 oct)**: Illegal function code in monitor call.

**Illegal index (3201 oct)**: Illegal sub-function code or index in monitor call.

**Illegal logical page number (301 oct)**: Logical page numbers must be in the range 0-63 decimal (0-77 octal).

**Illegal mass storage unit number (173 oct)**: Self-explanatory.

**Illegal on tape device (145 oct)**: This function is not allowed on magnetic tape.

**Illegal parameter (174 oct)**: One of the parameters can only have one of a few specific values, not including the value given.

**Illegal parity specified (350 oct)**: Attempt to specify illegal parity.

**Illegal program LAMU identifier (373 oct)**: The LAMU identifier specified is illegal.

**Illegal program LAMU size (376 oct)**: Size of LAMU is out of range.

**Illegal range of LAMU identifier (271 oct)**: LAMU identifier is out of range.

**Illegal segment (371 oct)**: Attempt to change one of the active segments for a program to an illegal segment.

**Illegal segment name (336 oct)**: The segment name specified is illegal.

**Illegal size of parameter array (354 oct)**: Parameter array in monitor call too small.

**Illegal when not originally own terminal (352 oct)**: Error in monitor call - attempt to reset other terminal to become own.

**Incompatible device sizes (224 oct)**: Incompatible device sizes in COPY-DEVICE.

**Inconsistent directory (246 oct)**: The information in the file directory contradicts itself, as when the same disk blocks are allocated to several files, or some blocks does not appear as belonging to any file, nor in the free list. This error may be a symptom of hardware errors that garble the data on write or read time.

**Input while escape/local off is illegal (314 oct)**: Illegal input function when escape handling is disabled.

**LAMU in use (272 oct)**: Attempt to access LAMU already in use.

**LAMU not connected (276 oct)**: Attempt to access LAMU without connecting it.

**LAMU not defined (300 oct)**: Attempt to access undefined LAMU.

**LAMU table full (273 oct)**: Too many LAMUs defined.

**LRC error (170 oct)**: Hardware error on LRC.

**Logical LAMU overlap (302 oct)**: Two LAMUs may not overlap.

**Main directory must be default (150 oct)**: Attempt to clear default directory flag on a main directory.

**Main directory not last one released (042 oct)**: Attempt to release the main directory while some other directory is still entered.

**Master block transfer error (035 oct)**: Unable to read system information on the device.

**Maximum number of LAMUs per RT-program reached (275 oct)**: Attempt to connect too many LAMUs.

**Missing parameter (157 oct)**: Self-explanatory.

**ND-100 panel clock incorrect (334 oct)**: The ND-100 panel clock is incorrect, probably as a result of a power fail.

**No answer from remote system; file-access connection aborted (264 oct)**: Self-explanatory.

**No EOF mark found (177 oct)**: A tape device was unable to find the End-Of-File mark.

**No LAMU area big enough (277 oct)**: Attempt to define a LAMU which is too big to fit into any LAMU areas.

**No file open with this number (132 oct)**: Possibly a symptom of an earlier error. Attempt to access an open file, referencing it by an open-file-number that did not correspond to any open file, either because the number is wrong, or because the file was closed.

**No main directory (043 oct)**: No main directory entered (only user SYSTEM may log in - on the console).

**No more allocate-memory-table indexes available (370 oct)**: Too many allocated areas.

**No more buffer space (131 oct)**: Cannot open further files.

**No more pages available for this user (075 oct)**: All pages reserved for the user are occupied by his files.

**No more physical memory available (367 oct)**: Too much physical memory is reserved by programs that use fixed pages or fixed segments, leaving insufficient swapping space.

**No more remote file-access data segments available (313 oct)**: Too many simultaneous users of remote file accessed.

**No more spooling pages left (344 oct)**: The total size of the objects in the spooling queue (the printing queue) exceeds the space set aside for this purpose.

**No more tracks available (037 oct)**: Self-explanatory.

**No more unused spooling files available (245 oct)**: Self-explanatory.

**No more versions can be created in this object block (345 oct)**: All versions of a file must reside in the same object block in the file system. Attempt to create a new version when no free object entries in current object block.

**No previous version (101 oct)**: Inconsistency in the file system. Attempt to access a version of a file when no such version exists.

**No scratch file open (137 oct)**: Self-explanatory.

**No service is available for this code (330 oct)**: Self-explanatory.

**No space in default directories (073 oct)**: Self-explanatory.

**No spooling for this device (211 oct)**: Self-explanatory.

**No such access code (104 oct)**: Attempt set own, friend or public access for some object specifying an undefined access code.

**No such block (143 oct)**: Self-explanatory.

**No such device name (030 oct)**: Attempt to access a device with a name that is unknown to the system.

**No such file name (056 oct)**: The given file specification does not match any file name on the current directory.

**No such file version (074 oct)**: There is a file with the specified file name, but not with the specified version number.

**No such friend (064 oct)**: The friend specification does not match the name of any existing friend.

**No such logical unit (033 oct)**: Wrong logical unit number, or wrong device name specified.

**No such page (022 oct)**: Inconsistency in the file system or attempt to access an indexed file as if it were contiguous.

**No such program LAMU (375 oct)**: Attempt to refer to a non-existing program LAMU.

**No such queue (212 oct)**: Self-explanatory.

**No such queue entry (207 oct)**: A queue entry has been specified, but not found.

**No such user index (117 oct)**: A user number was specified, but no user has that index number.

**No such user name (046 oct)**: The user specification does not match any user name.

**No such user name in main directory (047 oct)**: The user specification does not match any user name in the main directory.

**No termination handling defined (312 oct)**: Attempt to enable termination handling when no termination handling is defined.

**No user entered (062 oct)**: Attempt to get main directory for current user when no user is entered.

**Not 8 bits character length (356 oct)**: Attempt to select 8-bit character conversion when device is using a different character size.

**Not a master terminal (360 oct)**: Error while using the display facility.

**Not a multiple of hardware block size (252 oct)**: Self-explanatory.

**Not allowed now (3200 oct)**: Function is not allowed at this point.

**Not append access (120 oct)**: Attempt to open a file for append access, thereby violating your access privileges for that file.

**Not connected to specified master terminal (361 oct)**: Error while using the display facility.

**Not contiguous file (340 oct)**: The file specified must be a contiguous file, but is not.

**Not decimal number (023 oct)**: A parameter was given in another number system than expected, or in a non-numeric form when numeric form was expected.

**Not deleted record (231 oct)**: Attempt to read a deleted record which was not deleted.

**Not directory access (070 oct)**: Attempt to perform actions that require higher access privileges.

**Not indexed file (253 oct)**: The file specified should be an indexed file, but is not.

**Not last file on tape (151 oct)**: End of tape was expected, but not found.

**Not last used by you (215 oct)**: Attempt to abort current printout or current batch job, but the job was not yours.

**Not legal from RT-program (351 oct)**: Function illegal from RT-programs.

**Not mass storage file (133 oct)**: The file specified is not a mass storage file.

**Not octal number (024 oct)**: A parameter was expected to be given in the octal number system, but the given input did not look like an octal number.

**Not octal number (337 oct)**: The number specified contains some illegal characters. Watch out for the letters O and L, and for the digits 8 and 9. Periods are also illegal.

**Not open for random read (126 oct)**: Attempt to do a random read when the file was not opened for that kind of access. It was perhaps opened for sequential access.

**Not open for random write (125 oct)**: Attempt to do a random write when the file was not opened for that kind of access. It was perhaps opened for sequential access.

**Not open for sequential read (124 oct)**: Attempt to do a sequential read when the file was not opened for that kind of access. It was perhaps opened for random access.

**Not open for sequential write (123 oct)**: Attempt to do a sequential write when the file was not opened for that kind of access. It was perhaps opened for random access.

**Not peripheral file (206 oct)**: Attempt to access a disk file as if it were a peripheral device.

**Not read access (111 oct)**: Attempt to open a file for read access, thereby violating your access privileges for that file.

**Not read and common access (114 oct)**: Attempt to open a file for several kinds of access, of which some exceeds your access privileges to that file.

**Not read and write access (113 oct)**: Attempt to open a file for several kinds of access, of which some exceeds your access privileges to that file.

**Not read, write and common access (112 oct)**: Attempt to open a file for several kinds of access, of which some exceeds your access privileges to that file.

**Not so much space left (210 oct)**: Self-explanatory.

**Not so much space unreserved in directory (054 oct)**: Attempt to reserve more space than available.

**Not tape device (152 oct)**: Attempt to perform some function that is only allowed on tape devices, on a device of a different kind.

**Not write access (106 oct)**: Attempt to open a file for write access, thereby violating your access privileges for that file.

**Not write and append access (110 oct)**: Attempt to open a file for several kinds of access, of which some exceeds your access privileges to that file.

**Object entry not used (247 oct)**: Attempt to dump an unused object entry.

**Object index too large (305 oct)**: Object index specified is too large.

**Odd number of bytes (right byte in last word insignificant) (234 oct)**: An odd number of bytes was read.

**Outside device limits (100 oct)**: Attempt to access a mass storage device, with some parameter(s) out of range.

**Overflow in read (163 oct)**: Physical block read was too large to fit in buffer.

**Overflow in write (237 oct)**: Block size too large.

**Paper fault (203 oct)**: A printer device ran out of paper or the paper jammed.

**Parity error (167 oct)**: Parity error detected when reading or writing to a device.

**Please terminate current service before requesting new service (331 oct)**: Self-explanatory.

**Program LAMU already exists (374 oct)**: Attempt to create an already existing program LAMU.

**Program LAMU not connected (377 oct)**: Attempt to access a program LAMU without connecting it.

**Program is active (366 oct)**: Attempt to clear a segment containing an active RT-program.

**Protected command (342 oct)**: The command (or subsystem) specified is restricted to users RT and/or SYSTEM only.

**Queue empty (213 oct)**: Attempt to get an entry from an empty queue.

**Queue full (214 oct)**: Attempt to push an element onto a queue when the queue cannot hold more entries.

**Remote file server is not available (333 oct)**: Self-explanatory.

**Reserved space already used (055 oct)**: Attempt to reserve space which is already used.

**Segment not contiguously fixed (241 oct)**: The segment is not fixed at all, or fixed, but its pages are scattered in physical memory. Some programs need contiguously fixed memory in order to communicate with devices that reads physical memory directly.

**Segment not fixed (242 oct)**: Attempt to UNFIX, or access as fixed, a segment which was not fixed in memory.

**Source and destination equal (144 oct)**: The same file has been specified as original and copy in a copying command.

**Source empty (154 oct)**: Attempt to make a copy when the original file contains no data. Ensure that you are copying in the right direction.

**Space already allocated (072 oct)**: Attempt to allocate a specific space to a file or to a user when that space is already allocated to another file or user.

**Space not available to expand file (071 oct)**: Self-explaining.

**TAD protocol error, illegal or inconsistent message (315 oct)**: Internal error when accessing a remote system.

**Tape format error (226 oct)**: Illegal or unsupported magnetic tape format.

**Terminal already in a display table (357 oct)**: Attempt to include a terminal in more than one display table.

**Terminal line not connected (316 oct)**: Self-explanatory.

**The specified device is not a terminal (311 oct)**: Self-explanatory.

**This code is only legal within a service (332 oct)**: Self-explanatory.

**Timeout (no data block found) (202 oct)**: Timeout while waiting for data transfer from a device.

**Too long parameter (044 oct)**: Possibly missing separating character between two parameters.

**Transfer error (141 oct)**: Error accessing a device.

**Two pages must be left unreserved (160 oct)**: Attempt to give the last to pages on a disk device to a user.

**Unit occupied (034 oct)**: A directory is already entered on the specified device and unit, possibly by another user. It may be in use, or he may have forgotten to release it.

**Unknown remote system name (266 oct)**: Self-explanatory.

**Unknown user-control code (327 oct)**: The user control code specified is undefined.

**User already entered (061 oct)**: Self-explanatory.

**User already exists (051 oct)**: There exists already a user with the specified name.

**User does not exist (250 oct)**: A user number has appeared that is out of range, or a user specification that does not match any user name.

**User does not exist in the same main directory as you (257 oct)**: Attempt to create a user on another main directory as friend.

**User has files (052 oct)**: Attempt to delete a user before deleting its files.

**User is entered (053 oct)**: Attempt to delete or rename a user while he is logged in.

**Volume not on specified unit (230 oct)**: Both a (tape) volume and a (tape) drive unit have been specified, but the volume was not found on the unit. Possibly wrong tape mounted.

**Warning; 2-bank prog. file, but segment is only 1-bank (307 oct)**: Attempt to run 2-bank programs when background segment is 1-bank.

**Warning; no such page in data bank, program starts as 1-bank (310 oct)**: Attempt to access non-existing part of a 2-bank program.

**Write-protect violation (175 oct)**: Attempt to write to a device that has been (physically) write-protected.

**Wrong format in file (325 oct)**: Self-explanatory.

**Wrong password (060 oct)**: The given password does not match the given user name.

**Wrong project password (321 oct)**: Self-explanatory.

**You are not authorized to do this (025 oct)**: Attempt to use functions which are restricted to users RT and/or SYSTEM only.

**You can only log in on a main directory (303 oct)**: Attempt to specify a directory, which is not a main directory, at login.


---

# APPENDIX D
## STANDARD PERIPHERAL FILE NAMES

The following standard peripheral file names are recognized in SINTRAN III.

| ND no.             | Description           | Peripheral File Name                              | Notes |
|--------------------|-----------------------|---------------------------------------------------|-------|
| 202, 204–228       | Terminals             | TERMINAL                                          | 1. Refers to own terminal in background. 2. Terminals can also be PRINTER. |
| 252, 254           | Intercomputer link    | CHANNEL-0 … CHANNEL-15                            | If only one link. Links with background programs are usually not included. |
|                    |                       | L1-CH-0 … L1-CH-15; L2-CH-0 … L2-CH-15            | If two or more links. |
| 301, 302           | Paper Tape Reader     | TAPE-READER                                       | Suffix "-1", "-2", etc., is used if more than one device. |
| 303                | Paper Tape Punch      | TAPE-PUNCH                                        | See ND-301. |
| 305, etc.          | Floppy Disk           | FLOPPY-1, FLOPPY-2, …                             | These names only work with one controller. |
| 400, etc.          | Card Reader           | CARD-READER                                       | See ND-301. |
| 430, 431, etc.     | Line Printer          | LINE-PRINTER                                      | See ND-301. |
| 414, 415, 417      | Matrix Printer        | PRINTER                                           | See ND-301. |
| 420                | Card Punch            | CARD-PUNCH                                        | See ND-301. |
| 515, etc.          | Magnetic Tape         | MAG-TAPE-0, MAG-TAPE-1, …                         | These names only work with one controller. |
| 603, 604, 605, 606 | Versatec Printer/Plotter | LINE-PRINTER-1, LINE-PRINTER-2, …                | If no other line printer on the system. |
|                    |                       | VERSATEC-1, VERSATEC-2, …                         | If another line printer on the system. |


---

# APPENDIX E
## STANDARD NAMES OF MASS STORAGE DEVICES

The following standard names are recognized for mass storage devices in SINTRAN III.

| Device Name | Description |
|-------------|-------------|
| DISC-10MB-1 | 10-Megabyte cartridge disk controller 1 |
| DISC-10MB-2 | 10-Megabyte cartridge disk controller 2 |
| DISC-14MB-1 | 14-Megabyte disk controller 1 |
| DISC-14MB-2 | 14-Megabyte disk controller 2 |
| DISC-16MB-1 | 16-Megabyte disk controller 1 |
| DISC-16MB-2 | 16-Megabyte disk controller 2 |
| DISC-21MB-1 | 21-Megabyte disk controller 1 |
| DISC-21MB-2 | 21-Megabyte disk controller 2 |
| DISC-23MB-1 | 23-Megabyte disk controller 1 |
| DISC-23MB-2 | 23-Megabyte disk controller 2 |
| DISC-28MB-1 | 28-Megabyte disk controller 1 |
| DISC-28MB-2 | 28-Megabyte disk controller 2 |
| DISC-30MB-1 | 30-Megabyte big cartridge disk controller 1 |
| DISC-30MB-2 | 30-Megabyte bit cartridge disk controller 2 |
| DISC-33MB-1 | 33-Megabyte disk controller 1 |
| DISC-33MB-2 | 33-Megabyte disk controller 2 |
| DISC-36MB-1 | 36-Megabyte disk controller 1 (Butterfly) |
| DISC-36MB-2 | 36-Megabyte disk controller 2 |
| DISC-38MB-1 | 38-Megabyte disk controller 1 |
| DISC-38MB-2 | 38-Megabyte disk controller 2 |
| DISC-45MB-1 | 45-Megabyte disk controller 1 |
| DISC-45MB-2 | 45-Megabyte disk controller 2 |
| DISC-60MB-1 | 60-Megabyte disk controller 1 |
| DISC-60MB-2 | 60-Megabyte disk controller 2 |
| DISC-66MB-1 | 66-Megabyte disk controller 1 |
| DISC-66MB-2 | 66-Megabyte disk controller 2 |
| DISC-70MB-1 | 70-Megabyte disk controller 1 |
| DISC-70MB-2 | 70-Megabyte disk controller 2 |
| DISC-70MB-3 | 70-Megabyte disk controller 3 |
| DISC-70MB-4 | 70-Megabyte disk controller 4 |
| DISC-74MB-1 | 74-Megabyte disk controller 1 |
| DISC-74MB-2 | 74-Megabyte disk controller 2 |
| DISC-75MB-1 | 75-Megabyte disk controller 1 |
| DISC-75MB-2 | 75-Megabyte disk controller 2 |
| DISC-75MB-3 | 75-Megabyte disk controller 3 |
| DISC-75MB-4 | 75-Megabyte disk controller 4 |
| DISC-90MB-1 | 90-Megabyte disk controller 1 |
| DISC-90MB-2 | 90-Megabyte disk controller 2 |
| DISC-140MB-1-F | 140-Megabyte disk controller 1 (fixed) |
| DISC-140MB-2-F | 140-Megabyte disk controller 2 (fixed) |
| DISC-140MB-3-F | 140-Megabyte disk controller 3 (fixed) |
| DISC-140MB-4-F | 140-Megabyte disk controller 4 (fixed) |
| DISC-2-70MB-1-F | Subdivided 140-Megabyte disk controller 1 (fixed) |
| DISC-2-70MB-2-F | Subdivided 140-Megabyte disk controller 2 (fixed) |
| DISC-2-70MB-3-F | Subdivided 140-Megabyte disk controller 3 (fixed) |
| DISC-2-70MB-4-F | Subdivided 140-Megabyte disk controller 4 (fixed) |
| DISC-2-75MB-1 | Subdivided 150-Megabyte disk controller 1 |
| DISC-2-75MB-2 | Subdivided 150-Megabyte disk controller 2 |
| DISC-225MB-1-R | 225-Megabyte disk controller 1 (removable) |
| DISC-225MB-2-R | 225-Megabyte disk controller 2 (removable) |
| DISC-225MB-3-R | 225-Megabyte disk controller 3 (removable) |
| DISC-225MB-4-R | 225-Megabyte disk controller 4 (removable) |
| DISC-3-75MB-1 | Subdivided 225-Megabyte disk controller 1 |
| DISC-3-75MB-2 | Subdivided 225-Megabyte disk controller 2 |
| DISC-288MB-1-F | 288-Megabyte disk controller 1 (fixed) |
| DISC-288MB-2-F | 288-Megabyte disk controller 2 (fixed) |
| DISC-288MB-3-F | 288-Megabyte disk controller 3 (fixed) |
| DISC-288MB-4-F | 288-Megabyte disk controller 4 (fixed) |
| DISC-288MB-1-E | 288-Megabyte disk controller 1 (EMD) |
| DISC-288MB-2-E | 288-Megabyte disk controller 2 (EMD) |
| DISC-288MB-3-E | 288-Megabyte disk controller 3 (EMD) |
| DISC-288MB-4-E | 288-Megabyte disk controller 4 (EMD) |
| DISC-288MB-1-R | 288-Megabyte disk controller 1 (removable) |
| DISC-288MB-2-R | 288-Megabyte disk controller 2 (removable) |
| DISC-288MB-3-R | 288-Megabyte disk controller 3 (removable) |
| DISC-288MB-4-R | 288-Megabyte disk controller 4 (removable) |
| DISC-4-70MB-1-F | Subdivided 280-Megabyte disk controller 1 (fixed) |
| DISC-4-70MB-2-F | Subdivided 280-Megabyte disk controller 2 (fixed) |
| DISC-4-70MB-3-F | Subdivided 280-Megabyte disk controller 3 (fixed) |
| DISC-4-70MB-4-F | Subdivided 280-Megabyte disk controller 4 (fixed) |
| DISC-4-70MB-1-E | Subdivided 280-Megabyte disk controller 1 (EMD) |
| DISC-4-70MB-2-E | Subdivided 280-Megabyte disk controller 2 (EMD) |
| DISC-4-70MB-3-E | Subdivided 280-Megabyte disk controller 3 (EMD) |
| DISC-4-70MB-4-E | Subdivided 280-Megabyte disk controller 4 (EMD) |
| DISC-4-70MB-1-R | Subdivided 280-Megabyte disk controller 1 (remov.) |
| DISC-4-70MB-2-R | Subdivided 280-Megabyte disk controller 2 (remov.) |
| DISC-4-70MB-3-R | Subdivided 280-Megabyte disk controller 3 (remov.) |
| DISC-4-70MB-4-R | Subdivided 280-Megabyte disk controller 4 (remov.) |
| DISC-450MB-1-F | 450-Megabyte disk controller 1 (fixed) |
| DISC-450MB-2-F | 450-Megabyte disk controller 2 (fixed) |
| DISC-450MB-3-F | 450-Megabyte disk controller 3 (fixed) |
| DISC-450MB-4-F | 450-Megabyte disk controller 4 (fixed) |
| DISC-450MB-1-N | 450-Megabyte disk controller 1 (NEC) |
| DISC-450MB-2-N | 450-Megabyte disk controller 2 (NEC) |
| DISC-450MB-3-N | 450-Megabyte disk controller 3 (NEC) |
| DISC-450MB-4-N | 450-Megabyte disk controller 4 (NEC) |
| DISC-6-70MB-1-F | Subdivided 420-Megabyte disk controller 1 (fixed) |
| DISC-6-70MB-2-F | Subdivided 420-Megabyte disk controller 2 (fixed) |
| DISC-6-70MB-3-F | Subdivided 420-Megabyte disk controller 3 (fixed) |
| DISC-6-70MB-4-F | Subdivided 420-Megabyte disk controller 4 (fixed) |
| DISC-6-70MB-1-N | Subdivided 420-Megabyte disk controller 1 (NEC) |
| DISC-6-70MB-2-N | Subdivided 420-Megabyte disk controller 2 (NEC) |
| DISC-6-70MB-3-N | Subdivided 420-Megabyte disk controller 3 (NEC) |
| DISC-6-70MB-4-N | Subdivided 420-Megabyte disk controller 4 (NEC) |
| DISC-2-225MB-1-F | Subdivided 450-Megabyte disk controller 1 (fixed) |
| DISC-2-225MB-2-F | Subdivided 450-Megabyte disk controller 2 (fixed) |
| DISC-2-225MB-3-F | Subdivided 450-Megabyte disk controller 3 (fixed) |
| DISC-2-225MB-4-F | Subdivided 450-Megabyte disk controller 4 (fixed) |
| DISC-2-225MB-1-N | Subdivided 450-Megabyte disk controller 1 (NEC) |
| DISC-2-225MB-2-N | Subdivided 450-Megabyte disk controller 2 (NEC) |
| DISC-2-225MB-3-N | Subdivided 450-Megabyte disk controller 3 (NEC) |
| DISC-2-225MB-4-N | Subdivided 450-Megabyte disk controller 4 (NEC) |
| MAG-TAPE-1 | Magnetic tape controller 1 (fixed) |
| MAG-TAPE-2 | Magnetic tape controller 2 (fixed) |
| MAG-TAPE-3 | Magnetic tape controller 3 (fixed) |
| MAG-TAPE-4 | Magnetic tape controller 4 (fixed) |
| FLOPPY-DISC-1 | Floppy disk controller 1 |
| FLOPPY-DISC-2 | Floppy disk controller 2 |


---

