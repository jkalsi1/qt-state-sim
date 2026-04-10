#pragma once
#include <vector>
#include <variant>
#include "gates.h"
#include <iostream>

enum class OperationType
{
    SingleGate,
    CNOT,
    PHASE
};

struct Operation
{
    OperationType type;
    std::vector<std::variant<int, double, Matrix2x2, QuantumState>> args;
};

class QuantumCircuit
{
    QuantumState state;
    std::vector<Operation> operations;

    explicit QuantumCircuit(int n) : state(n) {};

private:
    void add_single_gate(int target, Matrix2x2 gate)
    {
        Operation op;
        op.type = OperationType::SingleGate;
        op.args = {gate, target};
        operations.push_back(op);
    }

public:
    void h(int target)
    {
        this->add_single_gate(target, Gate::H);
    }

    void x(int target)
    {
        this->add_single_gate(target, Gate::X);
    }

    void y(int target)
    {
        this->add_single_gate(target, Gate::Y);
    }

    void z(int target)
    {
        this->add_single_gate(target, Gate::Z);
    }

    void cnot(int control, int target)
    {
        Operation op;
        op.type = OperationType::CNOT;
        op.args = {control, target};
        operations.push_back(op);
    }

    void phase(double theta, int target)
    {
        Operation op;
        op.type = OperationType::PHASE;
        op.args = {theta, target};
        operations.push_back(op);
    }

    void print_circuit()
    {
        for (Operation op : this->operations)
        {
            std::cout << static_cast<int>(op.type);
        }
    }
};