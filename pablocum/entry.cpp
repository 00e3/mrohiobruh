#include "includes.h"

int __stdcall DllMain( HMODULE self, ulong_t reason, void* reserved ) {
	if ( reason != DLL_PROCESS_ATTACH )
		return 1;

	HANDLE thread = CreateThread( nullptr, 0, Client::init, self, 0, nullptr );
	if ( !thread )
		return 0;

	CloseHandle( thread );
	return 1;
}