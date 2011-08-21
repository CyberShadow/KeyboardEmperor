#include "KeyboardEmperor.h"

#if _X86_
 #if _WIN32_WINNT==0x0501
  #define OFFSET_DUETIME 0x128
  #define OFFSET_PERIOD  0x130
 #endif
#endif

#if _AMD64_
 #if _WIN32_WINNT==0x0600
  #define OFFSET_DUETIME 0x1C0
  #define OFFSET_PERIOD  0x1C8
 #endif
#endif

#if !defined(OFFSET_DUETIME) || !defined(OFFSET_PERIOD)
 #error Operating system not supported
#endif

VOID
GetDriverObjectList()
{
	DRIVER_OBJECT *driverobj;
	UNICODE_STRING name;
	DEVICE_OBJECT *device;
	PVOID extension;
	PLARGE_INTEGER duetime;
	PLONG period;

	//DbgPrint("Begin enum!\n");

	RtlInitUnicodeString(&name, L"\\Driver\\kbdhid");

	ObReferenceObjectByName(&name, 0, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &driverobj);

	// This is not thread-safe. Hold your breath and hope the device chain won't be updated as this list is traversed.
	for (device = driverobj->DeviceObject; device != NULL; device = device->NextDevice)
	{
		extension = device->DeviceExtension;
		duetime = (PLARGE_INTEGER)((PCHAR)extension + OFFSET_DUETIME);
		period  = (PLONG         )((PCHAR)extension + OFFSET_PERIOD );
		DbgPrint("Before: duetime=%d, period=%d\n", duetime->LowPart, *period);
		*period = 10;
		duetime->LowPart = -2000000;
		DbgPrint("After : duetime=%d, period=%d\n", duetime->LowPart, *period);
	}

	ObDereferenceObject(driverobj);

	//DbgPrint("End enum!\n");
}

DRIVER_INITIALIZE DriverEntry;

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)

{
	GetDriverObjectList();

	DriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}

VOID
DriverUnload(
	IN PDRIVER_OBJECT DriverObject
	)

{
	//DbgPrint("Driver is being unloaded!\n");
}
