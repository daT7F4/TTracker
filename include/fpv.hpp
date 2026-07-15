#pragma once

#include <assert.h>
#include <stdint.h>

#include <concepts>

#define FP_TYPE int32_t

inline constexpr FP_TYPE FP_PRECISION = 8;
inline constexpr auto FP_FLOAT = 1 << FP_PRECISION;
inline constexpr auto FP_POSITIVE_INFINITY =
    (1 << (sizeof(FP_TYPE) * 8 - FP_PRECISION - 1)) - 1;
inline constexpr auto FP_NEGATIVE_INFINITY = ~FP_POSITIVE_INFINITY;

struct fpv {
    fpv() = default;
    fpv(auto in) {
        if constexpr (sizeof(in) >= sizeof(FP_TYPE)) {
            // assert to make sure we won't overflow when initialising fpv
            assert(in <= static_cast<decltype(in)>(FP_POSITIVE_INFINITY));
            assert(in >= static_cast<decltype(in)>(FP_NEGATIVE_INFINITY));
        }
        v = (FP_TYPE)(in * FP_FLOAT);
    }

    FP_TYPE toInt() const { return v >> FP_PRECISION; }
    float toFloat() const { return (float)v / FP_FLOAT; }

    fpv operator+(fpv rhs) const { return v + rhs.v; }
    friend fpv operator+(auto lhs, fpv rhs) { return fpv{lhs} + rhs; }
    fpv& operator+=(fpv rhs) {
        v += rhs.v;
        return *this;
    }

    fpv operator-(fpv rhs) const { return v - rhs.v; }
    friend fpv operator-(auto lhs, fpv rhs) { return fpv{lhs} - rhs; }
    fpv& operator-=(fpv rhs) {
        v -= rhs.v;
        return *this;
    }

    fpv operator*(fpv rhs) const {
        fpv ret{};
        ret.v = (int32_t)(((int64_t)v * rhs.v) >> FP_PRECISION);
        return ret;
    }
    friend fpv operator*(auto lhs, fpv rhs) { return fpv{lhs} * rhs; }
    fpv& operator*=(fpv rhs) {
        *this = *this * rhs;
        return *this;
    }

    fpv operator/(fpv rhs) const {
        fpv ret{};
        ret.v = (int32_t)((((int64_t)v << FP_PRECISION) / rhs.v));
        return ret;
    }
    friend fpv operator/(auto lhs, fpv rhs) { return fpv{lhs} / rhs; }
    fpv& operator/=(fpv rhs) {
        *this = *this / rhs;
        return *this;
    }

    bool operator==(fpv rhs) { return v == rhs.v; }
    bool operator!=(fpv rhs) { return v != rhs.v; }
    bool operator<(fpv rhs) { return v < rhs.v; }
    bool operator>(fpv rhs) { return v > rhs.v; }
    bool operator<=(fpv rhs) { return v <= rhs.v; }
    bool operator>=(fpv rhs) { return v >= rhs.v; }

    FP_TYPE v = 0;
};