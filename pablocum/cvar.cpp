#include "includes.h"

int Hooks::DebugSpreadGetInt( ) {
	return g_hooks.m_debug_spread.GetOldMethod< GetInt_t >( ConVar::GETINT )( this );
}

bool Hooks::NetShowFragmentsGetInt( ) {
	return g_hooks.m_net_show_fragments.GetOldMethod< GetInt_t >(ConVar::GETBOOL)(this);
}
bool Hooks::ExtrapolateGetInt( ) {
	return 0;
}