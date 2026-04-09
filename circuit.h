#pragma once
#include <vector>
#include <variant>
#include "gates.h"

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

public:
    void h(int target)
    {
        Operation op;
        op.type = OperationType::SingleGate;
        op.args = {state, Gate::H, target};
        operations.push_back(op);
    }
};