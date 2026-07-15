#pragma once

#include <map>

#include "fpv.hpp"
#include "essentials.hpp"

struct effectBase{
    effectBase() = default;
    virtual ~effectBase() = default;
    virtual void resize(size_t bufferSize);
    virtual void process(int16_t* in, size_t bufferSize);
    virtual void customVariable(void* data);

    void loadVariable(uint16_t id, specialVariable var);

    uint16_t type = 0;

    std::map<uint16_t, specialVariable> data;
};