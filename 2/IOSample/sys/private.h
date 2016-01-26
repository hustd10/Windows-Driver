#ifndef _PRIVATE_H
#define _PRIVATE_H

#include <ntddk.h>
#include <wdf.h>

#include "public.h"

/****************** 函数声明 *******************************/

// 入口函数
NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath );

// 添加设备回调函数
NTSTATUS EvtDriverDeviceAdd( IN WDFDRIVER Driver,
							IN PWDFDEVICE_INIT DeviceInit );

// Create
VOID EvtDeviceFileCreate( IN WDFDEVICE Device,
						 IN WDFREQUEST Request,
						 IN WDFFILEOBJECT FileObject );

// Close
VOID EvtFileClose( IN WDFFILEOBJECT FileObject );

// 读回调函数
VOID EvtIoRead( IN WDFQUEUE Queue,
			   IN WDFREQUEST Request,
			   IN size_t Length );

// 写回调函数
VOID EvtIoWrite( IN WDFQUEUE Queue,
				IN WDFREQUEST Request,
				IN size_t Length );

// IO控制回调函数
VOID EvtIoDeviceControl( IN WDFQUEUE Queue,
						IN WDFREQUEST Request,
						IN size_t OutputBufferLength,
						IN size_t InputBufferLength,
						IN ULONG IoControlCode );

#endif