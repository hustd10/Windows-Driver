#include <Windows.h>
#include <SetupAPI.h>
#include <Guiddef.h>

#include <stdio.h>

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
		// ����MSDN���˴ε��÷���ֵΪFALSE������GetLastErrorΪ122����Ҫ���д�����
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