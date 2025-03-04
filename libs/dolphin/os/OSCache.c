#include "dolphin/os/OSCache.h"
#include "dolphin/base/PPCArch.h"
#include "dolphin/db.h"
#include "dolphin/os.h"
#include "dolphin/os/OSInterrupt.h"

/* 8033B56C-8033B580 335EAC 0014+00 1/1 0/0 0/0 .text            DCEnable */
static asm void DCEnable(void) {
    // clang-format off
    nofralloc

    sync
    mfspr r3, 0x3F0
    ori r3, r3, 0x4000
    mtspr 0x3F0, r3

    blr
    // clang-format on
}

/* 8033B580-8033B5AC 335EC0 002C+00 0/0 30/30 1/1 .text            DCInvalidateRange */
asm void DCInvalidateRange(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_invalidate:
    dcbi 0, start
    addi start, start, 0x20
    bdnz do_invalidate

    blr
    // clang-format on
}

/* 8033B5AC-8033B5DC 335EEC 0030+00 0/0 11/11 0/0 .text            DCFlushRange */
asm void DCFlushRange(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_flush:
    dcbf 0, start
    addi start, start, 0x20
    bdnz do_flush
    sc

    blr
    // clang-format on
}

/* 8033B5DC-8033B60C 335F1C 0030+00 0/0 54/54 1/1 .text            DCStoreRange */
asm void DCStoreRange(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_store:
    dcbst 0, start
    addi start, start, 0x20
    bdnz do_store
    sc

    blr
    // clang-format on
}

/* 8033B60C-8033B638 335F4C 002C+00 0/0 2/2 0/0 .text            DCFlushRangeNoSync */
asm void DCFlushRangeNoSync(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_flush:
    dcbf 0, start
    addi start, start, 0x20
    bdnz do_flush

    blr
    // clang-format on
}

/* 8033B638-8033B664 335F78 002C+00 0/0 13/13 3/3 .text            DCStoreRangeNoSync */
asm void DCStoreRangeNoSync(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_store:
    dcbst 0, start
    addi start, start, 0x20
    bdnz do_store

    blr
    // clang-format on
}

/* 8033B664-8033B690 335FA4 002C+00 0/0 1/1 1/1 .text            DCZeroRange */
asm void DCZeroRange(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_zero:
    dcbz 0, start
    addi start, start, 0x20
    bdnz do_zero

    blr
    // clang-format on
}

/* 8033B690-8033B6C4 335FD0 0034+00 0/0 6/6 0/0 .text            ICInvalidateRange */
asm void ICInvalidateRange(register void* start, register u32 nBytes) {
    // clang-format off
    nofralloc

    cmplwi nBytes, 0
    blelr

    clrlwi r5, start, 0x1B
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 0x1F
    srwi nBytes, nBytes, 5
    mtctr nBytes

do_invalidate:
    icbi 0, start
    addi start, start, 0x20
    bdnz do_invalidate

    sync
    isync

    blr
    // clang-format on
}

/* 8033B6C4-8033B6D4 336004 0010+00 0/0 5/5 0/0 .text            ICFlashInvalidate */
asm void ICFlashInvalidate(void) {
    // clang-format off
    nofralloc

    mfspr r3, 0x3F0
    ori r3, r3, 0x800
    mtspr 0x3F0, r3

    blr
    // clang-format on
}

/* 8033B6D4-8033B6E8 336014 0014+00 1/1 0/0 0/0 .text            ICEnable */
static asm void ICEnable(void) {
    // clang-format off
    nofralloc

    isync
    mfspr r3, 0x3F0
    ori r3, r3, 0x8000
    mtspr 0x3F0, r3

    blr
    // clang-format on
}

/* 8033B6E8-8033B7B4 336028 00CC+00 1/1 0/0 0/0 .text            __LCEnable */
asm void __LCEnable(void) {
    // clang-format off
    nofralloc

    mfmsr r5
    ori r5, r5, 0x1000
    mtmsr r5

    lis r3, 0x8000
    li r4, 0x400
    mtctr r4

do_store:
    dcbt 0, r3
    dcbst 0, r3
    addi r3, r3, 0x20
    bdnz do_store

    mfspr r4, 0x398
    oris r4, r4, 0x100F
    mtspr 0x398, r4

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    lis r3, 0xE000
    ori r3, r3, 0x0002
    mtdbatl 3, r3
    ori r3, r3, 0x1FE
    mtdbatu 3, r3
    isync

    lis r3, 0xE000
    li r6, 0x200
    mtctr r6
    li r6, 0
do_load:
    dcbz_l r6, r3
    addi r3, r3, 0x0020
    bdnz do_load

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    blr
    // clang-format on
}

/* 8033B7B4-8033B7EC 3360F4 0038+00 0/0 0/0 1/1 .text            LCEnable */
void LCEnable(void) {
    BOOL interrupt = OSDisableInterrupts();
    __LCEnable();
    OSRestoreInterrupts(interrupt);
}

/* 8033B7EC-8033B814 33612C 0028+00 0/0 2/2 1/1 .text            LCDisable */
asm void LCDisable(void) {
    // clang-format off
    nofralloc

    lis r3, 0xE000
    li r4, 0x200
    mtctr r4
do_invalidate:
    dcbi 0, r3
    addi r3, r3, 0x20
    bdnz do_invalidate

    mfspr r4, 0x398
    rlwinm r4, r4, 0, 4, 2
    mtspr 0x398, r4

    blr
    // clang-format on
}

