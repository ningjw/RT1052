各个指针在运行时指向的地址
s_TestUfiBuffer 在发送UFI指令时,该变量保存了返回的数据.
deviceInstance  变量中包含了设备描述符信息. 位于0x20204520地址
s_EnumEntries   数组中包含了枚举过程需要调用的函数.

1.获取设备描述符
USB_HostEhciTaskFunction
	USB_HostAttachDevice
		USB_HostProcessState (usb_host_devices.c) 进行枚举流程
			USB_HOSTStandardSetGetDescriptor
			   USB_HostCh9RequestCommon
			      USB_HostSendSetup
					  USB_HostEhciWritePipe
					     USB_HostEhciStartAsync
						 |
USB_HostEnumerationTransferCallback
    USB_HostProcessCallback
	    USB_HostEhciIoctl
		
		
