#include "includes.h"
#include "minhook/minhook.h"

Hooks                g_hooks{ };;
CustomEntityListener g_custom_entity_listener{ };;

void Pitch_proxy( CRecvProxyData* data, Address ptr, Address out ) {
	// normalize this fucker.
	math::NormalizeAngle( data->m_Value.m_Float );

	// clamp to remove retardedness.
	math::clamp( data->m_Value.m_Float, -90.f, 90.f );

	// call original netvar proxy.
	if ( g_hooks.m_Pitch_original )
		g_hooks.m_Pitch_original( data, ptr, out );
}

void Force_proxy( CRecvProxyData* data, Address ptr, Address out ) {
	// convert to ragdoll.
	Ragdoll* ragdoll = ptr.as< Ragdoll* >( );

	// get ragdoll owner.
	Player* player = ragdoll->GetPlayer( );

	// we only want this happening to noobs we kill.
	if ( g_menu.main.misc.ragdoll_force.get( ) && g_cl.m_local && player && player->enemy( g_cl.m_local ) ) {
		// get m_vecForce.
		vec3_t vel = { data->m_Value.m_Vector[ 0 ], data->m_Value.m_Vector[ 1 ], data->m_Value.m_Vector[ 2 ] };

		// give some speed to all directions.
		vel *= 1000.f;

		// boost z up a bit.
		if ( vel.z <= 1.f )
			vel.z = 2.f;

		vel.z *= 2.f;

		// don't want crazy values for this... probably unlikely though?
		math::clamp( vel.x, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );
		math::clamp( vel.y, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );
		math::clamp( vel.z, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );

		// set new velocity.
		data->m_Value.m_Vector[ 0 ] = vel.x;
		data->m_Value.m_Vector[ 1 ] = vel.y;
		data->m_Value.m_Vector[ 2 ] = vel.z;
	}

	if ( g_hooks.m_Force_original )
		g_hooks.m_Force_original( data, ptr, out );
}

void __fastcall VoiceData::Hook( void* ecx, void* edx, const CSVCMsg_VoiceData& msg ) {
	int iEntity = msg.client + 1;

	if ( iEntity == g_csgo.m_engine->GetLocalPlayer( ) ) {
		return original( ecx, edx, msg );
	}

	player_info_t info{};

	if ( g_csgo.m_engine->GetPlayerInfo( iEntity, &info ) ) {
		AimPlayer* data = &g_aimbot.m_players[ iEntity - 1 ];

		const int offset = msg.uncompressed_sample_offset;

		if ( g_menu.main.misc.print_network.get( ) )
			g_cl.print( "RECEIVING PACKET: player: %s, xuid_high: %i, xuid_low: %i, sequence_byte: %i, section_number: %i, uncompressed offset: %i, svtick: %i\n",
				info.m_name,
				msg.xuid_high,
				msg.xuid_low,
				msg.sequence_bytes,
				msg.section_number,
				msg.uncompressed_sample_offset,
				g_csgo.m_cl->m_server_tick );

		if ( std::abs( offset - g_csgo.m_cl->m_server_tick ) <= 64 )
			data->m_cheat = Cheats::MONEYBOT;

		/*if ( g_menu.main.misc.print_network.get( ) ) {
			VoiceDataWrapper* packetData = &msg->GetData( );
			if ( packetData && packetData->sectionNumber != 0 && packetData->sequenceBytes != 0
				&& packetData->uncompressedSampleOffset != 0 ) {
				CustomVoiceData* customData = ( CustomVoiceData* )msg->voice_data;
				if ( customData ) {
					g_cl.print( "name: %s id: %s flt: %f int: %i ang: %f %f %f", customData->cheatName, customData->packetIdentifier, customData->testFloat, customData->testInt, customData->testAng.x, customData->testAng.y, customData->testAng.z );
				}
			}
		}*/
	}

	return original( ecx, edx, msg );
}

bool __fastcall SkipAnimationFrame::Hook( void* ecx, void* edx ) {
	return false;
}

void SetupBones::CorrectMatrix( int i, int size, matrix3x4_t* matrix, vec3_t origin ) {
	if ( !matrix )
		return;

	for ( int j = 0; j < size; j++ ) {
		matrix[ j ] = SetupBones::BoneCache[ i - 1 ][ j ];
		matrix[ j ].SetOrigin( matrix[ j ].GetOrigin( ) - SetupBones::BoneOrigin[ i - 1 ] + origin );
	}
}