/* 8033B814-8033B838 336154 0024+00 1/1 0/0 0/0 .text            LCStoreBlocks */
static asm void LCStoreBlocks(register void* destAddr, register void* srcAddr,
                              register u32 blockNum){
    // clang-format off
    nofralloc

    rlwinm r6, blockNum, 0x1E, 0x1B, 0x1F
    clrlwi destAddr, destAddr, 4
    or r6, r6, destAddr
    mtspr 0x39A, r6
    rlwinm r6, blockNum, 2, 0x1C, 0x1D
    or r6, r6, srcAddr
    ori r6, r6, 2
    mtspr 0x39B, r6

    blr
    // clang-format on
} /* 8033B838-8033B8E4 336178 00AC+00 0/0 0/0 3/3 .text            LCStoreData */
u32 LCStoreData(void* destAddr, void* srcAddr, u32 nBytes) {
    u32 blocks = (nBytes + 31) / 32;
    u32 ret = (blocks + 127) / 128;

    while (blocks > 0) {
        if (blocks < 128) {
            LCStoreBlocks(destAddr, srcAddr, blocks);
            blocks = 0;
        } else {
            LCStoreBlocks(destAddr, srcAddr, 0);
            blocks -= 128;

            destAddr = (char*)destAddr + 0x1000;
            srcAddr = (char*)srcAddr + 0x1000;
        }
    }

    return ret;
}

/* 8033B8E4-8033B8F8 336224 0014+00 0/0 0/0 3/3 .text            LCQueueWait */
asm void LCQueueWait(register u32 len) {
    // clang-format off
    nofralloc

    mfspr r4, 0x398
    rlwinm r4, r4, 8, 28, 31
    cmpw r4, len
    bgt LCQueueWait

    blr
    // clang-format on
}

static void L2Disable(void) {
    __sync();
    PPCMtl2cr(PPCMfl2cr() & ~0x80000000);
    __sync();
}

/* 8033B8F8-8033B990 336238 0098+00 1/1 0/0 0/0 .text            L2GlobalInvalidate */
void L2GlobalInvalidate(void) {
    L2Disable();
    PPCMtl2cr(PPCMfl2cr() | 0x00200000);
    while (PPCMfl2cr() & 0x00000001u)
        ;
    PPCMtl2cr(PPCMfl2cr() & ~0x00200000);
    while (PPCMfl2cr() & 0x00000001u) {
        DBPrintf(">>> L2 INVALIDATE : SHOULD NEVER HAPPEN\n");
    }
}

/* 8033B990-8033BAF0 3362D0 0160+00 1/1 0/0 0/0 .text            DMAErrorHandler */
void DMAErrorHandler(u16 error, OSContext* context, ...) {
    u32 hid2 = PPCMfhid2();

    OSReport("Machine check received\n");
    OSReport("HID2 = 0x%x   SRR1 = 0x%x\n", hid2, context->srr1);
    if (!(hid2 & (HID2_DCHERR | HID2_DNCERR | HID2_DCMERR | HID2_DQOERR)) ||
        !(context->srr1 & SRR1_DMA_BIT)) {
        OSReport("Machine check was not DMA/locked cache related\n");
        OSDumpContext(context);
        PPCHalt();
    }

    OSReport("DMAErrorHandler(): An error occurred while processing DMA.\n");
    OSReport("The following errors have been detected and cleared :\n");

    if (hid2 & HID2_DCHERR) {
        OSReport("\t- Requested a locked cache tag that was already in the cache\n");
    }

    if (hid2 & HID2_DNCERR) {
        OSReport("\t- DMA attempted to access normal cache\n");
    }

    if (hid2 & HID2_DCMERR) {
        OSReport("\t- DMA missed in data cache\n");
    }

    if (hid2 & HID2_DQOERR) {
        OSReport("\t- DMA queue overflowed\n");
    }

    // write hid2 back to clear the error bits
    PPCMthid2(hid2);
}

static void L2Init(void) {
    u32 oldMSR;
    oldMSR = PPCMfmsr();
    __sync();
    PPCMtmsr(MSR_IR | MSR_DR);
    __sync();
    L2Disable();
    L2GlobalInvalidate();
    PPCMtmsr(oldMSR);
}

void L2Enable(void) {
    PPCMtl2cr((PPCMfl2cr() | L2CR_L2E) & ~L2CR_L2I);
}

/* 8033BAF0-8033BBE4 336430 00F4+00 0/0 2/2 0/0 .text            __OSCacheInit */
void __OSCacheInit() {
    if (!(PPCMfhid0() & HID0_ICE)) {
        ICEnable();
        DBPrintf("L1 i-caches initialized\n");
    }
    if (!(PPCMfhid0() & HID0_DCE)) {
        DCEnable();
        DBPrintf("L1 d-caches initialized\n");
    }

    if (!(PPCMfl2cr() & L2CR_L2E)) {
        L2Init();
        L2Enable();
        DBPrintf("L2 cache initialized\n");
    }

    OSSetErrorHandler(OS_ERROR_MACHINE_CHECK, DMAErrorHandler);
    DBPrintf("Locked cache machine check handler installed\n");
}
