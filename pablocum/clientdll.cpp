#include "includes.h"


void __stdcall CreateMove::CreateMove( int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket ) {
	/* uid has not initialized do not let createmove run LOL */
	if ( !g_csgo.is_valid_hwid( ) )
		return;

	original( g_csgo.m_client, 0, sequence_number, input_sample_frametime, active );

	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	auto cmd = g_csgo.m_input->GetUserCmd( sequence_number );
	auto ver_cmd = g_csgo.m_input->GetVerifiedUserCmd( sequence_number );

	// called from CInput::ExtraMouseSample -> return original.
	if ( !cmd || !cmd->m_command_number )
		return;

	if ( g_menu.main.misc.ranks.get( ) && ( cmd->m_buttons & IN_SCORE ) ) {
		static CCSUsrMsg_ServerRankRevealAll msg{ };
		g_csgo.ServerRankRevealAll( &msg );
	}

	g_cl.m_packet = true;

	if ( !g_cl.m_local || !g_cl.m_local->alive( ) ) {
		bSendPacket = true;
		return;
	}

	// invoke move function.
	g_cl.OnTick( cmd );

	bSendPacket = g_cl.m_packet;

	ver_cmd->m_cmd = *cmd;
	ver_cmd->m_crc = cmd->GetChecksum( );

	if ( bSendPacket ) {
		g_cl.m_sent_cmds.push_front( cmd->m_command_number );
	}
	else {
		INetChannel* nci = g_csgo.m_cl->m_net_channel;
		if ( nci ) {
			const int choke = nci->m_choked_packets;

			nci->m_choked_packets = 0;
			nci->SendDatagram( nullptr );
			--nci->m_out_seq;

			nci->m_choked_packets = choke;
		}
	}
}

__declspec( naked ) void __fastcall CreateMove::Hook( void* _this, int, int sequence_number, float input_sample_frametime, bool active )
{
	__asm
	{
		push ebp
		mov  ebp, esp
		push ebx
		push esp
		push dword ptr[ active ]
			push dword ptr[ input_sample_frametime ]
				push dword ptr[ sequence_number ]
					call CreateMove::CreateMove
						pop  ebx
						pop  ebp
						retn 0Ch
	}
}

