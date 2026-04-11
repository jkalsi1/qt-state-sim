#include <iostream>
#include <chrono>
#include <cmath>
#include <iomanip>
#include "grover.h"

using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::duration<double, std::milli>;

struct BenchmarkResult
{
    int num_qubits;
    int num_states;
    int num_iterations;
    double avg_ms;
    double min_ms;
    double max_ms;
    int correct;
    int trials;
};

BenchmarkResult benchmark_qubit_count(int num_qubits, int trials = 5)
{
    int num_states = 1 << num_qubits;
    int num_iterations = (int)std::round(M_PI / 4.0 * std::sqrt(num_states));
    int marked = num_states / 2; // pick a fixed middle target

    double total_ms = 0;
    double min_ms = std::numeric_limits<double>::max();
    double max_ms = 0;
    int correct = 0;

    for (int t = 0; t < trials; t++)
    {
        auto start = Clock::now();
        int result = grover(marked, num_qubits);
        auto end = Clock::now();

        double ms = Ms(end - start).count();
        total_ms += ms;
        min_ms = std::min(min_ms, ms);
        max_ms = std::max(max_ms, ms);
        if (result == marked)
            correct++;
    }

    return {
        num_qubits,
        num_states,
        num_iterations,
        total_ms / trials,
        min_ms,
        max_ms,
        correct,
        trials};
}

void print_header()
{
    std::cout << std::left
              << std::setw(8) << "Qubits"
              << std::setw(10) << "States"
              << std::setw(10) << "Iters"
              << std::setw(12) << "Avg (ms)"
              << std::setw(12) << "Min (ms)"
              << std::setw(12) << "Max (ms)"
              << std::setw(12) << "Accuracy"
              << "\n";
    std::cout << std::string(76, '-') << "\n";
}

void print_result(const BenchmarkResult &r)
{
    std::cout << std::left << std::fixed << std::setprecision(2)
              << std::setw(8) << r.num_qubits
              << std::setw(10) << r.num_states
              << std::setw(10) << r.num_iterations
              << std::setw(12) << r.avg_ms
              << std::setw(12) << r.min_ms
              << std::setw(12) << r.max_ms
              << r.correct << "/" << r.trials
              << "\n";
}

void run_correctness_check(int num_qubits)
{
    int num_states = 1 << num_qubits;
    int correct = 0;

    std::cout << "\nCorrectness check — " << num_qubits
              << " qubits, all " << num_states << " targets:\n";

    for (int marked = 0; marked < num_states; marked++)
    {
        int result = grover(marked, num_qubits);
        if (result == marked)
        {
            correct++;
        }
        else
        {
            std::cout << "  MISS: marked=" << marked
                      << " found=" << result << "\n";
        }
    }

    std::cout << "  " << correct << "/" << num_states << " correct ("
              << std::fixed << std::setprecision(1)
              << (100.0 * correct / num_states) << "%)\n";
}

void verify_oracle(int marked, int num_qubits)
{
    // run direct oracle
    QuantumCircuit direct(num_qubits);
    for (int q = 0; q < num_qubits; q++)
        direct.h(q);
    direct.run_and_reset();
    apply_oracle_direct(direct, marked, num_qubits);

    // run gate oracle
    QuantumCircuit gate(num_qubits);
    for (int q = 0; q < num_qubits; q++)
        gate.h(q);
    gate.run_and_reset();
    apply_oracle(gate, marked, num_qubits);
    gate.run_and_reset();

    // compare
    std::cout << "Oracle verification — marked=" << marked << "\n";
    bool match = true;
    for (int i = 0; i < (1 << num_qubits); i++)
    {
        Amplitude d = direct.get_amplitude(i);
        Amplitude g = gate.get_amplitude(i);
        bool same = std::abs(d.real - g.real) < 0.001 &&
                    std::abs(d.imag - g.imag) < 0.001;
        if (!same)
        {
            std::cout << "  MISMATCH at |" << i << ">: "
                      << "direct=(" << d.real << "," << d.imag << ") "
                      << "gate=(" << g.real << "," << g.imag << ")\n";
            match = false;
        }
    }
    if (match)
        std::cout << "  all amplitudes match\n";
}

void verify_diffusion(int num_qubits)
{
    // apply oracle to marked=0 first to give both a non-uniform state
    QuantumCircuit direct(num_qubits);
    for (int q = 0; q < num_qubits; q++)
        direct.h(q);
    direct.run_and_reset();
    apply_oracle_direct(direct, 0, num_qubits);
    apply_diffusion_direct(direct, num_qubits);

    QuantumCircuit gate(num_qubits);
    for (int q = 0; q < num_qubits; q++)
        gate.h(q);
    gate.run_and_reset();
    apply_oracle_direct(gate, 0, num_qubits); // use direct oracle so only diffusion differs
    apply_diffusion(gate, num_qubits);
    gate.run_and_reset();

    std::cout << "Diffusion verification\n";
    bool match = true;
    for (int i = 0; i < (1 << num_qubits); i++)
    {
        Amplitude d = direct.get_amplitude(i);
        Amplitude g = gate.get_amplitude(i);
        bool same = std::abs(d.real - g.real) < 0.001 &&
                    std::abs(d.imag - g.imag) < 0.001;
        if (!same)
        {
            std::cout << "  MISMATCH at |" << i << ">: "
                      << "direct=(" << d.real << "," << d.imag << ") "
                      << "gate=(" << g.real << "," << g.imag << ")\n";
            match = false;
        }
    }
    if (match)
        std::cout << "  all amplitudes match\n";
}

