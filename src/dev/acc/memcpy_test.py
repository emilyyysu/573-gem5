# import m5
# from m5.objects import *

# system = System()
# system.clk_domain = SrcClockDomain()
# system.clk_domain.clock = '1GHz'
# system.clk_domain.voltage_domain = VoltageDomain()
# system.mem_mode = 'timing'
# system.mem_ranges = [AddrRange('512MB')]

# system.cpu = TimingSimpleCPU()

# system.accel = MemCpyAccel()

# system.membus = SystemXBar()

# system.cpu.icache_port = system.membus.inst_port
# system.cpu.dcache_port = system.membus.data_port

# system.accel.pio = system.membus.slave
# system.accel.dma = system.membus.master

# system.mem_ctrl = DDR3_1600_8x8()
# system.mem_ctrl.range = system.mem_ranges[0]
# system.mem_ctrl.port = system.membus.master

# system.system_port = system.membus.slave

# process = Process()
# process.cmd = ['tests/test-progs/hello/bin/x86/linux/hello']
# system.cpu.workload = process
# system.cpu.createThreads()

# root = Root(full_system = False, system = system)
# m5.instantiate()

# print ("Beginning simulation!")
# exit_event = m5.simulate()
# print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))

import m5
from m5.objects import *

system = System()
system.mem_mode = 'timing'
system.clk_domain = SrcClockDomain(clock='1GHz', voltage_domain=VoltageDomain())

system.memcpy_pio = BasicPioDevice(pio_addr=0x10010000, pio_size=0x100, pio_latency='10ns')
system.memcpy_accel = MemCpyAccel(piodevice=system.memcpy_pio)
system.memcpy_pio.pio = system.membus.mem_side_ports

root = Root(full_system=False, system=system)
m5.instantiate()
