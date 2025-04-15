#include "includes.h"
#include <shellapi.h>

Client g_cl{ };

// init routine.
ulong_t __stdcall Client::init( void* arg ) {
	// if not in interwebz mode, the driver will not set the username.
	g_cl.m_user = XOR( "pablohook" );
	// stop here if we failed to acquire all the data needed from csgo.
	if ( !g_csgo.init( ) )
		return 0;

	g_notify.add( XOR( "Welcome\n" ) ) ;

	while ( !GetAsyncKeyState( VK_END ) )
		Sleep( 1000 );

	g_listener.unregister_events( );
	g_hooks.uninit( );

	FreeLibraryAndExitThread( static_cast< HMODULE >( arg ), EXIT_SUCCESS );
	//return 1; /* doesn't reach */
}

//void Client::DrawHUD( ) {
//	//if ( !g_menu.main.misc.watermark.get( ) )
//		//return;
//
//	// get time.
//	time_t t = std::time(nullptr);
//	std::ostringstream time;
//	time << std::put_time(std::localtime(&t), ("%H:%M:%S"));
//
//	// get round trip time in milliseconds.
//	int ms = std::max(0, (int)std::round(g_cl.m_latency * 1000.f));
//
//	// get tickrate.
//	int rate = (int)std::round(1.f / g_csgo.m_globals->m_interval);
//
//	std::string text = tfm::format(XOR("pablohook | rtt: %ims | rate: %i | %s"), ms, rate, time.str().data());
//	render::FontSize_t size = render::hud.size(text);
//
//	// background.
//	render::rect_filled(m_width - size.m_width - 20, 10, size.m_width + 10, size.m_height + 2, { 240, 110, 140, 130 });
//
//	// text.
//	render::hud.string(m_width - 15, 10, { 240, 160, 180, 250 }, text, render::ALIGN_RIGHT);
//}

void Client::KillFeed( ) {
	if ( !g_menu.main.misc.killfeed.get( ) )
		return;

	if ( !g_csgo.m_engine->IsInGame( ) )
		return;

	// get the addr of the killfeed.
	KillFeed_t* feed = g_csgo.m_hud->FindElement<KillFeed_t>( XOR( "SFHudDeathNoticeAndBotStatus" ) );
	if ( !feed )
		return;

	int size = feed->notices.Count( );
	if ( !size )
		return;

	for ( int i{ }; i < size; ++i ) {
		NoticeText_t* notice = &feed->notices[ i ];

		// this is a local player kill, delay it.
		if ( notice->fade == 1.5f )
			notice->fade = FLT_MAX;
	}
}

void Client::OnPaint( ) {
	// update screen size.
	g_csgo.m_engine->GetScreenSize( m_width, m_height );

	// render stuff.
	g_visuals.think( );
	g_grenades.paint( );
	g_notify.think( );

	//DrawHUD( );
	KillFeed( );

	g_notify.m_mode = g_menu.main.misc.notifications.get( 6 );

	// menu goes last.
	g_gui.think( );
}

void Client::OnMapload( ) {
	// store class ids.
	g_netvars.SetupClassData( );

	// createmove will not have been invoked yet.
	// but at this stage entites have been created.
	// so now we can retrive the pointer to the local player.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	// world materials.
	Visuals::ModulateWorld( );

	m_taps = 0;

	// init knife shit.
	g_skins.load( );

	m_sequences.clear( );
	m_sent_cmds.clear( );

	m_anim_data.m_fake_data.reset( );
	for ( int i = 1; i < g_csgo.m_globals->m_max_clients; i++ ) {
		m_anim_data.m_data[ i - 1 ].m_server_data.reset( );
	}

	// if the INetChannelInfo pointer has changed, store it for later.
	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo( );
}

