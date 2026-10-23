// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2026 kanadenadi
// Copyright (c) 2026 EDK2-SPRD contributors.

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <kern/clock.h>
#include <kern/thread_call.h>
#include <kern/task.h>
#include <libkern/c++/OSData.h>

#if !TARGET_OS_EMBEDDED || TARGET_OS_OSX
#error SprdEmmc requires the embedded IOStorage ABI
#endif

namespace {
constexpr UInt32 kBlockBytes = 512;
constexpr UInt32 kGateEnable = 1U << 11;
constexpr UInt32 kClockSelectorOffset = 0x90;
constexpr UInt32 kClockSelectorMask = 7;
constexpr UInt32 kBlockCount32 = 0x00;
constexpr UInt32 kBlockSize = 0x04;
constexpr UInt32 kBlockCount16 = 0x06;
constexpr UInt32 kArgument = 0x08;
constexpr UInt32 kTransferMode = 0x0c;
constexpr UInt32 kResponse = 0x10;
constexpr UInt32 kPresentState = 0x24;
constexpr UInt32 kHostControl = 0x28;
constexpr UInt32 kPowerControl = 0x29;
constexpr UInt32 kClockControl = 0x2c;
constexpr UInt32 kTimeoutControl = 0x2e;
constexpr UInt32 kSoftwareReset = 0x2f;
constexpr UInt32 kInterruptStatus = 0x30;
constexpr UInt32 kInterruptEnable = 0x34;
constexpr UInt32 kSignalEnable = 0x38;
constexpr UInt32 kHostControl2 = 0x3e;
constexpr UInt32 kHostVersion = 0xfe;
constexpr UInt32 kAdmaError = 0x54;
constexpr UInt32 kAdmaAddressLow = 0x58;
constexpr UInt32 kAdmaAddressHigh = 0x5c;
constexpr UInt32 kAdmaSegmentBytes = 65536;
constexpr UInt16 kAdmaTransferValid = 0x21;
constexpr UInt16 kAdmaEnd = 2;
constexpr UInt8 kAdmaMode = 0x10;
constexpr UInt16 kAddress64 = 1U << 13;
constexpr UInt16 kAdmaLength26 = 1U << 10;
constexpr UInt32 kDllDelay = 0x204;
constexpr UInt32 kDllDelayOffset = 0x208;
constexpr UInt32 kBusyPosition = 0x250;
constexpr UInt32 kDebounce = 0x28c;
constexpr UInt32 kCommandInhibit = 1;
constexpr UInt32 kDataInhibit = 2;
constexpr UInt32 kCommandComplete = 1;
constexpr UInt32 kTransferComplete = 2;
constexpr UInt32 kDmaTerminal = kTransferComplete | (1U << 16) | (1U << 20) | (1U << 22);
constexpr UInt32 kInterruptErrors = 0xffff8000U;
constexpr UInt32 kStatusEnable = 0x03ff0003;
constexpr UInt8 kResetAll = 1;
constexpr UInt8 kResetCommandData = 6;
constexpr UInt8 kCardResetHigh = 8;
constexpr UInt16 kResponseNone = 0;
constexpr UInt16 kResponseR1 = 0x1a;
constexpr UInt16 kResponseR1b = 0x1b;
constexpr UInt16 kResponseR2 = 0x09;
constexpr UInt16 kResponseR3 = 0x02;
constexpr UInt32 kCardErrorMask = 0xffff8000U | (1U << 7);
constexpr UInt32 kCardReady = 1U << 8;
constexpr UInt32 kCardStateMask = 0xfU << 9;
constexpr UInt32 kCardTransferState = 4U << 9;
constexpr UInt32 kOcrRequest = 0x40ff8080;
constexpr UInt32 kOcrBusy = 1U << 31;
constexpr UInt32 kOcrSectorMode = 1U << 30;
constexpr UInt32 kIdentificationClock = 400000;
constexpr UInt16 kCardAddress = 1;

bool readCell(IOService *provider, const char *name, UInt32 &value)
{
    OSData *property = OSDynamicCast(OSData, provider->getProperty(name));
    if (!property || property->getLength() != sizeof(value)) { return false; }
    memcpy(&value, property->getBytesNoCopy(), sizeof(value));
    return true;
}

void decodeR2(const UInt32 response[4], UInt32 value[4])
{
    // SDHCI omits the trailing CRC byte from its 136-bit response registers.
    for (unsigned index = 0; index < 4; index++) {
        value[index] = response[3 - index] << 8;
        if (index != 3) { value[index] |= response[2 - index] >> 24; }
    }
}
}

class SprdEmmc : public IOBlockStorageDevice {
    OSDeclareDefaultStructors(SprdEmmc)

    struct AdmaDescriptor {
        UInt16 attributes;
        UInt16 length;
        UInt32 addressLow;
        UInt32 addressHigh;
        UInt32 reserved;
    };
    static_assert(sizeof(AdmaDescriptor) == 16, "ADMA descriptor stride");

