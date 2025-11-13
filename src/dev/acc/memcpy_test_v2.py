import m5
from m5.objects import *
from m5.objects import (
    BasicPioDevice,
    MemCpyAccel,
)

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = "timing"
# system.mem_ranges = [
#     AddrRange((0x40000000,0x60000000)),   # normal DRAM
#     AddrRange((0x60000000,0x60000020))      # MemCpyAccel PIO registers
# ]
system.mem_ranges = [AddrRange(0xA0000000, size="512MiB")]

system.cpu = RiscvTimingSimpleCPU()

system.membus = SystemXBar()

system.memcpy_accel = MemCpyAccel(pioAddr=0xC0000000, pio_size=0x20)
system.memcpy_accel.dma = system.membus.cpu_side_ports
system.memcpy_accel.pio = system.membus.mem_side_ports


system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

system.cpu.createInterruptController()

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]

system.mem_ctrl.port = system.membus.mem_side_ports

system.system_port = system.membus.cpu_side_ports

# --- Workload ---
thispath = os.path.dirname(os.path.realpath(__file__))
# binary = os.path.join(thispath, "../../../small_test.elf")
# binary = os.path.join(thispath, "../../../big_test.elf")
binary = os.path.join(thispath, "../../../../build/simple_test")


# Set up SE workload
system.workload = SEWorkload.init_compatible(binary)

process = Process()
process.cmd = [binary]
system.cpu.workload = process
system.cpu.createThreads()

# --- Instantiate and simulate ---
root = Root(full_system=False, system=system)
m5.instantiate()

# Dedicate upper 1GB to device
# system.cpu.workload[0].map(0x40000000, 0x40000000, 0x00000100, cacheable=True)
# system.cpu.workload[0].map(0x80000000, 0x80000000, 0x20000000, cacheable=False)
system.cpu.workload[0].map(0x10000000, 0x10000000, 0x20000000, cacheable=True)
system.cpu.workload[0].map(0xA0000000, 0xA0000000, 0x20000000, cacheable=True)
system.cpu.workload[0].map(0xC0000000, 0xC0000000, 0x00000020, cacheable=False)


print("Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")

# print(f"Beginning simulation!")

# exit_event = m5.simulate()
# print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
