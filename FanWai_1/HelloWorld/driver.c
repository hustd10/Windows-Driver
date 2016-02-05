#include "private.h"

NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath )
{
	NTSTATUS status;
	WDF_DRIVER_CONFIG config;

	// 注册EvtDriverDeviceAdd回调函数
	WDF_DRIVER_CONFIG_INIT(&config, EvtDriverDeviceAdd);

	// 注册EvtDriverUnload回调函数
	config.EvtDriverUnload = EvtDriverUnload;

	// 创建驱动对象
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

	// 创建设备对象
	status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &hDevice);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("EvtDriverDeviceAdd failed.\n"));

		return status;
	}

	// 创建设备接口
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