    struct Synchronization {
        bool completed;
        IOReturn status;
    };

    struct Request {
        Request *next;
        IOMemoryDescriptor *buffer;
        UInt64 block;
        UInt64 blocks;
        IOStorageCompletion completion;
        Synchronization *synchronization;
    };

    IOMemoryMap *maps[3] = {};
    IOBufferMemoryDescriptor *dmaBuffer = nullptr;
    IOBufferMemoryDescriptor *dmaTable = nullptr;
    UInt64 dmaAddress = 0;
    UInt64 tableAddress = 0;
    bool dmaActive = false;
    uintptr_t registers = 0;
    uintptr_t clockRegisters = 0;
    uintptr_t gateRegisters = 0;
    IOLock *queueLock = nullptr;
    IOLock *controllerLock = nullptr;
    thread_call_t worker = nullptr;
    Request *first = nullptr;
    Request *last = nullptr;
    UInt32 parentFrequency = 0;
    UInt32 selector = 0;
    UInt32 maximumFrequency = 0;
    UInt32 busWidth = 0;
    UInt32 maximumTransferBytes = 0;
    UInt32 commandTimeout = 0;
    UInt32 dataTimeout = 0;
    UInt32 busyTimeout = 0;
    UInt32 initializationTimeout = 0;
    UInt32 sectors = 0;
    bool count32 = false;
    bool ready = false;
    bool reportedPresent = false;
    bool stopping = false;
    bool writeProtected = true;
    bool cacheEnabled = false;
    char vendor[16] = {};
    char product[7] = {};
    char revision[8] = {};

    UInt8 read8(UInt32 offset) const { return *reinterpret_cast<volatile UInt8 *>(registers + offset); }
    UInt16 read16(UInt32 offset) const { return *reinterpret_cast<volatile UInt16 *>(registers + offset); }
    UInt32 read32(UInt32 offset) const { return *reinterpret_cast<volatile UInt32 *>(registers + offset); }
    void write8(UInt32 offset, UInt8 value) {
        *reinterpret_cast<volatile UInt8 *>(registers + offset) = value;
        __asm__ volatile("dsb sy" ::: "memory");
    }
    void write16(UInt32 offset, UInt16 value) {
        *reinterpret_cast<volatile UInt16 *>(registers + offset) = value;
        __asm__ volatile("dsb sy" ::: "memory");
    }
    void write32(UInt32 offset, UInt32 value) {
        *reinterpret_cast<volatile UInt32 *>(registers + offset) = value;
        __asm__ volatile("dsb sy" ::: "memory");
    }

    bool mapResources(IOService *provider);
    bool readConfiguration(IOService *provider);
    IOReturn reset(UInt8 mask);
    IOReturn setClock(UInt32 frequency);
    IOReturn waitState(UInt32 mask, UInt32 expected, UInt32 timeout);
    IOReturn waitInterrupt(UInt32 mask, UInt32 timeout);
    IOReturn command(UInt16 index, UInt32 argument, UInt16 responseType,
                     UInt32 *response = nullptr, UInt16 transferMode = 0);
    IOReturn waitReady();
    IOReturn switchByte(UInt8 index, UInt8 value);
    IOReturn initializeCard();
    IOReturn transfer(UInt16 commandIndex, UInt32 block, UInt32 blocks,
                      IOMemoryDescriptor *buffer, UInt64 offset, UInt32 &completed);
    IOReturn flushCache();
    IOReturn enqueueRequest(Request *request);
    void finishRequest(Request *request, IOReturn status, UInt64 bytes = 0);
    void stopWorker();
    static void processRequests(thread_call_param_t owner, thread_call_param_t unused);

public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    void free() override;
    IOReturn doEjectMedia() override { return kIOReturnUnsupported; }
    IOReturn doFormatMedia(UInt64) override { return kIOReturnUnsupported; }
    UInt32 doGetFormatCapacities(UInt64 *, UInt32) const override { return 0; }
    char *getVendorString() override { return vendor; }
    char *getProductString() override { return product; }
    char *getRevisionString() override { return revision; }
    char *getAdditionalDeviceInfoString() override {
        static char info[] = "eMMC user area";
        return info;
    }
    IOReturn reportBlockSize(UInt64 *size) override { *size = kBlockBytes; return kIOReturnSuccess; }
    IOReturn reportMaxValidBlock(UInt64 *block) override { *block = sectors - 1; return kIOReturnSuccess; }
    IOReturn reportEjectability(bool *value) override { *value = false; return kIOReturnSuccess; }
    IOReturn reportRemovability(bool *value) override { *value = false; return kIOReturnSuccess; }
    IOReturn reportWriteProtection(bool *value) override { *value = writeProtected; return kIOReturnSuccess; }
    IOReturn reportMediaState(bool *present, bool *changed = nullptr) override;
    IOReturn getWriteCacheState(bool *enabled) override { *enabled = cacheEnabled; return kIOReturnSuccess; }
    IOReturn doSynchronize(UInt64 block, UInt64 blocks, IOStorageSynchronizeOptions options = 0) override;
    IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer, UInt64 block, UInt64 blocks,
                             IOStorageAttributes *attributes, IOStorageCompletion *completion) override;
};

