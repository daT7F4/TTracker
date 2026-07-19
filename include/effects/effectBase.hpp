#pragma once

#include <map>

#include "fpv.hpp"
#include "essentials.hpp"

struct effectBase{
    effectBase() = default;
    virtual ~effectBase() = 0;
    virtual void resize(size_t bufferSize) = 0;
    virtual void process(int16_t* in, size_t bufferSize) = 0;

    uint16_t type = 0;

    std::map<uint16_t, storedVariable> data;
};