#ifndef _PRIVATE_H
#define _PRIVATE_H

#include <ntddk.h>
#include <wdf.h>

#include "public.h"

// ����豸�ص�����
NTSTATUS EvtDriverDeviceAdd( IN WDFDRIVER Driver,
							IN PWDFDEVICE_INIT DeviceInit);
// ����ж�ػص�����
VOID EvtDriverUnload( IN WDFDRIVER Driver );

#endif