#ifndef __MEMCPYACCEL_HH__
#define __MEMCPYACCEL_HH__

#include "dev/dma_device.hh"
#include "dev/io_device.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "params/MemCpyAccel.hh"
#include "params/MemCpyPioDevice.hh"
#include "debug/MemCpyAccelDebug.hh"

namespace gem5
{

/**
 * Base helper class for memcpy-related devices.
 */
class MemCpyBase
{
  protected:
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
    uint32_t devId;   //!< optional device ID / version tag

  public:
    MemCpyPioDevice(const MemCpyPioDeviceParams &p, Addr pio_size);
};

class MemCpyAccel : public DmaDevice, public MemCpyBase
{
  protected:
    uint8_t *pendingReadBuf = nullptr;     // buffer we pass to dmaRead
    uint8_t *pendingWriteBuf = nullptr;    // buffer we pass to dmaWrite (freed on dmaWriteComplete)
    size_t pendingReadSize = 0;            // bytes
    size_t pendingWriteSize = 0;           // bytes
    Addr src;
    Addr dst;
    uint32_t ctrl_and_len;
    uint32_t len;

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
    //MemCpyDmaPort dmaPort;
    void performComputation(size_t bytes);
    void dmaReadComplete(size_t bytes);
    //void dmaReadComplete(PacketPtr pkt);
    void dmaWriteComplete();
   // bool handleDmaResp(PacketPtr pkt);
};

} // namespace gem5

#endif // __DEV_ACC_MEMCPYACCEL_HH__
