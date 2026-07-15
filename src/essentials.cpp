#include "essentials.hpp"

fpv env::getEnvelope(fpv pressTimestamp, fpv releaseTimestamp) {
    fpv pt = (millis() - pressTimestamp) / 1000;
    if(releaseTimestamp > 0)
        pt = (releaseTimestamp - pressTimestamp) / 1000;
    fpv rt = (millis() - releaseTimestamp) / 1000;
    if (rt > r || pt == 0) return 0;
    fpv result = 0;
    if (pt <= a)
        result = 1 / a * pt;
    else if (pt - a <= d)
        result = 1 + s * (a - pt) / d;
    else
        result = 1 - s;
    if (rt > 0) result *= 1 / r * rt;
    return result;
}