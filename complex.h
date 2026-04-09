#pragma once
#include <cmath>

template <typename T>
struct Complex
{
    T real, imag;

    Complex(T r = 0, T i = 0) : real(r), imag(i) {}

    Complex operator+(const Complex &o) const { return {real + o.real, imag + o.imag}; }
    Complex operator*(const Complex &o) const
    {
        return {real * o.real - imag * o.imag,
                real * o.imag + imag * o.real};
    }
    Complex &operator+=(const Complex &o)
    {
        *this = *this + o;
        return *this;
    }

    Complex conj() const { return {real, -imag}; }
    T abs2() const { return real * real + imag * imag; }
};