void __fastcall MaintainSequenceTransitions::Hook( void* _this, void* boneSetup, float flCycle, void* pos, void* q ) {
	return;
}

void __fastcall AttachmentHelper::Hook( void* ecx, void* edx, void* studiohdr ) {
	if ( Allow )
		return original( ecx, edx, studiohdr );
}

bool __fastcall SetupBones::Hook( void* ecx, void* edx, BoneArray* bone_to_world_out, int max_bones, int bone_mask, float curtime ) {
	Player* pl = reinterpret_cast< Player* >( reinterpret_cast< uintptr_t >( ecx ) - 0x4 );
	if ( !pl || !pl->IsPlayer( ) || !pl->alive( ) || pl->dormant( ) )
		return original( ecx, edx, bone_to_world_out, max_bones, bone_mask, curtime );

	//if (AllowSetup) {
	//	AttachmentHelper::Allow = false;
	//	const bool& setup = original(ecx, edx, bone_to_world_out, max_bones, bone_mask, curtime);
	//	AttachmentHelper::Allow = true;

	//	return setup;
	//}

	if ( bone_to_world_out && max_bones > 0 && max_bones <= 128 )
		pl->GetBones( bone_to_world_out );

	return true;
}

void __fastcall InterpolateServerEntities::Hook( ) {
	original( );

	struct LerpData {
		float poses[ 24 ];
		C_AnimationLayer layers[ 13 ];
		float abs_yaw;
	};

	static LerpData lerp;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !player || !player->alive( ) )
			continue;

		CStudioHdr* studiohdr = player->GetModelPtr( );
		if ( !studiohdr )
			continue;

		CBoneCache& cache = player->m_BoneCache( );
		if ( !cache.m_pCachedBones )
			continue;

		if ( player == g_cl.m_local ) {
			for ( auto& layer : g_cl.m_server_layers[ 1 ] )
				layer.m_owner = player;

			player->SetAnimLayers( g_cl.m_server_layers[ 1 ] );

			rebuilt_animstate_t* data = &g_cl.m_anim_data.m_data[ i - 1 ].m_server_data;

			static rebuilt_animstate_t data_copy;

			/* make a copy of the animation data*/
			g_cl.HandleAnimationCopy( data, data_copy );

			if ( g_menu.main.misc.animations.get( 3 ) )
				data_copy.m_poses[ 12 ] = ( g_cl.m_angle.x / 90.f ) + 0.5;

			if ( g_menu.main.misc.animations.get( 1 ) && data_copy.m_landing && data_copy.m_ground )
				data_copy.m_poses[ 12 ] = 0.5f;

			if ( g_menu.main.misc.animations.get( 0 ) && !data_copy.m_ground )
				data_copy.m_layers[ 4 ].m_weight = 1.f;

			g_cl.SetAngles( &data_copy );

			g_bones.Setup( player, BONE_USED_BY_ANYTHING, cache.m_pCachedBones, cache.m_CachedBoneCount, false, false );

			SetupBones::CorrectMatrix( i, 128, g_cl.m_fake_bones, player->GetAbsOrigin( ) );
		}
		else
			SetupBones::CorrectMatrix( i, cache.m_CachedBoneCount, cache.m_pCachedBones, player->GetAbsOrigin( ) );

		g_csgo.AttachmentHelper.as< AttachmentHelperFn  >( )( player, studiohdr );
	}
}

void __fastcall ModifyEyePosition::Hook( void* ecx, uintptr_t, vec3_t& position ) {
	CCSGOPlayerAnimState* state = ( CCSGOPlayerAnimState* )ecx;

	if ( !state || !state->m_player || !state->m_player->alive( ) )
		return;

	if ( state->m_player != g_cl.m_local )
		return;

	static uintptr_t wantedReturnAddress = pattern::find( g_csgo.m_client_dll, "8B 06 8B CE FF 90 ? ? ? ? 85 C0 74 4E" ); /* C_CSPlayer::CalcView */
	if ( ( uintptr_t )_ReturnAddress( ) == wantedReturnAddress )
		return;

	position = g_cl.m_shoot_pos;
}

void __fastcall ExtraBonesProcessing::Hook( void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7 ) {
	return;
}

void UpdateClientAnimations::DoUpdate( Player* player ) {
	if ( !player || !player->alive( ) || player->dormant( ) )
		return;

	static C_AnimationLayer layers[ 13 ];
	player->GetAnimLayers( layers );

	InUpdate = true;

	const bool backup = player->m_bClientSideAnimation( );

	player->m_bClientSideAnimation( ) = true;

	original( player );

	player->m_bClientSideAnimation( ) = backup;

	InUpdate = false;

	player->SetAnimLayers( layers );

	player->InvalidateAllPhysics( );
}

