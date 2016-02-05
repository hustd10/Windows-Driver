#ifndef _PRIVATE_H
#define _PRIVATE_H

#include <ntddk.h>
#include <wdf.h>

#include "public.h"

// 添加设备回调函数
NTSTATUS EvtDriverDeviceAdd( IN WDFDRIVER Driver,
							IN PWDFDEVICE_INIT DeviceInit);
// 驱动卸载回调函数
VOID EvtDriverUnload( IN WDFDRIVER Driver );

#endif