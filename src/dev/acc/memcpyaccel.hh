#ifndef __MEMCPYACCEL_HH__
#define __MEMCPYACCEL_HH__

#include "dev/dma_device.hh"
#include "dev/io_device.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "params/MemCpyAccel.hh"
#include "params/MemCpyPioDevice.hh"

namespace gem5
{

/**
 * Base helper class for memcpy-related devices.
 */
class MemCpyBase
{
  protected:
    /** common register offsets */
    static const int REG_SRC   = 0x00;
    static const int REG_DST   = 0x04;
    static const int REG_CTRL  = 0x08;

    bool readRegister(PacketPtr pkt, uint32_t &reg, Addr pioAddr);
};

/**
 * Simple PIO-based memcpy device.
 */
class MemCpyPioDevice : public BasicPioDevice, public MemCpyBase
{
  protected:
    uint64_t devId;   //!< optional device ID / version tag

  public:
    //typedef MemCpyPioDeviceParams Params;
    MemCpyPioDevice(const MemCpyPioDeviceParams &p, Addr pio_size);
};

class MemCpyAccel : public DmaDevice, public MemCpyBase
{
  protected:
    Addr src;
    Addr dst;
    uint32_t ctrl_and_len;
    int len;

    Addr pioAddr;
    Addr pioSize;
    Tick pioDelay;

  public:
    MemCpyAccel(const MemCpyAccelParams *p);
    
    /* PIO read/write access */
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

    /* DMA operations */
    void startMemcpy();
    AddrRangeList getAddrRanges() const override;
    void dmaReadComplete(PacketPtr pkt);
    void dmaWriteComplete(PacketPtr pkt);

};

} // namespace gem5

#endif // __DEV_ACC_MEMCPYACCEL_HH__

// namespace gem5
// {

//     class MemCpyAccel : public DmaDevice
//     {
//         private:  
//             Addr src; // from cpu: address of vector to copy 
//             Addr dst; // from cpu: address of output vector
//             // bit 31 is from cpu, 1 means start operation, 0 means it's not requesting it 
//             // bit 30 is from accelerator, write 1 when it's done and also clear the start bit
//             uint32_t ctrl_and_len; // from cpu: length of vector. could be up to 30 bits. bits 30-31 ctrl status
//             int len; // length extracted from ctrl_and_len 32   
//             BasicPioDevice *pio_dev;

//         public:
//             MemCpyAccel(const MemCpyAccelParams *p);
//             AddrRangeList getAddrRanges() const override;
//             Tick read(PacketPtr pkt) override;
//             Tick write(PacketPtr pkt) override;
//             void startMemcpy();
//             void dmaReadComplete(PacketPtr pkt);
//             void dmaWriteComplete(PacketPtr pkt);

        
//     };
//     class MemCpyPioDev : public BasicPioDevice
//     {
//     public:
//         MemCpyPioDev(const BasicPioDeviceParams &p, Addr size)
//             : BasicPioDevice(p, size) {}

//         Tick read(PacketPtr pkt) override { return 0; }
//         Tick write(PacketPtr pkt) override { return 0; }
//     };
// } // namespace gem5



// #endif 

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