void Client::DrawServerHitboxes( ) {
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) ) { // we checking both cus why not, can't hurt us.
		return;
	}

	if ( !g_csgo.m_input->CAM_IsThirdPerson( ) ) { // useless in first-person.
		return;
	}

	// Function to get a player by index
	auto getPlayerByIndex = [ ]( int index ) -> Player* {
		typedef Player* ( __fastcall* playerByIndex )( int );
		static auto playerIndexFunc = pattern::find( PE::GetModule( HASH( "server.dll" ) ), "85 C9 7E 2A A1" ).as<playerByIndex>( );

		if ( !playerIndexFunc ) {
			return nullptr;
		}

		return playerIndexFunc( index );
		};

	// Find the function address
	static auto functionAddress = pattern::find( PE::GetModule( HASH( "server.dll" ) ), "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE" ).as<uintptr_t>( );

	auto duration = -1.f;
	PVOID entity = nullptr;

	entity = getPlayerByIndex( g_cl.m_local->index( ) );

	if ( !entity ) {
		return;
	}

	// keep your assembly I won't...
	typedef void( __fastcall* DrawFunction )( PVOID entity, float duration );
	DrawFunction drawFn = reinterpret_cast< DrawFunction >( functionAddress );

	drawFn( entity, duration );
}

int Client::GetCheat( int i, bool update ) {
	static player_info_t info[ 64 ];
	static std::string   name;

	if ( !update ) {
		AimPlayer* data = &g_aimbot.m_players[ i - 1 ];

		name = info[ i - 1 ].m_name;

		if ( data->m_cheat != Cheats::MONEYBOT )
			data->m_cheat = Cheats::UNK;

		switch ( info[ i - 1 ].m_friends_id ) {
			//admins
		case 1428660150: //breaker #1
		case 1407819182: //breaker #2
		case 462633443: //xoperion
			//case 1517184904: //sap
			// 
			// 
			// 
			//users
		case 1708070462: //vouza #1
		case 1693358719: //vouza #2
		case 1844090672: //smug #1
		case 1591349336: //smug #2
		case 1853310460: //smug #3
		case 1003456880: //mediocre #1
		case 1609102785: //mediocre #2
		case 1355851452: //pedro
		case 1099593873: //misterstefan
		case 1087076296: //sap
		case 886456310: //fritz
			if ( data->m_cheat != Cheats::MONEYBOT )
				data->m_cheat = Cheats::RAX;
			break;

		case 1107751578: //distance
			/*if ( data->m_cheat != Cheats::MONEYBOT )
				data->m_cheat = Cheats::FAMILY;*/
			break;
		}

		if ( name.find( "patrick" ) != std::string::npos || name.find( "cookinwithkya" ) != std::string::npos || ( name.size( ) == 1 && name.find( "p" ) != std::string::npos ) || name.find( "lil (p)eep" ) != std::string::npos )
			data->m_cheat = Cheats::FAMILY;

		return data->m_cheat;
	}
	g_csgo.m_engine->GetPlayerInfo( i, &info[ i - 1 ] );

	return Cheats::UNK;
}

void Client::StartMove( CUserCmd* cmd ) {
	// save some usercmd stuff.
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_strafe_angles = m_view_angles = cmd->m_view_angles;

	// store max choke
	// TODO; 11 -> m_bIsValveDS
	m_fixed_movement = false;
	m_max_lag = 15;
	m_lag = g_csgo.m_cl->m_choked_commands;
	m_lerp = game::GetClientInterpAmount( );
	m_latency = g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING );
	m_server_tick = g_csgo.m_cl->m_server_tick;
	m_arrival_tick = m_server_tick + game::TIME_TO_TICKS( m_latency );
	m_fakewalk = g_input.GetKeyState( g_menu.main.movement.fakewalk.get( ) );
	m_auto_peek = g_input.GetKeyState( g_menu.main.movement.autopeek.get( ) );
	g_hvh.m_fakelag = false;
	m_latency_ticks = game::TIME_TO_TICKS(m_latency);

	bool old_override = m_update_override_correction;
	m_update_override_correction = g_input.GetKeyState( g_menu.main.aimbot.ov_correction.get( ) );

	if ( m_update_override_correction != old_override && m_update_override_correction )
		m_correction_override = !m_correction_override;

	if ( !m_correction_override ) {
		m_override_correction_ang = m_view_angles;
	}

	// make sure prediction has ran on all usercommands.
	// because prediction runs on frames, when we have low fps it might not predict all usercommands.
	// also fix the tick being inaccurate.
	g_inputpred.update( true );
	g_inputpred.m_velocity = m_local->m_vecVelocity( );

	// store some stuff about the local player.
	m_flags = m_local->m_fFlags( );

	// ...
	m_shot = false;
}

