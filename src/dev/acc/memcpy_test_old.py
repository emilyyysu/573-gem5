import m5
from m5.objects import *
from m5.objects import MemCpyAccel, BasicPioDevice

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = "timing"
# system.mem_ranges = [
#     AddrRange((0x40000000,0x60000000)),   # normal DRAM
#     AddrRange((0x60000000,0x60000020))      # MemCpyAccel PIO registers
# ]

system.mem_ranges = [
    AddrRange("512MiB"),   # normal DRAM
    AddrRange((0x60000000,0x60000020))      # MemCpyAccel PIO registers
]

#system.mem_ranges = [AddrRange("2GB")]

#system.memcpy_accel = MemCpyAccel(pio_addr=0x40000000, pio_size=0x10)


#system.mem_ranges = [AddrRange("512MiB")]
system.cpu = RiscvTimingSimpleCPU()

system.membus = SystemXBar()

# Create the PIO device
#system.memcpy_pio = BasicPioDevice()

# Instantiate the accelerator

system.memcpy_accel = MemCpyAccel(pioAddr=0x60000000, pio_size=0x20)
system.memcpy_accel.dma = system.membus.cpu_side_ports
#system.memcpy_accel.pio = system.membus.mem_side_ports

#system.pio_bus = SystemXBar()
system.memcpy_accel.pio = system.membus.mem_side_ports


system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

system.cpu.createInterruptController()

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
#system.mem_ctrl.dram.range = system.mem_ranges[0]

system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

system.system_port = system.membus.cpu_side_ports

# thispath = os.path.dirname(os.path.realpath(__file__))
# binary = os.path.join(
#     thispath,
#     "../../../",
#     "tests/test-progs/hello/bin/riscv/linux/hello",
# )

# system.workload = SEWorkload.init_compatible(binary)

# process = Process()
# process.cmd = [binary]
# system.cpu.workload = process
# #system.cpu.createThreads()

# root = Root(full_system=False, system=system)
# m5.instantiate()

# --- Workload ---
thispath = os.path.dirname(os.path.realpath(__file__))
binary = os.path.join(thispath, "../../../small_test.elf")  # your compiled ELF

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
system.cpu.workload[0].map(0x40000000, 0x40000000, 0x00000100, cacheable=True)
system.cpu.workload[0].map(0x60000000, 0x60000000, 0x00000020, cacheable=False)
system.cpu.workload[0].map(0x80000000, 0x80000000, 0x80000000, cacheable=True)


# Dedicate upper 1GB to device
#system.cpu.workload[0].map(0x60000000, 0x60000000, 0x80000000, cacheable=True)


print("Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")

# print(f"Beginning simulation!")

# exit_event = m5.simulate()
# print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
