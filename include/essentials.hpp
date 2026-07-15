#pragma once

#include <Arduino.h>

#include "fpv.hpp"
#include "whatever.hpp"

using any = whatever<heapAllocatedTag>;

struct env {
    fpv getEnvelope(fpv pressTimestamp, fpv releaseTimestamp);
    fpv a;
    fpv d;
    fpv s;
    fpv r;
};

struct specialVariable{
    String name;
    uint32_t type;
    any value;
};