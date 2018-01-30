#include <bytec/Interpreter/Interpreter.hpp>

#include <iostream>
#include <bitset>

namespace bytec {

void Interpreter::run(SandBox& sandbox, Program const& program) {
    Decoder decoder(program.instruction_at(sandbox.get_pc_increment()));
    switch(decoder.get_operations()) {

#define DEF_OP_2_ARGS(ope, func)       \
        case Operations::ope:       \
            return Interpreter::manage(sandbox, program, &Interpreter::op_ ## func, decoder.get_half_arguments());

#define DEF_OP_2_ARGS_LOAD(ope, func)       \
        case Operations::ope:       \
            return Interpreter::manage(sandbox, program, &Interpreter::op_ ## func, decoder.get_half_arguments(), true);

#define DEF_OP_ARG(ope, func)        \
        case Operations::ope:       \
            return Interpreter::manage(sandbox, program, &Interpreter::op_ ## func, decoder.get_full_argument());

#define DEF_OP_ARG_LOAD(ope, func)        \
        case Operations::ope:       \
            return Interpreter::manage(sandbox, program, &Interpreter::op_ ## func, decoder.get_full_argument(), true);

        DEF_OP_2_ARGS_LOAD(ADD, add)
        DEF_OP_2_ARGS_LOAD(SUB, sub)
        DEF_OP_2_ARGS_LOAD(MUL, mul)
        DEF_OP_2_ARGS_LOAD(MULU, mulu)
        DEF_OP_2_ARGS_LOAD(DIV, div)
        DEF_OP_2_ARGS_LOAD(DIVU, divu)

        DEF_OP_2_ARGS(MOV, mov)
        DEF_OP_2_ARGS_LOAD(SWP, swp)
        DEF_OP_ARG(POP, pop)
        DEF_OP_ARG(PUSH, push)

        DEF_OP_ARG(JMP, jmp)
        DEF_OP_ARG(JMPE, jmpe)
        DEF_OP_ARG(JMPNE, jmpne)
        DEF_OP_ARG(JMPG, jmpg)
        DEF_OP_ARG(JMPGE, jmpge)

        DEF_OP_2_ARGS_LOAD(AND, and)
        DEF_OP_2_ARGS_LOAD(OR, or)
        DEF_OP_2_ARGS_LOAD(XOR, xor)
        DEF_OP_ARG_LOAD(NEG, neg)

        DEF_OP_2_ARGS_LOAD(SHL, shl)
        DEF_OP_2_ARGS_LOAD(RTL, rtl)
        DEF_OP_2_ARGS_LOAD(SHR, shr)
        DEF_OP_2_ARGS_LOAD(RTR, rtr)

        DEF_OP_2_ARGS(CMP, cmp)

        DEF_OP_ARG_LOAD(INC, inc)
        DEF_OP_ARG_LOAD(DEC, dec)

        DEF_OP_ARG(OUT, out)

#undef DEF_OP_2_ARGS
#undef DEF_OP_ARG

        default:
            throw std::runtime_error("Operations Unknown");
    }
}

void Interpreter::write(SandBox& sandbox, Program const& program, ui32 value, Decoder::Argument const& arg) {
    switch(arg.type) {
        case Decoder::ArgumentType::Register:
            return sandbox.set_register(static_cast<ui8>(arg.reg), value);
        case Decoder::ArgumentType::DeferRegister:
            return sandbox.set_memory_at(sandbox.get_register(static_cast<ui8>(arg.reg)), value);
        case Decoder::ArgumentType::DeferRegisterDisp:
            return sandbox.set_memory_at(sandbox.get_register(static_cast<ui8>(arg.reg)) + arg.disp, value);
        case Decoder::ArgumentType::DeferRegisterNextDisp:
            return sandbox.set_memory_at(sandbox.get_register(static_cast<ui8>(arg.reg)) + program.instruction_at(sandbox.get_pc_increment()), value);
        case Decoder::ArgumentType::DeferImmValue:
            return sandbox.set_memory_at(arg.value, value);
        case Decoder::ArgumentType::DeferNextValue:
            return sandbox.set_memory_at(program.instruction_at(sandbox.get_pc_increment()), value);

        default:
            throw std::runtime_error("Unknown or unwritable write target");
    }
}

ui32 Interpreter::read(SandBox& sandbox, Program const& program, Decoder::Argument const& arg) {
    switch(arg.type) {
        case Decoder::ArgumentType::Register:
            return sandbox.get_register(static_cast<ui8>(arg.reg));
        case Decoder::ArgumentType::DeferRegister:
            return sandbox.get_memory_at(sandbox.get_register(static_cast<ui8>(arg.reg)));
        case Decoder::ArgumentType::DeferRegisterDisp:
            return sandbox.get_memory_at(sandbox.get_register(static_cast<ui8>(arg.reg)) + arg.disp);
        case Decoder::ArgumentType::DeferRegisterNextDisp:
            return sandbox.get_memory_at(sandbox.get_register(static_cast<ui8>(arg.reg)) + program.instruction_at(sandbox.get_pc_increment()));
        case Decoder::ArgumentType::ImmValue:
            return arg.value;
        case Decoder::ArgumentType::NextValue:
            return program.instruction_at(sandbox.get_pc_increment());
        case Decoder::ArgumentType::DeferImmValue:
            return sandbox.get_memory_at(arg.value);
        case Decoder::ArgumentType::DeferNextValue:
            return sandbox.get_memory_at(program.instruction_at(sandbox.get_pc_increment()));

        default:
            throw std::runtime_error("Unknown or unwritable write target");
    }
}

void Interpreter::manage(SandBox& sandbox, Program const& program, void(*func)(SandBox&, ui32, ui32&), 
                         std::array<Decoder::Argument, 2> const& args, bool load_target) {
    ui32 operand = Interpreter::read(sandbox, program, args[0]);
    ui32 target;
    if (load_target)
        target = Interpreter::read(sandbox, program, args[1]);
    func(sandbox, operand, target);
    Interpreter::write(sandbox, program, target, args[1]);
}

void Interpreter::manage(SandBox& sandbox, Program const& program, void(*func)(SandBox&, ui32, ui32), 
                         std::array<Decoder::Argument, 2> const& args) {
    ui32 operand0 = Interpreter::read(sandbox, program, args[0]);
    ui32 operand1 = Interpreter::read(sandbox, program, args[1]);
    func(sandbox, operand0, operand1);
}

void Interpreter::manage(SandBox& sandbox, Program const& program, void(*func)(SandBox&, ui32&, ui32&), 
                         std::array<Decoder::Argument, 2> const& args, bool load_targets) {
    ui32 target0, target1;
    if (load_targets) {
        target0 = Interpreter::read(sandbox, program, args[0]);
        target1 = Interpreter::read(sandbox, program, args[1]);
    }
    func(sandbox, target0, target1);
    Interpreter::write(sandbox, program, target0, args[0]);
    Interpreter::write(sandbox, program, target1, args[1]);
}

void Interpreter::manage(SandBox& sandbox, Program const& program, void(*func)(SandBox&, ui32), 
                         Decoder::Argument const& arg) {
    ui32 operand = Interpreter::read(sandbox, program, arg);
    func(sandbox, operand);
}

void Interpreter::manage(SandBox& sandbox, Program const& program, void(*func)(SandBox&, ui32&),
                         Decoder::Argument const& arg, bool load_target) {
    ui32 target;
    if (load_target)
        target = Interpreter::read(sandbox, program, arg);
    func(sandbox, target);
    Interpreter::write(sandbox, program, target, arg);
}

void Interpreter::op_add (SandBox&, ui32 operand, ui32& target) {
    target += operand;
}

void Interpreter::op_sub (SandBox&, ui32 operand, ui32& target) {
    target -= operand;
}

void Interpreter::op_mul (SandBox&, ui32 operand, ui32& target) {
    target = static_cast<ui32>(static_cast<i32>(target) * static_cast<i32>(operand));
}

void Interpreter::op_mulu (SandBox&, ui32 operand, ui32& target) {
    target *= operand;
}

void Interpreter::op_div (SandBox&, ui32 operand, ui32& target) {
    // TODO
    // if(operand1 == 0)
    //      sandbox.exception(...);
    target = static_cast<ui32>(static_cast<i32>(target) / static_cast<i32>(operand));
}

void Interpreter::op_divu (SandBox&, ui32 operand, ui32& target) {
    // TODO
    // if(operand1 == 0)
    //      sandbox.exception(...);
    target /= operand;
}

void Interpreter::op_mov (SandBox&, ui32 operand, ui32& target) {
    target = operand;
}

void Interpreter::op_pop (SandBox& sandbox, ui32& target) {
    target = sandbox.pop_32();
}

void Interpreter::op_push (SandBox& sandbox, ui32 operand) {
    sandbox.push_32(operand);
}

void Interpreter::op_jmp (SandBox& sandbox, ui32 operand) {
    sandbox.set_pc(operand);
}

void Interpreter::op_jmpe (SandBox& sandbox, ui32 operand) {
    if (sandbox.get_flag_zero())
        sandbox.set_pc(operand);
}

void Interpreter::op_jmpne (SandBox& sandbox, ui32 operand) {
    if (!sandbox.get_flag_zero())
        sandbox.set_pc(operand);
}

void Interpreter::op_jmpg (SandBox& sandbox, ui32 operand) {
    if (sandbox.get_flag_greater())
        sandbox.set_pc(operand);
}

void Interpreter::op_jmpge (SandBox& sandbox, ui32 operand) {
    if (sandbox.get_flag_zero() && sandbox.get_flag_greater())
        sandbox.set_pc(operand);
}

void Interpreter::op_and (SandBox&, ui32 operand, ui32& target) {
    target &= operand;
}

void Interpreter::op_or (SandBox&, ui32 operand, ui32& target) {
    target |= operand;
}

void Interpreter::op_xor (SandBox&, ui32 operand, ui32& target) {
    target ^= operand;
}

void Interpreter::op_neg (SandBox&, ui32& target) {
    target = ~target;
}

void Interpreter::op_shl (SandBox&, ui32 operand, ui32& target) {
    target >>= operand;
}

void Interpreter::op_rtl (SandBox&, ui32 operand, ui32& target) {
    operand &= 31;
    target = (target << operand) | (target >> (32 - operand));
}

void Interpreter::op_shr (SandBox&, ui32 operand, ui32& target) {
    target <<= operand;
}

void Interpreter::op_rtr (SandBox&, ui32 operand, ui32& target) {
    operand &= 31;
    target = (target >> operand) | (target << (32 - operand));
}

void Interpreter::op_swp (SandBox&, ui32& target0, ui32& target1) {
    std::swap(target0, target1);
}

void Interpreter::op_cmp (SandBox& sandbox, ui32 operand0, ui32 operand1) {
    sandbox.set_flag_zero(operand0 == operand1);
    sandbox.set_flag_greater(operand0 > operand1);
}

void Interpreter::op_inc (SandBox&, ui32& target) {
    ++target;
}

void Interpreter::op_dec (SandBox&, ui32& target) {
    --target;
}

void Interpreter::op_out (SandBox&, ui32 operand) {
    std::cout << operand << '\n';
}


}