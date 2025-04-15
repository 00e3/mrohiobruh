#include "includes.h"

bool Hooks::TempEntities( void* msg ) {
	return g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );
}

bool AllowPacket( int command ) {
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) || !g_csgo.m_engine->IsInGame( ) || g_cl.m_sent_cmds.empty( ) ) {
		if ( !g_cl.m_sent_cmds.empty( ) && !g_cl.m_local )
			g_cl.m_sent_cmds.clear( );
		return true;
	}

	for ( int i = 0; i < g_cl.m_sent_cmds.size( ); ++i ) {
		int cmd = g_cl.m_sent_cmds.at( i );
		if ( !cmd || cmd != command )
			continue;

		g_cl.m_sent_cmds.erase( g_cl.m_sent_cmds.begin( ) + i );
		return true;
	}

	return false;
}

void Hooks::PacketStart( int incoming_sequence, int outgoing_acknowledged ) {
	if ( AllowPacket( outgoing_acknowledged ) )
		return g_hooks.m_client_state.GetOldMethod< PacketStart_t >( CClientState::PACKETSTART )( this, incoming_sequence, outgoing_acknowledged );
}