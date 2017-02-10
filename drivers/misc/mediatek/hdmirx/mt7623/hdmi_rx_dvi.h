#ifndef     HDMI_RX_DVI_H
#define     HDMI_RX_DVI_H
#include "vga_timing.h"

#define RANGE_CHECKING(a, b, offset)  ((UINT32)((a)+(offset)-(b)) <= ((offset)*2))
#define CCIR_decode_444	 0

void hdmirx_irq_handle(void);
UINT8 u1GetRxCapturedTiming(void);
HDMI_RESOLUTION_MODE_T u4GetHdmiRxRes(void);
void vDviModeDetect(void);
void vDviChkModeChange(void);
BOOL fgCheckRxDetectDone(void);
UINT8 bDviStdTimingSearch(UINT8 bMode, UINT16   wDviHclk, UINT8 bDviVclk, UINT16 wDviHtotal,UINT16 wDviWidth);
void vHdmiRxDviStatus(void);
void vShowRxResoInfoStatus(void);
UINT8 bHDMI3DPacketVaild(void);
UINT16 wHDMI3DGetHVParameter(UINT8 bType);
void vDviInitial(void);
BOOL fgRxInputNtsc60(void);
HDMI_RESOLUTION_MODE_T vConvertHdmiRXResToVDORes(UINT8 u2Timing);
#endif
