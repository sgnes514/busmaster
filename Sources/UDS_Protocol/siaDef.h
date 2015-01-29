#pragma  once
typedef enum
{
    immoStateIdel,
    immoStateWaiting,
#if 0
    immoStateLearnSKok,
    immoStateLearnPINok,
    immoStateResetSKok,
    immoStateResetEcuok,
    immoStateEcmChallengeOK,
#endif
    immoStateOK,
    immoStateCfgError,
    immoStateTimeOut
} immoTestStates;

typedef enum
{
    NoRelMsg,
    PosMsgRsp,
    PosMsgRspSendNext,
    NegMsgRsp,
    EmsChall

} chkMsgRsp;