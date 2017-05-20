// FFManager.cpp
// Force Feedback handler
// 18th Sept 98
// 
// Limitations: Currently assumes first joystick


#ifndef TARGET_DC

#if 0

#include "FFManager.h"


FFManager* the_ff_manager;

FFManager::FFManager() {
	testeffect=0;
	ForceFeedback=0;
	DeviceInfo=the_input_manager.FindDevice(JOYSTICK,0,NULL,NULL);
	if (!DeviceInfo) return;
	lpdiInputDevice=DeviceInfo->lpdiInputDevice;

	ForceFeedback=FFSupported(lpdiInputDevice);
	if (ForceFeedback) TRACE("*** Force Feedback Detected ***\n");

}

FFManager::~FFManager() {
	ReleaseFX();
}

void FFManager::ReleaseFX() {
	if (testeffect) testeffect->Release();
}


BOOL FFManager::FFSupported(LPDIRECTINPUTDEVICE2 device) {
	DIDEVCAPS caps;

	if (!device) {
		return ForceFeedback;
	} else {
		caps.dwSize=sizeof(caps);
		device->GetCapabilities(&caps);
		if (caps.dwFlags&DIDC_FORCEFEEDBACK) return 1;
		return 0;
	}
}

BOOL FFManager::Test() {
	DIEFFECT effect;
	DICONSTANTFORCE diConstantForce;  
	DWORD    dwAxes[2] = { DIJOFS_X, DIJOFS_Y };
	LONG     lDirection[2] = { 18000, 0 };
	HRESULT  res;

	if ((!ForceFeedback)||(!lpdiInputDevice)) return FALSE;
	if (testeffect) testeffect->Release();

	effect.dwSize=sizeof(effect);
	effect.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS; 
	effect.dwDuration = (DWORD)( 0.5f * DI_SECONDS );
	effect.dwSamplePeriod = 0;               // = default 
	effect.dwGain = DI_FFNOMINALMAX;         // no scaling
	effect.dwTriggerButton = DIEB_NOTRIGGER; // not a button response
	effect.dwTriggerRepeatInterval = 0;      // not applicable
	effect.cAxes = 2; 
	effect.rgdwAxes = dwAxes;
	effect.rglDirection = lDirection; 
	effect.lpEnvelope = NULL; 
	effect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
	effect.lpvTypeSpecificParams = &diConstantForce;  

	diConstantForce.lMagnitude = DI_FFNOMINALMAX;   // full force 

	res=lpdiInputDevice->CreateEffect(GUID_ConstantForce,&effect,&testeffect,NULL);
	if (res==DI_OK) {
		res=testeffect->Start(1,0);
		return TRUE;
	}
	return FALSE;
	
}

#endif	// #if 0

#endif //#ifndef TARGET_DC