void Client::BackupPlayers( bool restore ) {
	if ( restore ) {
		// restore stuff.
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if ( !g_aimbot.IsValidTarget( player ) )
				continue;

			g_aimbot.m_backup[ i - 1 ].restore( player );
		}
	}

	else {
		// backup stuff.
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if ( !g_aimbot.IsValidTarget( player ) )
				continue;

			g_aimbot.m_backup[ i - 1 ].store( player );
		}
	}
}

void Client::DoMove( ) {
	penetration::PenetrationOutput_t tmp_pen_data{ };

	// run movement code before input prediction.
	g_movement.run( );
	m_in_jump = ( ( m_cmd->m_buttons & IN_JUMP ) && !( m_old_buttons & IN_JUMP ) ) || !( m_flags & FL_ONGROUND );

	// predict input.
	g_inputpred.run( );

	g_grenade_warning.on_create_move( m_cmd );

	// convert viewangles to directional forward vector.
	math::AngleVectors( m_view_angles, &m_forward_dir );

	m_curtime = game::TICKS_TO_TIME( m_local->m_nTickBase( ) );

	if ( !CLMove::ShiftedTick )
		m_curtime -= game::TICKS_TO_TIME( CLMove::ChargedTicks );

	// store stuff after input pred.
	UpdateShootPosition( );

	if ( !m_lag ) {
		g_hvh.m_old_desync = g_hvh.m_desync;
		g_hvh.m_desync = false;
		m_state_velocity = math::Approach( m_local->m_vecAbsVelocity( ), m_local->m_vecVelocity( ), 2000 * ( m_curtime - m_anim_time ) );

		g_movement.m_did_move = m_state_velocity.length_2d( ) > 0.1f;
	}

	// reset shit.
	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_player_fire = m_weapon_fire = false;

	// store weapon stuff.
	m_weapon = m_local->GetActiveWeapon( );

	if ( m_weapon ) {
		m_weapon_info = m_weapon->GetWpnData( );
		m_weapon_id = m_weapon->m_iItemDefinitionIndex( );
		m_weapon_type = m_weapon_info->m_weapon_type;

		// run autowall once for penetration crosshair if we have an appropriate weapon.
		if ( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon_type != WEAPONTYPE_C4 && m_weapon_type != WEAPONTYPE_GRENADE ) {
			penetration::PenetrationInput_t in;
			in.m_from = m_local;
			in.m_target = nullptr;
			in.m_pos = m_shoot_pos + ( m_forward_dir * m_weapon_info->m_range );
			in.m_damage = 1.f;
			in.m_damage_pen = 1.f;
			in.m_can_pen = true;

			// run autowall.
			penetration::run( &in, &tmp_pen_data );
		}

		// set pen data for penetration crosshair.
		m_pen_data = tmp_pen_data;

		// can the player fire.
		m_player_fire = m_curtime >= m_local->m_flNextAttack( ) && !g_csgo.m_gamerules->m_bFreezePeriod( ) && !( g_cl.m_flags & FL_FROZEN );

		UpdateRevolverCock( );
		m_weapon_fire = CanFireWeapon( );
	}

	// last tick defuse.
	// todo - dex;  figure out the range for CPlantedC4::Use?
	//              add indicator if valid (on ground, still have time, not being defused already, etc).
	//              move this? not sure where we should put it.
	if ( g_input.GetKeyState( g_menu.main.misc.last_tick_defuse.get( ) ) && g_visuals.m_c4_planted ) {
		float defuse = ( m_local->m_bHasDefuser( ) ) ? 5.f : 10.f;
		float remaining = g_visuals.m_planted_c4_explode_time - m_curtime;
		float dt = remaining - defuse - ( g_cl.m_latency / 2.f );

		m_cmd->m_buttons &= ~IN_USE;
		if ( dt <= game::TICKS_TO_TIME( 2 ) )
			m_cmd->m_buttons |= IN_USE;
	}

	if ( ( CLMove::ShiftedTick || ( ( !m_weapon_fire || ( !g_cl.m_lag && g_menu.main.antiaim.enable.get( ) && g_menu.main.antiaim.fake_yaw.get( ) ) )
		&& m_weapon_type != WEAPONTYPE_C4 && m_weapon_type != WEAPONTYPE_GRENADE ) ) && m_weapon_id != REVOLVER ) {
		m_cmd->m_buttons &= ~IN_ATTACK;
		m_cmd->m_buttons &= ~IN_ATTACK2;
	}

	if ( !CLMove::ShiftedTick ) {
		// grenade prediction.
		g_grenades.think( );

		// run fakelag.
		g_hvh.SendPacket( );

		// run aimbot.
		g_aimbot.Think( );

		if ( m_weapon && ( m_weapon_fire || m_weapon_type == WEAPONTYPE_GRENADE ) ) {
			if ( m_weapon_type == WEAPONTYPE_GRENADE ) {
				m_shot = m_weapon->m_fThrowTime( ) > 0.f;
			}
			else {
				if ( m_cmd->m_buttons & IN_ATTACK )
					m_shot = true;

				if ( ( m_cmd->m_buttons & IN_ATTACK2 ) && ( m_weapon_id == REVOLVER || m_weapon_type == WEAPONTYPE_KNIFE || m_weapon_type == WEAPONTYPE_GRENADE ) )
					m_shot = true;
			}
		}

		if ( m_shot )
			m_packet = CLMove::Enabled ? true : m_lag >= g_cl.m_max_lag;
	}
	else
		m_packet = true;

	// run antiaims.
	g_hvh.AntiAim( );

	if ( m_shot && m_lag ) {
		m_anim_shot = true;
		m_shot_pitch = m_cmd->m_view_angles.x;
	}
}

