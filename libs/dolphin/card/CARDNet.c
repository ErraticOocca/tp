#include "dolphin/card/CARDNet.h"
#include "dolphin/card.h"
#include "dolphin/card/CARDPriv.h"
#include "dolphin/dsp.h"

/* 80450A70 0002+00 data_80450A70 __CARDVendorID */
u16 __CARDVendorID = 0xFFFF;

/* 80450A72 0006+00 data_80450A72 None */
u8 data_80450A72 = 0x1C;

/* 80359158-8035921C 353A98 00C4+00 0/0 1/1 0/0 .text            CARDGetSerialNo */
s32 CARDGetSerialNo(s32 chan, u64* serialNo) {
    CARDControl* card;
    CARDID* id;
    int i;
    u64 code;
    s32 result;

    if (!(0 <= chan && chan < 2)) {
        return CARD_RESULT_FATAL_ERROR;
    }

    result = __CARDGetControlBlock(chan, &card);
    if (result < 0) {
        return result;
    }

    id = (CARDID*)card->workArea;
    for (code = 0, i = 0; i < sizeof(id->serial) / sizeof(u64); ++i) {
        code ^= *(u64*)&id->serial[sizeof(u64) * i];
    }
    *serialNo = code;

    return __CARDPutControlBlock(card, CARD_RESULT_READY);
}
