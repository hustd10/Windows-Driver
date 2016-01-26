#include "private.h"

/****************** �ڴ��� *******************************/
#define TAG_IO_BUFFER 0x00000001

/****************** ȫ�ֱ��� *******************************/

// �̶�20���ֽڴ�С�Ļ�������ģ���ļ�����
#define IO_BUFFER_SIZE 20 
PCHAR gFileBuffer = NULL;

// ģ���ļ�ָ��
UCHAR gFilePointer = 0; 

// Create
VOID EvtDeviceFileCreate( IN WDFDEVICE Device, 
						 IN WDFREQUEST Request, 
						 IN WDFFILEOBJECT FileObject )
{
	// �ڷǷ�ҳ�ڴ��Ϸ��仺����
	if( NULL == gFileBuffer )
	{
		gFileBuffer = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, IO_BUFFER_SIZE, TAG_IO_BUFFER);
	}

	// ����ʧ��
	if( NULL == gFileBuffer ) 
	{
		KdPrint(("[EvtDriverDeviceAdd] WdfMemoryCreate failed!"));

		WdfRequestComplete(Request, STATUS_UNSUCCESSFUL);
	}

	WdfRequestComplete(Request, STATUS_SUCCESS);
}

// Close
VOID EvtFileClose( IN WDFFILEOBJECT FileObject )
{
	// �ͷŻ�����
	if( NULL != gFileBuffer )
	{
		ExFreePoolWithTag((PVOID)gFileBuffer, TAG_IO_BUFFER);
		gFileBuffer = NULL;
	}

	gFilePointer = 0;
}

// ���ص�
VOID EvtIoRead( IN WDFQUEUE Queue, 
			   IN WDFREQUEST Request, 
			   IN size_t Length )
{
	NTSTATUS status;
	PVOID outBuf; // ���������
	size_t bufLength; // �����������С
	ULONG_PTR bytesRead = 0; // ʵ�ʶ�ȡ���ֽ���

	// ��ȡ�����������ַ
	status = WdfRequestRetrieveOutputBuffer(Request, 0, &outBuf, &bufLength);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtIoRead] WdfRequestRetrieveOutputBuffer failed!"));

		WdfRequestComplete(Request, status);
		return ;
	}

	// ʵ�ʶ�ȡ���ֽ������ļ�ָ���й�
	bytesRead = ((IO_BUFFER_SIZE - gFilePointer) < Length ) ? 
		IO_BUFFER_SIZE - gFilePointer : Length;

	// ������
	if( bufLength < bytesRead )
	{
		WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
		return;
	}

	// �ļ�ָ���Ѿ���ĩβ
	if( 0 == bytesRead )
	{
		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
		return;
	}

	// �������ݵ����������
	RtlCopyMemory(outBuf, gFileBuffer+gFilePointer, bytesRead);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bytesRead);
}

// д�ص�
VOID EvtIoWrite( IN WDFQUEUE Queue, 
				IN WDFREQUEST Request, 
				IN size_t Length )
{
	NTSTATUS status;
	PVOID inBuf;
	size_t bufLength;
	ULONG_PTR bytesWritten = 0; // ʵ��д����ֽ���

	// ��ȡ���뻺����
	status = WdfRequestRetrieveInputBuffer(Request, 0, &inBuf, &bufLength);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtIoWrite] WdfRequestRetrieveInputBuffer failed!"));

		WdfRequestComplete(Request, status);
		return ;
	}

	// ʵ��д����ֽ������ļ�ָ��λ���й�
	bytesWritten = ((IO_BUFFER_SIZE - gFilePointer) < Length ) ? 
		IO_BUFFER_SIZE - gFilePointer : Length;

	// �ļ�ָ���Ѿ���ĩβ
	if( 0 == bytesWritten )
	{
		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
		return;
	}

	// ��������
	RtlCopyMemory(gFileBuffer+gFilePointer, inBuf, bytesWritten);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bytesWritten);
}

// IO����
VOID EvtIoDeviceControl( IN WDFQUEUE Queue, 
						IN WDFREQUEST Request, 
						IN size_t OutputBufferLength, 
						IN size_t InputBufferLength, 
						IN ULONG IoControlCode )
{
	NTSTATUS status;
	PVOID inBuf; // ���뻺������ַ
	PVOID outBuf; // �����������ַ
	CHAR offset; // ָ���ƶ���

	switch( IoControlCode )
	{
		// �ƶ��ļ�ָ��
	case IOSample_IOCTL_FILE_SEEK:

		// ����������
		if( 0 == InputBufferLength || 0 == OutputBufferLength )
		{
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
			return;
		}

		// ��ȡ���뻺������ַ
		status = WdfRequestRetrieveInputBuffer(Request, 1, &inBuf, NULL);
		if( !NT_SUCCESS(status) )
		{
			KdPrint(("[EvtIoDeviceControl] WdfRequestRetrieveInputBuffer failed!"));

			WdfRequestComplete(Request, status);
			return ;
		}

		// ��ȡ�����������ַ
		status = WdfRequestRetrieveOutputBuffer(Request, 1, &outBuf, NULL);
		if( !NT_SUCCESS(status) )
		{
			KdPrint(("[EvtIoDeviceControl] WdfRequestRetrieveOutputBuffer failed!"));

			WdfRequestComplete(Request, status);
			return ;
		}

		offset = *(CHAR*)inBuf;

		// �ƶ��ļ�ָ��
		if( gFilePointer + offset < 0 )
			gFilePointer = 0;
		else if( gFilePointer + offset > IO_BUFFER_SIZE )
			gFilePointer = IO_BUFFER_SIZE;
		else
			gFilePointer = gFilePointer + offset;

		// �µ��ļ�ָ��λ��
		*(CHAR*)outBuf = gFilePointer;

		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 1);

		break;

	default:
		WdfRequestCompleteWithInformation(Request, STATUS_INVALID_DEVICE_REQUEST, 0);
		break;
	}
}