void Client::EndMove( CUserCmd* cmd ) {
	cmd->m_view_angles.SanitizeAngle( );

	// fix our movement.
	g_movement.FixMove( cmd );

	// this packet will be sent.
	if ( m_packet ) {
		g_hvh.m_step_switch = ( bool )g_csgo.RandomInt( 0, 1 );

		// we are sending a packet, so this will be reset soon.
		// store the old value.
		m_old_lag = m_lag;

		// get radar angles.
		m_radar = cmd->m_view_angles;
		m_radar.normalize( );

		// get current origin.
		vec3_t cur = m_local->m_vecOrigin( );

		// get prevoius origin.
		vec3_t prev = m_net_pos.empty( ) ? cur : m_net_pos.front( ).m_pos;

		m_old_lagcomp = m_lagcomp;
		// check if we broke lagcomp.
		m_lagcomp = ( cur - prev ).length_sqr( ) > 4096.f;

		// save sent origin and time.
		m_net_pos.emplace_front( m_curtime, cur );

		if ( m_lagcomp && m_old_lagcomp )
			g_hvh.m_fl_switch = !g_hvh.m_fl_switch;
		else
			g_hvh.m_fl_switch = false;
	}

	// store some values for next tick.
	m_old_packet = m_packet;
	m_old_shot = m_shot;
	m_old_buttons = cmd->m_buttons;

	// update client-side animations.
	UpdateInformation( );
	m_anim_shot = false;

	// restore curtime/frametime
	// and prediction seed/player.
	g_inputpred.restore( );
}

void Client::OnTick( CUserCmd* cmd ) {
	if ( !m_local || !m_local->alive( ) )
		return;

	// store some data and update prediction.
	StartMove( cmd );

	// save the original state of players.
	BackupPlayers( false );

	// run all movement related code.
	DoMove( );

	// store stome additonal stuff for next tick
	// sanetize our usercommand if needed and fix our movement.
	EndMove( cmd );

	// restore the players.
	BackupPlayers( true );
}

void Client::HandleAnimationCopy( rebuilt_animstate_t* data, rebuilt_animstate_t& copy ) {
	if ( !data->m_ground || data->m_vel_xy > 0.1f ) {
		std::memcpy( &copy, data, sizeof( rebuilt_animstate_t ) );
		return;
	}

	// skip desync animation.
	if ( g_hvh.m_desync && g_menu.main.misc.animations.get( 2 ) )
		return;

	// skip flick animation.
	if ( data->m_aa_flick && g_menu.main.misc.animations.get( 5 ) )
		return;

	std::memcpy( &copy, data, sizeof( rebuilt_animstate_t ) );

	if ( !g_menu.main.misc.animations.get( 5 ) || !g_menu.main.antiaim.body_fake_stand.get( ) )
		return;

	copy.m_foot_yaw = copy.m_angle.y;
	copy.m_poses[ 11 ] = 0.5f;
}