OSDefineMetaClassAndStructors(SprdEmmc, IOBlockStorageDevice)

bool SprdEmmc::mapResources(IOService *provider)
{
    IORegistryEntry *parent = provider->getParentEntry(gIODTPlane);
    OSData *ranges = parent ? OSDynamicCast(OSData, parent->getProperty("ranges")) : nullptr;
    OSData *reg = OSDynamicCast(OSData, provider->getProperty("reg"));
    UInt64 bus[3], regions[6];
    if (!ranges || ranges->getLength() != sizeof(bus) || !reg || reg->getLength() != sizeof(regions)) {
        return false;
    }
    memcpy(bus, ranges->getBytesNoCopy(), sizeof(bus));
    memcpy(regions, reg->getBytesNoCopy(), sizeof(regions));
    if (regions[1] < kDebounce + sizeof(UInt32) ||
        regions[3] < kClockSelectorOffset + sizeof(UInt32) || regions[5] < sizeof(UInt32)) {
        return false;
    }
    for (unsigned index = 0; index < 3; index++) {
        UInt64 offset = regions[index * 2], size = regions[index * 2 + 1];
        if (offset < bus[0] || offset - bus[0] > bus[2] ||
            size > bus[2] - (offset - bus[0]) || bus[1] > UINT64_MAX - (offset - bus[0])) {
            return false;
        }
        IOMemoryDescriptor *memory = IOMemoryDescriptor::withPhysicalAddress(
            bus[1] + offset - bus[0], size, kIODirectionInOut);
        if (!memory) { return false; }
        maps[index] = memory->map();
        memory->release();
        if (!maps[index]) { return false; }
    }
    registers = maps[0]->getVirtualAddress();
    clockRegisters = maps[1]->getVirtualAddress();
    gateRegisters = maps[2]->getVirtualAddress();
    return true;
}

bool SprdEmmc::readConfiguration(IOService *provider)
{
    UInt32 readOnly;
    if (!readCell(provider, "clock-frequency", parentFrequency) ||
        !readCell(provider, "clock-selector", selector) ||
        !readCell(provider, "max-frequency", maximumFrequency) ||
        !readCell(provider, "bus-width", busWidth) ||
        !readCell(provider, "read-only", readOnly) ||
        !readCell(provider, "max-transfer-bytes", maximumTransferBytes) ||
        !readCell(provider, "command-timeout-us", commandTimeout) ||
        !readCell(provider, "data-timeout-us", dataTimeout) ||
        !readCell(provider, "busy-timeout-us", busyTimeout) ||
        !readCell(provider, "initialization-timeout-us", initializationTimeout)) {
        return false;
    }
    writeProtected = readOnly != 0;
    return parentFrequency >= kIdentificationClock * 2 && selector <= kClockSelectorMask &&
        maximumFrequency >= kIdentificationClock && maximumFrequency <= 52000000 &&
        (busWidth == 1 || busWidth == 4 || busWidth == 8) &&
        maximumTransferBytes >= kBlockBytes && maximumTransferBytes / kBlockBytes <= UINT16_MAX &&
        maximumTransferBytes % kBlockBytes == 0 &&
        commandTimeout && dataTimeout && busyTimeout && initializationTimeout;
}

IOReturn SprdEmmc::waitState(UInt32 mask, UInt32 expected, UInt32 timeout)
{
    UInt64 deadline;
    clock_interval_to_deadline(timeout, kMicrosecondScale, &deadline);
    for (;;) {
        if (read32(kInterruptStatus) & kInterruptErrors) { return kIOReturnIOError; }
        if ((read32(kPresentState) & mask) == expected) { return kIOReturnSuccess; }
        if (mach_absolute_time() >= deadline) { return kIOReturnTimeout; }
        IODelay(10);
    }
}

IOReturn SprdEmmc::waitInterrupt(UInt32 mask, UInt32 timeout)
{
    UInt64 deadline;
    clock_interval_to_deadline(timeout, kMicrosecondScale, &deadline);
    for (;;) {
        UInt32 status = read32(kInterruptStatus);
        if (status & kInterruptErrors) { return kIOReturnIOError; }
        if ((status & mask) == mask) { return kIOReturnSuccess; }
        if (mach_absolute_time() >= deadline) { return kIOReturnTimeout; }
        IODelay(10);
    }
}

