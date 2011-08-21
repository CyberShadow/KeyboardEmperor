#include <ntddk.h>

// Function prototypes
DRIVER_UNLOAD DriverUnload;
VOID DriverUnload( IN PDRIVER_OBJECT DriverObject );

NTSTATUS
ObReferenceObjectByName (
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE AccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PVOID *Object
    );

extern POBJECT_TYPE* IoDriverObjectType;

