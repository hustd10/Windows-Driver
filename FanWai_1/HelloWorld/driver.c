#include "private.h"

NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath )
{
	NTSTATUS status;
	WDF_DRIVER_CONFIG config;

	// ע��EvtDriverDeviceAdd�ص�����
	WDF_DRIVER_CONFIG_INIT(&config, EvtDriverDeviceAdd);

	// ע��EvtDriverUnload�ص�����
	config.EvtDriverUnload = EvtDriverUnload;

	// ������������
	status = WdfDriverCreate( DriverObject, RegistryPath, 
		WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);

	return status;
}

NTSTATUS EvtDriverDeviceAdd(IN WDFDRIVER Driver, 
							IN PWDFDEVICE_INIT DeviceInit)
{
	NTSTATUS status;
	WDFDEVICE hDevice;

	PAGED_CODE();

	// �����豸����
	status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &hDevice);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("EvtDriverDeviceAdd failed.\n"));

		return status;
	}

	// �����豸�ӿ�
	status = WdfDeviceCreateDeviceInterface(hDevice,(LPGUID)&IOSample_DEVINTERFACE_GUID ,NULL);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfDeviceCreateDeviceInterface failed!"));
	}
	
	KdPrint(("Hello World!\n"));
	
	return status;
}

VOID EvtDriverUnload( IN WDFDRIVER Driver )
{
	KdPrint(("Thanks For Using!\n"));
}