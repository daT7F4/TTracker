#include "effects/effectBase.hpp"

void effectBase::resize(size_t bufferSize){}
void effectBase::process(int16_t* in, size_t bufferSize){}
void effectBase::loadVariable(uint16_t id, specialVariable var){
    if(data.contains(id))
        data[id] = var;
}