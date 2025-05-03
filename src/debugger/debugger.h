#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#ifdef _WIN32
    #define THREAD_FUNC DWORD WINAPI
    
    #define THREAD_RETURN(val) return val
    #define WIN_RESULT DWORD WINAPI 
#else
    #define THREAD_FUNC void *
    #define LPVOID void *
    #define THREAD_RETURN(val) return (void *)(val)
    #define WIN_RESULT void*
#endif


#endif