void __fastcall UpdateClientAnimations::Hook( void* ecx )
{
	Player* pl = ( Player* )ecx;
	if ( !pl || !pl->IsPlayer( ) || !pl->alive( ) || pl->dormant( ) )
		return original( ecx );
}

int __fastcall InterpolatePart::Hook( Entity* ent, void* edx, float& curtime, vec3_t& old_origin, ang_t& old_angles, int& no_more_changes ) {
	Player* player = reinterpret_cast< Player* >( ent );
	if ( !player || !player->IsPlayer( ) || player->m_bIsLocalPlayer( ) )
		return original( ent, edx, curtime, old_origin, old_angles, no_more_changes );

	const vec3_t origin = player->m_vecOrigin( );

	const int& ret = original( ent, edx, curtime, old_origin, old_angles, no_more_changes );

	player->SetAbsOrigin( origin, true );

	return ret;
}

ang_t* __fastcall GetEyeAngles::Hook( void* ecx, void* edx ) {
	Player* pl = ( Player* )ecx;

	static auto returnaddr1 = pattern::find( g_csgo.m_client_dll, XOR( "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?" ) );
	static auto returnaddr2 = pattern::find( g_csgo.m_client_dll, XOR( "F3 0F 10 55 ? 51 8B 8E ? ? ? ?" ) );
	static auto returnaddr3 = pattern::find( g_csgo.m_client_dll, XOR( "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5" ) );

	if ( ( _ReturnAddress( ) != returnaddr1 && _ReturnAddress( ) != returnaddr2 && _ReturnAddress( ) != returnaddr3 ) )
		return original( ecx, edx );

	else if ( !pl || pl != g_cl.m_local )
		return original( ecx, edx );

	return &g_cl.m_angle;
}

void __cdecl CLMove::Hook( float fSamples, bool bFinalTick ) {
	if ( !g_csgo.m_engine->IsConnected( ) || !g_csgo.m_engine->IsInGame( ) || !g_cl.m_local || !g_cl.m_local->alive( ) ) {
		NextDTTime = 0.f;
		ChargedTicks = 0;
		return original( fSamples, bFinalTick );;
	}

	WantedTicks = g_menu.main.aimbot.firerate.get( );

	if ( NextDTTime <= g_csgo.m_globals->m_realtime && Enabled && g_cl.m_old_packet ) {
		if ( ChargedTicks < WantedTicks ) {
			++ChargedTicks;
			return;
		}
	}

	ShiftedTick = false;

	original( fSamples, bFinalTick );

	ShiftedTick = true;

	while ( ( ChargedTicks + g_csgo.m_cl->m_choked_commands ) > g_cl.m_max_lag )
		--ChargedTicks;

	if ( g_cl.m_shot || ( !Enabled && ChargedTicks ) ) {
		if ( Enabled )
			NextDTTime = g_csgo.m_globals->m_realtime + 0.5f;

		while ( ChargedTicks ) {
			original( fSamples, bFinalTick );
			--ChargedTicks;
		}
	}
}

void __fastcall FireEvents::Hook( ) {
	//if ( !g_csgo.m_cl )
	//	return original( );

	//while ( g_csgo.m_cl->m_events )
	//{
	//	if ( g_csgo.m_cl->m_events->m_class_id == 171 )
	//		g_csgo.m_cl->m_events->m_fire_delay = 0.0f;

	//	g_csgo.m_cl->m_events = g_csgo.m_cl->m_events->m_next;
	//}

	return original( );
}