IOReturn SprdEmmc::reset(UInt8 mask)
{
    write8(kSoftwareReset, (read8(kSoftwareReset) & kCardResetHigh) | mask);
    UInt64 deadline;
    clock_interval_to_deadline(commandTimeout, kMicrosecondScale, &deadline);
    while (read8(kSoftwareReset) & mask) {
        if (mach_absolute_time() >= deadline) { return kIOReturnTimeout; }
        IODelay(10);
    }
    write32(kInterruptEnable, kStatusEnable);
    write32(kSignalEnable, 0);
    write8(kTimeoutControl, 0x0e);
    write32(kInterruptStatus, UINT32_MAX);
    return kIOReturnSuccess;
}

IOReturn SprdEmmc::setClock(UInt32 frequency)
{
    UInt64 denominator = UInt64(frequency) * 2;
    UInt32 divider = parentFrequency <= denominator ? 0 :
        UInt32((UInt64(parentFrequency) + denominator - 1) / denominator);
    divider = (divider + 1) / 2;
    if (divider > 1023) { return kIOReturnUnsupported; }
    write16(kClockControl, 0);
    UInt16 clock = UInt16(((divider & 0x300) >> 2) | ((divider & 0xff) << 8) | 1);
    write16(kClockControl, clock);
    UInt64 deadline;
    clock_interval_to_deadline(commandTimeout, kMicrosecondScale, &deadline);
    while (!(read16(kClockControl) & 2)) {
        if (mach_absolute_time() >= deadline) { return kIOReturnTimeout; }
        IODelay(10);
    }
    write16(kClockControl, clock | 4);
    UInt32 busy = read32(kBusyPosition);
    UInt32 automaticClocks = (1U << 24) | (1U << 25);
    write32(kBusyPosition, frequency > kIdentificationClock ? busy | automaticClocks : busy & ~automaticClocks);
    UInt32 inversion = (1U << 13) | (1U << 21);
    UInt32 delay = read32(kDllDelayOffset);
    write32(kDllDelayOffset, frequency <= kIdentificationClock ? delay | inversion : delay & ~inversion);
    if (frequency <= kIdentificationClock) { write32(kDllDelay, 0); }
    return kIOReturnSuccess;
}

IOReturn SprdEmmc::command(UInt16 index, UInt32 argument, UInt16 responseType,
                           UInt32 *response, UInt16 transferMode)
{
    bool data = transferMode != 0;
    UInt32 inhibit = kCommandInhibit;
    if (data || (responseType == kResponseR1b && index != 12)) { inhibit |= kDataInhibit; }
    IOReturn status = waitState(inhibit, 0, commandTimeout);
    if (status != kIOReturnSuccess) { return status; }
    write32(kInterruptStatus, UINT32_MAX);
    write32(kArgument, argument);
    UInt16 flags = UInt16(responseType | (data ? 0x20 : 0) | (index == 12 ? 0xc0 : 0));
    if (data) { dmaActive = true; }
    write32(kTransferMode, (UInt32((index << 8) | flags) << 16) | transferMode);
    status = waitInterrupt(kCommandComplete, commandTimeout);
    if (status != kIOReturnSuccess) {
        IOLog("SprdEmmc: CMD%u failed: 0x%x interrupt=0x%x\n", index, status, read32(kInterruptStatus));
        return status;
    }
    UInt32 reply[4] = {};
    if (responseType != kResponseNone) {
        reply[0] = read32(kResponse);
        if (responseType == kResponseR2) {
            for (unsigned word = 1; word < 4; word++) { reply[word] = read32(kResponse + word * 4); }
        }
    }
    write32(kInterruptStatus, kCommandComplete);
    if (response) { memcpy(response, reply, sizeof(reply)); }
    if ((responseType == kResponseR1 || responseType == kResponseR1b) && (reply[0] & kCardErrorMask)) {
        IOLog("SprdEmmc: CMD%u card status=0x%x\n", index, reply[0]);
        return kIOReturnIOError;
    }
    if (responseType == kResponseR1b) {
        status = waitInterrupt(kTransferComplete, busyTimeout);
        if (status != kIOReturnSuccess) { return status; }
        write32(kInterruptStatus, kTransferComplete);
        return waitState(kDataInhibit, 0, busyTimeout);
    }
    return kIOReturnSuccess;
}

IOReturn SprdEmmc::waitReady()
{
    UInt64 deadline;
    clock_interval_to_deadline(busyTimeout, kMicrosecondScale, &deadline);
    do {
        UInt32 response[4];
        IOReturn status = command(13, UInt32(kCardAddress) << 16, kResponseR1, response);
        if (status != kIOReturnSuccess) { return status; }
        if ((response[0] & (kCardStateMask | kCardReady)) == (kCardTransferState | kCardReady)) {
            return kIOReturnSuccess;
        }
        IOSleep(1);
    } while (mach_absolute_time() < deadline);
    return kIOReturnTimeout;
}

