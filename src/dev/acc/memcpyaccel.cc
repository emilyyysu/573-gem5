#include "dev/acc/memcpyaccel.hh"
#include "base/trace.hh"
#include <iostream>

namespace gem5
{
    MemCpyAccel::MemCpyAccel(const MemCpyAccelParams *p)
        : DmaDevice(*p),
        src(0), dst(0), ctrl_and_len(0), len(0)
    {}

    
    // MemCpyAccel::MemCpyAccel(const MemCpyAccelParams *p)
    //     : DmaDevice(*p),
    //     src(0), dst(0), ctrl_and_len(0), len(0), pio_dev(nullptr)
    // {
    //     pio_dev = p->piodevice;
    // }


// MemCpyAccel::MemCpyAccel(const MemCpyAccelParams &p)
//     : DmaDevice(p), BasicPioDevice(*p.piodevice, 12),
//       src(0), dst(0), ctrl_and_len(0),
//       len(0)
// {
//     pioSize = 12; // 3 4-byte registers
// }

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
    // uint8_t *data = new uint8_t[len];
    // memcpy(data, pkt->getConstPtr<uint8_t>(), len);  
    // dmaWrite(dst, static_cast<unsigned long>(len), nullptr, data);
    //dmaWrite(dst, static_cast<unsigned long>(len), nullptr, pkt->getConstPtr<uint8_t>());

    // Extract base and exponent from memory
    const uint64_t *data = pkt->getConstPtr<uint64_t>();
    
    // Assume 'len' is the number of elements (not bytes)
    std::vector<double> output(len);

    for (int i = 0; i < len; i++) {
        output[i] = std::exp(data[i]);
    }
    
   // 3. Allocate memory for DMA writeback
    uint8_t *writeData = new uint8_t[len * sizeof(double)];
    std::memcpy(writeData, output.data(), len * sizeof(double));
     // 4. Kick off the DMA write
     
    dmaWrite(dst, len * sizeof(double), nullptr, writeData);
    
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
    //Addr offset = pkt->getAddr() - pio_dev->getAddrRanges().front().start();
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

AddrRangeList
MemCpyAccel::getAddrRanges() const
{
   return {RangeSize(0x2F000000, 0x0C)};
}

MemCpyAccel* MemCpyAccelParams::create() const
{
    return new MemCpyAccel(this);
}

} // namespace gem5 