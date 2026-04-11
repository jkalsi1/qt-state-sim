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

using OperationArg = std::variant<int, double, Matrix2x2, QuantumState>;

struct Operation
{
    OperationType type;
    std::vector<OperationArg> args;
};

class QuantumCircuit
{
    QuantumState state;
    std::vector<Operation> operations;

private:
    void add_single_gate(int target, Matrix2x2 gate)
    {
        Operation op;
        op.type = OperationType::SingleGate;
        op.args = {gate, target};
        operations.push_back(op);
    }

public:
    explicit QuantumCircuit(int n) : state(n) {};

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

    int measure()
    {
        return this->state.measure();
    }

    void run()
    {
        for (Operation op : this->operations)
        {
            try
            {
                switch (op.type)
                {
                case OperationType::SingleGate:
                    apply_gate(this->state, std::get<Matrix2x2>(op.args[0]), std::get<int>(op.args[1]));
                    break;
                case OperationType::CNOT:
                    apply_cnot(this->state, std::get<int>(op.args[0]), std::get<int>(op.args[1]));
                    break;
                case OperationType::PHASE:
                    apply_phase(this->state, std::get<double>(op.args[0]), std::get<int>(op.args[1]));
                    break;
                }
            }
            catch (const std::bad_variant_access &e)
            {
                std::cout << "Error: bad argument type in operation ";
                switch (op.type)
                {
                case OperationType::SingleGate:
                    std::cout << "SingleGate";
                    break;
                case OperationType::CNOT:
                    std::cout << "CNOT";
                    break;
                case OperationType::PHASE:
                    std::cout << "PHASE";
                    break;
                }
                std::cout << " — " << e.what() << "\n";
            }
        }
    }

    void print_circuit()
    {
        for (Operation op : this->operations)
        {
            switch (op.type)
            {
            case OperationType::SingleGate:
                std::cout << "Gate    qubit=" << std::get<int>(op.args[1]) << "\n";
                break;
            case OperationType::CNOT:
                std::cout << "CNOT    control=" << std::get<int>(op.args[0])
                          << " target=" << std::get<int>(op.args[1]) << "\n";
                break;
            case OperationType::PHASE:
                std::cout << "PHASE   theta=" << std::get<double>(op.args[0])
                          << " qubit=" << std::get<int>(op.args[1]) << "\n";
                break;
            }
        }
    }
};