#ifndef _PRIVATE_H
#define _PRIVATE_H

#include <ntddk.h>
#include <wdf.h>

#include "public.h"

/****************** �������� *******************************/

// ��ں���
NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath );

// ����豸�ص�����
NTSTATUS EvtDriverDeviceAdd( IN WDFDRIVER Driver,
							IN PWDFDEVICE_INIT DeviceInit );

// Create
VOID EvtDeviceFileCreate( IN WDFDEVICE Device,
						 IN WDFREQUEST Request,
						 IN WDFFILEOBJECT FileObject );

// Close
VOID EvtFileClose( IN WDFFILEOBJECT FileObject );

// ���ص�����
VOID EvtIoRead( IN WDFQUEUE Queue,
			   IN WDFREQUEST Request,
			   IN size_t Length );

// д�ص�����
VOID EvtIoWrite( IN WDFQUEUE Queue,
				IN WDFREQUEST Request,
				IN size_t Length );

// IO���ƻص�����
VOID EvtIoDeviceControl( IN WDFQUEUE Queue,
						IN WDFREQUEST Request,
						IN size_t OutputBufferLength,
						IN size_t InputBufferLength,
						IN ULONG IoControlCode );

#endif