void Client::UpdateShootPosition( ) {
	if ( !m_local || !m_local->alive( ) )
		return;

	CCSGOPlayerAnimState* state = m_local->m_PlayerAnimState( );
	if ( !state )
		return;

	m_origin = m_local->m_vecOrigin( );

	const CBoneCache& cache = m_local->m_BoneCache( );
	if ( !cache.m_pCachedBones ) {
		m_local->GetEyePos( &m_shoot_pos );
		return;
	}

	rebuilt_animstate_t data = m_anim_data.m_data[ g_cl.m_local->index( ) - 1 ].m_server_data;

	BoneArray backup_bones[ 128 ];
	float backup_poses[ 24 ];
	C_AnimationLayer backup_layers[ 13 ];

	m_local->GetPoseParameters( backup_poses );
	m_local->GetAnimLayers( backup_layers );
	m_local->GetBones( backup_bones );

	data.m_poses[ 12 ] = 0.5f;

	SetAngles( &data );

	/* we dont need to do the m_packet check since we dont choke shot */
	g_bones.Setup( m_local, BONE_USED_BY_ANYTHING, m_shot_bones, 128, true, false );

	m_local->GetEyePos( &m_shoot_pos );

	m_local->ModifyEyePosition( m_shot_bones, &data, m_shoot_pos );

	m_local->SetPoseParameters( backup_poses );
	m_local->SetAnimLayers( backup_layers );
	m_local->SetBones( backup_bones );
}

void Client::SetAngles( rebuilt_animstate_t* data ) {
	if ( !m_local || !m_local->alive( ) )
		return;

	m_anim_data.SetLayers( m_local, data );
	m_local->SetPoseParameters( data->m_poses );
	m_local->SetAbsAngles( ang_t( 0.f, data->m_foot_yaw, 0.f ), true );
}

void Client::UpdateInformation( ) {
	const int i = m_local->index( );

	rebuilt_animstate_t* fake_data = &m_anim_data.m_fake_data;
	rebuilt_animstate_t* data = &m_anim_data.m_data[ i - 1 ].m_server_data;

	m_anim_data.DoAnimationEvent( g_cl.m_local, g_inputpred.m_old_ground_ent, data );

	fake_data->m_buttons = data->m_buttons;
	fake_data->m_old_buttons = data->m_old_buttons;

	if ( m_lag ) {
		if ( m_anim_shot )
			data->m_poses[ 12 ] = ( m_cmd->m_view_angles.x / 90 ) + 0.5f;
		return;
	}

	CCSGOPlayerAnimState* state = m_local->m_PlayerAnimState( );
	if ( !state )
		return;

	// update time.
	m_anim_frame = m_curtime - m_anim_time;
	m_anim_time = m_curtime;

	// current angle will be animated.
	m_angle = m_cmd->m_view_angles;

	math::clamp( m_angle.x, -90.f, 90.f );
	m_angle.normalize( );

	//m_local->m_iEFlags( ) &= ~0x800;
	//m_local->m_iEFlags( ) &= ~0x1000;

	// CCSGOPlayerAnimState::Update, bypass already animated checks.
	if ( state->m_last_update_frame == g_csgo.m_globals->m_frame )
		--state->m_last_update_frame;

	// call original, bypass hook.
	UpdateClientAnimations::DoUpdate( m_local );

	m_anim_data.UpdateLayers( m_local, m_angle, data, m_curtime );

	m_abs_angle = { 0, data->m_foot_yaw, 0 };

	if ( data->m_ground ) {
		if ( data->m_vel_xy > 0.1f ) {
			m_lby_increment = 0;
			m_move_yaw = m_angle.y;
		}
		else if ( m_body_pred != data->m_body_update )
			++m_lby_increment;
	}

	m_body = data->m_body_yaw;
	m_body_pred = data->m_body_update;
	m_ground = data->m_ground;

	m_local->m_flLowerBodyYawTarget( ) = data->m_body_yaw;

	data->m_aa_flick = m_flick;

	m_anim_data.UpdateLayers( m_local, { 0, data->m_body_yaw, 0 }, fake_data, m_curtime, false );

	SetAngles( fake_data );

	m_fake_bones_setup = g_bones.Setup( m_local, BONE_USED_BY_ANYTHING, m_fake_bones, 128, true, true );

	std::memcpy( g_cl.m_server_layers[ 1 ], g_cl.m_server_layers[ 0 ], sizeof( C_AnimationLayer ) * 13 );
}

