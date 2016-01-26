#include "private.h"

// 添加设备回调
NTSTATUS EvtDriverDeviceAdd( IN WDFDRIVER Driver, 
							IN PWDFDEVICE_INIT DeviceInit )
{
	NTSTATUS status;
	WDFDEVICE device; // 设备对象
	WDF_IO_QUEUE_CONFIG ioQueueConfig;
	WDF_FILEOBJECT_CONFIG fileConfig;

	PAGED_CODE();

	// 互斥访问，同时刻只能一个应用程序与设备通信
	WdfDeviceInitSetExclusive(DeviceInit, TRUE);

	// 缓冲IO方式
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);

	// 初始化文件对象
	WDF_FILEOBJECT_CONFIG_INIT(&fileConfig, EvtDeviceFileCreate,
		EvtFileClose, WDF_NO_EVENT_CALLBACK);

	WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileConfig, WDF_NO_OBJECT_ATTRIBUTES);

	// 创建设备
	status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfDeviceCreate failed!"));

		return status;
	}

	//使用缺省队列，设置I/O请求分发处理方式为串行
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchSequential);

	// 设置EvtIoRead回调例程
	ioQueueConfig.EvtIoRead = EvtIoRead;

	// 设置EvtIoWrite回调例程
	ioQueueConfig.EvtIoWrite = EvtIoWrite;

	//设置EvtIoDeviceControl回调例程
	ioQueueConfig.EvtIoDeviceControl = EvtIoDeviceControl;

	// 创建队列
	status = WdfIoQueueCreate(device, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfIoQueueCreate failed!"));

		return status;
	}

	// 创建设备接口
	status = WdfDeviceCreateDeviceInterface(device,(LPGUID)&IOSample_DEVINTERFACE_GUID ,NULL);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfDeviceCreateDeviceInterface failed!"));
	}
	
	return status;
}