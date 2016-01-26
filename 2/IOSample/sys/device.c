#include "private.h"

// ����豸�ص�
NTSTATUS EvtDriverDeviceAdd( IN WDFDRIVER Driver, 
							IN PWDFDEVICE_INIT DeviceInit )
{
	NTSTATUS status;
	WDFDEVICE device; // �豸����
	WDF_IO_QUEUE_CONFIG ioQueueConfig;
	WDF_FILEOBJECT_CONFIG fileConfig;

	PAGED_CODE();

	// ������ʣ�ͬʱ��ֻ��һ��Ӧ�ó������豸ͨ��
	WdfDeviceInitSetExclusive(DeviceInit, TRUE);

	// ����IO��ʽ
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);

	// ��ʼ���ļ�����
	WDF_FILEOBJECT_CONFIG_INIT(&fileConfig, EvtDeviceFileCreate,
		EvtFileClose, WDF_NO_EVENT_CALLBACK);

	WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileConfig, WDF_NO_OBJECT_ATTRIBUTES);

	// �����豸
	status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfDeviceCreate failed!"));

		return status;
	}

	//ʹ��ȱʡ���У�����I/O����ַ�����ʽΪ����
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchSequential);

	// ����EvtIoRead�ص�����
	ioQueueConfig.EvtIoRead = EvtIoRead;

	// ����EvtIoWrite�ص�����
	ioQueueConfig.EvtIoWrite = EvtIoWrite;

	//����EvtIoDeviceControl�ص�����
	ioQueueConfig.EvtIoDeviceControl = EvtIoDeviceControl;

	// ��������
	status = WdfIoQueueCreate(device, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfIoQueueCreate failed!"));

		return status;
	}

	// �����豸�ӿ�
	status = WdfDeviceCreateDeviceInterface(device,(LPGUID)&IOSample_DEVINTERFACE_GUID ,NULL);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfDeviceCreateDeviceInterface failed!"));
	}
	
	return status;
}