void Hooks::init( ) {
	// hook wndproc.
	m_old_wndproc = ( WNDPROC )g_winapi.SetWindowLongA( g_csgo.m_game->m_hWindow, GWL_WNDPROC, util::force_cast< LONG >( Hooks::WndProc ) );

	// setup normal VMT hooks.
	m_panel.init( g_csgo.m_panel );
	m_panel.add( IPanel::PAINTTRAVERSE, util::force_cast( &Hooks::PaintTraverse ) );

	m_client.init( g_csgo.m_client );
	m_client.add( CHLClient::LEVELINITPREENTITY, util::force_cast( &Hooks::LevelInitPreEntity ) );
	m_client.add( CHLClient::LEVELINITPOSTENTITY, util::force_cast( &Hooks::LevelInitPostEntity ) );
	m_client.add( CHLClient::LEVELSHUTDOWN, util::force_cast( &Hooks::LevelShutdown ) );
	//m_client.add( CHLClient::INKEYEVENT, util::force_cast( &Hooks::IN_KeyEvent ) );
	m_client.add( CHLClient::FRAMESTAGENOTIFY, util::force_cast( &Hooks::FrameStageNotify ) );

	m_engine.init( g_csgo.m_engine );
	//m_engine.add(IVEngineClient::ISCONNECTED, util::force_cast(&Hooks::IsConnected));
	m_engine.add( IVEngineClient::ISHLTV, util::force_cast( &Hooks::IsHLTV ) );

	m_engine_sound.init( g_csgo.m_sound );
	m_engine_sound.add( IEngineSound::EMITSOUND, util::force_cast( &Hooks::EmitSound ) );

	m_prediction.init( g_csgo.m_prediction );
	m_prediction.add( CPrediction::INPREDICTION, util::force_cast( &Hooks::InPrediction ) );
	m_prediction.add( CPrediction::RUNCOMMAND, util::force_cast( &Hooks::RunCommand ) );

	m_client_mode.init( g_csgo.m_client_mode );
	m_client_mode.add( IClientMode::SHOULDDRAWPARTICLES, util::force_cast( &Hooks::ShouldDrawParticles ) );
	m_client_mode.add( IClientMode::SHOULDDRAWFOG, util::force_cast( &Hooks::ShouldDrawFog ) );
	m_client_mode.add( IClientMode::OVERRIDEVIEW, util::force_cast( &Hooks::OverrideView ) );
	//m_client_mode.add(IClientMode::CREATEMOVE, util::force_cast(&Hooks::CreateMove));
	m_client_mode.add( IClientMode::DOPOSTSPACESCREENEFFECTS, util::force_cast( &Hooks::DoPostScreenSpaceEffects ) );

	m_surface.init( g_csgo.m_surface );
	//m_surface.add( ISurface::GETSCREENSIZE, util::force_cast( &Hooks::GetScreenSize ) );
	m_surface.add( ISurface::LOCKCURSOR, util::force_cast( &Hooks::LockCursor ) );
	m_surface.add( ISurface::PLAYSOUND, util::force_cast( &Hooks::PlaySound ) );
	m_surface.add( ISurface::ONSCREENSIZECHANGED, util::force_cast( &Hooks::OnScreenSizeChanged ) );

	m_model_render.init( g_csgo.m_model_render );
	m_model_render.add( IVModelRender::DRAWMODELEXECUTE, util::force_cast( &Hooks::DrawModelExecute ) );

	m_render_view.init( g_csgo.m_render_view );
	m_render_view.add( IVRenderView::SCENEEND, util::force_cast( &Hooks::SceneEnd ) );

	m_shadow_mgr.init( g_csgo.m_shadow_mgr );
	m_shadow_mgr.add( IClientShadowMgr::COMPUTESHADOWDEPTHTEXTURES, util::force_cast( &Hooks::ComputeShadowDepthTextures ) );

	m_view_render.init( g_csgo.m_view_render );
	m_view_render.add( CViewRender::ONRENDERSTART, util::force_cast( &Hooks::OnRenderStart ) );
	m_view_render.add( CViewRender::RENDERVIEW, util::force_cast( &Hooks::RenderView ) );
	m_view_render.add( CViewRender::RENDER2DEFFECTSPOSTHUD, util::force_cast( &Hooks::Render2DEffectsPostHUD ) );
	m_view_render.add( CViewRender::RENDERSMOKEOVERLAY, util::force_cast( &Hooks::RenderSmokeOverlay ) );

	m_match_framework.init( g_csgo.m_match_framework );
	m_match_framework.add( CMatchFramework::GETMATCHSESSION, util::force_cast( &Hooks::GetMatchSession ) );

	m_material_system.init( g_csgo.m_material_system );
	m_material_system.add( IMaterialSystem::OVERRIDECONFIG, util::force_cast( &Hooks::OverrideConfig ) );

	static auto clientstate_instance = reinterpret_cast< void* >( ( CClientState* )( uint32_t( g_csgo.m_cl ) + 0x8 ) );

	m_client_state.init( clientstate_instance );
	m_client_state.add( CClientState::PACKETSTART, util::force_cast( &Hooks::PacketStart ) );
	//m_client_state.add( CClientState::TEMPENTITIES, util::force_cast( &Hooks::TempEntities ) );

	static auto fire_events_target = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 83 EC 08 53 8B 1D ? ? ? ? 56 57 83" ) ).as<void*>( );
	static auto createmove_target = util::get_method( g_csgo.m_client, CHLClient::CREATEMOVE );
	static auto setup_bones_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9" ) ).as<void*>( );
	static auto interpolateserverentities_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53" ) ).as<void*>( );
	static auto skipanimationframe_traget = pattern::find( g_csgo.m_client_dll, XOR( "57 8B F9 8B 07 8B 80 60" ) ).as<void*>( );
	static auto modifyeyeposition_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60" ) ).as<void*>( );
	static auto update_clientside_animation_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36" ) ).as<void*>( );
	static auto do_extra_bones_processing_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 81 EC FC 00 00 00 53 56 8B F1 57" ) ).as<void*>( );
	static auto interpolate_part_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 51 8B 45 14 56" ) ).as<void*>( );
	static auto attachment_helper_target = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC ? 53 8B 5D ? 89 4D F4 56" ) ).as<void*>( );
	static auto send_datagram_target = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 18" ) ).as<void*>( );
	static auto process_packet_target = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 83 E4 C0 81 EC B4 00 00 00 53 56 57 8B 7D 08 8B D9" ) ).as<void*>( );
	static auto get_eye_ang_target = pattern::find( g_csgo.m_client_dll, XOR( "56 8B F1 85 F6 74 32" ) ).as<void*>( );
	static auto maintain_sequence_transitions_target = pattern::find( g_csgo.m_client_dll, XOR( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 18 56 57 8B F9 F3 0F 11 55 FC 80 BF F0 09 00 00 00" ) ).as<void*>( );
	static auto voice_data_target = util::get_method( clientstate_instance, 24 );
	static auto clmove_target = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A" ) ).as<void*>( );

	MH_Initialize( );

	MH_CreateHook( fire_events_target, FireEvents::Hook, ( void** )&FireEvents::original );
	MH_CreateHook( clmove_target, CLMove::Hook, ( void** )&CLMove::original );
	MH_CreateHook( maintain_sequence_transitions_target, MaintainSequenceTransitions::Hook, ( void** )&MaintainSequenceTransitions::original );
	MH_CreateHook( voice_data_target, VoiceData::Hook, ( void** )&VoiceData::original );
	MH_CreateHook( createmove_target, CreateMove::Hook, ( void** )&CreateMove::original );
	MH_CreateHook( setup_bones_target, SetupBones::Hook, ( void** )&SetupBones::original );
	MH_CreateHook( interpolateserverentities_target, InterpolateServerEntities::Hook, ( void** )&InterpolateServerEntities::original );
	MH_CreateHook( skipanimationframe_traget, SkipAnimationFrame::Hook, ( void** )&SkipAnimationFrame::original );
	MH_CreateHook( modifyeyeposition_target, ModifyEyePosition::Hook, ( void** )&ModifyEyePosition::original );
	MH_CreateHook( update_clientside_animation_target, UpdateClientAnimations::Hook, ( void** )&UpdateClientAnimations::original );
	MH_CreateHook( do_extra_bones_processing_target, ExtraBonesProcessing::Hook, ( void** )&ExtraBonesProcessing::original );
	MH_CreateHook( interpolate_part_target, InterpolatePart::Hook, ( void** )&InterpolatePart::original );
	MH_CreateHook( attachment_helper_target, AttachmentHelper::Hook, ( void** )&AttachmentHelper::original );
	MH_CreateHook( send_datagram_target, SendDataGram::Hook, ( void** )&SendDataGram::original );
	MH_CreateHook( process_packet_target, ProcessPacket::Hook, ( void** )&ProcessPacket::original );
	MH_CreateHook( get_eye_ang_target, GetEyeAngles::Hook, ( void** )&GetEyeAngles::original );

	MH_EnableHook( nullptr );

	// register our custom entity listener.
	// todo - dex; should we push our listeners first? should be fine like this.
	//g_custom_entity_listener.init();

	// cvar hooks.
	//m_debug_spread.init(g_csgo.weapon_debug_spread_show);
	//m_debug_spread.add(ConVar::GETINT, util::force_cast(&Hooks::DebugSpreadGetInt));

	//m_net_show_fragments.init(g_csgo.net_showfragments);
	//m_net_show_fragments.add(ConVar::GETINT, util::force_cast(&Hooks::NetShowFragmentsGetInt));

	//m_cl_extrapolate.init(g_csgo.cl_extrapolate);
	//m_cl_extrapolate.add(ConVar::GETINT, util::force_cast(&Hooks::ExtrapolateGetInt));

	// set netvar proxies.
	g_netvars.SetProxy( HASH( "DT_CSPlayer" ), HASH( "m_angEyeAngles[0]" ), Pitch_proxy, m_Pitch_original );
	g_netvars.SetProxy( HASH( "DT_CSRagdoll" ), HASH( "m_vecForce" ), Force_proxy, m_Force_original );
}

void Hooks::uninit( ) {
	g_winapi.SetWindowLongA( g_csgo.m_game->m_hWindow, GWL_WNDPROC, util::force_cast< LONG >( m_old_wndproc ) );

	/*g_custom_entity_listener.uninit();
	for (int i = 1; i <= g_csgo.m_globals->m_max_clients; i++) {
		VMT* vmt = &g_hooks.m_player[i - 1];
		if (vmt) {
			vmt->remove(Player::BUILDTRANSFORMATIONS);
			vmt->remove(Player::GETACTIVEWEAPON);
			vmt->reset();
		}
	}*/

	if ( g_gui.m_open ) {
		g_csgo.m_surface->LockCursor( );
		g_csgo.m_input_system->EnableInput( true );
	}

	m_panel.remove( IPanel::PAINTTRAVERSE );
	m_panel.reset( );

	m_client.remove( CHLClient::LEVELINITPREENTITY );
	m_client.remove( CHLClient::LEVELINITPOSTENTITY );
	m_client.remove( CHLClient::LEVELSHUTDOWN );
	//m_client.remove( CHLClient::INKEYEVENT);
	m_client.remove( CHLClient::FRAMESTAGENOTIFY );
	m_client.reset( );

	//m_engine.remove(IVEngineClient::ISCONNECTED);
	m_engine.remove( IVEngineClient::ISHLTV );
	m_engine.reset( );

	m_engine_sound.remove( IEngineSound::EMITSOUND );
	m_engine_sound.reset( );

	m_prediction.remove( CPrediction::INPREDICTION );
	m_prediction.remove( CPrediction::RUNCOMMAND );
	m_prediction.reset( );

	m_client_mode.remove( IClientMode::SHOULDDRAWPARTICLES );
	m_client_mode.remove( IClientMode::SHOULDDRAWFOG );
	m_client_mode.remove( IClientMode::OVERRIDEVIEW );
	//m_client_mode.remove(IClientMode::CREATEMOVE);
	m_client_mode.remove( IClientMode::DOPOSTSPACESCREENEFFECTS );
	m_client_mode.reset( );

	//m_surface.remove( ISurface::GETSCREENSIZE);
	m_surface.remove( ISurface::LOCKCURSOR );
	m_surface.remove( ISurface::PLAYSOUND );
	m_surface.remove( ISurface::ONSCREENSIZECHANGED );
	m_surface.reset( );

	m_model_render.remove( IVModelRender::DRAWMODELEXECUTE );
	m_model_render.reset( );

	m_render_view.remove( IVRenderView::SCENEEND );
	m_render_view.reset( );

	m_shadow_mgr.remove( IClientShadowMgr::COMPUTESHADOWDEPTHTEXTURES );
	m_shadow_mgr.reset( );

	m_view_render.remove( CViewRender::ONRENDERSTART );
	m_view_render.remove( CViewRender::RENDERVIEW );
	m_view_render.remove( CViewRender::RENDER2DEFFECTSPOSTHUD );
	m_view_render.remove( CViewRender::RENDERSMOKEOVERLAY );
	m_view_render.reset( );

	m_match_framework.remove( CMatchFramework::GETMATCHSESSION );
	m_match_framework.reset( );

	m_material_system.remove( IMaterialSystem::OVERRIDECONFIG );
	m_material_system.reset( );

	static auto clientstate_instance = reinterpret_cast< void* >( ( CClientState* )( uint32_t( g_csgo.m_cl ) + 0x8 ) );

	m_client_state.remove( CClientState::PACKETSTART );
	//m_client_state.remove(CClientState::TEMPENTITIES);
	m_client_state.reset( );

	MH_DisableHook( MH_ALL_HOOKS );
	MH_RemoveHook( MH_ALL_HOOKS );
	MH_Uninitialize( );

	g_netvars.Restore( HASH( "DT_CSPlayer" ), HASH( "m_angEyeAngles[0]" ), m_Pitch_original );
	g_netvars.Restore( HASH( "DT_CSRagdoll" ), HASH( "m_vecForce" ), m_Force_original );
}