#ifndef _OPENVR_BINDINGS_H_
#define _OPENVR_BINDINGS_H_

#include <v8.h>
#include <nan/nan.h>

using namespace v8;

/// inline IVRSystem *VR_Init( EVRInitError *peError, EVRApplicationType eApplicationType );
NAN_METHOD(VR_Init);

/// inline void VR_Shutdown();
NAN_METHOD(VR_Shutdown);

/// VR_INTERFACE bool VR_CALLTYPE VR_IsHmdPresent();
NAN_METHOD(VR_IsHmdPresent);

/// VR_INTERFACE bool VR_CALLTYPE VR_IsRuntimeInstalled();
NAN_METHOD(VR_IsRuntimeInstalled);

/// VR_INTERFACE const char *VR_CALLTYPE VR_RuntimePath();
NAN_METHOD(VR_RuntimePath);

/// VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsSymbol( EVRInitError error );
NAN_METHOD(VR_GetVRInitErrorAsSymbol);

/// VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsEnglishDescription( EVRInitError error );
NAN_METHOD(VR_GetVRInitErrorAsEnglishDescription);

/// VR_INTERFACE void *VR_CALLTYPE VR_GetGenericInterface( const char *pchInterfaceVersion, EVRInitError *peError );
// Not implemented.

/// VR_INTERFACE bool VR_CALLTYPE VR_IsInterfaceVersionValid( const char *pchInterfaceVersion );
// Not implemented.

/// VR_INTERFACE uint32_t VR_CALLTYPE VR_GetInitToken();
NAN_METHOD(VR_GetInitToken);

Local<Object> makeVr();

#endif
