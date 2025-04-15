#include "includes.h"

bool Hooks::InPrediction( ) {
	Stack stack;
	ang_t* angles;

	// note - dex; first 2 'test al, al' instructions in C_BasePlayer::CalcPlayerView.
	static Address CalcPlayerView_ret1{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C" ) ) };
	static Address CalcPlayerView_ret2{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 08 57 8B CE E8 ? ? ? ? 8B 06" ) ) };
	static Address MaintainSequenceTransitions_ret{ pattern::find( g_csgo.m_client_dll, XOR( "E8 ? ? ? ? F3 0F 10 45 ? 8B 07" ) ) };


	if ( SetupBones::AllowSetup )
		return false;

	if ( UpdateClientAnimations::InUpdate )
		return false;

	if ( stack.ReturnAddress( ) == MaintainSequenceTransitions_ret )
		return false;

	return g_hooks.m_prediction.GetOldMethod< InPrediction_t >( CPrediction::INPREDICTION )( this );
}

void Hooks::RunCommand( Player* ent, CUserCmd* cmd, IMoveHelper* movehelper ) {

	const int tickrate = 1.f / g_csgo.m_globals->m_interval;
	static ConVar* sv_max_usercmd_future_ticks = g_csgo.m_cvar->FindVar( HASH( "sv_max_usercmd_future_ticks" ) );
	if ( cmd->m_tick > ( g_csgo.m_globals->m_tick_count + tickrate + sv_max_usercmd_future_ticks->GetInt( ) ) ) {
		cmd->m_predicted = true;

		ent->SetAbsOrigin( ent->m_vecOrigin( ), true );
		return;
	}

	g_hooks.m_prediction.GetOldMethod< RunCommand_t >( CPrediction::RUNCOMMAND )( this, ent, cmd, movehelper );

	ent->m_vphysicsCollisionState( ) = 0;

	// store non compressed netvars.
	g_netdata.store( );
}