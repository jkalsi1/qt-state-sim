#include <iostream>
#include <cmath>
#include "state.h"
#include "gates.h"

// ---- helpers ----

bool approx_eq(double a, double b, double eps = 0.001)
{
    return std::abs(a - b) < eps;
}

bool approx_eq_complex(Amplitude a, Amplitude b, double eps = 0.001)
{
    return approx_eq(a.real, b.real, eps) && approx_eq(a.imag, b.imag, eps);
}

void pass(const std::string &name) { std::cout << "\033[32m[PASS]\033[0m " << name << "\n"; }
void fail(const std::string &name) { std::cout << "\033[31m[FAIL]\033[0m " << name << "\n"; }

void check(bool result, const std::string &name)
{
    result ? pass(name) : fail(name);
}

// ---- normalization ----

void test_initial_state_normalized()
{
    QuantumState q(3);
    check(q.is_normalized(), "initial state is normalized");
}

void test_after_hadamard_normalized()
{
    QuantumState q(2);
    apply_gate(q, Gate::H, 0);
    check(q.is_normalized(), "state normalized after H");
}

void test_after_cnot_normalized()
{
    QuantumState q(2);
    apply_gate(q, Gate::H, 0);
    apply_cnot(q, 0, 1);
    check(q.is_normalized(), "state normalized after CNOT");
}

// ---- initial state ----

void test_initial_state_is_zero()
{
    QuantumState q(2);
    check(approx_eq(q.probability(0), 1.0), "initial state has prob 1 at |00>");
    check(approx_eq(q.probability(1), 0.0), "initial state has prob 0 at |01>");
    check(approx_eq(q.probability(2), 0.0), "initial state has prob 0 at |10>");
    check(approx_eq(q.probability(3), 0.0), "initial state has prob 0 at |11>");
}

// ---- pauli X ----

void test_x_flips_zero_to_one()
{
    QuantumState q(1);
    apply_gate(q, Gate::X, 0);
    check(approx_eq(q.probability(0), 0.0), "X: prob 0 at |0>");
    check(approx_eq(q.probability(1), 1.0), "X: prob 1 at |1>");
}

void test_x_flips_one_to_zero()
{
    QuantumState q(1);
    apply_gate(q, Gate::X, 0); // put into |1>
    apply_gate(q, Gate::X, 0); // flip back
    check(approx_eq(q.probability(0), 1.0), "X applied twice returns to |0>");
}

void test_x_only_affects_target_qubit()
{
    QuantumState q(2);
    apply_gate(q, Gate::X, 0); // flip qubit 0 only
    check(approx_eq(q.probability(0), 0.0), "X on q0: |00> = 0");
    check(approx_eq(q.probability(1), 1.0), "X on q0: |01> = 1");
    check(approx_eq(q.probability(2), 0.0), "X on q0: |10> = 0");
    check(approx_eq(q.probability(3), 0.0), "X on q0: |11> = 0");
}

// ---- pauli Z ----

void test_z_leaves_zero_unchanged()
{
    QuantumState q(1);
    apply_gate(q, Gate::Z, 0);
    check(approx_eq(q.probability(0), 1.0), "Z on |0>: probability unchanged");
    check(approx_eq_complex(q.amplitudes[0], Amplitude{1, 0}), "Z on |0>: amplitude unchanged");
}

void test_z_flips_phase_of_one()
{
    QuantumState q(1);
    apply_gate(q, Gate::X, 0); // put into |1>
    apply_gate(q, Gate::Z, 0);
    check(approx_eq_complex(q.amplitudes[1], Amplitude{-1, 0}), "Z on |1>: amplitude is -1");
    check(approx_eq(q.probability(1), 1.0), "Z on |1>: probability unchanged");
}

// ---- hadamard ----

void test_h_creates_superposition()
{
    QuantumState q(1);
    apply_gate(q, Gate::H, 0);
    check(approx_eq(q.probability(0), 0.5), "H on |0>: prob 0.5 at |0>");
    check(approx_eq(q.probability(1), 0.5), "H on |0>: prob 0.5 at |1>");
}

void test_h_applied_twice_returns_to_zero()
{
    QuantumState q(1);
    apply_gate(q, Gate::H, 0);
    apply_gate(q, Gate::H, 0);
    check(approx_eq(q.probability(0), 1.0), "H applied twice returns to |0>");
    check(approx_eq(q.probability(1), 0.0), "H applied twice: prob 0 at |1>");
}

// ---- cnot ----

void test_cnot_no_flip_when_control_zero()
{
    QuantumState q(2);   // |00>
    apply_cnot(q, 0, 1); // control=0 is |0>, so nothing happens
    check(approx_eq(q.probability(0), 1.0), "CNOT: no flip when control is |0>");
}

