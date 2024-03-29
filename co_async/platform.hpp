#pragma once

#if defined(_WIN32)
#define CO_ASYNC_PLAT_WIN32 1
#elif defined(__linux__)
#define CO_ASYNC_PLAT_LINUX 1
#elif defined(__APPLE__)
#define CO_ASYNC_PLAT_APPLE 1
#else
#define CO_ASYNC_PLAT_UNKNOWN 1
#endif