void verify_toffoli(int num_qubits)
{
    // test 1: |111> should get phase flipped to -|111>
    {
        QuantumCircuit c(num_qubits);
        c.x(0);
        c.x(1);
        c.x(2);
        c.run_and_reset();

        Amplitude before = c.get_amplitude(7);
        std::cout << "CCZ test 1 — |111> before: (" << before.real << "," << before.imag << ")\n";

        apply_ccz(c);
        c.run_and_reset();

        Amplitude after = c.get_amplitude(7);
        std::cout << "CCZ test 1 — |111> after:  (" << after.real << "," << after.imag << ")\n";
        bool correct = std::abs(after.real - (-1.0)) < 0.001 && std::abs(after.imag) < 0.001;
        std::cout << (correct ? "  [PASS] amplitude negated\n" : "  [FAIL] amplitude not negated\n");
    }

    // test 2: |110> should be unchanged
    {
        QuantumCircuit c(num_qubits);
        c.x(1);
        c.x(2);
        c.run_and_reset();

        apply_ccz(c);
        c.run_and_reset();

        Amplitude after = c.get_amplitude(6);
        std::cout << "CCZ test 2 — |110> after:  (" << after.real << "," << after.imag << ")\n";
        bool correct = std::abs(after.real - 1.0) < 0.001 && std::abs(after.imag) < 0.001;
        std::cout << (correct ? "  [PASS] amplitude unchanged\n" : "  [FAIL] amplitude incorrectly modified\n");
    }

    // test 3: |101> should be unchanged
    {
        QuantumCircuit c(num_qubits);
        c.x(0);
        c.x(2);
        c.run_and_reset();

        apply_ccz(c);
        c.run_and_reset();

        Amplitude after = c.get_amplitude(5);
        std::cout << "CCZ test 3 — |101> after:  (" << after.real << "," << after.imag << ")\n";
        bool correct = std::abs(after.real - 1.0) < 0.001 && std::abs(after.imag) < 0.001;
        std::cout << (correct ? "  [PASS] amplitude unchanged\n" : "  [FAIL] amplitude incorrectly modified\n");
    }

    // test 4: |011> should be unchanged — all single-control-missing cases
    {
        QuantumCircuit c(num_qubits);
        c.x(0);
        c.x(1);
        c.run_and_reset();

        apply_ccz(c);
        c.run_and_reset();

        Amplitude after = c.get_amplitude(3);
        std::cout << "CCZ test 4 — |011> after:  (" << after.real << "," << after.imag << ")\n";
        bool correct = std::abs(after.real - 1.0) < 0.001 && std::abs(after.imag) < 0.001;
        std::cout << (correct ? "  [PASS] amplitude unchanged\n" : "  [FAIL] amplitude incorrectly modified\n");
    }
}

int main()
{
    std::cout << "=== Grover benchmark ===\n\n";

    std::cout << "=== Toffoli verification ===\n\n";
    verify_toffoli(3);

    std::cout << "=== Gate decomposition verification ===\n\n";
    verify_oracle(0, 3);
    verify_oracle(3, 3);
    verify_oracle(7, 3);
    verify_diffusion(3);

    // correctness check on small systems first
    run_correctness_check(3);
    run_correctness_check(4);

    // performance benchmark across qubit counts
    std::cout << "\n=== Performance ===\n\n";
    print_header();

    // stop at 20 — beyond that expect minutes+ per run
    for (int q = 3; q <= 20; q++)
    {
        // reduce trials at high qubit counts to keep runtime reasonable
        int trials = q <= 12 ? 10 : q <= 16 ? 5
                                            : 2;
        BenchmarkResult r = benchmark_qubit_count(q, trials);
        print_result(r);

        // warn if getting slow
        if (r.avg_ms > 10000)
        {
            std::cout << "  (avg > 10s, stopping early)\n";
            break;
        }
    }

    // show exponential growth explicitly
    std::cout << "\n=== State space growth ===\n\n";
    std::cout << std::left
              << std::setw(8) << "Qubits"
              << std::setw(14) << "States (2^n)"
              << std::setw(10) << "Iters"
              << "\n";
    std::cout << std::string(32, '-') << "\n";
    for (int q = 3; q <= 20; q++)
    {
        std::cout << std::setw(8) << q
                  << std::setw(14) << (1 << q)
                  << std::setw(10) << (int)std::round(M_PI / 4.0 * std::sqrt(1 << q))
                  << "\n";
    }

    return 0;
}