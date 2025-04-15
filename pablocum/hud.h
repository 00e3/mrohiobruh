#pragma once

struct NoticeText_t {
	wchar_t text[ 512 ];
	int unk0; // 0x400
	float unk1; // 0x404
	float unk2; // 0x408
	int unk3;   // 0x40C
	float time; // 0x410
	int unk4;	// 0x414
	float fade; // 0x418
	int unk5;   // 0x41C
};

struct KillFeed_t {
	PAD( 0x7C );
	CUtlVector< NoticeText_t > notices;
};

class CHudElement {
public:
	__forceinline const char* GetName( ) {
		return util::get_method< const char* ( __thiscall* )( void* ) >( this, 12 )( this );
	}
};

class CHud {
public:
	PAD( 0x1C );
	CUtlVector< CHudElement* > list;

	template<typename T>
	T* FindElement( const char* pName ) {
		typedef T* ( __thiscall* FindElementFN )( void*, const char* );
		static auto FindElement = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28" ) ).as<FindElementFN>( );

		return FindElement( this, pName );
	}
};

class CHudChat {
public:
	void ChatPrintf( const char* fmt, ... ) {
		typedef void( __cdecl* ChatPrintfFN )( void*, int, int, const char*, ... );

		char msg[ 1024 ];
		va_list args;
		va_start( args, fmt );
		vsnprintf( msg, 1024, fmt, args );
		util::get_method<ChatPrintfFN>( this, 26 )( this, 0, 0, fmt );
		va_end( args );
	}
};