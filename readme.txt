����ָ��������ʱָ��ĵ�ַ
s_TestUfiBuffer �ڷ���UFIָ��ʱ,�ñ��������˷��ص�����.
deviceInstance  �����а������豸��������Ϣ. λ��0x20204520��ַ
s_EnumEntries   �����а�����ö�ٹ�����Ҫ���õĺ���.

1.��ȡ�豸������
USB_HostEhciTaskFunction
	USB_HostAttachDevice
		USB_HostProcessState (usb_host_devices.c) ����ö������
			USB_HOSTStandardSetGetDescriptor
			   USB_HostCh9RequestCommon
			      USB_HostSendSetup
					  USB_HostEhciWritePipe
					     USB_HostEhciStartAsync
						 |
USB_HostEnumerationTransferCallback
    USB_HostProcessCallback
	    USB_HostEhciIoctl
		
		
