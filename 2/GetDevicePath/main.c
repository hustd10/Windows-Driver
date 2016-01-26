#include <Windows.h>
#include <SetupAPI.h>
#include <Guiddef.h>

#include <stdio.h>

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
		// 根据MSDN，此次调用返回值为FALSE，而且GetLastError为122，不要进行错误检查
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

		//return DeviceInterfaceDetailData->DevicePath;
		printf("DevicePath: %s\n", DeviceInterfaceDetailData->DevicePath);
		LocalFree(DeviceInterfaceDetailData);
	}

	return NULL;
}

int main( int argc, char* argv[] )
{
	PCHAR DevicePath;

	printf("starting...\n");

	DevicePath = GetDevicePath(&GUID_DEVINTERFACE_VOLUME);

	if( DevicePath != NULL )
		printf("DevicePath: %s\n", DevicePath);

	system("pause");

	return 0;
}