from m5.params import *
from m5.objects.Device import DmaDevice, BasicPioDevice

class MemCpyAccel(DmaDevice):
    type = 'MemCpyAccel'
    cxx_header = "dev/acc/memcpyaccel.hh"
    cxx_class = "gem5::MemCpyAccel"
    dma = MasterPort("DMA master port")

    # piodevice = Param.BasicPioDevice("PIO interface for MemCpyAccel")

class MemCpyPioDevice(BasicPioDevice):
    type = 'MemCpyPioDevice'
    cxx_header = "dev/acc/memcpyaccel.hh"
    cxx_class = "gem5::MemCpyPioDevice"

# from m5.params import *
# from m5.SimObject import SimObject
# from m5.objects.Device import (
#     DmaDevice, BasicPioDevice
# )

# class MemCpyAccel(DmaDevice):
#     type = 'MemCpyAccel'
#     cxx_header = "dev/acc/memcpyaccel.hh"
#     cxx_class = "gem5::MemCpyAccel" 
#     # Create a subdevice parameter of type BasicPioDevice
#     piodevice = Param.BasicPioDevice("PIO interface for MemCpyAccel")


# class MemCpyIODevice(BasicPioDevice):
#     type = "MemCpyIoDevice"
#     cxx_header = "dev/acc/mem_cpy_io_dev.hh"
#     cxx_class = "gem5::BasicPioDevice"
