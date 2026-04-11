#pragma once
#include <array>
#include "state.h"
#include <cmath>
#include <complex>

using Matrix2x2 = std::array<std::array<Amplitude, 2>, 2>;

using namespace std::complex_literals;

// Standard single-qubit gates
namespace Gate
{
    const double INV_SQRT2 = 1.0 / std::sqrt(2.0);

    const Matrix2x2 X = {{

        {{Amplitude{0, 0}, Amplitude{1, 0}}},
        {{Amplitude{1, 0}, Amplitude{0, 0}}}

    }};

    const Matrix2x2 Y = {{

        {{Amplitude{0, 0}, Amplitude{0, -1}}},
        {{Amplitude{0, 1}, Amplitude{0, 0}}}

    }};

    const Matrix2x2 Z = {{

        {{Amplitude{1, 0}, Amplitude{0, 0}}},
        {{Amplitude{0, 0}, Amplitude{-1, 0}}}

    }};

    const Matrix2x2 H = {{

        {{Amplitude{INV_SQRT2, 0}, Amplitude{INV_SQRT2, 0}}},
        {{Amplitude{INV_SQRT2, 0}, Amplitude{-INV_SQRT2, 0}}}

    }};

}

// Apply a single-qubit gate to qubit `target` in an n-qubit state
void apply_gate(QuantumState &state, const Matrix2x2 &gate, int target)
{
    int n = state.num_qubits;
    if (target < 0 || target >= n)
        throw std::out_of_range("Target qubit out of range");

    int target_qubit_index = 1 << target; // distance between paired basis states

    for (int i = 0; i < state.amplitudes.size(); ++i)
    {
        // Only process pairs where the target bit is 0
        if (i & target_qubit_index)
            continue;

        int j = i | target_qubit_index; // partner with target bit = 1
        Amplitude a = state.amplitudes[i];
        Amplitude b = state.amplitudes[j];

        state.amplitudes[i] = gate[0][0] * a + gate[0][1] * b;
        state.amplitudes[j] = gate[1][0] * a + gate[1][1] * b;
    }
}

void apply_cnot(QuantumState &state, int control, int target)
{
    if (control == target)
    {
        throw std::invalid_argument("Control qubit must not be the equal to target qubit");
    }

    int n = state.num_qubits;
    if (target < 0 || target >= n)
        throw std::out_of_range("Target qubit out of range");
    if (control < 0 || control >= n)
        throw std::out_of_range("Control qubit out of range");

    int control_qubit_index = 1 << control;
    int target_qubit_index = 1 << target;

    for (int i = 0; i < state.amplitudes.size(); i++)
    {
        if (!(i & control_qubit_index))
            continue;
        if (i & target_qubit_index)
            continue;
        // set j = to i (current bits set) OR'd with target qubit index (1 << target)
        int j = i | target_qubit_index;

        // 'flip' the bit, by swapping the amplitude of its opposite
        std::swap(state.amplitudes[i], state.amplitudes[j]);
    }
}

void apply_phase(QuantumState &state, double theta, int target)
{
    int n = state.num_qubits;
    if (target < 0 || target >= n)
        throw std::out_of_range("Target qubit out of range");

    int target_qubit_index = 1 << target;

    Amplitude phase_factor{std::cos(theta), std::sin(theta)};

    for (int i = 0; i < state.amplitudes.size(); i++)
    {
        if (!(i & target_qubit_index))
            continue;

        state.amplitudes[i] = state.amplitudes[i].operator*(phase_factor);
    }
}