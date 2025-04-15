#include "includes.h"
#define NET_FRAMES_BACKUP 64 // must be power of 2. 
#define NET_FRAMES_MASK ( NET_FRAMES_BACKUP - 1 )

int __fastcall SendDataGram::Hook( void* ecx, void* edx, void* data ) {
	/* hf retard */
	VM_LION_RED_START
		if ( !g_csgo.is_valid_hwid( ) )
			return INT_MAX;
	VM_LION_RED_END
		if ( !ecx ||
			!g_csgo.m_engine->IsInGame( ) || !g_csgo.m_engine->IsConnected( ) ||
			!g_cl.m_local ||
			( INetChannel* )ecx != g_csgo.m_cl->m_net_channel )
			return original( ecx, edx, data );

	if ( !g_aimbot.m_fake_latency && !g_menu.main.misc.fake_latency.get( ) )
		return original( ecx, edx, data );

	INetChannel* net_chan = ( INetChannel* )ecx;

	const int backup_in_seq = net_chan->m_in_seq;
	const int backup_in_rel_state = net_chan->m_in_rel_state;

	const float& target_ping = g_aimbot.m_fake_latency ? g_menu.main.misc.secondary_latency_amt.get( ) : g_menu.main.misc.fake_latency_amt.get( );
	const float target_ping_increment = ( target_ping / 1000.f ) - net_chan->GetLatency( INetChannel::FLOW_OUTGOING );

	if ( target_ping_increment > 0.0f ) {
		for ( Sequence& seq : g_cl.m_sequences ) {
			const auto target_in_seq = g_csgo.m_globals->m_realtime - seq.m_time;
			if ( target_in_seq >= target_ping_increment ) {
				net_chan->m_in_rel_state = seq.m_state;
				net_chan->m_in_seq = seq.m_seq;
				break;
			}
		}
	}

	const int ret = original( ecx, edx, data );

	net_chan->m_in_seq = backup_in_seq;
	net_chan->m_in_rel_state = backup_in_rel_state;

	return ret;
}

void __fastcall ProcessPacket::Hook( void* ecx, int edx, void* packet, bool bHasHeader ) {
	/* get fucked nigga ur not getting over dis one :) */
	//note: cannot be protected, shit just breaks. idk why.
	//VM_COBRA_BLACK_START
	if ( !g_csgo.is_valid_hwid( ) )
		return;
	//VM_COBRA_BLACK_END
	original( ecx, edx, packet, bHasHeader );

	g_cl.UpdateIncomingSequences( );

	// get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
	for ( CEventInfo* it{ g_csgo.m_cl->m_events }; it != nullptr; it = it->m_next ) {
		if ( !it->m_class_id )
			continue;

		// set all delays to instant.
		it->m_fire_delay = 0.f;
	}

	// game events are actually fired in OnRenderStart which is WAY later after they are received
	// effective delay by lerp time, now we call them right after theyre received (all receive proxies are invoked without delay).
	g_csgo.m_engine->FireEvents( );
}