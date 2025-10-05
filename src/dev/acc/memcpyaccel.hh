#ifndef __MEMCPYACCEL_HH__
#define __MEMCPYACCEL_HH__

#include "params/MemCpyAccel.hh"
#include "dev/dma_device.hh"
#include "dev/io_device.hh"

namespace gem5
{

    class MemCpyAccel : public DmaDevice, public BasicPioDevice
    {
        private: 
            Addr src; // from cpu: address of vector to copy 
            Addr dst; // from cpu: address of output vector
            // bit 31 is from cpu, 1 means start operation, 0 means it's not requesting it 
            // bit 30 is from accelerator, write 1 when it's done and also clear the start bit
            uint32_t ctrl_and_len; // from cpu: length of vector. could be up to 30 bits. bits 30-31 ctrl status
            int len; // length extracted from ctrl_and_len 32


        public:
            MemCpyAccel(const MemCpyAccelParams &p);
            Tick read(PacketPtr pkt);
            Tick write(PacketPtr pkt);
            void startMemcpy();
            void dmaReadComplete(PacketPtr pkt);
            void dmaWriteComplete(PacketPtr pkt);

        
    };

} // namespace gem5

#endif // __LEARNING_GEM5_HELLO_OBJECT_HH__

/*
#ifndef __DEV_MEMCPY_ACCEL_HH__
#define __DEV_MEMCPY_ACCEL_HH__

#include "dev/dma_device.hh"
#include "dev/io_device.hh"
#include "params/MemcpyAccel.hh"

class MemcpyAccel : public DmaDevice, public BasicPioDevice
{
  private:
    // registers
    Addr regSrc;
    Addr regDst;
    uint64_t regLen;
    uint32_t regCmd;
    uint32_t regStatus;

    // state
    Addr curSrc;
    Addr curDst;
    uint64_t remaining;

  public:
    MemcpyAccel(const MemcpyAccelParams *p);

    // PIO access (CPU reads/writes regs)
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

    // DMA callbacks
    void dmaReadComplete(PacketPtr pkt) override;
    void dmaWriteComplete(PacketPtr pkt) override;

    void startMemcpy();
};

#endif // __DEV_MEMCPY_ACCEL_HH__
// */
