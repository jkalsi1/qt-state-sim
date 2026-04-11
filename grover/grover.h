#pragma once
#include "../circuit.h"
#include <cmath>
#include <iostream>

void apply_ccz(QuantumCircuit &c)
{
    c.phase(M_PI / 4, 0);
    c.phase(M_PI / 4, 1);
    c.phase(M_PI / 4, 2);
    c.cnot(0, 1);
    c.phase(-M_PI / 4, 1);
    c.cnot(0, 1);
    c.cnot(1, 2);
    c.phase(-M_PI / 4, 2);
    c.cnot(0, 2);
    c.phase(M_PI / 4, 2);
    c.cnot(1, 2);
    c.phase(-M_PI / 4, 2);
    c.cnot(0, 2);
}

// phase oracle: flips the sign of the marked state's amplitude
// implemented as a multi-controlled Z — flips phase when all
// qubits match the marked state's bit pattern
void apply_oracle(QuantumCircuit &c, int marked, int num_qubits)
{
    // flip any qubit that is 0 in the marked state
    // this transforms the marked state into |111...1>
    // so a standard multi-controlled Z can target it
    for (int q = 0; q < num_qubits; q++)
    {
        if (!((marked >> q) & 1))
        {
            c.x(q);
        }
    }

    apply_ccz(c);

    // undo the bit flips from before
    for (int q = 0; q < num_qubits; q++)
    {
        if (!((marked >> q) & 1))
        {
            c.x(q);
        }
    }
}

// diffusion operator: reflects amplitudes around their average
// implemented as H⊗n · (2|0><0| - I) · H⊗n
void apply_diffusion(QuantumCircuit &c, int num_qubits)
{
    // step 1: hadamard all qubits
    for (int q = 0; q < num_qubits; q++)
        c.h(q);

    // step 2: flip all qubits (transforms |0> to |1...1>)
    for (int q = 0; q < num_qubits; q++)
        c.x(q);

    // step 3: apply same multi-controlled Z as oracle but targeting |111>
    apply_ccz(c);

    // step 4: undo flips
    for (int q = 0; q < num_qubits; q++)
        c.x(q);

    // step 5: hadamard all qubits again
    for (int q = 0; q < num_qubits; q++)
        c.h(q);
}

void apply_ccz(QuantumCircuit &c)
{
    c.phase(M_PI / 4, 0);
    c.phase(M_PI / 4, 1);
    c.phase(M_PI / 4, 2);
    c.cnot(0, 1);
    c.phase(-M_PI / 4, 1);
    c.cnot(0, 1);
    c.cnot(1, 2);
    c.phase(-M_PI / 4, 2);
    c.cnot(0, 2);
    c.phase(M_PI / 4, 2);
    c.cnot(1, 2);
    c.phase(-M_PI / 4, 2);
    c.cnot(0, 2);
}

int grover(int marked, int num_qubits = 3)
{
    int num_iterations = (int)std::round(M_PI / 4.0 * std::sqrt(1 << num_qubits));

    QuantumCircuit c(num_qubits);

    // initial uniform superposition
    for (int q = 0; q < num_qubits; q++)
        c.h(q);
    c.run_and_reset();

    for (int i = 0; i < num_iterations; i++)
    {
        apply_oracle(c, marked, num_qubits);
        apply_diffusion(c, num_qubits);
    }

    return c.measure();
}

void run_grover()
{
    int num_qubits = 3;

    for (int marked = 0; marked < (1 << num_qubits); marked++)
    {
        int result = grover(marked, num_qubits);
        std::cout << "Marked: " << marked
                  << "  Found: " << result
                  << (result == marked ? "  [CORRECT]" : "  [WRONG]")
                  << "\n";
    }
}
