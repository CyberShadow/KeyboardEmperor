#include "KeyboardEmperor.h"

#if _X86_
 #if _WIN32_WINNT==0x0501
  #define OFFSET_DUETIME 0x128
  #define OFFSET_PERIOD  0x130
 #endif
#endif

#if _AMD64_
 #if (_WIN32_WINNT==0x0600) || (_WIN32_WINNT==0x0601)
  #define OFFSET_DUETIME 0x1C0
  #define OFFSET_PERIOD  0x1C8
 #endif
#endif

#if !defined(OFFSET_DUETIME) || !defined(OFFSET_PERIOD)
 #error Operating system not supported
#endif

LARGE_INTEGER old_duetime;
LONG old_period;
BOOLEAN initialization_status;

BOOLEAN
SetKeyboardParameters(LARGE_INTEGER new_duetime, LONG new_period, PLARGE_INTEGER old_duetime, PLONG old_period)
{
	DRIVER_OBJECT *driverobj;
	UNICODE_STRING name;
	DEVICE_OBJECT *device;
	PVOID extension;
	PLARGE_INTEGER duetime;
	PLONG period;

	//DbgPrint("Begin enum!\n");

	RtlInitUnicodeString(&name, L"\\Driver\\kbdhid");

	driverobj = NULL;
	ObReferenceObjectByName(&name, 0, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &driverobj);

	if (!driverobj)
	{
		DbgPrint("kbdhid driver not loaded\n");
		return FALSE;
	}

	if (!driverobj->DeviceObject)
	{
		DbgPrint("kbdhid driver has no devices\n");
		return FALSE;
	}

	// This is not thread-safe. Hold our breath and hope the device chain won't be updated as this list is traversed.
	#pragma warning(disable:28175)
	for (device = driverobj->DeviceObject; device != NULL; device = device->NextDevice)
	{
		extension = device->DeviceExtension;
		duetime = (PLARGE_INTEGER)((PCHAR)extension + OFFSET_DUETIME);
		period  = (PLONG         )((PCHAR)extension + OFFSET_PERIOD );

		if (old_duetime) *old_duetime = *duetime;
		if (old_period ) *old_period  = *period ;
		DbgPrint("Before: duetime=%I64d, period=%d\n", duetime->QuadPart, *period);

		*duetime = new_duetime;
		*period  = new_period;
		DbgPrint("After : duetime=%I64d, period=%d\n", duetime->QuadPart, *period);
	}

	ObDereferenceObject(driverobj);

	//DbgPrint("End enum!\n");

	return TRUE;
}

DRIVER_INITIALIZE DriverEntry;

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)

{
	LARGE_INTEGER new_duetime;

	DriverObject->DriverUnload = DriverUnload;

	new_duetime.QuadPart = -2000000LL;
	initialization_status = SetKeyboardParameters(new_duetime, 10, &old_duetime, &old_period);

	if (initialization_status)
		return STATUS_SUCCESS;
	else
		return STATUS_NOT_FOUND;
}

VOID
DriverUnload(
	IN PDRIVER_OBJECT DriverObject
	)

{
	//DbgPrint("Driver is being unloaded!\n");
	if (initialization_status)
		SetKeyboardParameters(old_duetime, old_period, NULL, NULL);
}