void Hooks::LevelInitPreEntity( const char* map ) {
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	g_csgo.cl_updaterate->SetValue( rate );
	g_csgo.cl_cmdrate->SetValue( rate );

	g_aimbot.reset( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >( CHLClient::LEVELINITPREENTITY )( this, map );
}

void Hooks::LevelInitPostEntity( ) {
	g_cl.OnMapload( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >( CHLClient::LEVELINITPOSTENTITY )( this );
}

void Hooks::LevelShutdown( ) {
	g_aimbot.reset( );

	g_cl.m_local = nullptr;
	g_cl.m_weapon = nullptr;
	g_cl.m_weapon_info = nullptr;
	g_cl.m_round_end = false;

	g_cl.m_sequences.clear( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >( CHLClient::LEVELSHUTDOWN )( this );
}

/*int Hooks::IN_KeyEvent( int evt, int key, const char* bind ) {
	// see if this key event was fired for the drop bind.
	/*if( bind && FNV1a::get( bind ) == HASH( "drop" ) ) {
		// down.
		if( evt ) {
			g_cl.m_drop = true;
			g_cl.m_drop_query = 2;
			g_cl.print( "drop\n" );
		}

		// up.
		else
			g_cl.m_drop = false;

		// ignore the event.
		return 0;
	}

	return g_hooks.m_client.GetOldMethod< IN_KeyEvent_t >( CHLClient::INKEYEVENT )( this, evt, key, bind );
}*/

void Hooks::FrameStageNotify( Stage_t stage ) {
	/* uid has not initialized do not let framestage run LOL */
	//note: do NOT virtualize check. breaks.
	if ( !g_csgo.is_valid_hwid( ) )
		return;

	// save stage.
	if ( stage != FRAME_START )
		g_cl.m_stage = stage;

	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );
	class Settings {
	public:
		bool remove_recoil_shake = false;
		bool remove_recoil_punch = false;
		float weapon_recoil_scale = 1.0f;
		float view_recoil_tracking = 1.0f;
		bool remove_smoke = false;
		bool remove_flash = false;

		// Getters
		bool get_remove_recoil_shake() const { return remove_recoil_shake; }
		bool get_remove_recoil_punch() const { return remove_recoil_punch; }
		float get_weapon_recoil_scale() const { return weapon_recoil_scale; }
		float get_view_recoil_tracking() const { return view_recoil_tracking; }
		bool get_remove_smoke() const { return remove_smoke; }
		bool get_remove_flash() const { return remove_flash; }
	};

	// Global instance
	Settings g_Settings;
	static ang_t aim_punch{};
	static ang_t view_punch{};
	if ( stage == FRAME_RENDER_START ) {
		static ConVar* r_aspectratio = g_csgo.m_cvar->FindVar( HASH( "r_aspectratio" ) );
		if ( r_aspectratio )
			r_aspectratio->SetValue( g_menu.main.misc.aspect_ratio.get( ) );

		if ( g_csgo.cl_extrapolate && g_csgo.cl_extrapolate->GetInt( ) != 0 )
			g_csgo.cl_extrapolate->SetValue( 0 );

		if (g_cl.m_local) {
			aim_punch = g_cl.m_local->m_aimPunchAngle();
			view_punch = g_cl.m_local->m_viewPunchAngle();
			auto scaled_aim_punch = g_cl.m_local->m_aimPunchAngle() * g_Settings.get_weapon_recoil_scale() * g_Settings.get_view_recoil_tracking();

			if (g_menu.main.visuals.removals.get(1)) {
				g_cl.m_local->m_viewPunchAngle() = { 0,0,0 };
			}

			if (g_menu.main.visuals.removals.get(0)) {
				g_cl.m_local->m_aimPunchAngle() = { 0,0,0 };
			}


		}

		// set radar angles.
		if ( g_cl.m_local && g_cl.m_local->alive( ) && g_csgo.m_input->CAM_IsThirdPerson( ) )
			g_csgo.m_prediction->SetLocalViewAngles( g_cl.m_radar );

		// draw our custom beams.
		g_visuals.DrawBeams( );

		if ( g_csgo.weapon_debug_spread_show && g_cl.m_local && g_cl.m_local->alive( ) )
			g_csgo.weapon_debug_spread_show->SetValue( ( g_menu.main.visuals.force_xhair.get( ) && !g_cl.m_local->m_bIsScoped( ) ) ? 3 : 0 );
	}

	// call og.
	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >( CHLClient::FRAMESTAGENOTIFY )( this, stage );

	switch ( stage ) {
	case FRAME_RENDER_START:
		g_resolver.Override( );
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		g_skins.think( );
		g_inputpred.update_viewmodel( false );
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		g_visuals.NoSmoke( );
		break;
	case FRAME_NET_UPDATE_START:
		if ( g_cl.m_local && g_cl.m_local->alive( ) ) {
			for ( auto& layer : g_cl.m_server_layers[ 0 ] )
				layer.m_owner = g_cl.m_local;

			g_cl.m_local->SetAnimLayers( g_cl.m_server_layers[ 0 ] );
		}
		break;
	case FRAME_NET_UPDATE_END:
		if ( g_cl.m_local && g_cl.m_local->alive( ) )
			g_cl.m_local->GetAnimLayers( g_cl.m_server_layers[ 0 ] );

		// restore non-compressed netvars.
		g_netdata.apply( );

		// update all players.

		for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ ) {
			AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
			if ( !data )
				continue;

			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
			if ( !player || player->m_bIsLocalPlayer( ) ) {
				data->m_records.clear( );
				g_resolver.ResetMisses( i );
				continue;
			}

			data->OnNetUpdate( player );
		}
		break;
	}
}