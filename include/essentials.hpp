#pragma once

#include <Arduino.h>

#include "fpv.hpp"

struct env {
    fpv getEnvelope(fpv pressTimestamp, fpv releaseTimestamp);
    fpv a;
    fpv d;
    fpv s;
    fpv r;
};

struct storedVariable{
    void* ptr;
    size_t size;
    String name;
};