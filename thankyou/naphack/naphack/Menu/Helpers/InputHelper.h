#pragma once

namespace InputHelper {
	inline bool KeyState[ 256 ];
	inline bool PrevKeyState[ 256 ];

	inline int Scroll;

	void Update( );
	bool Pressed( int key );
	bool Down( int key );
	bool Released( int key );
}