IOReturn SprdEmmc::switchByte(UInt8 index, UInt8 value)
{
    IOReturn status = command(6, (3U << 24) | (UInt32(index) << 16) | (UInt32(value) << 8), kResponseR1b);
    return status == kIOReturnSuccess ? waitReady() : status;
}

IOReturn SprdEmmc::transfer(UInt16 commandIndex, UInt32 block, UInt32 blocks,
                            IOMemoryDescriptor *buffer, UInt64 offset, UInt32 &completed)
{
    completed = 0;
    bool reading = buffer->getDirection() == kIODirectionIn;
    IOReturn status;
    if (blocks > 1) {
        status = command(23, blocks, kResponseR1);
        if (status != kIOReturnSuccess) { return status; }
    }
    status = waitState(kCommandInhibit | kDataInhibit, 0, commandTimeout);
    if (status != kIOReturnSuccess) { return status; }
    UInt32 bytes = blocks * kBlockBytes;
    void *bounce = dmaBuffer->getBytesNoCopy();
    if (!reading && buffer->readBytes(offset, bounce, bytes) != bytes) { return kIOReturnVMError; }
    auto *descriptors = static_cast<AdmaDescriptor *>(dmaTable->getBytesNoCopy());
    UInt32 entries = (bytes + kAdmaSegmentBytes - 1) / kAdmaSegmentBytes;
    for (UInt32 entry = 0, consumed = 0; entry < entries; entry++) {
        UInt32 remaining = bytes - consumed;
        UInt32 length = remaining < kAdmaSegmentBytes ? remaining : kAdmaSegmentBytes;
        UInt64 address = dmaAddress + consumed;
        descriptors[entry] = {
            UInt16(kAdmaTransferValid | (entry + 1 == entries ? kAdmaEnd : 0)),
            UInt16(length), UInt32(address), UInt32(address >> 32), 0,
        };
        consumed += length;
    }
    status = IOFlushProcessorCache(kernel_task, reinterpret_cast<IOVirtualAddress>(bounce), bytes);
    if (status != kIOReturnSuccess) { return status; }
    status = IOFlushProcessorCache(kernel_task, reinterpret_cast<IOVirtualAddress>(descriptors),
                                   entries * sizeof(AdmaDescriptor));
    if (status != kIOReturnSuccess) { return status; }
    write8(kHostControl, (read8(kHostControl) & ~UInt8(0x18)) | kAdmaMode);
    write16(kHostControl2, (read16(kHostControl2) & ~kAdmaLength26) | kAddress64);
    write32(kAdmaAddressLow, UInt32(tableAddress));
    write32(kAdmaAddressHigh, UInt32(tableAddress >> 32));
    write16(kBlockSize, kBlockBytes | (7U << 12));
    if (count32) { write32(kBlockCount32, blocks); }
    else { write16(kBlockCount16, UInt16(blocks)); }
    UInt16 mode = UInt16(1 | (reading ? 0x10 : 0) | (blocks > 1 ? 0x22 : 0));
    status = command(commandIndex, block, kResponseR1, nullptr, mode);
    if (status == kIOReturnSuccess) { status = waitInterrupt(kTransferComplete, dataTimeout); }
    if (status != kIOReturnSuccess) {
        IOLog("SprdEmmc: transfer CMD%u LBA=%u count=%u failed: 0x%x interrupt=0x%x\n",
              commandIndex, block, blocks, status, read32(kInterruptStatus));
        IOLog("SprdEmmc: state=0x%x mode=0x%x block=0x%x count=0x%x adma=0x%x\n",
              read32(kPresentState), read32(kTransferMode), read16(kBlockSize),
              count32 ? read32(kBlockCount32) : read16(kBlockCount16), read8(kAdmaError));
        UInt64 deadline;
        clock_interval_to_deadline(busyTimeout, kMicrosecondScale, &deadline);
        bool terminal = !dmaActive;
        while (!terminal) {
            if (read32(kInterruptStatus) & kDmaTerminal) {
                terminal = true;
                break;
            }
            if (mach_absolute_time() >= deadline) { break; }
            IOSleep(1);
        }
        IOReturn resetStatus = kIOReturnNotReady;
        IOReturn stopStatus = kIOReturnSuccess;
        if (terminal) {
            write32(kInterruptEnable, 0);
            resetStatus = reset(kResetCommandData);
            if (resetStatus == kIOReturnSuccess) {
                dmaActive = false;
                if (blocks > 1) { stopStatus = command(12, 0, kResponseR1b); }
            }
        }
        IOLog("SprdEmmc: transfer recovery active=%u reset=0x%x stop=0x%x\n",
              dmaActive, resetStatus, stopStatus);
        if (dmaActive || resetStatus != kIOReturnSuccess || stopStatus != kIOReturnSuccess) {
            IOLockLock(queueLock);
            ready = false;
            IOLockUnlock(queueLock);
            write16(kClockControl, 0);
        }
        return status;
    }
    dmaActive = false;
    write32(kInterruptStatus, UINT32_MAX);
    if (reading) {
        status = IOFlushProcessorCache(kernel_task, reinterpret_cast<IOVirtualAddress>(bounce), bytes);
        if (status != kIOReturnSuccess) { return status; }
        if (buffer->writeBytes(offset, bounce, bytes) != bytes) { return kIOReturnVMError; }
    } else {
        status = waitReady();
        if (status != kIOReturnSuccess) { return status; }
    }
    completed = blocks;
    return kIOReturnSuccess;
}

