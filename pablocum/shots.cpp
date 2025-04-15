#include "includes.h"

Shots g_shots{ };

void Shots::HandleMisses( AimPlayer* data ) {
	if ( !g_cl.m_local )
		return;

	const bool& log_misses = g_menu.main.misc.notifications.get( 6 );

	for ( auto& impact : g_shots.m_impacts ) {
		if ( !impact.m_shot->m_matched )
			continue;

		if ( impact.m_shot->m_record.m_player != data->m_player )
			continue;

		if ( impact.m_scanned )
			continue;

		impact.m_scanned = true;

		const bool is_resolved = impact.m_shot->m_record.m_mode == Modes::RESOLVE_NONE ||
			impact.m_shot->m_record.m_mode == Modes::RESOLVE_WALK ||
			impact.m_shot->m_record.m_mode == Modes::RESOLVE_DESYNC;

		if ( impact.m_hurt ) {
			if ( log_misses && game::IsValidHitgroup( impact.m_hitgroup ) ) {
				// we should have not hit the player at all.
				if ( impact.m_miss == 1 ) {
					if ( is_resolved ) {
						g_notify.add( XOR( "Missed target point due to animations and spread\n" ), g_gui.m_color );
						continue;
					}

					g_notify.add( XOR( "Missed target point due to fake angles and spread\n" ), g_gui.m_color );
					continue;
				}

				// we should have hit the hitgroup we targetted.
				if ( impact.m_shot->m_impact_group == impact.m_shot->m_hitgroup ) {
					if ( impact.m_hitgroup != impact.m_shot->m_hitgroup ) {
						if ( is_resolved ) {
							g_notify.add( XOR( "Missed target point due to animations\n" ), g_gui.m_color );
							continue;
						}

						g_notify.add( XOR( "Missed target point due to fake angles\n" ), g_gui.m_color );
					}

					// the shot landed perfectly.
					continue;
				}

				// our bullet hit another hitgroup due to spread.
				g_notify.add( XOR( "Missed target point due to spread\n" ), g_gui.m_color );
			}

			continue;
		}

		if ( !data->m_player || !data->m_player->alive( ) ) {
			if ( log_misses )
				g_notify.add( XOR( "Missed shot due to target death\n" ), g_gui.m_color );
			continue;
		}

		if ( impact.m_miss == 1 ) {
			if ( log_misses )
				g_notify.add( XOR( "Missed shot due to spread\n" ), g_gui.m_color );
			continue;
		}

		if ( impact.m_miss == 2 ) {
			if ( log_misses )
				g_notify.add( XOR( "Missed shot due to occolusion\n" ), g_gui.m_color );
			continue;
		}

		if ( is_resolved ) {
			if ( log_misses )
				g_notify.add( XOR( "Missed shot due to animations\n" ), g_gui.m_color );
			continue;
		}

		switch ( impact.m_shot->m_record.m_mode ) {
		case Modes::RESOLVE_STAND:
			++data->m_stand_index[ StandBrute::NORMAL ];
			break;
		case Modes::RESOLVE_STAND_NOUPDATE:
			++data->m_stand_index[ StandBrute::LOGIC ];
			break;
		case Modes::RESOLVE_STAND_LOGIC:
			++data->m_stand_index[ StandBrute::LOGIC ];
			break;
		case Modes::RESOLVE_STAND_TWIST:
			++data->m_stand_index[ StandBrute::TWIST ];
			break;
		case Modes::RESOLVE_STAND_ROTATE:
			++data->m_stand_index[ StandBrute::ROTATE ];
			break;
		case Modes::RESOLVE_STAND_BREAKMOVE:
			++data->m_stand_index[ StandBrute::BREAKMOVE ];
			break;
		case Modes::RESOLVE_STAND_STOPPEDMOVING:
			++data->m_stand_index[ StandBrute::STOPPEDMOVING ];
			break;
		case Modes::RESOLVE_AIR:
			++data->m_air_index;
			break;
		case Modes::RESOLVE_UPDATE:
			// doing some kind of exploit make sure we wont shoot flick anymore
			if ( impact.m_shot->m_safe )
				data->m_body_index = INT_MAX;
			else
				++data->m_body_index;
			break;
		}

		++data->m_missed_shots;

		if ( log_misses )
			g_notify.add( XOR( "Missed shot due to fake angles\n" ), g_gui.m_color );
	}
}

