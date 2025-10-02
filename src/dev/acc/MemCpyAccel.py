from m5.params import *
from m5.SimObject import SimObject

class MemCpyAccel(SimObject):
    type = 'MemCpyAccel'
    cxx_header = "dev/acc/memcpyaccel.hh"
    cxx_class = "gem5::MemCpyAccel" 