IOReturn SprdEmmc::initializeCard()
{
    volatile UInt32 *gate = reinterpret_cast<volatile UInt32 *>(gateRegisters);
    *gate |= kGateEnable;
    __asm__ volatile("dsb sy" ::: "memory");
    IODelay(1000);
    volatile UInt32 *mux = reinterpret_cast<volatile UInt32 *>(clockRegisters + kClockSelectorOffset);
    *mux = (*mux & ~kClockSelectorMask) | selector;
    __asm__ volatile("dsb sy" ::: "memory");
    if ((*mux & kClockSelectorMask) != selector) { return kIOReturnIOError; }
    UInt16 version = read16(kHostVersion);
    if (version == UINT16_MAX) { return kIOReturnNotFound; }
    count32 = (version & 0xff) >= 3;
    IOReturn status = reset(kResetAll);
    if (status != kIOReturnSuccess) { return status; }
    write8(kHostControl, read8(kHostControl) & ~UInt8(0x3e));
    write16(kHostControl2, (read16(kHostControl2) & ~UInt16(kAdmaLength26 | 7)) | kAddress64);
    IOLog("SprdEmmc: host=0x%x ctrl=0x%x ctrl2=0x%x count=%u-bit\n",
          version, read8(kHostControl), read16(kHostControl2), count32 ? 32 : 16);
    write32(kDebounce, read32(kDebounce) | 3);
    write8(kPowerControl, 0x0b);
    status = setClock(kIdentificationClock);
    if (status != kIOReturnSuccess) { return status; }
    IODelay(10000);
    status = command(0, 0, kResponseNone);
    if (status != kIOReturnSuccess) { return status; }
    IODelay(2000);
    UInt32 response[4] = {};
    UInt64 deadline;
    clock_interval_to_deadline(initializationTimeout, kMicrosecondScale, &deadline);
    do {
        status = command(1, kOcrRequest, kResponseR3, response);
        if (status != kIOReturnSuccess) { return status; }
        if (response[0] & kOcrBusy) { break; }
        IOSleep(1);
    } while (mach_absolute_time() < deadline);
    if (!(response[0] & kOcrBusy)) { return kIOReturnTimeout; }
    if (!(response[0] & kOcrSectorMode)) { return kIOReturnUnsupported; }
    status = command(2, 0, kResponseR2, response);
    if (status != kIOReturnSuccess) { return status; }
    UInt32 cid[4];
    UInt8 cidBytes[16];
    decodeR2(response, cid);
    for (unsigned index = 0; index < sizeof(cidBytes); index++) {
        cidBytes[index] = UInt8(cid[index / 4] >> ((3 - index % 4) * 8));
    }
    snprintf(vendor, sizeof(vendor), "MMC %02x", cidBytes[0]);
    memcpy(product, cidBytes + 3, sizeof(product) - 1);
    snprintf(revision, sizeof(revision), "%u.%u", cidBytes[9] >> 4, cidBytes[9] & 15);
    status = command(3, UInt32(kCardAddress) << 16, kResponseR1);
    if (status != kIOReturnSuccess) { return status; }
    status = command(9, UInt32(kCardAddress) << 16, kResponseR2, response);
    if (status != kIOReturnSuccess) { return status; }
    UInt32 csd[4];
    decodeR2(response, csd);
    writeProtected |= (csd[3] & (3U << 12)) != 0;
    status = command(7, UInt32(kCardAddress) << 16, kResponseR1b);
    if (status != kIOReturnSuccess) { return status; }
    status = waitReady();
    if (status != kIOReturnSuccess) { return status; }
    IOBufferMemoryDescriptor *extended = IOBufferMemoryDescriptor::withCapacity(kBlockBytes, kIODirectionIn);
    if (!extended) { return kIOReturnNoMemory; }
    UInt32 completed;
    status = transfer(8, 0, 1, extended, 0, completed);
    UInt8 partition = 0;
    if (status == kIOReturnSuccess) {
        const UInt8 *bytes = static_cast<const UInt8 *>(extended->getBytesNoCopy());
        sectors = UInt32(bytes[212]) | (UInt32(bytes[213]) << 8) |
            (UInt32(bytes[214]) << 16) | (UInt32(bytes[215]) << 24);
        cacheEnabled = (bytes[33] & 1) != 0;
        partition = bytes[179];
        if (!sectors || !(bytes[196] & 2)) { status = kIOReturnUnsupported; }
    }
    extended->release();
    if (status != kIOReturnSuccess) { return status; }
    if (partition & 7) {
        status = switchByte(179, partition & ~UInt8(7));
        if (status != kIOReturnSuccess) { return status; }
    }
    status = switchByte(183, busWidth == 8 ? 2 : busWidth == 4 ? 1 : 0);
    if (status != kIOReturnSuccess) { return status; }
    UInt8 control = read8(kHostControl) & ~UInt8(0x22);
    write8(kHostControl, control | (busWidth == 8 ? 0x20 : busWidth == 4 ? 2 : 0));
    status = switchByte(185, 1);
    if (status != kIOReturnSuccess) { return status; }
    write8(kHostControl, read8(kHostControl) | 4);
    return setClock(maximumFrequency);
}

