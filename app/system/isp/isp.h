/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2023.6.14
----------------------------------------------------------------*/

#pragma once

#include "mi_isp.h"

class Isp{
public:
	static Isp* getInstance();
	
	int getExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit);
	int setExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit);

	int setExpoTimeUs(unsigned int expoTimeUs);
	int setExpoAuto();

	int getWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam);
	int setWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam);

	int enableWDR(int autoOrManual = 0);
	int disableWDR();

	int openIqServer(MI_U16 width, MI_U16 height, MI_S32 vpeChn);
	int closeIqServer();
	int setIqServerDataPath(char *iqFilePath);

private:
	Isp();
	~Isp();
	Isp(const Isp&);
	Isp& operator=(const Isp&);

private:
	const unsigned int iSpCh = 0;
};

