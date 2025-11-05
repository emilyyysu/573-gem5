from m5.params import *
from m5.objects.Device import DmaDevice, BasicPioDevice

class MemCpyAccel(DmaDevice):
    type = 'MemCpyAccel'
    cxx_header = "dev/acc/memcpyaccel.hh"
    cxx_class = "gem5::MemCpyAccel"
    pioAddr = Param.Addr(0xC0000000, "Base address for PIO registers")
    pio_size = Param.Addr(0x20, "Size of PIO register space")

class MemCpyPioDevice(BasicPioDevice):
    type = 'MemCpyPioDevice'
    cxx_header = "dev/acc/memcpyaccel.hh"
    cxx_class = "gem5::MemCpyPioDevice"