bool SprdEmmc::start(IOService *provider)
{
    if (!IOBlockStorageDevice::start(provider)) { return false; }
    queueLock = IOLockAlloc();
    controllerLock = IOLockAlloc();
    if (!queueLock || !controllerLock || !readConfiguration(provider) || !mapResources(provider)) {
        IOLog("SprdEmmc: invalid resources or allocation failed\n");
        return false;
    }
    IOOptionBits dmaOptions = kIOMemoryPhysicallyContiguous | kIOMemoryMapperNone | kIODirectionInOut;
    UInt32 tableBytes = ((maximumTransferBytes + kAdmaSegmentBytes - 1) / kAdmaSegmentBytes) * sizeof(AdmaDescriptor);
    dmaBuffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, dmaOptions, maximumTransferBytes, PAGE_SIZE);
    dmaTable = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, dmaOptions, tableBytes, PAGE_SIZE);
    if (!dmaBuffer || !dmaTable) {
        IOLog("SprdEmmc: DMA allocation failed\n");
        return false;
    }
    IOByteCount dataSpan, tableSpan;
    dmaAddress = dmaBuffer->getPhysicalSegment(0, &dataSpan, kIOMemoryMapperNone);
    tableAddress = dmaTable->getPhysicalSegment(0, &tableSpan, kIOMemoryMapperNone);
    if (!dmaAddress || !tableAddress || dataSpan < maximumTransferBytes || tableSpan < tableBytes) {
        IOLog("SprdEmmc: DMA memory is not physically contiguous\n");
        return false;
    }
    worker = thread_call_allocate_with_options(&SprdEmmc::processRequests, this,
        THREAD_CALL_PRIORITY_KERNEL, THREAD_CALL_OPTIONS_ONCE);
    if (!worker) { return false; }
    IOReturn status = initializeCard();
    if (status != kIOReturnSuccess) {
        IOLog("SprdEmmc: initialization failed: 0x%x\n", status);
        return false;
    }
    ready = true;
    IOLog("SprdEmmc: %s %s revision %s, %u sectors, %s, ADMA64\n",
          vendor, product, revision, sectors, writeProtected ? "read-only" : "writable");
    registerService();
    return true;
}

IOReturn SprdEmmc::reportMediaState(bool *present, bool *changed)
{
    IOLockLock(controllerLock);
    *present = ready;
    if (changed) { *changed = ready != reportedPresent; }
    reportedPresent = ready;
    IOLockUnlock(controllerLock);
    return kIOReturnSuccess;
}

IOReturn SprdEmmc::doAsyncReadWrite(IOMemoryDescriptor *buffer, UInt64 block, UInt64 blocks,
                                   IOStorageAttributes *attributes, IOStorageCompletion *completion)
{
    if (attributes && (attributes->options & kIOStorageOptionForceUnitAccess)) {
        return kIOReturnUnsupported;
    }
    if (!buffer || !completion || !completion->action || !blocks ||
        block >= sectors || blocks > sectors - block || buffer->getLength() < blocks * kBlockBytes ||
        (buffer->getDirection() != kIODirectionIn && buffer->getDirection() != kIODirectionOut)) {
        return kIOReturnBadArgument;
    }
    if (buffer->getDirection() == kIODirectionOut && writeProtected) { return kIOReturnNotWritable; }
    Request *request = static_cast<Request *>(IOMalloc(sizeof(Request)));
    if (!request) { return kIOReturnNoMemory; }
    request->next = nullptr;
    request->buffer = buffer;
    request->block = block;
    request->blocks = blocks;
    request->completion = *completion;
    request->synchronization = nullptr;
    buffer->retain();
    IOReturn status = enqueueRequest(request);
    if (status != kIOReturnSuccess) {
        buffer->release();
        IOFree(request, sizeof(Request));
    }
    return status;
}

IOReturn SprdEmmc::enqueueRequest(Request *request)
{
    IOLockLock(queueLock);
    if (stopping || !ready) {
        IOLockUnlock(queueLock);
        return kIOReturnOffline;
    }
    if (last) { last->next = request; }
    else { first = request; }
    last = request;
    thread_call_enter(worker);
    IOLockUnlock(queueLock);
    return kIOReturnSuccess;
}

