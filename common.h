#ifndef COMMON_H
#define COMMON_H
	
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define UINT8_T_LIMIT 255
#define UINT16_T_LIMIT 65535

#define CALL_FRAMES_MAX 128

#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE
#define DEBUG_LOG_GC
#define EXCESSIVE_GC_MODE

#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_PRINT_CODE
#undef DEBUG_LOG_GC
#undef EXCESSIVE_GC_MODE


#endif
