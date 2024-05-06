#include <math.h>
#include <stdint.h>
#include "fbase.h"

double fourier(size_t mode, double t) {
    if (mode == 0) {
        return 1.0;
    }
    double shift = ((mode % 2) == 0) ? 0.5 : 0.0;
    return cos((((double)((mode + 1) / 2)) * t * 2.0 + shift) * M_PI) * sqrt(2.0);
}

double heaviside(size_t mode, double t) {
    if (mode == 0) {
        return 1.0;
    }
    if (mode > UINT32_MAX) {
        return 1.0;
    }
    uint32_t mode32 = mode;
    mode32 |= mode32 >> 16;
    mode32 |= mode32 >> 8;
    mode32 |= mode32 >> 4;
    mode32 |= mode32 >> 2;
    mode32 |= mode32 >> 1;
    mode32 += 1;
    mode32 >>= 1;
    uint32_t submode = mode;
    submode ^= mode32;
    double u = (t * ((double)mode32) - submode) * 2.0 - 1.0;
    double k = sqrt((double)mode32);
    if (u < 0.0) {
        if (u < -1.0) {
            return 0.0;
        } else {
            return -k;
        }
    } else {
        if (u < 1.0) {
            return k;
        } else {
            return 0.0;
        }
    }
}

double legendre(size_t mode, double t) {
    if (mode == 0) {
        return 1.0;
    }
    t = t * 2.0 - 1.0;
    double a = 1.0;
    double b = t;
    size_t n = 1;
    while (n < mode) {
        double h = ((double)(2 * n + 1)) * t * b - ((double)n) * a;
        a = b;
        ++n;
        b = h / ((double)n);
    }
    return b * sqrt((double)(2 * mode + 1));
}
