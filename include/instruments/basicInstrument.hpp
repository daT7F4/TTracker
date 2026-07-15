#pragma once

#include "instrumentBase.hpp"

class basicInstrument : public instrumentBase {
   public:
    basicInstrument() = default;
    ~basicInstrument();
    void resize(size_t bufferSize) override;
};