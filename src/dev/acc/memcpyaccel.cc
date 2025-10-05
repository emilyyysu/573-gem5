#include "dev/acc/memcpyaccel.hh"
#include "base/trace.hh"
#include "debug/MemcpyAccel.hh"

#include <iostream>

namespace gem5
{

/*MemCpyAccel::MemCpyAccel(const MemCpyAccelParams &params) :
    SimObject(params)
{
    std::cout << "Hello World! From a SimObject!" << std::endl;
}
*/

class DmaDevice : virtual public SimObject {};
class BasicPioDevice : virtual public SimObject {};

MemCpyAccel::MemCpyAccel(const MemCpyAccelParams &p)
    : DmaDevice(p), BasicPioDevice(p, 12),
      src(0), dst(0), ctrl_and_len(0),
      len(0)
{
    pioSize = 12; // 3 4-byte registers
}

void
MemCpyAccel::startMemcpy()
{
    // issue first read
    // int len = ctrl_and_len & ~(0b11 << 30); // clear left two bits 
    dmaRead(src, len, nullptr, 0); // read all of them
}

void
MemCpyAccel::dmaReadComplete(PacketPtr pkt)
{
    // issue a write with the same data
    uint8_t *data = new uint8_t[len];
    memcpy(data, pkt->getConstPtr<uint8_t>(), len);  
    dmaWrite(dst, static_cast<unsigned long>(len), nullptr, data);
    //dmaWrite(dst, static_cast<unsigned long>(len), nullptr, pkt->getConstPtr<uint8_t>());
}


void
MemCpyAccel::dmaWriteComplete(PacketPtr pkt)
{
    //delete pkt; // free write packet

    ctrl_and_len |= 1 << 30; // set done bit
    ctrl_and_len &= ~(1 << 31); // clear start bit 
}

Tick
MemCpyAccel::read(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;

    uint64_t data = 0;
    switch (offset) {
      case 0x00: data = src; break;
      case 0x04: data = dst; break;
      case 0x08: data = ctrl_and_len; break;
      default: panic("MemcpyAccel: bad read offset %#x\n", offset);
    }

    pkt->setUintX(data, ByteOrder::little);
    pkt->makeResponse();
    return pioDelay; // how long it takes to read from the register
}

Tick
MemCpyAccel::write(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;

    uint64_t data = pkt->getUintX(ByteOrder::little);

    switch (offset) {
      case 0x00: src = data; break;
      case 0x04: dst = data; break;
      case 0x08: 
        ctrl_and_len = data;
        len = ctrl_and_len & ~(0b11 << 30);
        if (ctrl_and_len >> 31 & 0x1) { //if we are trying to start an op
            startMemcpy();
        }
        break;
      default: panic("MemcpyAccel: bad write offset %#x\n", offset);
    }
    pkt->makeResponse();
    return pioDelay;
}

} // namespace gem5 
