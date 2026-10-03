#pragma once
#include <stdio.h>

extern bool debugTrace;

// Arguments are only evaluated when tracing is on, so disabled tracing costs one branch.
#define TRACE(...) do { if (debugTrace) { printf(__VA_ARGS__); } } while (0)
