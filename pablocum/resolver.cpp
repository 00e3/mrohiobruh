#include "includes.h"

Resolver g_resolver{};;

void Resolver::ResetMisses( int index ) {
	AimPlayer* data = &g_aimbot.m_players[ index - 1 ];

	for ( auto& stand_index : data->m_stand_index )
		stand_index = 0;
	data->m_air_index = 0;
	data->m_body_index = 0;
}

float Resolver::AutoDirection( LagRecord& record, float multiplier ) {
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	const float& away = math::NormalizedAngle( GetAwayAngle( record.m_player ) + 180.f );

	// best target.
	static vec3_t enemypos;
	record.m_player->GetEyePos( &enemypos );

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back( away, 0.f );
	angles.emplace_back( away, 90.f );
	angles.emplace_back( away, -90.f );

	// start the trace at the enemy shoot pos.
	static vec3_t start;
	g_cl.m_local->GetEyePos( &start );

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {
		const float& yaw = math::NormalizedAngle( it->m_yaw + it->m_add );

		// compute the 'rough' estimation of where our head will be.
		const vec3_t end{ enemypos.x + std::cos( math::deg_to_rad( yaw ) ) * RANGE,
			enemypos.y + std::sin( math::deg_to_rad( yaw ) ) * RANGE,
			enemypos.z };

		// compute the direction.
		vec3_t dir = start - end;
		const float& len = dir.normalize( );

		// should never happen.
		if ( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		for ( float i{ 0.f }; i < len; i += STEP ) {
			// get the current step position.
			vec3_t point = end + ( dir * i );

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents( point, MASK_SHOT_HULL );

			// contains nothing that can stop a bullet.
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if ( i > ( len * 0.5f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if ( i > ( len * 0.75f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if ( i > ( len * 0.9f ) )
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += ( STEP * mult );

			// mark that we found anything.
			valid = true;
		}
	}

	if ( !valid )
		return math::NormalizedAngle( away - 180.f );

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
		[ ]( const AdaptiveAngle& a, const AdaptiveAngle& b ) {
			return a.m_dist > b.m_dist;
		} );

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front( );

	return math::NormalizedAngle( best->m_yaw + ( best->m_add * multiplier ) );
}

float Resolver::GetAwayAngle( Player* player ) {
	static ang_t away;
	math::VectorAngles( g_cl.m_local->m_vecOrigin( ) - player->m_vecOrigin( ), away );
	return away.y;
}

ang_t calculateAngle( const vec2_t& point1, const vec2_t& point2 ) {
	static ang_t angle;
	vec2_t delta = point2 - point1;

	angle.x = atan2( delta.y, delta.x ) * ( 180.f / math::pi );
	angle.y = atan2( delta.x, delta.y ) * ( 180.f / math::pi );
	angle.z = 0.f;

	return angle;
}

void Resolver::Override( ) {
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	ang_t viewangles;
	g_csgo.m_engine->GetViewAngles( viewangles );

	static vec3_t from;
	g_cl.m_local->GetEyePos( &from );

	if ( g_cl.m_correction_override && !g_cl.m_update_override_correction )
		return;

	OverrideAngle* ov_data = &m_override;

	ov_data->m_player = nullptr;

	float best_fov = FLT_MAX;
	vec3_t best_point = { 0, 0, 0 };

	for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ ) {
		Player* player = ( Player* )g_csgo.m_entlist->GetClientEntity( i );
		if ( !player || !player->alive( ) || player->dormant( ) || !player->enemy( g_cl.m_local ) )
			continue;

		AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
		if ( !data )
			continue;

		vec3_t enemy_eye_pos{};
		player->GetEyePos( &enemy_eye_pos );

		const float fov = math::GetFOV( g_cl.m_override_correction_ang, from, enemy_eye_pos );

		if ( fov < best_fov ) {
			ov_data->m_player = player;
			best_point = enemy_eye_pos;
			best_fov = fov;
		}
	}

	if ( best_fov == FLT_MAX )
		return;

	float dist = ( best_point - from ).length( );

	vec3_t dir[ 2 ];
	math::AngleVectors( g_cl.m_override_correction_ang, &dir[ 0 ] );
	dir[ 0 ] *= dist;

	math::AngleVectors( viewangles, &dir[ 1 ] );
	dir[ 1 ] *= dist;

	ov_data->m_start = from + dir[ 0 ];
	ov_data->m_end = from + dir[ 1 ];

	vec2_t screen_pos[ 2 ];

	render::WorldToScreen( ov_data->m_start, screen_pos[ 0 ] );
	render::WorldToScreen( ov_data->m_end, screen_pos[ 1 ] );

	ov_data->m_yaw = calculateAngle( screen_pos[ 0 ], screen_pos[ 1 ] ).y;
}

void Resolver::MatchShot( AimPlayer* data, LagRecord& record, LagRecord* previous ) {
	if ( !previous )
		return;

	Weapon* weapon = data->m_player->GetActiveWeapon( );
	if ( !weapon )
		return;

	const float& shot_time = weapon->m_fLastShotTime( );
	if ( game::TIME_TO_TICKS( shot_time ) == game::TIME_TO_TICKS( record.m_anim_time ) )
		return;

	if ( shot_time <= previous->m_sim_time || shot_time > record.m_sim_time )
		return;

	record.m_eye_angles.x = previous->m_eye_angles.x;
}

#include <cstdio>
void Resolver::CorrectFootYaw( LagRecord& record, LagRecord* previous, CCSGOPlayerAnimState* state ) {
	AimPlayer* data = &g_aimbot.m_players[ record.m_player->index( ) - 1 ];
	if ( !data )
		return;

	if ( !previous )
		return;

	/* the layer does not get played when standing still or in air */
	if ( record.m_anim_velocity.length_2d( ) <= 0.1f || !( record.m_anim_flags & FL_ONGROUND ) )
		return;

	if ( previous->m_mode == Modes::RESOLVE_DESYNC ) {
		record.m_mode = Modes::RESOLVE_DESYNC;
		return;
	}

	// if so, move yaw = move yaw ideal, and we can calculate foot yaw by an accuracy of ~45 degrees
	constexpr float angles[ ] = { 180.f, 135.f, 90.f, 45.f, 0.f, -45.f, -90.f, -135.f };

	const float& local_cycle_increment = record.m_layers[ 6 ].m_playback_rate;
	const int& move_seq = record.m_layers[ 6 ].m_sequence;

	float move_yaw = FLT_MAX;
	float raw_yaw_ideal = atan2( -record.m_anim_velocity.y, -record.m_anim_velocity.x ) * 180.f / math::pi;

	if ( raw_yaw_ideal < 0.f )
		raw_yaw_ideal += 360.f;

	for (int i = ANIMTAG_STARTCYCLE_N; i <= ANIMTAG_STARTCYCLE_NW; i++) {
		const float pred_cycle = math::ClampCycle(
			g_cl.m_anim_data.GetFirstSequenceAnimTag(record.m_player, move_seq, i, 0.f, 1.f) + local_cycle_increment);

		printf("[CorrectFootYaw] Checking anim tag %d: pred_cycle=%.3f vs actual=%.3f\n",
			i, pred_cycle, record.m_layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_cycle);

		if (static_cast<int>(pred_cycle * 1000.f) == static_cast<int>(record.m_layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_cycle * 1000.f)) {
			move_yaw = angles[i - ANIMTAG_STARTCYCLE_N];
			printf("[CorrectFootYaw] Matched move_yaw: %.2f (tag index: %d)\n", move_yaw, i);
			break;
		}
	}

	if (move_yaw != FLT_MAX) {
		state->m_abs_yaw = math::AngleDiff(raw_yaw_ideal, move_yaw);
		record.m_mode = Modes::RESOLVE_DESYNC;

		printf("[CorrectFootYaw] Calculated abs_yaw: %.2f | Mode set to RESOLVE_DESYNC\n", state->m_abs_yaw);
	}
	else {
		printf("[CorrectFootYaw] Failed to determine move_yaw.\n");
	}
}

void Resolver::OnBodyUpdate( AimPlayer* data, LagRecord& record, LagRecord* previous ) {
	record.m_mode = Modes::RESOLVE_UPDATE;
	data->m_body_update = record.m_anim_time + 1.1f;

	if ( !data->m_update_count )
		data->m_body_delta = math::NormalizedAngle( record.m_body - previous->m_body );

	data->m_body_data.emplace_front( record.m_body );
	++data->m_update_count;
}

void Resolver::SetResolverMode( AimPlayer* data, LagRecord& record, LagRecord* previous, const int& act ) {
	if ( !( record.m_anim_flags & FL_ONGROUND ) ) {
		record.m_mode = Modes::RESOLVE_AIR;
		return;
	}

	if ( record.m_anim_velocity.length_2d( ) > 0.1f ) {
		record.m_mode = Modes::RESOLVE_WALK;

		data->m_moved = true;
		data->m_walk_record.m_anim_time = record.m_anim_time;
		data->m_walk_record.m_origin = record.m_origin;
		data->m_walk_record.m_body = record.m_body;

		data->m_body_update = record.m_anim_time + 0.22f;
		data->m_body_delta = 0.f;
		data->m_update_count = 0;
		data->m_body_data.clear( );
		return;
	}

	if ( previous ) {
		if ( record.m_body != previous->m_body ) {
			OnBodyUpdate( data, record, previous );
			return;
		}

		if ( record.m_anim_time > data->m_body_update ) {
			OnBodyUpdate( data, record, previous );
			return;
		}
	}

	record.m_mode = Modes::RESOLVE_STAND;
}

void Resolver::ResolveAngles( Player* player, LagRecord& record, LagRecord* previous ) {
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	if ( !player->enemy( g_cl.m_local ) )
		return;

	CCSGOPlayerAnimState* state = player->m_PlayerAnimState( );
	if ( !state )
		return;

	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
	if ( !data )
		return;

	/* handle lby data*/
	const int& act = data->m_player->GetSequenceActivity( record.m_layers[ 3 ].m_sequence );
	SetResolverMode( data, record, previous, act );

	while ( data->m_body_data.size( ) > 4 )
		data->m_body_data.pop_back( );

	static player_info_t info;
	g_csgo.m_engine->GetPlayerInfo( player->index( ), &info );

	if ( info.m_fake_player )
		return;

	if ( previous ) {
		if ( record.m_lag == 1 && previous->m_lag == 1 )
			return;
	}

	if ( ( record.m_mode == Modes::RESOLVE_STAND || record.m_mode == Modes::RESOLVE_AIR ) && g_cl.m_correction_override && m_override.m_player == record.m_player ) {
		record.m_eye_angles.y = GetAwayAngle( record.m_player ) + m_override.m_yaw;
	}
	else {
		switch ( record.m_mode ) {
		case Modes::RESOLVE_WALK: {
			record.m_eye_angles.y = record.m_body;
		} break;
		case Modes::RESOLVE_UPDATE: {
			record.m_eye_angles.y = record.m_body;
		} break;
		case Modes::RESOLVE_STAND: {
			ResolveStand( data, record, previous, act );
		} break;
		case Modes::RESOLVE_AIR: {
			ResolveAir( data, record, previous, state );
		} break;
		}
	}
}

void Resolver::ResolveStand( AimPlayer* data, LagRecord& record, LagRecord* previous, const int& act ) {
	const float& away = math::NormalizedAngle( GetAwayAngle( record.m_player ) + 180.f );

	if ( data->m_moved ) {
		if ( record.m_anim_time <= ( data->m_walk_record.m_anim_time - 0.22f ) && data->m_stand_index[ StandBrute::BREAKMOVE ] < 2 ) {
			record.m_mode = Modes::RESOLVE_STAND_STOPPEDMOVING;
			record.m_eye_angles.y = record.m_body;
			return;
		}

		float body_delta = data->m_body_delta;
		if ( record.m_layers[ 3 ].m_weight <= 0.0f || act == 980 ) {
			body_delta = std::min( std::max( body_delta, -35.f ), 120.f );

			if ( !data->m_update_count )
				body_delta = 0.f;
		}
		else {
			if ( data->m_update_count ) {
				const float delta = math::NormalizedAngle( record.m_body - data->m_walk_record.m_body );
				if ( fabs( delta ) > 70.f && fabs( body_delta ) <= 35.f )
					body_delta = 180.f;

				const float backwards_delta = math::NormalizedAngle( away - record.m_body );
				if ( fabs( backwards_delta ) <= 35.f ) {
					record.m_eye_angles.y = away;
				}
			}
		}

		record.m_mode = Modes::RESOLVE_STAND_LOGIC;
		switch ( data->m_stand_index[ StandBrute::LOGIC ] % 3 ) {
		case 0:
			record.m_eye_angles.y = record.m_body - body_delta;
			break;
		case 1:
			record.m_eye_angles.y = data->m_freestand[ 0 ];
			break;
		case 2:
			record.m_eye_angles.y = data->m_freestand[ 1 ];
			break;
		}
	}

	record.m_mode = Modes::RESOLVE_STAND;
	switch ( data->m_stand_index[ StandBrute::NORMAL ] % 4 ) {
	case 0:
		record.m_eye_angles.y = record.m_body;
		break;
	case 1:
		record.m_eye_angles.y = data->m_freestand[ 0 ];
		break;
	case 2:
		record.m_eye_angles.y = data->m_freestand[ 1 ];
		break;
	case 3:
		record.m_eye_angles.y = away;
		break;
	}
}

void Resolver::ResolveAir( AimPlayer* data, LagRecord& record, LagRecord* previous, CCSGOPlayerAnimState* state ) {
	if ( g_menu.main.config.mode.get( ) == 1 ) {
		AirNS( data, record );
		return;
	}

	const float& away = math::NormalizedAngle( GetAwayAngle( record.m_player ) + 180.f );
	ang_t velocity_angle = { 0, away, 0 };

	if ( record.m_velocity.length_2d( ) >= 25.f ) {
		math::VectorAngles( record.m_velocity, velocity_angle );
		velocity_angle.y += 180.f;
	}

	switch ( data->m_air_index % 3 ) {
	case 0:
		record.m_eye_angles.y = record.m_body;
		break;
	case 1:
		record.m_eye_angles.y = velocity_angle.y;
		break;
	case 2:
		record.m_eye_angles.y = away;
		break;
	}
}

void Resolver::StandNS( AimPlayer* data, LagRecord& record ) {
	// get away angles.
	float away = GetAwayAngle( record.m_player );

	switch ( data->m_shots % 8 ) {
	case 0:
		record.m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record.m_eye_angles.y = away + 90.f;
		break;
	case 2:
		record.m_eye_angles.y = away - 90.f;
		break;

	case 3:
		record.m_eye_angles.y = away + 45.f;
		break;
	case 4:
		record.m_eye_angles.y = away - 45.f;
		break;

	case 5:
		record.m_eye_angles.y = away + 135.f;
		break;
	case 6:
		record.m_eye_angles.y = away - 135.f;
		break;

	case 7:
		record.m_eye_angles.y = away + 0.f;
		break;

	default:
		break;
	}
}

void Resolver::AirNS( AimPlayer* data, LagRecord& record ) {
	// get away angles.
	float away = GetAwayAngle( record.m_player );

	switch ( data->m_shots % 8 ) {
	case 0:
		record.m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record.m_eye_angles.y = away + 90.f;
		break;
	case 2:
		record.m_eye_angles.y = away;
		break;

	case 3:
		record.m_eye_angles.y = away - 90.f;
		break;
	case 4:
		record.m_eye_angles.y = away + 135.f;
		break;

	case 5:
		record.m_eye_angles.y = away - 135.f;
		break;
	case 6:
		record.m_eye_angles.y = away + 45.f;
		break;

	case 7:
		record.m_eye_angles.y = away - 45.f;
		break;

	default:
		break;
	}
}