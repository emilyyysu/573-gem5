#include "dev/acc/memcpyaccel.hh"
#include "base/trace.hh"
#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>
#include <array>
#include "debug/MemCpyAccelDebug.hh"

namespace gem5
{
    MemCpyAccel::MemCpyAccel(const MemCpyAccelParams *p)
        : DmaDevice(*p),
          src(0), dst(0), ctrl_and_len(0), len(0),
          pioAddr(0xC0000000), pendingReadBuf(nullptr), pendingWriteBuf(nullptr),
          pendingReadSize(0), pendingWriteSize(0)
    {
        std::cout << "does this construct? " << std::endl;
        DPRINTF(MemCpyAccelDebug, "MemCpyAccel constructed\n");
        prevDivNum.fill(0.0f);
        prev0.fill(0.0f);
        prev1.fill(0.0f);
        prev2.fill(0.0f);
        prev3.fill(0.0f);
        prev4.fill(0.0f);
        prev5 = 0.0f;
    }

float
MemCpyAccel::adderTree32(const float* in, uint64_t& switchingCountRef, uint64_t& sameInputCountRef)
{
    // Stage 0 → 32
    float s0[32];
    for (int i = 0; i < 32; ++i) {
        s0[i] = in[i];
        if (s0[i] == prev0[i]) sameInputCountRef++;
        else switchingCountRef++;
        prev0[i] = s0[i];
    }

    // Stage 1 → 16
    float s1[16];
    for (int i = 0; i < 16; ++i) {
        float v = s0[2*i] + s0[2*i+1];
        if (v == prev1[i]) sameInputCountRef++;
        else switchingCountRef++;
        s1[i] = v;
        prev1[i] = v;
    }

    // Stage 2 → 8
    float s2[8];
    for (int i = 0; i < 8; ++i) {
        float v = s1[2*i] + s1[2*i+1];
        if (v == prev2[i]) sameInputCountRef++;
        else switchingCountRef++;
        s2[i] = v;
        prev2[i] = v;
    }

    // Stage 3 → 4
    float s3[4];
    for (int i = 0; i < 4; ++i) {
        float v = s2[2*i] + s2[2*i+1];
        if (v == prev3[i]) sameInputCountRef++;
        else switchingCountRef++;
        s3[i] = v;
        prev3[i] = v;
    }

    // Stage 4 → 2
    float s4[2];
    for (int i = 0; i < 2; ++i) {
        float v = s3[2*i] + s3[2*i+1];
        if (v == prev4[i]) sameInputCountRef++;
        else switchingCountRef++;
        s4[i] = v;
        prev4[i] = v;
    }

    // Stage 5 final → 1
    float result = s4[0] + s4[1];
    if (result == prev5) sameInputCountRef++;
    else switchingCountRef++;
    prev5 = result;

    return result;
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

    std::vector<float> exps(num_u32);
    std::vector<float> output(num_u32);

    // Find max(x) for numerical stability
    float max_x = -std::numeric_limits<float>::infinity();
    for (size_t i = 0; i < num_u32; ++i) {
        float x = reinterpret_cast<const float &>(data[i]);
        if (x > max_x) {
            max_x = x;
        }
    }

    // Compute exp
    constexpr float cutoff = -0.8f; // parametrize this
    size_t skipped = 0;

    for (size_t i = 0; i < num_u32; ++i) {
        float x = reinterpret_cast<const float &>(data[i]); // interpret input as float
        if(x < cutoff) {
            exps[i] = 0.0f;
            skipped++;
            zeroCount++;
            continue;
        }
        exps[i] = std::exp(x); // compute exp(x)
    }
    
    // ---- zero count ----
    DPRINTF(MemCpyAccelDebug,
        "ZeroCount (cumulative) = %llu\n",
        (unsigned long long)zeroCount);

    // ---- adder tree ----
    uint64_t s0 = switchingAdd;
    uint64_t r0 = sameAdd;
    float exp_sum = 0.0f;
    size_t i = 0;

    while (i < num_u32) {
        // Take next 32 elements or remaining elements
        float chunk[32] = {0.0f};
        size_t chunkSize = std::min((size_t)32, num_u32 - i);

        for (size_t j = 0; j < chunkSize; j++)
            chunk[j] = exps.data()[i + j];

        // Add this chunk with the 32-input adder tree
        float chunkSum = adderTree32(chunk, switchingAdd, sameAdd);

        // Accumulate to running sum
        exp_sum += chunkSum;

        i += chunkSize;
    }

    //float exp_sum = adderTree32(exps.data(), switchingAdd, sameAdd);

    DPRINTF(MemCpyAccelDebug,
        "AdderTree: switches=%llu→%llu same=%llu→%llu\n",
        (unsigned long long)s0, (unsigned long long)switchingAdd,
        (unsigned long long)r0, (unsigned long long)sameAdd);
    
    // ---- div switching count ----
    uint64_t divSwitch_before = switchingDiv;
    uint64_t divSame_before   = sameDiv;

    i = 0;
    while (i < num_u32) {
        // Take next 32 elements or remaining elements
        size_t chunkSize = std::min((size_t)32, num_u32 - i);

        // Process division switching/same counts for this chunk
        for (size_t j = 0; j < chunkSize; j++) {
            if (exps[i + j] == prevDivNum[j])
                sameDiv++;
            else
                switchingDiv++;

            prevDivNum[j] = exps[i + j];
        }

        i += chunkSize;
    }


    // for (int i = 0; i < 32; i++) {
    //     if (exps[i] == prevDivNum[i])
    //         sameDiv++;
    //     else
    //         switchingDiv++;

    //     prevDivNum[i] = exps[i];
    // }

    DPRINTF(MemCpyAccelDebug,
        "Division: switching %llu→%llu  same %llu→%llu\n",
        (unsigned long long)divSwitch_before,
        (unsigned long long)switchingDiv,
        (unsigned long long)divSame_before,
        (unsigned long long)sameDiv);

    for (size_t i = 0; i < num_u32; ++i) {
        if(exps[i] == 0.0f) {
            output[i] = 0.0f;
        } else {
            output[i] = exps[i] / exp_sum;
        }
    }
    // prepare write-back buffer (bytes) containing doubles
    if (pendingWriteBuf) {
        delete[] pendingWriteBuf;
        pendingWriteBuf = nullptr;
    }
    pendingWriteSize = num_u32 * sizeof(uint32_t);
    pendingWriteBuf = new uint8_t[pendingWriteSize];
    std::memcpy(pendingWriteBuf, output.data(), pendingWriteSize);


    // DPRINTF(MemCpyAccelDebug,
    //     "Launching DMA write: dst=%#llx len=%zu bytes\n",
    //     static_cast<unsigned long long>(dst), pendingWriteSize);

    // // Note: write back the number of bytes we actually produced (pendingWriteSize).
    // dmaWrite(static_cast<Addr>(dst),
    //     static_cast<int>(pendingWriteSize),
    //     new EventFunctionWrapper([this]{ this->dmaWriteComplete(); },
    //                              "MemcpyAccel DMA write complete"),
    //     pendingWriteBuf);

    // --- calculate compute latency (based on skipped) ---
    size_t computed = num_u32 - skipped;

    // unsigned long long maxPassCycles = Cycles(1)  * num_u32;  // max pass
    // unsigned long long expPassCycles = Cycles(20) * computed; // exp pass
    Cycles computeCycles = Cycles(0);
    computeCycles = Cycles(10 * (num_u32/32 + 1)); // let's just say 12 cycles per thing
    // computeCycles += Cycles(1 * num_u32);   // max pass
    // computeCycles += Cycles(20 * computed);  // exp pass


    Tick computeDelay = computeCycles * clockPeriod();

    // --- schedule DMA WRITE *after* compute delay ---
    schedule(new EventFunctionWrapper([this]() {
        dmaWrite(static_cast<Addr>(dst),
                static_cast<int>(pendingWriteSize),
                new EventFunctionWrapper([this]{ this->dmaWriteComplete(); },
                                        "MemcpyAccel DMA write complete"),
                pendingWriteBuf);
    }, "MemcpyAccel Compute Delay"), curTick() + computeDelay);
}

void
MemCpyAccel::dmaReadComplete(size_t bytes)
{
    DPRINTF(MemCpyAccelDebug, "ENTER dmaReadComplete (bytes=%zu)\n", bytes);

    const double bytes_per_cycle = 128;  // effective bandwidth of the accelerator (bytes per cycle)
    const Cycles baseSetupCycles = Cycles(8); // constant control/setup overhead

    // Compute memory-related delay proportional to the number of bytes
    Cycles memLatencyCycles = Cycles(static_cast<uint64_t>(
    std::ceil(static_cast<double>(bytes) / bytes_per_cycle)));

    Cycles totalCycles = baseSetupCycles + memLatencyCycles;
    Tick computeDelay = totalCycles * clockPeriod();
    // Cycles computeCycles = Cycles(100); // simulated compute latency
    // Tick computeDelay = computeCycles * clockPeriod();

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
   DPRINTF(MemCpyAccelDebug, "Register address range: [0xC0000000, 0xC0000020)\n");
   return {RangeSize(0xC0000000, 0x20)};
}

MemCpyAccel* MemCpyAccelParams::create() const
{
    return new MemCpyAccel(this);
}

} // namespace gem5