void SprdEmmc::processRequests(thread_call_param_t owner, thread_call_param_t unused)
{
    (void)unused;
    SprdEmmc *device = static_cast<SprdEmmc *>(owner);
    for (;;) {
        IOLockLock(device->queueLock);
        Request *request = device->first;
        if (device->stopping || !request) {
            IOLockUnlock(device->queueLock);
            return;
        }
        device->first = request->next;
        if (!device->first) { device->last = nullptr; }
        IOLockUnlock(device->queueLock);
        UInt64 bytes = 0;
        IOReturn status = kIOReturnSuccess;
        IOLockLock(device->controllerLock);
        if (!device->ready) {
            status = kIOReturnOffline;
        } else if (request->synchronization) {
            status = device->flushCache();
        } else {
            bool reading = request->buffer->getDirection() == kIODirectionIn;
            while (bytes / kBlockBytes < request->blocks && status == kIOReturnSuccess) {
                UInt64 remaining = request->blocks - bytes / kBlockBytes;
                UInt32 limit = device->maximumTransferBytes / kBlockBytes;
                UInt32 count = UInt32(remaining < limit ? remaining : limit);
                UInt32 commandIndex = reading ? (count == 1 ? 17 : 18) : (count == 1 ? 24 : 25);
                UInt32 completed;
                status = device->transfer(UInt16(commandIndex), UInt32(request->block + bytes / kBlockBytes),
                                          count, request->buffer, bytes, completed);
                bytes += UInt64(completed) * kBlockBytes;
            }
        }
        IOLockUnlock(device->controllerLock);
        device->finishRequest(request, status, bytes);
    }
}

IOReturn SprdEmmc::flushCache()
{
    if (!cacheEnabled) { return kIOReturnSuccess; }
    return switchByte(32, 1);
}

IOReturn SprdEmmc::doSynchronize(UInt64 block, UInt64 blocks, IOStorageSynchronizeOptions options)
{
    if (block >= sectors || (blocks && blocks > sectors - block) ||
        (options & ~kIOStorageSynchronizeOptionBarrier)) { return kIOReturnBadArgument; }
    Request *request = static_cast<Request *>(IOMalloc(sizeof(Request)));
    if (!request) { return kIOReturnNoMemory; }
    Synchronization synchronization = {false, kIOReturnOffline};
    request->next = nullptr;
    request->synchronization = &synchronization;
    IOReturn status = enqueueRequest(request);
    if (status != kIOReturnSuccess) {
        IOFree(request, sizeof(Request));
        return status;
    }
    IOLockLock(queueLock);
    while (!synchronization.completed) {
        IOLockSleep(queueLock, &synchronization, THREAD_UNINT);
    }
    status = synchronization.status;
    IOLockUnlock(queueLock);
    return status;
}

void SprdEmmc::finishRequest(Request *request, IOReturn status, UInt64 bytes)
{
    if (request->synchronization) {
        IOLockLock(queueLock);
        request->synchronization->status = status;
        request->synchronization->completed = true;
        IOLockWakeup(queueLock, request->synchronization, false);
        IOLockUnlock(queueLock);
    } else {
        IOStorage::complete(&request->completion, status, bytes);
        request->buffer->release();
    }
    IOFree(request, sizeof(Request));
}

void SprdEmmc::stopWorker()
{
    if (!worker) { return; }
    IOLockLock(queueLock);
    stopping = true;
    IOLockUnlock(queueLock);
    thread_call_cancel_wait(worker);
    thread_call_free(worker);
    worker = nullptr;
    while (first) {
        Request *request = first;
        first = request->next;
        finishRequest(request, kIOReturnOffline);
    }
    last = nullptr;
}

void SprdEmmc::stop(IOService *provider)
{
    stopWorker();
    IOLockLock(controllerLock);
    IOReturn status = ready ? flushCache() : kIOReturnOffline;
    if (status != kIOReturnSuccess) { IOLog("SprdEmmc: shutdown flush failed: 0x%x\n", status); }
    ready = false;
    write32(kSignalEnable, 0);
    write16(kClockControl, 0);
    IOLockUnlock(controllerLock);
    IOBlockStorageDevice::stop(provider);
}

void SprdEmmc::free()
{
    stopWorker();
    if (dmaActive) {
        IOLog("SprdEmmc: controller still active; retaining DMA memory until reboot\n");
    } else {
        if (dmaTable) { dmaTable->release(); }
        if (dmaBuffer) { dmaBuffer->release(); }
    }
    for (auto &map : maps) {
        if (map) { map->release(); }
    }
    if (queueLock) { IOLockFree(queueLock); }
    if (controllerLock) { IOLockFree(controllerLock); }
    IOBlockStorageDevice::free();
}
