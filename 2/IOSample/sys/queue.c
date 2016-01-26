#include "private.h"

/****************** 内存标记 *******************************/
#define TAG_IO_BUFFER 0x00000001

/****************** 全局变量 *******************************/

// 固定20个字节大小的缓冲区，模拟文件内容
#define IO_BUFFER_SIZE 20 
PCHAR gFileBuffer = NULL;

// 模拟文件指针
UCHAR gFilePointer = 0; 

// Create
VOID EvtDeviceFileCreate( IN WDFDEVICE Device, 
						 IN WDFREQUEST Request, 
						 IN WDFFILEOBJECT FileObject )
{
	// 在非分页内存上分配缓冲区
	if( NULL == gFileBuffer )
	{
		gFileBuffer = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, IO_BUFFER_SIZE, TAG_IO_BUFFER);
	}

	// 分配失败
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
	// 释放缓冲区
	if( NULL != gFileBuffer )
	{
		ExFreePoolWithTag((PVOID)gFileBuffer, TAG_IO_BUFFER);
		gFileBuffer = NULL;
	}

	gFilePointer = 0;
}

// 读回调
VOID EvtIoRead( IN WDFQUEUE Queue, 
			   IN WDFREQUEST Request, 
			   IN size_t Length )
{
	NTSTATUS status;
	PVOID outBuf; // 输出缓冲区
	size_t bufLength; // 输出缓冲区大小
	ULONG_PTR bytesRead = 0; // 实际读取的字节数

	// 获取输出缓冲区地址
	status = WdfRequestRetrieveOutputBuffer(Request, 0, &outBuf, &bufLength);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtIoRead] WdfRequestRetrieveOutputBuffer failed!"));

		WdfRequestComplete(Request, status);
		return ;
	}

	// 实际读取的字节数与文件指针有关
	bytesRead = ((IO_BUFFER_SIZE - gFilePointer) < Length ) ? 
		IO_BUFFER_SIZE - gFilePointer : Length;

	// 检查参数
	if( bufLength < bytesRead )
	{
		WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
		return;
	}

	// 文件指针已经到末尾
	if( 0 == bytesRead )
	{
		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
		return;
	}

	// 拷贝内容到输出缓冲区
	RtlCopyMemory(outBuf, gFileBuffer+gFilePointer, bytesRead);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bytesRead);
}

// 写回调
VOID EvtIoWrite( IN WDFQUEUE Queue, 
				IN WDFREQUEST Request, 
				IN size_t Length )
{
	NTSTATUS status;
	PVOID inBuf;
	size_t bufLength;
	ULONG_PTR bytesWritten = 0; // 实际写入的字节数

	// 获取输入缓冲区
	status = WdfRequestRetrieveInputBuffer(Request, 0, &inBuf, &bufLength);
	if( !NT_SUCCESS(status) )
	{
		KdPrint(("[EvtIoWrite] WdfRequestRetrieveInputBuffer failed!"));

		WdfRequestComplete(Request, status);
		return ;
	}

	// 实际写入的字节数与文件指针位置有关
	bytesWritten = ((IO_BUFFER_SIZE - gFilePointer) < Length ) ? 
		IO_BUFFER_SIZE - gFilePointer : Length;

	// 文件指针已经到末尾
	if( 0 == bytesWritten )
	{
		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
		return;
	}

	// 拷贝内容
	RtlCopyMemory(gFileBuffer+gFilePointer, inBuf, bytesWritten);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bytesWritten);
}

// IO控制
VOID EvtIoDeviceControl( IN WDFQUEUE Queue, 
						IN WDFREQUEST Request, 
						IN size_t OutputBufferLength, 
						IN size_t InputBufferLength, 
						IN ULONG IoControlCode )
{
	NTSTATUS status;
	PVOID inBuf; // 输入缓冲区地址
	PVOID outBuf; // 输出缓冲区地址
	CHAR offset; // 指针移动量

	switch( IoControlCode )
	{
		// 移动文件指针
	case IOSample_IOCTL_FILE_SEEK:

		// 检查输入参数
		if( 0 == InputBufferLength || 0 == OutputBufferLength )
		{
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
			return;
		}

		// 获取输入缓冲区地址
		status = WdfRequestRetrieveInputBuffer(Request, 1, &inBuf, NULL);
		if( !NT_SUCCESS(status) )
		{
			KdPrint(("[EvtIoDeviceControl] WdfRequestRetrieveInputBuffer failed!"));

			WdfRequestComplete(Request, status);
			return ;
		}

		// 获取输出缓冲区地址
		status = WdfRequestRetrieveOutputBuffer(Request, 1, &outBuf, NULL);
		if( !NT_SUCCESS(status) )
		{
			KdPrint(("[EvtIoDeviceControl] WdfRequestRetrieveOutputBuffer failed!"));

			WdfRequestComplete(Request, status);
			return ;
		}

		offset = *(CHAR*)inBuf;

		// 移动文件指针
		if( gFilePointer + offset < 0 )
			gFilePointer = 0;
		else if( gFilePointer + offset > IO_BUFFER_SIZE )
			gFilePointer = IO_BUFFER_SIZE;
		else
			gFilePointer = gFilePointer + offset;

		// 新的文件指针位置
		*(CHAR*)outBuf = gFilePointer;

		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 1);

		break;

	default:
		WdfRequestCompleteWithInformation(Request, STATUS_INVALID_DEVICE_REQUEST, 0);
		break;
	}
}