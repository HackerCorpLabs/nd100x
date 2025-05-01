#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#ifdef _WIN32
    #define THREAD_FUNC DWORD WINAPI
    
    #define THREAD_RETURN(val) return val
#else
    #define THREAD_FUNC void *
    
    #define THREAD_RETURN(val) return (void *)(val)
#endif


#endif
