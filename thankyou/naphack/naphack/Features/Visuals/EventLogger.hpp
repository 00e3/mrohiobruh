#pragma once
#include "../../SDK/sdk.hpp"

class  EventLogger {
public:
	void Main( );
	void PushEvent( std::string msg, Color_f clr, bool visualise = true, std::string prefix = "", bool bConsole = true );

	EventLogger( ) { };
	~EventLogger( ) { };
};

extern EventLogger g_EventLog;