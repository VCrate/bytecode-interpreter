#include <bytec/Interpreter/Assembly.hpp>

#include <bytec/Interpreter/Operations.hpp>
#include <bytec/Interpreter/BinRepr.hpp>

namespace bytec { namespace assembly {

bool Argument::get_potential_next_12(ui32&) const {
    return false;
}

bool Argument::get_potential_next_24(ui32&) const {
    return false;
}

Value::Value(ui32 value) : value(value) {}

ui16 Value::as_12() const {
    if (value > bin_repr::arg12_value_max)
        return bin_repr::arg12_type_encode(0x07); // next value
    return bin_repr::arg12_type_encode(0x06) | bin_repr::arg12_value_encode(value); // Immediate value
}

ui32 Value::as_24() const {
    if (value > bin_repr::arg24_value_max)
        return bin_repr::arg24_type_encode(0x07); // next value
    return bin_repr::arg24_type_encode(0x06) | bin_repr::arg24_value_encode(value); // Immediate value
}

bool Value::get_potential_next_12(ui32& v) const {
    v = value;
    return value > bin_repr::arg12_value_max;
}

bool Value::get_potential_next_24(ui32& v) const {
    v = value;
    return value > bin_repr::arg24_value_max;
}

DeferValue::DeferValue(ui32 value) : Value(value) {}

ui16 DeferValue::as_12() const {
    if (value > bin_repr::arg12_value_max)
        return bin_repr::arg12_type_encode(0x05); // next value
    return bin_repr::arg12_type_encode(0x04) | bin_repr::arg12_value_encode(value); // Immediate value
}

ui32 DeferValue::as_24() const {
    if (value > bin_repr::arg24_value_max)
        return bin_repr::arg24_type_encode(0x05); // next value
    return bin_repr::arg24_type_encode(0x04) | bin_repr::arg24_value_encode(value); // Immediate value
}

const Register Register::A  = Register{ 0x0 };
const Register Register::B  = Register{ 0x1 };
const Register Register::C  = Register{ 0x2 };
const Register Register::D  = Register{ 0x3 };
const Register Register::E  = Register{ 0x4 };
const Register Register::F  = Register{ 0x5 };
const Register Register::G  = Register{ 0x6 };
const Register Register::H  = Register{ 0x7 };
const Register Register::SP = Register{ 0x8 };

Register::Register(ui8 reg) : reg(reg) {}

ui32 Register::as_24() const {
    return bin_repr::arg24_type_encode(0x00) | bin_repr::arg24_register_encode(reg);
}

ui16 Register::as_12() const {
    return bin_repr::arg12_type_encode(0x00) | bin_repr::arg12_register_encode(reg);
}

DeferRegister::DeferRegister(ui8 reg) : Register(reg) {}

ui32 DeferRegister::as_24() const {
    return bin_repr::arg24_type_encode(0x01) | bin_repr::arg24_register_encode(reg);
}

ui16 DeferRegister::as_12() const {
    return bin_repr::arg12_type_encode(0x01) | bin_repr::arg12_register_encode(reg);
}

DeferRegisterDisp::DeferRegisterDisp(ui8 reg, ui32 disp) : Register(reg), disp(disp) {}

ui32 DeferRegisterDisp::as_24() const {
    if (disp > bin_repr::arg24_disp_max)
        return bin_repr::arg24_type_encode(0x03) | bin_repr::arg24_register_encode(reg);
    return bin_repr::arg24_type_encode(0x02) | bin_repr::arg24_register_encode(reg) | bin_repr::arg24_disp_encode(disp / 4);
}

ui16 DeferRegisterDisp::as_12() const {
    if (disp > bin_repr::arg12_disp_max * 4 || (disp % 4) != 0)
        return bin_repr::arg12_type_encode(0x03) | bin_repr::arg12_register_encode(reg);
    return bin_repr::arg12_type_encode(0x02) | bin_repr::arg12_register_encode(reg) | bin_repr::arg12_disp_encode(disp / 4);
}

bool DeferRegisterDisp::get_potential_next_12(ui32& v) const {
    v = disp;
    return disp > bin_repr::arg12_disp_max * 4 || (disp % 4) != 0;
}

bool DeferRegisterDisp::get_potential_next_24(ui32& v) const {
    v = disp;
    return disp > bin_repr::arg24_disp_max;
}

void append_instruction_2_args(Program& program, Operations operation, Argument const& from, Argument const& to) {
    program.append_instruction(
        bin_repr::operation_encode(static_cast<ui8>(operation)) |
        bin_repr::arg0_encode(from.as_12()) |
        bin_repr::arg1_encode(to.as_12())
    );

    ui32 value;
    if (from.get_potential_next_12(value))
        program.append_instruction(value);
    if (to.get_potential_next_12(value))
        program.append_instruction(value);
}

void append_instruction_1_arg(Program& program, Operations operation, Argument const& target) {
    program.append_instruction(
        bin_repr::operation_encode(static_cast<ui8>(operation)) |
        bin_repr::arg_encode(target.as_24())
    );

    ui32 value;
    if (target.get_potential_next_24(value))
        program.append_instruction(value);
}

void append_ADD(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::ADD, from, to);
}

void append_SUB(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::SUB, from, to);
}

void append_MUL(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::MUL, from, to);
}

void append_MULU(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::MULU, from, to);
}

void append_DIV(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::DIV, from, to);
}

void append_DIVU(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::DIVU, from, to);
}

void append_MOV(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::MOV, from, to);
}

void append_SWP(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::SWP, from, to);
}

void append_PUSH(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::PUSH, target);
}

void append_POP(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::POP, target);
}

void append_JMP(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::JMP, target);
}

void append_JMPC(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::JMPC, from, to);
}

void append_AND(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::AND, from, to);
}

void append_OR(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::OR, from, to);
}

void append_XOR(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::XOR, from, to);
}

void append_NEG(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::NEG, target);
}

void append_SHL(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::SHL, from, to);
}

void append_RTL(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::RTL, from, to);
}

void append_SHR(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::SHR, from, to);
}

void append_RTR(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::RTR, from, to);
}

void append_CMPE(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::CMPE, from, to);
}

void append_CMPNE(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::CMPNE, from, to);
}

void append_CMPG(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::CMPG, from, to);
}

void append_CMPGE(Program& program, Argument const& from, Argument const& to) {
    append_instruction_2_args(program, Operations::CMPGE, from, to);
}

void append_INC(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::INC, target);
}

void append_DEC(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::DEC, target);
}

void append_OUT(Program& program, Argument const& target) {
    append_instruction_1_arg(program, Operations::OUT, target);
}


}}