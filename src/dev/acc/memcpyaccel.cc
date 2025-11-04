#include "dev/acc/memcpyaccel.hh"
#include "base/trace.hh"
#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>
#include "debug/MemCpyAccelDebug.hh"

namespace gem5
{
    MemCpyAccel::MemCpyAccel(const MemCpyAccelParams *p)
        : DmaDevice(*p),
          src(0), dst(0), ctrl_and_len(0), len(0),
          pioAddr(0x60000000), pendingReadBuf(nullptr), pendingWriteBuf(nullptr),
          pendingReadSize(0), pendingWriteSize(0)
    {
        std::cout << "does this construct? " << std::endl;
        DPRINTF(MemCpyAccelDebug, "MemCpyAccel constructed\n");
    }

void
MemCpyAccel::startMemcpy()
{
    DPRINTF(MemCpyAccelDebug,
        "Starting memcpy: src=%#llx dst=%#llx len=%u (bytes)\n",
        static_cast<unsigned long long>(src),
        static_cast<unsigned long long>(dst),
        len);

    // Interpret `len` as number of *bytes* (matches your test: len = 8 means 8 bytes)
    size_t sizeBytes = static_cast<size_t>(len);

    // Free previous pending read buffer
    if (pendingReadBuf) {
        delete[] pendingReadBuf;
        pendingReadBuf = nullptr;
    }

    pendingReadBuf = new uint8_t[sizeBytes];
    pendingReadSize = sizeBytes;

    DPRINTF(MemCpyAccelDebug,
        "About to call dmaRead: addr=%#llx size=%lu pendingReadBuf=%p\n",
        static_cast<unsigned long long>(src),
        (unsigned long) sizeBytes,
        pendingReadBuf);

    // dmaRead expects an Addr (64-bit), size in bytes, callback event, and destination buffer
    dmaRead(static_cast<Addr>(src),
        static_cast<int>(sizeBytes),
        new EventFunctionWrapper([this, sizeBytes]{ this->dmaReadComplete(sizeBytes); },
                                 "MemcpyAccel DMA read complete"),
        pendingReadBuf);
}

void MemCpyAccel::performComputation(size_t bytes) {

    size_t num_u32 = bytes / sizeof(uint32_t);
    const uint8_t *srcBytes = pendingReadBuf;
    const uint32_t *data = reinterpret_cast<const uint32_t *>(srcBytes);

    std::vector<float> output(num_u32);
    for (size_t i = 0; i < num_u32; ++i) {
        float x = reinterpret_cast<const float &>(data[i]); // interpret input as float
        output[i] = std::exp(x); // compute exp(x)
    }
    // prepare write-back buffer (bytes) containing doubles
    if (pendingWriteBuf) {
        delete[] pendingWriteBuf;
        pendingWriteBuf = nullptr;
    }
    pendingWriteSize = num_u32 * sizeof(uint32_t);
    pendingWriteBuf = new uint8_t[pendingWriteSize];
    std::memcpy(pendingWriteBuf, output.data(), pendingWriteSize);


    DPRINTF(MemCpyAccelDebug,
        "Launching DMA write: dst=%#llx len=%zu bytes\n",
        static_cast<unsigned long long>(dst), pendingWriteSize);

    // Note: write back the number of bytes we actually produced (pendingWriteSize).
    dmaWrite(static_cast<Addr>(dst),
        static_cast<int>(pendingWriteSize),
        new EventFunctionWrapper([this]{ this->dmaWriteComplete(); },
                                 "MemcpyAccel DMA write complete"),
        pendingWriteBuf);
}

void
MemCpyAccel::dmaReadComplete(size_t bytes)
{
    DPRINTF(MemCpyAccelDebug, "ENTER dmaReadComplete (bytes=%zu)\n", bytes);

    Cycles computeCycles = Cycles(100); // simulated compute latency
    Tick computeDelay = computeCycles * clockPeriod();

    schedule(new EventFunctionWrapper([this, bytes]() {
        this->performComputation(bytes);
    }, "MemcpyAccel Computation Complete"), curTick() + computeDelay);
}

void
MemCpyAccel::dmaWriteComplete()
{
    DPRINTF(MemCpyAccelDebug, "ENTER dmaWriteComplete\n");

    const uint32_t *wdata = reinterpret_cast<const uint32_t *>(pendingWriteBuf);
    size_t elems = pendingWriteSize / sizeof(uint32_t);


    // Clean up write buffer
    if (pendingWriteBuf) {
        delete[] pendingWriteBuf;
        pendingWriteBuf = nullptr;
    }

    // Optionally free the read buffer
    if (pendingReadBuf) {
        delete[] pendingReadBuf;
        pendingReadBuf = nullptr;
        pendingReadSize = 0;
    }

    // update control bits: set done (bit 30), clear start (bit 31)
    ctrl_and_len |= (1u << 30);
    ctrl_and_len &= ~(1u << 31);

    DPRINTF(MemCpyAccelDebug,
        "DMA write complete: dst=%#llx produced %zu bytes; done bit set\n",
        static_cast<unsigned long long>(dst), pendingWriteSize);
}

Tick
MemCpyAccel::read(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;

    uint32_t data = 0;
    switch (offset) {
      case 0x00: data = static_cast<uint32_t>(src); break;
      case 0x04: data = static_cast<uint32_t>(dst); break;
      case 0x08: data = ctrl_and_len; break;
      default: panic("MemcpyAccel: bad read offset %#llx\n", static_cast<long long>(offset));
    }

    // return 32-bit register value (little-endian)
    pkt->setLE<uint32_t>(data);
    pkt->makeResponse();
    return pioDelay;
}

Tick
MemCpyAccel::write(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;
    DPRINTF(MemCpyAccelDebug,
        "PIO write: addr=%#llx pioAddr=%#llx\n",
        static_cast<unsigned long long>(pkt->getAddr()),
        static_cast<unsigned long long>(pioAddr));

    // read 32-bit value from pkt (little-endian)
    uint32_t data = pkt->getLE<uint32_t>();
    DPRINTF(MemCpyAccelDebug,
        "PIO write: offset=%#llx value=%#x\n",
        static_cast<long long>(offset), data);

    switch (offset) {
      case 0x00:
        src = static_cast<Addr>(data);
        DPRINTF(MemCpyAccelDebug, "  Set src=%#llx\n", static_cast<unsigned long long>(src));
        break;

      case 0x04:
        dst = static_cast<Addr>(data);
        DPRINTF(MemCpyAccelDebug, "  Set dst=%#llx\n", static_cast<unsigned long long>(dst));
        break;

      case 0x08:
        ctrl_and_len = data;
        // mask out the top two control bits (bits 31 and 30) to get a byte length
        len = ctrl_and_len & ~(static_cast<uint32_t>(0b11u) << 30);
        DPRINTF(MemCpyAccelDebug, "  ctrl_and_len=%#x len=%u\n", ctrl_and_len, len);
        if (ctrl_and_len & (1u << 31)) { // start bit set?
            DPRINTF(MemCpyAccelDebug, "  Start bit set, calling startMemcpy()\n");
            startMemcpy();
        }
        break;

      default:
        panic("MemcpyAccel: bad write offset %#llx\n", static_cast<long long>(offset));
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


// #include "dev/acc/memcpyaccel.hh"
// #include "base/trace.hh"
// #include <iostream>
// #include "debug/MemCpyAccelDebug.hh"


// namespace gem5
// {
//     MemCpyAccel::MemCpyAccel(const MemCpyAccelParams *p)
//         : DmaDevice(*p), 
//         src(0), dst(0), ctrl_and_len(0), len(0), pioAddr(0x60000000), pendingReadBuf(nullptr), pendingWriteBuf(nullptr)
//     {
//         std::cout << "does this construct? " << std::endl;
//         DPRINTF(MemCpyAccelDebug, "MemCpyAccel constructed\n");
//     }

// void
// MemCpyAccel::startMemcpy()
// {
//     // issue first read
//     // int len = ctrl_and_len & ~(0b11 << 30); // clear left two bits 
//    DPRINTF(MemCpyAccelDebug,
//         "Starting memcpy: src=%#x dst=%#x len=%d (elements?)\n",
//         src, dst, len);

//     size_t sizeBytes = static_cast<size_t>(len) * sizeof(uint32_t);

//     // Free any previous pending read buffer (if present)
//     if (pendingReadBuf) {
//         delete[] pendingReadBuf;
//         pendingReadBuf = nullptr;
//     }

//     // Allocate the buffer in bytes
//     pendingReadBuf = new uint8_t[sizeBytes];
//     pendingReadSize = sizeBytes;

//     // dmaRead expects size in bytes
//     DPRINTF(MemCpyAccelDebug,
//     "About to call dmaRead: addr=%#llx size=%lu pendingReadBuf=%p\n",
//     static_cast<long long>(src),
//     (unsigned long) sizeBytes,
//     pendingReadBuf);
    
//     dmaRead(static_cast<Addr>(src),
//         static_cast<int>(sizeBytes),
//         new EventFunctionWrapper([this, sizeBytes]{ this->dmaReadComplete(sizeBytes); },
//                                  "MemcpyAccel DMA read complete"),
//         pendingReadBuf);
//     //dmaRead(src, static_cast<int>(sizeBytes), nullptr, pendingReadBuf);
// }

// void MemCpyAccel::performComputation(size_t bytes) {
    
//     // compute number of u32 words
//     size_t num_u32 = bytes / sizeof(uint32_t);

//     const uint8_t *srcBytes = pendingReadBuf;
//     const uint32_t *data = reinterpret_cast<const uint32_t *>(srcBytes);
    

//     size_t out_elems = num_u32;
//     std::vector<double> output(out_elems);
//     for (size_t i = 0; i < out_elems; ++i) {
//         output[i] = std::exp(static_cast<double>(data[i])); // your transform
//     }

//     // prepare write-back buffer (bytes)
//     if (pendingWriteBuf) {
//         delete[] pendingWriteBuf;
//         pendingWriteBuf = nullptr;
//     }
//     pendingWriteSize = out_elems * sizeof(double);
//     pendingWriteBuf = new uint8_t[pendingWriteSize];
//     std::memcpy(pendingWriteBuf, output.data(), pendingWriteSize);

//     DPRINTF(MemCpyAccelDebug,
//         "Launching DMA write: dst=%#x len=%zu bytes\n", dst, pendingWriteSize);

//     // dmaWrite expects size in bytes, event pointer, then data pointer
//     // dmaWrite(dst, static_cast<int>(pendingWriteSize), nullptr, pendingWriteBuf);
//     size_t sizeBytes = static_cast<size_t>(len) * sizeof(uint32_t);
//     dmaWrite(static_cast<Addr>(dst),
//         static_cast<int>(sizeBytes),
//         new EventFunctionWrapper([this, sizeBytes]{ this->dmaWriteComplete(); },
//                                  "MemcpyAccel DMA read complete"),
//         pendingWriteBuf);    
// }

// void
// MemCpyAccel::dmaReadComplete(size_t bytes)
// {
//     // issue a write with the same data
//     DPRINTF(MemCpyAccelDebug, "ENTER dmaReadComplete\n");

//     Cycles computeCycles = Cycles(100); // pick whatever latency you want
//     Tick computeDelay = computeCycles * clockPeriod(); // convert to simulation ticks

//     schedule(new EventFunctionWrapper([this, bytes]() {
//         this->performComputation(bytes);
//     }, "MemcpyAccel Computation Complete"), curTick() + computeDelay);
// }

// void
// MemCpyAccel::dmaWriteComplete()
// {
//     DPRINTF(MemCpyAccelDebug, "ENTER dmaWriteComplete\n");


//     const double *wdata = reinterpret_cast<const double *>(pendingWriteBuf);
//     size_t elems = pendingWriteSize / sizeof(double);


//     if (pendingWriteBuf) {
//         delete[] pendingWriteBuf;
//         pendingWriteBuf = nullptr;
//     }

//     // Optionally free the pending read buffer if you don't need it anymore
//     if (pendingReadBuf) {
//         delete[] pendingReadBuf;
//         pendingReadBuf = nullptr;
//         pendingReadSize = 0;
//     }

//     // update control bits
//     ctrl_and_len |= 1 << 30; // set done bit
//     ctrl_and_len &= ~(1 << 31); // clear start bit 

//     DPRINTF(MemCpyAccelDebug,
//         "DMA write complete: dst=%#x len=%d bytes; done bit set\n",
//         dst, len * static_cast<int>(sizeof(double)));
// }

// Tick
// MemCpyAccel::read(PacketPtr pkt)
// {
//     //Addr offset = pkt->getAddr() - pio_dev->getAddrRanges().front().start();
//     Addr offset = pkt->getAddr() - pioAddr;

//     uint32_t data = 0;
//     switch (offset) {
//       case 0x00: data = src; break;
//       case 0x04: data = dst; break;
//       case 0x08: data = ctrl_and_len; break;
//       default: panic("MemcpyAccel: bad read offset %#x\n", offset);
//     }
//     pkt->setUintX(data, ByteOrder::little);
//     pkt->makeResponse();
//     return pioDelay; // how long it takes to read from the register
// }

// Tick
// MemCpyAccel::write(PacketPtr pkt)
// {
//     Addr offset = pkt->getAddr() - pioAddr;
//   DPRINTF(MemCpyAccelDebug,
//         "PIO write: piogetAddr=%#x pioAddr=%#llx\n", pkt->getAddr(), pioAddr);
    
//     uint32_t data = pkt->getUintX(ByteOrder::little);
//   DPRINTF(MemCpyAccelDebug,
//         "PIO write: offset=%#x value=%#llx\n", offset, data);
//     switch (offset) {
//       case 0x00: 
//         src = data; 
//         DPRINTF(MemCpyAccelDebug, "  Set src=%#x\n", src);
//         break;
      
//       case 0x04: 
//         dst = data; 
        
//         DPRINTF(MemCpyAccelDebug, "  Set dst=%#x\n", dst);
//         break;
//       case 0x08: 
//         ctrl_and_len = data;
//         len = ctrl_and_len & ~(0b11 << 30);
//         DPRINTF(MemCpyAccelDebug, "  ctrl_and_len=%#x len=%d\n", ctrl_and_len, len);
//         if (ctrl_and_len >> 31 & 0x1) { //if we are trying to start an op
//             DPRINTF(MemCpyAccelDebug, "  Start bit set, calling startMemcpy()\n");
//             startMemcpy();
//         }
//         break;
//       default: panic("MemcpyAccel: bad write offset %#x\n", offset);
//     }
//     pkt->makeResponse();
//     return pioDelay;
// }

// AddrRangeList
// MemCpyAccel::getAddrRanges() const
// {
//    DPRINTF(MemCpyAccelDebug, "Register address range: [0x60000000, 0x60000020)\n");
//    return {RangeSize(0x60000000, 0x20)};
// }

// MemCpyAccel* MemCpyAccelParams::create() const
// {
//     return new MemCpyAccel(this);
// }

// } // namespace gem5 