#include "math.h"
// TODO: Faster fabs in assembly
float fabs(float f) {
    return f < 0.0 ? -f : f;
}
#include <stdlib.h>
#include <stdio.h>
double pow(double x, double y) {
    if(y == 0.0) return 1.0;
    if(y == 1.0) return x;
    double result = 1.0;
    int neg = y < 0;
    if(neg) y = -y;
    while(y > 0) {
        if((int)y & 1) result *= x;
        x *= x;
        y /= 2.0;
    }
    return neg ? 1.0 / result : result;
}

double ldexp(double x, int exponent) {
    double too=1.0;
    while(exponent > 0) {
        too *= 2.0;
        exponent--;
    }
    return x*too;
}
// TODO: Proper floor. This is not exactly correct as it doesn't account for sign
double floor(double f) {
    return (double)((unsigned long long)f);
}
#include <errno.h>
double fmod(double x, double y) {
    if (y == 0) {
        errno = EDOM;
        return x;
    }
    double q = x / y;
    double r = x - (y * floor(q));
    return r;
}

double frexp(double x, int* expptr) {
    if(x == 0.0) { *expptr = 0; return 0.0; }
    int exp = 0;
    while(x >= 2.0) { x /= 2.0; exp++; }
    while(x < 1.0)  { x *= 2.0; exp--; }
    *expptr = exp;
    return x;
}

double sin(double x) {
    int deg = (int)(x * 57.29577951308232) % 360;
    if (deg < 0) deg += 360;
    return sin_table[deg] / 1000.0;
}
double cos(double x) {
    return sin(x + 1.5707963267948966); // + PI/2
}
double tan(double x) {
    double c = cos(x);
    if(c == 0.0) return 1e308;
    return sin(x) / c;
}
double asin(double x) {
    if(x < -1 || x > 1) return 0;
    return atan2(x, sqrt(1.0 - x*x));
}
double acos(double x) {
    if(x < -1 || x > 1) return 0;
    return 3.14159265358979323846 / 2.0 - asin(x);
}
double atan(double x) {
    return atan2(x, 1.0);
}
double atan2(double y, double x) {
    if(x == 0 && y == 0) return 0;
    double r = sqrt(x*x + y*y);
    double cos_val = x / r;
    double angle = acos(cos_val);
    if(y < 0) angle = -angle;
    return angle;
}
double sinh(double x) {
    double e = exp(x);
    return (e - 1.0/e) / 2.0;
}
double cosh(double x) {
    double e = exp(x);
    return (e + 1.0/e) / 2.0;
}
double tanh(double x) {
    double e2 = exp(2*x);
    return (e2 - 1) / (e2 + 1);
}
double asin(double x) {
    fprintf(stderr, "TODO: asin(double x) is a stub");
    exit(1);
}
double acos(double x) {
    fprintf(stderr, "TODO: acos(double x) is a stub");
    exit(1);
}
double atan(double x) {
    fprintf(stderr, "TODO: atan(double x) is a stub");
    exit(1);
}
double sinh(double x) {
    fprintf(stderr, "TODO: sinh(double x) is a stub");
    exit(1);
}
double cosh(double x) {
    fprintf(stderr, "TODO: cosh(double x) is a stub");
    exit(1);
}
double tanh(double x) {
    fprintf(stderr, "TODO: tanh(double x) is a stub");
    exit(1);
}
double log(double x) {
    fprintf(stderr, "TODO: log(double x) is a stub");
    exit(1);
}
double log10(double x) {
    fprintf(stderr, "TODO: log10(double x) is a stub");
    exit(1);
}
double log2(double x) {
    fprintf(stderr, "TODO: log2(double x) is a stub");
    exit(1);
}
double exp(double x) {
    fprintf(stderr, "TODO: exp(double x) is a stub");
    exit(1);
}
double sqrt(double x) {
    if(x < 0) return -1;
    if(x == 0) return 0;
    double guess = x / 2.0;
    for(int i = 0; i < 50; i++) {
        guess = (guess + x / guess) / 2.0;
    }
    return guess;
}
double ceil(double x) {
    int i = (int)x;
    return x > 0 && x != (double)i ? i + 1.0 : i;
}
double exp(double x) {
    if(x == 0) return 1.0;
    double sum = 1.0;
    double term = 1.0;
    for(int i = 1; i < 30; i++) {
        term *= x / i;
        sum += term;
    }
    return sum;
}
double log(double x) {
    if(x <= 0) return -1e308;
    double result = 0;
    int n = 0;
    while(x > 2.0) { x /= 2.718281828459045; n++; }
    while(x < 0.5) { x *= 2.718281828459045; n--; }
    double y = (x - 1) / (x + 1);
    double y2 = y * y;
    double term = y;
    for(int i = 1; i < 20; i++) {
        result += term / (2 * i - 1);
        term *= y2;
    }
    return result * 2.0 + n;
}
double log10(double x) {
    return log(x) / 2.302585092994046;
}
double log2(double x) {
    return log(x) / 0.6931471805599453;
}
double ceil(double x) {
    fprintf(stderr, "TODO: ceil(double x) is a stub");
    exit(1);
}
double atan2(double y, double x) {
    fprintf(stderr, "TODO: atan2(double y, double x) is a stub");
    exit(1);
}
float sinf(float f) {
    return (float)sin((double)f);
}

float cosf(float f) {
    return (float)cos((double)f);
}