void Shots::OnShotFire( Player* target, const int& damage, LagRecord* record, const vec3_t& end_pos, const int& hitgroup, const bool& safety ) {

	ShotRecord& shot = m_shots.emplace_front( );

	shot.m_hitgroup = hitgroup;
	shot.m_safe = safety;
	std::memcpy( &shot.m_record, record, sizeof( LagRecord ) );
	shot.m_tick = g_cl.m_server_tick;
	shot.m_lat = g_cl.m_latency;
	shot.m_start = g_cl.m_shoot_pos;
	shot.m_end = end_pos;

	if ( g_cl.m_weapon && g_cl.m_weapon_info )
		shot.m_range = g_cl.m_weapon_info->m_range;

	if ( target ) {
		AimPlayer* data = &g_aimbot.m_players[ target->index( ) - 1 ];
		if ( data )
			++data->m_shots;

		static player_info_t info;
		g_csgo.m_engine->GetPlayerInfo( target->index( ), &info );

		if ( g_menu.main.misc.notifications.get( 5 ) ) {
			g_notify.add( tfm::format( "Shot at %s for %i damage in the %s with %i tick backtrack \n",
				info.m_name,
				damage,
				m_groups[ hitgroup ],
				shot.m_tick - game::TIME_TO_TICKS( shot.m_record.m_sim_time ) ) );
		}
	}

	while ( m_shots.size( ) > 128 )
		m_shots.pop_back( );
}

