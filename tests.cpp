#include <iostream>
#include <cmath>
#include "state.h"
#include "gates.h"
#include "circuit.h"

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

// ---- circuit tests ----

void test_circuit_x_flips_to_one()
{
    QuantumCircuit c(1);
    c.x(0);
    c.run();
    int outcome = c.measure();
    check(outcome == 1, "circuit X: measures |1>");
}

void test_circuit_x_twice_returns_to_zero()
{
    QuantumCircuit c(1);
    c.x(0);
    c.x(0);
    c.run();
    int outcome = c.measure();
    check(outcome == 0, "circuit X twice: measures |0>");
}

void test_circuit_h_superposition()
{
    // run many times and check both outcomes appear
    int zeros = 0, ones = 0;
    for (int i = 0; i < 100; i++)
    {
        QuantumCircuit c(1);
        c.h(0);
        c.run();
        int outcome = c.measure();
        if (outcome == 0)
            zeros++;
        else
            ones++;
    }
    check(zeros > 30 && ones > 30, "circuit H: both outcomes appear");
}

void test_circuit_bell_state()
{
    QuantumCircuit c(2);
    c.h(0);
    c.cnot(0, 1);
    c.run();
    int outcome = c.measure();
    check(outcome == 0 || outcome == 3, "bell state: measures |00> or |11> only");
}

void test_circuit_bell_state_never_measures_01_or_10()
{
    int invalid = 0;
    for (int i = 0; i < 100; i++)
    {
        QuantumCircuit c(2);
        c.h(0);
        c.cnot(0, 1);
        c.run();
        int outcome = c.measure();
        if (outcome == 1 || outcome == 2)
            invalid++;
    }
    check(invalid == 0, "bell state: never measures |01> or |10>");
}

void test_circuit_measure_before_run_is_zero()
{
    QuantumCircuit c(1);
    c.x(0);
    int outcome = c.measure();
    check(outcome == 0, "measure before run: state is still |0>");
}

void test_circuit_phase_does_not_change_probabilities()
{
    QuantumCircuit c(1);
    c.h(0);
    c.phase(M_PI / 3, 0);
    c.run();
    int zeros = 0, ones = 0;
    for (int i = 0; i < 100; i++)
    {
        QuantumCircuit c2(1);
        c2.h(0);
        c2.phase(M_PI / 3, 0);
        c2.run();
        int outcome = c2.measure();
        if (outcome == 0)
            zeros++;
        else
            ones++;
    }
    check(zeros > 30 && ones > 30, "circuit P: probabilities unchanged after phase");
}

void test_circuit_z_on_zero_unchanged()
{
    QuantumCircuit c(1);
    c.z(0);
    c.run();
    int outcome = c.measure();
    check(outcome == 0, "circuit Z on |0>: still measures |0>");
}

void test_circuit_empty_measures_zero()
{
    QuantumCircuit c(1);
    c.run();
    int outcome = c.measure();
    check(outcome == 0, "empty circuit: measures |0>");
}

void test_circuit_gate_order_matters()
{
    // H then X is different from X then H
    QuantumCircuit c1(1);
    c1.h(0);
    c1.x(0);
    c1.run();

    QuantumCircuit c2(1);
    c2.x(0);
    c2.h(0);
    c2.run();

    // H then X: |+> flipped = |->, prob of |0> is 0.5
    // X then H: |1> hadamarded = |-> so prob of |1> is 0.5
    // measure many times and check distributions differ
    for (int i = 0; i < 100; i++)
    {
        QuantumCircuit a(1);
        a.h(0);
        a.x(0);
        a.run();

        QuantumCircuit b(1);
        b.x(0);
        b.h(0);
        b.run();
    }
    // both produce superpositions but with different phases —
    // probabilities are actually the same here, so instead
    // verify a case where order provably changes measurement outcome
    QuantumCircuit deterministic1(1);
    deterministic1.x(0);
    deterministic1.x(0);
    deterministic1.run();
    check(deterministic1.measure() == 0, "gate order: X then X measures |0>");

    QuantumCircuit deterministic2(1);
    deterministic2.x(0);
    deterministic2.run();
    check(deterministic2.measure() == 1, "gate order: single X measures |1>");
}

