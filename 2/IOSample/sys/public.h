#ifndef _PUBLIC_H
#define _PUBLIC_H

#include <InitGuid.h>

//设备接口 {95CC5F28-95D3-44fd-8DCD-264D4CE5FECC}
DEFINE_GUID(IOSample_DEVINTERFACE_GUID, 
			0x95cc5f28, 0x95d3, 0x44fd, 0x8d, 0xcd, 0x26, 0x4d, 0x4c, 0xe5, 0xfe, 0xcc);

// IO控制码

// 移动文件指针
#define IOSample_IOCTL_FILE_SEEK CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif