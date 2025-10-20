#ifndef __ARCH_RISCV_INST_SOFTMAX_HH__
#define __ARCH_RISCV_INST_SOFTMAX_HH__

#include <string>

#include "arch/riscv/insts/static_inst.hh"
#include "cpu/exec_context.hh"
#include "cpu/static_inst.hh"

namespace gem5
{

namespace RiscvISA
{

class Softmax : public RiscvStaticInst
{
  protected:
    using RiscvStaticInst::RiscvStaticInst;

    std::string generateDisassembly(
        Addr pc, const loader::SymbolTable *symtab) const override;
};

} // namespace RiscvISA
} // namespace gem5

#endif // __ARCH_RISCV_INST_MEM_HH__