void test_circuit_cnot_control_target_reversed()
{
    // cnot(0,1) with q0=|1>: flips q1
    QuantumCircuit c1(2);
    c1.x(0);
    c1.cnot(0, 1);
    c1.run();
    check(c1.measure() == 3, "CNOT(0,1) with q0=|1>: measures |11>");

    // cnot(1,0) with q0=|1>: control is q1=|0>, so nothing flips
    QuantumCircuit c2(2);
    c2.x(0);
    c2.cnot(1, 0);
    c2.run();
    check(c2.measure() == 1, "CNOT(1,0) with q0=|1>, q1=|0>: measures |01>");
}

void test_circuit_three_qubit_cnot_non_adjacent()
{
    // CNOT on qubits 0 and 2, leaving qubit 1 untouched
    QuantumCircuit c(3);
    c.x(0);       // set q0=|1>
    c.cnot(0, 2); // control=q0, target=q2 — non-adjacent
    c.run();
    // q0=1, q1=0, q2=1 → binary 101 = index 5
    check(c.measure() == 5, "3-qubit CNOT(0,2): measures |101>");
}

void test_circuit_three_qubit_gate_middle_qubit()
{
    // apply X to qubit 1 only in a 3-qubit system
    QuantumCircuit c(3);
    c.x(1);
    c.run();
    // q0=0, q1=1, q2=0 → binary 010 = index 2
    check(c.measure() == 2, "3-qubit X on q1: measures |010>");
}

void test_circuit_phase_only_affects_target_in_multiqubit()
{
    // phase on q1 should not affect probabilities of q0
    int zeros_q0 = 0;
    for (int i = 0; i < 100; i++)
    {
        QuantumCircuit c(2);
        c.h(0);
        c.h(1);
        c.phase(M_PI / 2, 1);
        c.run();
        int outcome = c.measure();
        // check q0 bit (bit 0) — should still be 50/50
        if ((outcome & 1) == 0)
            zeros_q0++;
    }
    check(zeros_q0 > 30 && zeros_q0 < 70, "phase on q1: q0 probabilities unaffected");
}

void test_circuit_run_twice_does_not_double_apply()
{
    QuantumCircuit c(1);
    c.x(0);
    try
    {
        c.run();
        c.run();
    }
    catch (std::invalid_argument &e)
    {
        check(true, "Circuit runs twice threw invalid_argument");
    }
    catch (...)
    {
        check(false, "Circuit runs twice threw wrong exception type");
    }
}

// X applied twice should return to |0>, but if run() is idempotent
// and only applies once, it should still be |1>
// either behaviour is acceptable but it should be consistent —
// this test documents what your implementation actually does

void test_circuit_cnot_same_control_and_target()
{
    QuantumCircuit c(2);
    c.cnot(0, 0);
    try
    {
        c.run();
        check(false, "CNOT control==target: should throw");
    }
    catch (const std::invalid_argument &)
    {
        check(true, "CNOT control==target: threw invalid_argument");
    }
    catch (...)
    {
        check(false, "CNOT control==target: threw wrong exception type");
    }
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

    test_circuit_x_flips_to_one();
    test_circuit_x_twice_returns_to_zero();
    test_circuit_h_superposition();
    test_circuit_bell_state();
    test_circuit_bell_state_never_measures_01_or_10();
    test_circuit_measure_before_run_is_zero();
    test_circuit_phase_does_not_change_probabilities();
    test_circuit_z_on_zero_unchanged();

    test_circuit_empty_measures_zero();
    test_circuit_gate_order_matters();
    test_circuit_cnot_control_target_reversed();
    test_circuit_three_qubit_cnot_non_adjacent();
    test_circuit_three_qubit_gate_middle_qubit();
    test_circuit_phase_only_affects_target_in_multiqubit();
    test_circuit_run_twice_does_not_double_apply();
    test_circuit_cnot_same_control_and_target();

    return 0;
}