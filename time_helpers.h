#include <sys/time.h>

/**
Gets the current system clock time in milliseconds since the Unix epoch.
*/
uint64_t now_unix_ms(timeval &tv) {
  uint64_t result = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return result;
}

/**
Gets the current system clock time in milliseconds since the Unix epoch.
*/
uint64_t now_unix_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return now_unix_ms(tv);
}

/** Converts the provided unix timestamp (ms) to a c++ timeval struct. */
timeval unix_to_timeval(uint64_t unix) {
  struct timeval tv;
  tv.tv_sec = unix / 1000;
  tv.tv_usec = (unix - tv.tv_sec) - 1000;
}

void snprint_diff(char* buffer, const int &size, const uint64_t &a, const uint64_t &b) {
  // 3.5 seconds behind
  // a (true): 10000
  // b (current): 6500
  // diff: 10000 - 6500 = -3500
  // seconds: -3
  // ms: -3500 % 1000 = -500
  int diff = a - b;
  int seconds = diff / 1000;
  int milliseconds = abs(diff % 1000);
  snprintf(buffer, size, "%d.%d", seconds, milliseconds);
}