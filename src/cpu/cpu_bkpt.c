/*
 * nd100x - ND100 Virtual Machine
 *
 *
 * This file is originated from the nd100x project.
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

#include <string.h>

#include "cpu_types.h"
#include "cpu_protos.h"


BreakpointManager *mgr;


/// @brief Hash function for the breakpoint manager
/// @param address Memory address to hash
/// @return Hash value  
static int hash_address(uint16_t address)
{
    return address % HASH_SIZE;
}

/// @brief Initialize the breakpoint manager
/// @return void
/// @note Initialize the breakpoint manager
void breakpoint_manager_init()
{
    mgr= (BreakpointManager *)malloc(sizeof(BreakpointManager));
    if (!mgr)
    {
        fprintf(stderr, "Failed to allocate memory for BreakpointManager\n");
        exit(EXIT_FAILURE);
    }

    memset(mgr->buckets, 0, sizeof(mgr->buckets));
}

/// @brief Cleanup the breakpoint manager
/// @return void
/// @note Clean up and free the breakpoint manager memory
void breakpoint_manager_cleanup()
{
    if (mgr) {
        breakpoint_manager_clear();
        free(mgr);
    }
}

/// @brief Set the step count to 1
/// @return void
/// @note Used by the debugger to single step
void breakpoint_manager_step_one()
{
    mgr->step_count=1;
}

/// @brief Add breakpoint (create new entry or append to list)
/// @param address Memory address to add breakpoint to
/// @param type Breakpoint type (user, function, data, instruction, temporary)
/// @param condition Condition expression (optional - not yet supported)
/// @param hitCondition Hit condition expression (optional)
/// @param logMessage Log message (optional)
void breakpoint_manager_add(uint16_t address, BreakpointType type, const char *condition, const char *hitCondition, const char *logMessage)
{

    if (mgr == NULL) {
        breakpoint_manager_init();
    }

    int h = hash_address(address);

    // Prevent adding duplicate temporary breakpoint at address
    if (type == BP_TYPE_TEMPORARY) {
    BreakpointEntry* curr = mgr->buckets[h];
    while (curr) {
        if (curr->address == address && curr->type == BP_TYPE_TEMPORARY) {
            printf("[BreakpointManager] Temporary breakpoint already exists at %04X\n", address);
            return; // skip adding
        }
        curr = curr->next;
    }
    }

    BreakpointEntry *entry = (BreakpointEntry *)malloc(sizeof(BreakpointEntry));
    entry->address = address;
    entry->type = type;
    entry->condition = condition ? strdup(condition) : NULL;
    entry->hitCondition = hitCondition ? strdup(hitCondition) : NULL;
    entry->logMessage = logMessage ? strdup(logMessage) : NULL;
    entry->hitCount = 0;
    entry->next = mgr->buckets[h];

    mgr->buckets[h] = entry;
}

/// @brief Remove entries at address matching type (or all if type == -1)
/// @param address Memory address to remove breakpoints from
/// @param type Breakpoint type to remove (or all if type == -1)
void breakpoint_manager_remove(uint16_t address, int type)
{
    int h = hash_address(address);
    BreakpointEntry *prev = NULL;
    BreakpointEntry *curr = mgr->buckets[h];

    while (curr)
    {
        if (curr->address == address && (type == -1 || curr->type == type))
        {
            BreakpointEntry *to_delete = curr;
            if (prev)
                prev->next = curr->next;
            else
                mgr->buckets[h] = curr->next;

            free(to_delete->condition);
            free(to_delete->hitCondition);
            free(to_delete->logMessage);
            free(to_delete);
            curr = (prev) ? prev->next : mgr->buckets[h];
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}

/// @brief Clear all breakpoints   
/// @return void
/// @note Clear all breakpoints
void breakpoint_manager_clear()
{
    for (int h = 0; h < HASH_SIZE; h++)
    {
        BreakpointEntry *curr = mgr->buckets[h];
        while (curr)
        {
            BreakpointEntry *next = curr->next;
            free(curr->condition);
            free(curr->hitCondition);
            free(curr->logMessage);
            free(curr);
            curr = next;
        }
        mgr->buckets[h] = NULL;
    }
}

/// @brief Query breakpoints at address
/// @param address Memory address to query breakpoints from
/// @param matches Array to store matching breakpoints
/// @param matchCount Number of matching breakpoints
/// @return Number of matching entries
int breakpoint_manager_check( uint16_t address, BreakpointEntry** matches[], int* matchCount) {
    int h = hash_address(address);
    BreakpointEntry* curr = mgr->buckets[h];

    BreakpointEntry* tempList[10];
    int tempCount = 0;
    BreakpointEntry* userList[10];
    int userCount = 0;

    while (curr) {
        if (curr->address == address) {
            if (curr->type == BP_TYPE_TEMPORARY) {
                tempList[tempCount++] = curr;
                if (tempCount >= 10) break;
            } else {
                userList[userCount++] = curr;
                if (userCount >= 10) break;
            }
        }
        curr = curr->next;
    }

    if (tempCount > 0) {
        *matches = malloc(sizeof(BreakpointEntry*) * tempCount);
        memcpy(*matches, tempList, sizeof(BreakpointEntry*) * tempCount);
        *matchCount = tempCount;
    } else if (userCount > 0) {
        *matches = malloc(sizeof(BreakpointEntry*) * userCount);
        memcpy(*matches, userList, sizeof(BreakpointEntry*) * userCount);
        *matchCount = userCount;
    } else {
        *matches = NULL;
        *matchCount = 0;
    }

    return *matchCount;
}

/// @brief Check if the current program counter (PC) matches any breakpoints.
/// @return BreakpointType
/// @note Check if the current program counter (PC) matches any breakpoints.
int check_for_breakpoint(void)
{

    // Auto-initialize the breakpoint manager if it is not initialized
    if (mgr == NULL) {
        breakpoint_manager_init();
    }

    BreakpointEntry** hits;
    int hitCount;
    BreakpointType btType = BT_NONE;
    uint16_t pc = gPC;

    // Check for single step
    if (mgr->step_count > 0) {
        mgr->step_count--;
        if (mgr->step_count == 0) {
            set_cpu_stop_reason(STOP_REASON_STEP);
            set_cpu_run_mode(CPU_BREAKPOINT);
            return STOP_REASON_STEP;
        }
    }

    // Check for breakpoints
    if (breakpoint_manager_check(pc, &hits, &hitCount)) {
        for (int i = 0; i < hitCount; i++) {
            BreakpointEntry* bp = hits[i];
            printf("[CPU] Hit breakpoint at %06o type=%d hitCount=%d\n", pc, bp->type, bp->hitCount);
    
            // Evaluate condition expression if applicable
            bool condition_ok = true; // TODO: implement evaluator
            bool hit_ok = true;
    
            if (bp->hitCondition) {
                int hitCondVal = atoi(bp->hitCondition);
                hit_ok = (bp->hitCount == hitCondVal);
            }
    
            if (condition_ok && hit_ok) {
                if (bp->logMessage) {
                    // Expand log message vars (simple demo)
                    printf("[LOGPOINT] %s\n", bp->logMessage);
                } else {
                    // Trigger stop event
                    btType = bp->type;
                    set_cpu_stop_reason(bp->type == BP_TYPE_TEMPORARY ? STOP_REASON_STEP : STOP_REASON_BREAKPOINT);
                    set_cpu_run_mode(CPU_BREAKPOINT);                    
                }
    
                if (bp->type == BP_TYPE_TEMPORARY) {
                    // Auto-remove temp breakpoint
                    breakpoint_manager_remove(bp->address, BP_TYPE_TEMPORARY);
                }
            }
        }
        free(hits); // free list
    }

    return btType;
}

/// @brief Convert breakpoint type to stop reason
/// @param t Breakpoint type
/// @return Stop reason
CpuStopReason stopReasonFromBreakpoint(BreakpointType t) {
    switch (t) {
        case BP_TYPE_USER: return STOP_REASON_BREAKPOINT;
        case BP_TYPE_FUNCTION: return STOP_REASON_FUNCTION_BREAKPOINT;
        case BP_TYPE_DATA: return STOP_REASON_DATA_BREAKPOINT;
        case BP_TYPE_INSTRUCTION: return STOP_REASON_INSTRUCTION_BREAKPOINT;
        default: return STOP_REASON_BREAKPOINT;
    }
}