void Client::print( const std::string text, ... ) {
	va_list     list;
	int         size;
	std::string buf;

	if ( text.empty( ) )
		return;

	va_start( list, text );

	// count needed size.
	size = std::vsnprintf( 0, 0, text.c_str( ), list );

	// allocate.
	buf.resize( size );

	// print to buffer.
	std::vsnprintf( buf.data( ), size + 1, text.c_str( ), list );

	va_end( list );

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf( g_gui.m_color, XOR( "[pablohook] " ) );
	g_csgo.m_cvar->ConsoleColorPrintf( colors::white, buf.c_str( ) );
}

bool Client::CanFireWeapon( ) {
	// the player cant fire.
	if ( !m_player_fire )
		return false;

	if ( m_weapon_type == WEAPONTYPE_GRENADE )
		return false;

	// if we have no bullets, we cant shoot.
	if ( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1( ) < 1 )
		return false;

	// do we have any burst shots to handle?
	if ( ( m_weapon_id == GLOCK || m_weapon_id == FAMAS ) && m_weapon->m_iBurstShotsRemaining( ) > 0 ) {
		// new burst shot is coming out.
		if ( m_curtime >= m_weapon->m_fNextBurstShot( ) )
			return true;
	}

	// r8 revolver.
	if ( m_weapon_id == REVOLVER ) {
		int act = m_weapon->m_Activity( );

		// mouse1.
		if ( !m_revolver_fire ) {
			if ( ( act == 185 || act == 193 ) && m_revolver_cock == 0 )
				return m_curtime >= m_weapon->m_flNextPrimaryAttack( );

			return false;
		}
	}

	// yeez we have a normal gun.
	if ( m_curtime >= m_weapon->m_flNextPrimaryAttack( ) )
		return true;

	return false;
}

void Client::UpdateRevolverCock( ) {
	// default to false.
	m_revolver_fire = false;

	// reset properly.
	if ( m_revolver_cock == -1 )
		m_revolver_cock = 0;

	// we dont have a revolver.
	// we have no ammo.
	// player cant fire
	// we are waiting for we can shoot again.
	if ( m_weapon_id != REVOLVER || m_weapon->m_iClip1( ) < 1 || !m_player_fire || m_curtime < m_weapon->m_flNextPrimaryAttack( ) ) {
		// reset.
		m_revolver_cock = 0;
		m_revolver_query = 0;
		return;
	}

	// calculate max number of cocked ticks.
	// round to 6th decimal place for custom tickrates..
	int shoot = ( int )( 0.25f / ( std::round( g_csgo.m_globals->m_interval * 1000000.f ) / 1000000.f ) );

	// amount of ticks that we have to query.
	m_revolver_query = shoot - 1;

	// we held all the ticks we needed to hold.
	if ( m_revolver_query == m_revolver_cock ) {
		// reset cocked ticks.
		m_revolver_cock = -1;

		// we are allowed to fire, yay.
		m_revolver_fire = true;
	}

	else {
		// we still have ticks to query.
		// apply inattack.
		if ( g_menu.main.config.mode.get( ) == 0 && m_revolver_query > m_revolver_cock )
			m_cmd->m_buttons |= IN_ATTACK;

		// count cock ticks.
		// do this so we can also count 'legit' ticks
		// that didnt originate from the hack.
		if ( m_cmd->m_buttons & IN_ATTACK )
			m_revolver_cock++;

		// inattack was not held, reset.
		else m_revolver_cock = 0;
	}

	// remove inattack2 if cocking.
	if ( m_revolver_cock > 0 )
		m_cmd->m_buttons &= ~IN_ATTACK2;
}

void Client::UpdateIncomingSequences( ) {
	/* update net channel ptr */
	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo( );

	if ( !g_csgo.m_net )
		return;

	if ( m_sequences.empty( ) || g_csgo.m_net->m_in_seq > m_sequences.front( ).m_seq ) {
		// store new stuff.
		m_sequences.emplace_front( g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq );
	}

	// do not save too many of these.
	while ( m_sequences.size( ) > 2048 )
		m_sequences.pop_back( );
}