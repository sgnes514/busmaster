#pragma once
#include "afxwin.h"
#include "Sia_16_Protocol.h"
#include "Sia_17_Protocol.h"







class Sia_Protocol :
    public CWinApp
{
public:
    Sia_Protocol(void);
    ~Sia_Protocol(void);
    Sia_16_Protocol* mSia16Ptr;
    Sia_17_Protocol* mSia17Ptr;
    void checkEmsResp(STCAN_MSG);


    immoTestStates siaStates;
};

