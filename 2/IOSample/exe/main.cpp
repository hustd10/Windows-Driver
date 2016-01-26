#include <Windows.h>
#include <SetupAPI.h>
#include <Guiddef.h>

#include <stdio.h>

// 驱动头文件
#include "public.h"

#pragma comment(lib, "SetupApi.lib")

// 根据设备接口GUID获取设备路径（符号链接名）
// InterfaceGuid : 设备接口GUID的指针
// 如果失败，则返回空指针NULL
PCHAR GetDevicePath( IN  LPGUID InterfaceGuid )
{  
	// 设备信息集
	HDEVINFO DeviceInfoSet; 
	// 设备接口信息
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData; 
	// 指向设备接口详细信息的指针
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = NULL;
	// 函数指向结果返回值
	BOOL bResult;
	// 遍历
	DWORD dwIndex;

	ULONG Length, RequiredLength = 0;

	// 获取指定设备接口类的设备信息集
	DeviceInfoSet = SetupDiGetClassDevs(
		InterfaceGuid,
		NULL,  // Enumerator
		NULL,
		(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE) );
	if( INVALID_HANDLE_VALUE == DeviceInfoSet )
	{
		printf("SetupDiGetClassDevs failed! ErrNo:%d\n", GetLastError());
		return NULL;
	}

	// 枚举设备信息集下的所有设备的设备接口
	for( dwIndex = 0; ; dwIndex++ )
	{
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		bResult = SetupDiEnumDeviceInterfaces(
			DeviceInfoSet,
			NULL,
			InterfaceGuid,
			dwIndex, // 第dwIndex个设备
			&DeviceInterfaceData);
		if( !bResult )
		{
			// 259错误表示遍历完成，这里没有做特殊判断，也会打印出来
			printf("SetupDiEnumDeviceInterfaces failed! ErrNo:%d\n", GetLastError());

			SetupDiDestroyDeviceInfoList(DeviceInfoSet); 
			break;
		}

		// 获取设备接口详细信息
		// 第一次调用，DeviceInterfaceDetailData置为NULL，获取待分配缓冲区的大小
		// 根据MSDN，此次调用返回值为FALSE，而且GetLastError为122，如果进行错误检查，需要注意
		SetupDiGetDeviceInterfaceDetail(
			DeviceInfoSet,
			&DeviceInterfaceData, // 待获取详细信息的设备接口
			NULL,
			0,
			&RequiredLength,
			NULL); 

		DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LMEM_FIXED, RequiredLength);
		if( NULL == DeviceInterfaceDetailData )
		{
			printf("Failed to allocate memory!\n");

			SetupDiDestroyDeviceInfoList(DeviceInfoSet); 
			return NULL;
		}

		DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		Length = RequiredLength;

		// 第二次调用，返回设备接口详细信息
		bResult = SetupDiGetDeviceInterfaceDetail(
			DeviceInfoSet,
			&DeviceInterfaceData,
			DeviceInterfaceDetailData,
			Length,
			&RequiredLength,
			NULL);

		if( !bResult )
		{
			printf("SetupDiGetDeviceInterfaceDetail failed(second)! ErrNo:%d\n", GetLastError());

			SetupDiDestroyDeviceInfoList(DeviceInfoSet); 
			LocalFree(DeviceInterfaceDetailData);
			return NULL;
		}

		return DeviceInterfaceDetailData->DevicePath;
		//printf("DevicePath: %s\n", DeviceInterfaceDetailData->DevicePath);
		LocalFree(DeviceInterfaceDetailData);
	}

	return NULL;
}

int main( int argc, char* argv[] )
{
	PCHAR DevicePath;
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	BOOL bRetVal;

	DevicePath = GetDevicePath((LPGUID)&IOSample_DEVINTERFACE_GUID);

	if( NULL == DevicePath )
		goto exit;

	printf("device path: %s\n", DevicePath);

	hDevice = CreateFile(	DevicePath,
							GENERIC_READ|GENERIC_WRITE,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							NULL );

	if( INVALID_HANDLE_VALUE == hDevice )
	{
		printf("ERROR opening device: (%0x) returned from CreateFile\n", GetLastError());
		goto exit;
	}

	printf("open device successfully\n");

	// 写入
	CHAR bufWrite[] = "Hello World!";
	DWORD bytesWritten;
	bRetVal = WriteFile(hDevice, bufWrite, sizeof(bufWrite), &bytesWritten, NULL );
	if( !bRetVal )
	{
		printf("ERROR writting: (%0x) returned from WriteFile\n", GetLastError());
		goto exit;
	}

	printf("%d characters has been written successfully!\n", bytesWritten);

	// 读取
	CHAR bufRead[21];
	DWORD bytesRead;
	bRetVal = ReadFile(hDevice, bufRead, sizeof(bufRead), &bytesRead, NULL);
	if( !bRetVal )
	{
		printf("ERROR reading: (%0x) returned from ReadFile\n", GetLastError());
		goto exit;
	}

	bufRead[bytesRead] = '\0';
	printf("string: %s\n", bufRead);

	// IO控制，移动文件指针到'W'字符处
	CHAR offset = 6;
	CHAR filePointer;
	bRetVal = DeviceIoControl(hDevice, IOSample_IOCTL_FILE_SEEK, 
		&offset, 1, &filePointer, 1, &bytesRead, NULL);
	if( !bRetVal )
	{
		printf("ERROR: (%0x) returned from DeviceIoControl\n", GetLastError());
		goto exit;
	}

	printf("file pointer now: %d\n", filePointer);

	// 读取
	bRetVal = ReadFile(hDevice, bufRead, sizeof(bufRead), &bytesRead, NULL);
	if( !bRetVal )
	{
		printf("ERROR reading: (%0x) returned from ReadFile\n", GetLastError());
		goto exit;
	}

	bufRead[bytesRead] = '\0';
	printf("new string: %s\n", bufRead);

	CloseHandle(hDevice);

exit:
	system("pause");
}