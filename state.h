#pragma once
#include <vector>
#include <cmath>
#include <stdexcept>
#include <random>
#include "complex.h"
#include <numeric>
#include <stdlib.h>

using Amplitude = Complex<double>;

class QuantumState
{
public:
    std::vector<Amplitude> amplitudes;
    int num_qubits;

    explicit QuantumState(int n) : amplitudes(1 << n, {0, 0}), num_qubits(n)
    {
        amplitudes[0] = {1.0, 0};
    }

    void print_amplitudes()
    {
        for (const Amplitude &a : amplitudes)
        {
            std::cout << "Real: " << a.real << ", Imag: " << a.imag << "\n";
        }
    }

    double probability(int i) const
    {
        return amplitudes[i].abs2();
    }

    int measure()
    {
        std::vector<double> probs;
        for (auto &a : amplitudes)
        {
            // get abs probability of a with abs2() and push it onto probability arr
            probs.push_back(a.abs2());
        }

        // get random seed from OS entropy source
        std::random_device rd;
        // get pseudorandom number generator from seed
        std::mt19937 gen(rd());
        // builds a weighted distribution based on probablities
        std::discrete_distribution<int> dist(probs.begin(), probs.end());

        int outcome = dist(gen);

        // collapse amplitudes array after measure
        for (int i = 0; i < amplitudes.size(); ++i)
        {
            amplitudes[i] = (i == outcome) ? Amplitude{1, 0} : Amplitude{0, 0};
        }

        return outcome;
    }

    bool is_normalized()
    {
        double sum =
            std::accumulate(this->amplitudes.begin(),
                            this->amplitudes.end(), 0.0,
                            [](double acc, const Amplitude &a)
                            { return acc + a.abs2(); });

        return std::abs(1 - sum) < 0.001;
    }
};