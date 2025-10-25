#include "dev/acc/memcpyaccel.hh"
#include "base/trace.hh"
#include <iostream>
#include "debug/MemCpyAccelDebug.hh"


namespace gem5
{
    MemCpyAccel::MemCpyAccel(const MemCpyAccelParams *p)
        : DmaDevice(*p),
        src(0), dst(0), ctrl_and_len(0), len(0), pioAddr(0x60000000)
    {
        std::cout << "does this construct? " << std::endl;
        DPRINTF(MemCpyAccelDebug, "MemCpyAccel constructed\n");
    }

    
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
    DPRINTF(MemCpyAccelDebug,
        "Starting memcpy: src=%#x dst=%#x len=%d (bytes)\n",
        src, dst, len);
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
    DPRINTF(MemCpyAccelDebug,
        "DMA read complete: src=%#x len=%d bytes\n", src, len);
    // Extract base and exponent from memory
    const uint64_t *data = pkt->getConstPtr<uint64_t>();
    DPRINTF(MemCpyAccelDebug, "First few input words: ");
     for (int i = 0; i < std::min(len, 4); i++)
        DPRINTF(MemCpyAccelDebug, "%#llx ", data[i]);
    DPRINTF(MemCpyAccelDebug, "\n");
    // Assume 'len' is the number of elements (not bytes)
    std::vector<double> output(len);

    for (int i = 0; i < len; i++) {
        output[i] = std::exp(data[i]);
    }
    
   // 3. Allocate memory for DMA writeback
    uint8_t *writeData = new uint8_t[len * sizeof(double)];
    std::memcpy(writeData, output.data(), len * sizeof(double));
     // 4. Kick off the DMA write
     DPRINTF(MemCpyAccelDebug,
        "Launching DMA write: dst=%#x len=%d bytes\n", dst, len * sizeof(double));

    dmaWrite(dst, len * sizeof(double), nullptr, writeData);
    
}


void
MemCpyAccel::dmaWriteComplete(PacketPtr pkt)
{
    //delete pkt; // free write packet

    ctrl_and_len |= 1 << 30; // set done bit
    ctrl_and_len &= ~(1 << 31); // clear start bit 
      DPRINTF(MemCpyAccelDebug,
        "DMA write complete: dst=%#x len=%d bytes; done bit set\n",
        dst, len * sizeof(double));
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
DPRINTF(MemCpyAccelDebug,
        "PIO read: offset=%#x -> value=%#llx\n", offset, data);

    pkt->setUintX(data, ByteOrder::little);
    pkt->makeResponse();
    return pioDelay; // how long it takes to read from the register
}

Tick
MemCpyAccel::write(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;
  DPRINTF(MemCpyAccelDebug,
        "PIO write: piogetAddr=%#x pioAddr=%#llx\n", pkt->getAddr(), pioAddr);
    
    uint64_t data = pkt->getUintX(ByteOrder::little);
  DPRINTF(MemCpyAccelDebug,
        "PIO write: offset=%#x value=%#llx\n", offset, data);
    switch (offset) {
      case 0x00: 
        src = data; 
        DPRINTF(MemCpyAccelDebug, "  Set src=%#x\n", src);
        break;
      
      case 0x04: 
        dst = data; 
        
        DPRINTF(MemCpyAccelDebug, "  Set dst=%#x\n", dst);
        break;
      case 0x08: 
        ctrl_and_len = data;
        len = ctrl_and_len & ~(0b11 << 30);
        DPRINTF(MemCpyAccelDebug, "  ctrl_and_len=%#x len=%d\n", ctrl_and_len, len);
        if (ctrl_and_len >> 31 & 0x1) { //if we are trying to start an op
            DPRINTF(MemCpyAccelDebug, "  Start bit set, calling startMemcpy()\n");
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
   DPRINTF(MemCpyAccelDebug, "Register address range: [0x60000000, 0x60000020)\n");
   return {RangeSize(0x60000000, 0x20)};
}

MemCpyAccel* MemCpyAccelParams::create() const
{
    return new MemCpyAccel(this);
}

} // namespace gem5 