void Shots::OnImpact( IGameEvent* evt ) {
	// screw this.
	if ( !evt || !g_cl.m_local )
		return;

	// get attacker, if its not us, screw it.
	const int& attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if ( attacker != g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	// decode impact coordinates and convert to vec3.
	vec3_t pos = {
		evt->m_keys->FindKey( HASH( "x" ) )->GetFloat( ),
		evt->m_keys->FindKey( HASH( "y" ) )->GetFloat( ),
		evt->m_keys->FindKey( HASH( "z" ) )->GetFloat( )
	};

	const int& tick = g_csgo.m_cl->m_server_tick;

	m_vis_impacts.push_back( { pos, g_cl.m_shoot_pos, tick } );

	// we did not take a shot yet.
	if ( m_shots.empty( ) )
		return;

	ShotRecord* current = nullptr;

	const int tickrate = 1.f / g_csgo.m_globals->m_interval;
	for ( ShotRecord& shot : m_shots ) {
		if ( shot.m_matched )
			break;

		if ( abs( shot.m_tick - tick ) > tickrate )
			break;

		current = &shot;
	}

	static vec3_t start, end;

	ImpactRecord* previous = m_impacts.empty( ) ? nullptr : &m_impacts.front( );
	if ( previous && previous->m_tick == tick && pos.dist_to( end ) < previous->m_pos.dist_to( end ) ) {
		previous->m_pos = pos;
	}

	if ( !current ) {
		return;
	}

	start = current->m_start;
	end = current->m_end;

	current->m_matched = true;

	// create new impact instance that we can match with a player hurt.
	ImpactRecord& impact = m_impacts.emplace_front( );
	impact.m_miss = 1;
	impact.m_hurt = false;
	impact.m_scanned = false;
	impact.m_shot = current;
	impact.m_tick = tick;
	impact.m_pos = pos;

	static std::vector<int> hitboxes;
	if ( hitboxes.empty( ) ) {
		for ( int i = HITBOX_HEAD; i < HITBOX_MAX; i++ ) {
			hitboxes.emplace_back( i );
		}
	}

	static penetration::PenetrationInput_t input;
	static penetration::PenetrationOutput_t output;

	static bool can_hit = false;
	static bool did_hit = false;
	if ( !previous || previous->m_tick != tick ) {
		const vec3_t extrapolated = start + ( pos - start ).normalized( ) * current->m_range;

		can_hit = false;
		for ( const int& hitbox : hitboxes ) {
			if ( math::RayIntersectHitbox( current->m_record.m_player, start, extrapolated, current->m_record.m_bones, hitbox, false ) ) {
				can_hit = true;
				break;
			}
		}

		input.m_from = g_cl.m_local;
		input.m_target = current->m_record.m_player;
		input.m_start = start;
		input.m_pos = extrapolated;
		input.m_damage = input.m_damage_pen = 1;
		input.m_can_pen = true;

		static BackupRecord backup;
		backup.store( current->m_record.m_player );

		current->m_record.cache( );

		did_hit = penetration::run( &input, &output );

		// restore player to his original state.
		backup.restore( current->m_record.m_player );

		current->m_impact_group = output.m_hitgroup;
	}

	// we cant even hit the player.
	if ( !can_hit ) {
		impact.m_miss = 1;
		return;
	}

	// we should hit the player.
	if ( did_hit ) {
		impact.m_miss = 3;
		return;
	}

	// we should have hit the player but something stopped our bullet.
	impact.m_miss = 2;
}

void Shots::OnHurt( IGameEvent* evt ) {
	if ( !evt || !g_cl.m_local )
		return;

	const int& attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt( ) );
	const int& victim = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );

	// we were not the attacker or we hurt ourselves.
	if ( attacker != g_csgo.m_engine->GetLocalPlayer( ) || victim == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	const int& group = evt->m_keys->FindKey( HASH( "hitgroup" ) )->GetInt( );

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if ( !game::IsValidHitgroup( group ) )
		return;

	// get the player that was hurt.
	Player* target = g_csgo.m_entlist->GetClientEntity< Player* >( victim );
	if ( !target )
		return;

	// get player info.
	player_info_t info;
	if ( !g_csgo.m_engine->GetPlayerInfo( victim, &info ) )
		return;

	// get player name;
	const std::string& name = std::string( info.m_name ).substr( 0, 24 );

	// get damage reported by the server.
	const int& damage = evt->m_keys->FindKey( HASH( "dmg_health" ) )->GetInt( );

	// get remaining hp.
	const int& hp = evt->m_keys->FindKey( HASH( "health" ) )->GetInt( );

	// hitmarker.
	if ( g_menu.main.misc.hitsound.get( ) ) {
		g_csgo.m_sound->EmitAmbientSound( XOR( "buttons/arena_switch_press_02.wav" ), 1.f );
	}

	// print this shit.
	if ( g_menu.main.misc.notifications.get( 1 ) ) {
		std::string out = tfm::format( XOR( "Hit %s in the %s for %i damage (%i health remaining)\n" ), name, m_groups[ group ], damage, hp );
		g_notify.add( out );
	}

	if ( group == HITGROUP_GENERIC )
		return;

	// increment taps.
	if ( group == HITGROUP_HEAD && hp == 0 )
		++g_cl.m_taps;

	// if we hit a player, mark vis impacts.
	if ( !m_vis_impacts.empty( ) ) {
		for ( auto& i : m_vis_impacts ) {
			if ( i.m_tickbase == g_cl.m_local->m_nTickBase( ) )
				i.m_hit_player = true;
		}
	}

	// no impacts to match.
	if ( m_impacts.empty( ) )
		return;

	const int& tick = g_csgo.m_cl->m_server_tick;

	ImpactRecord* current_impact = nullptr;
	for ( ImpactRecord& impact : m_impacts ) {
		if ( impact.m_tick == tick ) {
			current_impact = &impact;
			break;
		}
	}

	if ( !current_impact )
		return;

	current_impact->m_hurt = true;
	current_impact->m_hitgroup = group;
}