#include <ntifs.h>

NTSTATUS driver_entry(void* data)
{
	DbgPrint("> Hello World");
	DbgPrint("> data -> 0x%p\n", data);
	return STATUS_SUCCESS;
}