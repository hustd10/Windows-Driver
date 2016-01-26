#include <Windows.h>
#include <SetupAPI.h>
#include <Guiddef.h>

#include <stdio.h>

// ����ͷ�ļ�
#include "public.h"

#pragma comment(lib, "SetupApi.lib")

// �����豸�ӿ�GUID��ȡ�豸·����������������
// InterfaceGuid : �豸�ӿ�GUID��ָ��
// ���ʧ�ܣ��򷵻ؿ�ָ��NULL
PCHAR GetDevicePath( IN  LPGUID InterfaceGuid )
{  
	// �豸��Ϣ��
	HDEVINFO DeviceInfoSet; 
	// �豸�ӿ���Ϣ
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData; 
	// ָ���豸�ӿ���ϸ��Ϣ��ָ��
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = NULL;
	// ����ָ��������ֵ
	BOOL bResult;
	// ����
	DWORD dwIndex;

	ULONG Length, RequiredLength = 0;

	// ��ȡָ���豸�ӿ�����豸��Ϣ��
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

	// ö���豸��Ϣ���µ������豸���豸�ӿ�
	for( dwIndex = 0; ; dwIndex++ )
	{
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		bResult = SetupDiEnumDeviceInterfaces(
			DeviceInfoSet,
			NULL,
			InterfaceGuid,
			dwIndex, // ��dwIndex���豸
			&DeviceInterfaceData);
		if( !bResult )
		{
			// 259�����ʾ������ɣ�����û���������жϣ�Ҳ���ӡ����
			printf("SetupDiEnumDeviceInterfaces failed! ErrNo:%d\n", GetLastError());

			SetupDiDestroyDeviceInfoList(DeviceInfoSet); 
			break;
		}

		// ��ȡ�豸�ӿ���ϸ��Ϣ
		// ��һ�ε��ã�DeviceInterfaceDetailData��ΪNULL����ȡ�����仺�����Ĵ�С
		// ����MSDN���˴ε��÷���ֵΪFALSE������GetLastErrorΪ122��������д����飬��Ҫע��
		SetupDiGetDeviceInterfaceDetail(
			DeviceInfoSet,
			&DeviceInterfaceData, // ����ȡ��ϸ��Ϣ���豸�ӿ�
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

		// �ڶ��ε��ã������豸�ӿ���ϸ��Ϣ
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

	// д��
	CHAR bufWrite[] = "Hello World!";
	DWORD bytesWritten;
	bRetVal = WriteFile(hDevice, bufWrite, sizeof(bufWrite), &bytesWritten, NULL );
	if( !bRetVal )
	{
		printf("ERROR writting: (%0x) returned from WriteFile\n", GetLastError());
		goto exit;
	}

	printf("%d characters has been written successfully!\n", bytesWritten);

	// ��ȡ
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

	// IO���ƣ��ƶ��ļ�ָ�뵽'W'�ַ���
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

	// ��ȡ
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