void test_cnot_flips_target_when_control_one()
{
    QuantumState q(2);
    apply_gate(q, Gate::X, 0); // set qubit 0 to |1>, state is now |01>
    apply_cnot(q, 0, 1);       // control=q0=1, so flip q1 → |11>
    check(approx_eq(q.probability(3), 1.0), "CNOT: flips target when control is |1>");
}

void test_cnot_produces_bell_state()
{
    // H on q0, then CNOT(q0, q1) should produce (|00> + |11>) / sqrt(2)
    QuantumState q(2);
    apply_gate(q, Gate::H, 0);
    apply_cnot(q, 0, 1);
    check(approx_eq(q.probability(0), 0.5), "Bell state: prob 0.5 at |00>");
    check(approx_eq(q.probability(1), 0.0), "Bell state: prob 0   at |01>");
    check(approx_eq(q.probability(2), 0.0), "Bell state: prob 0   at |10>");
    check(approx_eq(q.probability(3), 0.5), "Bell state: prob 0.5 at |11>");
}

void test_cnot_applied_twice_returns_to_original()
{
    QuantumState q(2);
    apply_gate(q, Gate::X, 0);
    apply_cnot(q, 0, 1);
    apply_cnot(q, 0, 1);
    check(approx_eq(q.probability(1), 1.0), "CNOT applied twice returns to original");
}

// ---- measurement ----

void test_measure_returns_valid_index()
{
    QuantumState q(2);
    apply_gate(q, Gate::H, 0);
    apply_gate(q, Gate::H, 1);
    int outcome = q.measure();
    check(outcome >= 0 && outcome < 4, "measure returns valid index");
}

void test_measure_collapses_state()
{
    QuantumState q(1);
    apply_gate(q, Gate::H, 0);
    int outcome = q.measure();
    check(approx_eq(q.probability(outcome), 1.0), "measure collapses to outcome");
    check(approx_eq(q.probability(1 - outcome), 0.0), "measure zeros other amplitude");
}

void test_measure_definite_state()
{
    QuantumState q(1); // definitely |0>
    int outcome = q.measure();
    check(outcome == 0, "measure of |0> always returns 0");
}

void test_phase_leaves_zero_unchanged()
{
    QuantumState q(1);
    apply_phase(q, M_PI / 2, 0);
    check(approx_eq_complex(q.amplitudes[0], Amplitude{1, 0}), "P(pi/2): |0> amplitude unchanged");
}

void test_phase_pi_equals_z_gate()
{
    QuantumState q1(1);
    QuantumState q2(1);
    apply_gate(q1, Gate::X, 0);
    apply_gate(q2, Gate::X, 0);
    apply_gate(q1, Gate::Z, 0);
    apply_phase(q2, M_PI, 0);
    check(approx_eq_complex(q1.amplitudes[1], q2.amplitudes[1]), "P(pi) == Z on |1>");
}

void test_phase_pi_over_2_is_s_gate()
{
    QuantumState q(1);
    apply_gate(q, Gate::X, 0);
    apply_phase(q, M_PI / 2, 0);
    check(approx_eq_complex(q.amplitudes[1], Amplitude{0, 1}), "P(pi/2): |1> amplitude is i");
}

void test_phase_does_not_change_probabilities()
{
    QuantumState q(1);
    apply_gate(q, Gate::H, 0);
    apply_phase(q, M_PI / 3, 0);
    check(approx_eq(q.probability(0), 0.5), "P(pi/3) after H: prob 0.5 at |0>");
    check(approx_eq(q.probability(1), 0.5), "P(pi/3) after H: prob 0.5 at |1>");
}

void test_phase_zero_is_identity()
{
    QuantumState q(1);
    apply_gate(q, Gate::H, 0);
    Amplitude before_0 = q.amplitudes[0];
    Amplitude before_1 = q.amplitudes[1];
    apply_phase(q, 0.0, 0);
    check(approx_eq_complex(q.amplitudes[0], before_0), "P(0): state unchanged");
    check(approx_eq_complex(q.amplitudes[1], before_1), "P(0): state unchanged");
}

// ---- run all ----

int main()
{
    test_initial_state_normalized();
    test_after_hadamard_normalized();
    test_after_cnot_normalized();

    test_initial_state_is_zero();

    test_x_flips_zero_to_one();
    test_x_flips_one_to_zero();
    test_x_only_affects_target_qubit();

    test_z_leaves_zero_unchanged();
    test_z_flips_phase_of_one();

    test_h_creates_superposition();
    test_h_applied_twice_returns_to_zero();

    test_cnot_no_flip_when_control_zero();
    test_cnot_flips_target_when_control_one();
    test_cnot_produces_bell_state();
    test_cnot_applied_twice_returns_to_original();

    test_measure_returns_valid_index();
    test_measure_collapses_state();
    test_measure_definite_state();

    test_phase_leaves_zero_unchanged();
    test_phase_pi_equals_z_gate();
    test_phase_pi_over_2_is_s_gate();
    test_phase_does_not_change_probabilities();
    test_phase_zero_is_identity();

    return 0;
}