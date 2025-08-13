#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#if DEBUG
#define DLOG(fmt, ...)                                                       \
  do {                                                                       \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)
#else
#define DLOG(fmt, ...) \
  do {                 \
  } while (0)  // No-op in release builds
#endif

#endif  // LOG_H