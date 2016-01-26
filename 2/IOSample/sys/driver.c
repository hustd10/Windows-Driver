#include "private.h"

NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath )
{
	NTSTATUS status;
	WDF_DRIVER_CONFIG config;

	// 注册EvtDriverDeviceAdd回调函数
	WDF_DRIVER_CONFIG_INIT(&config, EvtDriverDeviceAdd);

	// 创建驱动对象
	status = WdfDriverCreate(DriverObject, RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE );

	return status;
}