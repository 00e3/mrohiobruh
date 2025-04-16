#include "includes.h"

HVH g_hvh{ };;

void HVH::IdealPitch( ) {
	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if ( !state )
		return;

	g_cl.m_cmd->m_view_angles.x = state->m_aim_pitch_min;
}

void HVH::AntiAimPitch( ) {
	bool safe = g_menu.main.config.mode.get( ) == 0;

	switch ( m_pitch ) {
	case 1:
		// down.
		g_cl.m_cmd->m_view_angles.x = safe ? 89.f : 720.f;
		break;

	case 2:
		// up.
		g_cl.m_cmd->m_view_angles.x = safe ? -89.f : -720.f;
		break;

	case 3:
		// random.
		g_cl.m_cmd->m_view_angles.x = g_csgo.RandomFloat( safe ? -89.f : -720.f, safe ? 89.f : 720.f );
		break;

	case 4:
		// ideal.
		IdealPitch( );
		break;

	default:
		break;
	}
}

void HVH::AutoDirection( ) {
	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	if ( !m_target ) {
		// we have a timeout.
		if ( m_auto_last > 0.f && m_auto_time > 0.f && g_cl.m_curtime < ( m_auto_last + m_auto_time ) )
			return;

		// set angle to backwards.
		m_auto = math::NormalizedAngle( m_view - 180.f );
		m_auto_dist = -1.f;
		return;
	}

	/*
	* data struct
	* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	*/

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back( m_view, 180.f );
	angles.emplace_back( m_view, 90.f );
	angles.emplace_back( m_view, -90.f );

	// start the trace at the enemy shoot pos.
	vec3_t start;
	m_target->GetEyePos( &start );

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {
		float yaw = math::NormalizedAngle( it->m_yaw + it->m_add );

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ g_cl.m_shoot_pos.x + std::cos( math::deg_to_rad( yaw ) ) * RANGE,
			g_cl.m_shoot_pos.y + std::sin( math::deg_to_rad( yaw ) ) * RANGE,
			g_cl.m_shoot_pos.z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = start - end;
		float len = dir.normalize( );

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

	if ( !valid ) {
		// set angle to backwards.
		m_auto = math::NormalizedAngle( m_view - 180.f );
		m_auto_dist = -1.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
		[ ]( const AdaptiveAngle& a, const AdaptiveAngle& b ) {
			return a.m_dist > b.m_dist;
		} );

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front( );

	// check if we are not doing a useless change.
	if ( best->m_dist != m_auto_dist ) {
		// set yaw to the best result.
		m_auto = math::NormalizedAngle( best->m_yaw + best->m_add );
		m_auto_dist = best->m_dist;
		m_auto_last = g_cl.m_curtime;
	}
}

float OFFSET;
void HVH::AntiLastMove() {
	if (g_input.GetKeyState(g_menu.main.movement.fakewalk.get()))
		return;

	if (!g_cl.m_local->GetGroundEntity())
		return;

	bool pressed_move = g_cl.m_cmd->m_buttons & IN_FORWARD || g_cl.m_cmd->m_buttons & IN_MOVELEFT || g_cl.m_cmd->m_buttons & IN_BACK || g_cl.m_cmd->m_buttons & IN_MOVERIGHT || g_cl.m_cmd->m_buttons & IN_JUMP;

	if (pressed_move)
		return;

	vec3_t velocity = g_cl.m_local->m_vecVelocity();

	if (velocity.length() < 0.1f)
		return;

	float friction = g_csgo.sv_friction->GetFloat() * g_cl.m_local->m_surfaceFriction();

	int ticks_to_stop;
	for (ticks_to_stop = 0; ticks_to_stop < 16; ++ticks_to_stop) {
		float speed = velocity.length();

		if (speed < 0.1f)
			break;

		float control = std::max(speed, g_csgo.sv_stopspeed->GetFloat());
		float drop = control * friction * g_csgo.m_globals->m_interval;
		float newspeed = std::max(0.f, speed - drop);
		if (newspeed != speed) {
			newspeed /= speed;
			velocity *= newspeed;
		}
	}

	if (ticks_to_stop < (14 - g_csgo.m_cl->m_choked_commands) && !g_csgo.m_cl->m_choked_commands)
		g_notify.add("AVOID-LBY", g_gui.m_color);
		g_cl.m_cmd->m_view_angles.y += OFFSET; // OFFSET = how much you will flick your last moving lby
}

void HVH::GetAntiAimDirection( ) {
	if ( !m_yaw ) {
		m_direction = g_cl.m_cmd->m_view_angles.y;
		return;
	}

	// edge aa.
	if ( g_menu.main.antiaim.edge.get( m_mode ) && g_cl.m_local->m_vecVelocity( ).length( ) < 320.f ) {
		ang_t ang;
		if ( DoEdgeAntiAim( g_cl.m_local, ang ) ) {
			m_direction = ang.y;
			return;
		}
	}

	m_view = g_cl.m_view_angles.y;

	m_target = nullptr;
	m_at_target = std::numeric_limits< float >::max( );

	float  best_fov{ std::numeric_limits< float >::max( ) };
	float  best_dist{ std::numeric_limits< float >::max( ) };

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* target = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		if ( !g_aimbot.IsValidTarget( target ) )
			continue;

		if ( target->dormant( ) )
			continue;

		// 'away distance'.
		if ( m_base_angle == 3 ) {
			// check if a player was closer to us.
			const float& dist = ( target->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ) ).length_sqr( );
			if ( dist < best_dist ) {
				best_dist = dist;
				m_target = target;
			}
			continue;
		}

		// check if a player was closer to our crosshair.
		const float& fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, target->WorldSpaceCenter( ) );
		if ( fov < best_fov ) {
			best_fov = fov;
			m_target = target;
		}
	}

	if ( m_target ) {
		// todo - dex; calculate only the yaw needed for this (if we're not going to use the x component that is).
		ang_t angle;
		math::VectorAngles( m_target->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ), angle );
		m_at_target = angle.y;
	}

	if ( m_base_angle > 0 ) {
		// 'static'.
		if ( m_base_angle == 1 )
			m_view = 0.f;

		// away options.
		else {
			if ( m_target )
				m_view = m_at_target;
		}
	}

	// switch direction modes.
	switch ( m_dir ) {

		// auto.
	case 0:
		AutoDirection( );
		m_direction = m_auto;
		break;

		// backwards.
	case 1:
		m_direction = m_view + 180.f;
		break;

		// left.
	case 2:
		m_direction = m_view + 90.f;
		break;

		// right.
	case 3:
		m_direction = m_view - 90.f;
		break;

		// custom.
	case 4:
		m_direction = m_view + m_dir_custom;
		break;

	default:
		break;
	}

	if ( m_front )
		m_direction = m_view;

	if ( m_back )
		m_direction = m_view + 180.f;

	if ( m_right )
		m_direction = m_view - 90.f;

	if ( m_left )
		m_direction = m_view + 90.f;

	// normalize the direction.
	math::NormalizeAngle( m_direction );
}

bool HVH::DoEdgeAntiAim( Player* player, ang_t& out ) {
	CGameTrace trace;
	static CTraceFilterSimple_game filter{ };

	if ( player->m_MoveType( ) == MOVETYPE_LADDER )
		return false;

	// skip this player in our traces.
	filter.SetPassEntity( player );

	// get player bounds.
	vec3_t mins = player->m_vecMins( );
	vec3_t maxs = player->m_vecMaxs( );

	// make player bounds bigger.
	mins.x -= 20.f;
	mins.y -= 20.f;
	maxs.x += 20.f;
	maxs.y += 20.f;

	// get player origin.
	vec3_t start = player->GetAbsOrigin( );

	// offset the view.
	start.z += 56.f;

	g_csgo.m_engine_trace->TraceRay( Ray( start, start, mins, maxs ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );
	if ( !trace.m_startsolid )
		return false;

	float  smallest = 1.f;
	vec3_t plane;

	// trace around us in a circle, in 20 steps (anti-degree conversion).
	// find the closest object.
	for ( float step{ }; step <= math::pi_2; step += ( math::pi / 10.f ) ) {
		// extend endpoint x units.
		vec3_t end = start;

		// set end point based on range and step.
		end.x += std::cos( step ) * 32.f;
		end.y += std::sin( step ) * 32.f;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f } ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );

		// we found an object closer, then the previouly found object.
		if ( trace.m_fraction < smallest ) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// no valid object was found.
	if ( smallest == 1.f || plane.z >= 0.1f )
		return false;

	// invert the normal of this object
	// this will give us the direction/angle to this object.
	vec3_t inv = -plane;
	vec3_t dir = inv;
	dir.normalize( );

	// extend point into object by 24 units.
	vec3_t point = start;
	point.x += ( dir.x * 24.f );
	point.y += ( dir.y * 24.f );

	// check if we can stick our head into the wall.
	if ( g_csgo.m_engine_trace->GetPointContents( point, CONTENTS_SOLID ) & CONTENTS_SOLID ) {
		// trace from 72 units till 56 units to see if we are standing behind something.
		g_csgo.m_engine_trace->TraceRay( Ray( point + vec3_t{ 0.f, 0.f, 16.f }, point ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );

		// we didnt start in a solid, so we started in air.
		// and we are not in the ground.
		if ( trace.m_fraction < 1.f && !trace.m_startsolid && trace.m_plane.m_normal.z > 0.7f ) {
			// mean we are standing behind a solid object.
			// set our angle to the inversed normal of this object.
			out.y = math::rad_to_deg( std::atan2( inv.y, inv.x ) );
			return true;
		}
	}

	// if we arrived here that mean we could not stick our head into the wall.
	// we can still see if we can stick our head behind/asides the wall.

	// adjust bounds for traces.
	mins = { ( dir.x * -3.f ) - 1.f, ( dir.y * -3.f ) - 1.f, -1.f };
	maxs = { ( dir.x * 3.f ) + 1.f, ( dir.y * 3.f ) + 1.f, 1.f };

	// move this point 48 units to the left 
	// relative to our wall/base point.
	vec3_t left = start;
	left.x = point.x - ( inv.y * 48.f );
	left.y = point.y - ( inv.x * -48.f );

	g_csgo.m_engine_trace->TraceRay( Ray( left, point, mins, maxs ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );
	float l = trace.m_startsolid ? 0.f : trace.m_fraction;

	// move this point 48 units to the right 
	// relative to our wall/base point.
	vec3_t right = start;
	right.x = point.x + ( inv.y * 48.f );
	right.y = point.y + ( inv.x * -48.f );

	g_csgo.m_engine_trace->TraceRay( Ray( right, point, mins, maxs ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );
	float r = trace.m_startsolid ? 0.f : trace.m_fraction;

	// both are solid, no edge.
	if ( l == 0.f && r == 0.f )
		return false;

	// set out to inversed normal.
	out.y = math::rad_to_deg( std::atan2( inv.y, inv.x ) );

	// left started solid.
	// set angle to the left.
	if ( l == 0.f ) {
		out.y += 90.f;
		return true;
	}

	// right started solid.
	// set angle to the right.
	if ( r == 0.f ) {
		out.y -= 90.f;
		return true;
	}

	return false;
}

bool HVH::HandleLby( rebuilt_animstate_t& data, const int& body_mode ) {
	if ( m_mode != AntiAimMode::STAND )
		return false;

	if ( body_mode < 1 )
		return false;

	if ( ( g_cl.m_curtime - 1.1f ) > g_cl.m_body_pred )
		return false;

	if ( g_menu.main.antiaim.disable_body_fake_duck.get( ) && ( g_cl.m_cmd->m_buttons & IN_DUCK ) )
		return false;

	if ( g_cl.m_curtime > g_cl.m_body_pred ) {
		g_cl.m_cmd->m_view_angles.y = m_break_yaw;
		g_cl.m_flick = true;
		return true;
	}

	if ( g_cl.m_curtime <= ( g_cl.m_body_pred - g_csgo.m_globals->m_interval ) )
		return false;

	m_break_yaw = m_direction;

	switch ( body_mode ) {
	case 1: {
		m_break_yaw -= 110.f * ( m_inverter ? 1.f : -1.f );
	} break;
	case 2: {
		m_break_yaw -= 180.f;
	} break;
	case 3: {
		m_break_yaw -= 110.f * ( ( g_cl.m_lby_increment % 2 ) ? 1.f : -1.f );
	} break;
	case 4: {
		m_break_yaw = g_cl.m_move_yaw;
	} break;
	case 5: {
		m_break_yaw += 35.f * ( m_inverter ? 1.f : -1.f );
	} break;
	}

	math::NormalizeAngle( m_break_yaw );

	g_cl.m_anim_data.UpdateLayers( g_cl.m_local, { 0, m_break_yaw, 0 }, &data, g_cl.m_body_pred + ( g_csgo.m_globals->m_interval * 2.f ), false );

	const float delta = math::NormalizedAngle( data.m_foot_yaw - math::NormalizedAngle( m_direction ) );

	if ( fabs( delta ) <= 100.f ) {
		g_cl.m_flick = true;

		if ( delta > 0.f ) {
			g_cl.m_cmd->m_view_angles.y = m_break_yaw - 100.f;
			return true;
		}

		g_cl.m_cmd->m_view_angles.y = m_break_yaw + 100.f;
		return true;
	}

	return false;
}

void HVH::DoRealAntiAim( ) {
	rebuilt_animstate_t data = g_cl.m_anim_data.m_data[ g_cl.m_local->index( ) - 1 ].m_server_data;
	const rebuilt_animstate_t* fake_data = &g_cl.m_anim_data.m_fake_data;

	int body_fake = g_menu.main.antiaim.body_fake_stand.get( );

	if ( !m_yaw )
		return;

	if ( m_exploit )
		body_fake = 0;

	if ( HandleLby( data, body_fake ) )
		return;

	// if we have a yaw active, which is true if we arrived here.
	// set the yaw to the direction before applying any other operations.
	g_cl.m_cmd->m_view_angles.y = m_direction;
	float range = m_jitter_range / 2.f;
	if ( !m_exploit ) {
		switch ( m_yaw ) {

			// direction.
		case 1:
			// do nothing, yaw already is direction.
			break;

			// jitter.
		case 2: {

			// get the range from the menu.
			AntiLastMove();
			// set angle.
			//g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat( -range, range );
			break;
		}
		case 3: {
			g_cl.m_cmd->m_view_angles.y += m_jitter ? -m_jitter_range : m_jitter_range;
			m_jitter = !m_jitter;

		}

			  // rotate.
		case 4: {
			// set base angle.
			g_cl.m_cmd->m_view_angles.y = ( m_direction - m_rot_range / 2.f );

			// apply spin.
			g_cl.m_cmd->m_view_angles.y += std::fmod( g_cl.m_curtime * ( m_rot_speed * 20.f ), m_rot_range );

			break;
		}

			  // random.
		case 5:
			// get the range from the menu.


			// set angle.
			g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat(-range, range);
			break;
		default:
			break;
		}

		if ( body_fake == 5 )
			m_break_yaw -= 35.f * ( m_inverter ? 1.f : -1.f );
	}

	if ( m_mode == AntiAimMode::STAND || !m_exploit ) {
		m_desync_yaw = math::NormalizedAngle( fake_data->m_foot_yaw );
		m_balance_desync = false;
	}

	if ( g_menu.main.antiaim.fake_desync.get( ) || m_exploit ) {
		if ( !m_exploit )
			g_cl.m_cmd->m_view_angles.y -= g_menu.main.antiaim.desync_yaw_add.get( ) * ( m_inverter ? 1.f : -1.f );

		if ( m_mode == AntiAimMode::STAND ) {
			if ( g_cl.m_curtime <= g_cl.m_body_pred ) {
				if ( m_exploit )
					g_cl.m_cmd->m_view_angles.y = data.m_body_yaw;

				if ( !m_old_desync ) {
					m_twist_side = ( m_inverter ? 1.f : -1.f );

					g_cl.m_cmd->m_view_angles.y += 116.f * m_twist_side;
					m_desync = true;
					m_extend_desync = false;
				}
			}

			return;
		}
	}

	if ( m_exploit ) {
		if ( m_mode == AntiAimMode::WALK ) {
			const float wanted_yaw = math::NormalizedAngle( math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y ) - ( g_menu.main.antiaim.desync_yaw_add.get( ) * m_twist_side ) );
			const float add = fabs( m_exploit_data.m_increment * ( 30.0f + 20.0f * m_exploit_data.m_walk_to_run ) );
			const float approached_yaw = math::NormalizedAngle( math::ApproachAngle( wanted_yaw, m_desync_yaw, add ) );
			const float desync_to_wanted = math::NormalizedAngle( wanted_yaw - m_desync_yaw );

			if ( ( desync_to_wanted > 0.f && m_twist_side == -1 || desync_to_wanted < 0.f && m_twist_side == 1 ) &&
				fabs( math::NormalizedAngle( wanted_yaw - m_desync_yaw ) ) >= add ) {

				m_balance_desync = false;
				m_desync_yaw = approached_yaw;
			}
			else
				m_balance_desync = !m_balance_desync;

			g_cl.m_cmd->m_view_angles.y = m_desync_yaw + ( m_balance_desync ? ( 58.f * m_exploit_data.m_max_yaw_multiplier * m_twist_side ) : 0.f );
			return;
		}

		const float foot_delta = math::NormalizedAngle( math::NormalizedAngle( g_cl.m_abs_angle.y ) - math::NormalizedAngle( m_desync_yaw ) );
		if ( fabs( foot_delta ) > 0.001f ) {
			if ( foot_delta > 0.000f ) {
				m_twist_side = 1;
			}
			else {
				m_twist_side = -1;
			}
		}

		return;
	}
	else if ( m_mode == AntiAimMode::WALK && !m_old_desync && g_menu.main.antiaim.fake_desync.get( ) == 3 ) {
		g_cl.m_anim_data.UpdateLayers( g_cl.m_local, g_cl.m_angle, &data, g_cl.m_curtime, false );

		const float max_abs = 58.f * data.m_max_yaw_multiplier;
		const float delta = math::NormalizedAngle( data.m_foot_yaw - math::NormalizedAngle( g_cl.m_angle.y ) );
		const float add = fabs( delta ) > 0.f ? ( delta > 0.f ? ( max_abs / 2.f ) : -( max_abs / 2.f ) ) : 0.f;

		if ( !m_extend_desync ) {
			g_cl.m_cmd->m_view_angles.y = g_cl.m_angle.y + 180.f + add;
		}

		m_extend_desync = !m_extend_desync;
	}
	else if ( g_menu.main.antiaim.avoid_lby.get( ) ) {
		static int side = 0;
		switch ( m_mode ) {
		case AntiAimMode::STAND: {
			if ( body_fake ) {
				const float delta = math::NormalizedAngle( data.m_body_yaw - math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y ) );
				if ( fabs( delta ) > 0.0000f ) {
					side = delta < 0.0000f ? 1 : -1;
				}

				if ( fabs( delta ) <= 35.f ) {
					g_cl.m_cmd->m_view_angles.y = data.m_body_yaw + ( 35.f * side );
				}
			}
			else {
				const float delta = math::NormalizedAngle( data.m_body_yaw - math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y ) );

				if ( fabs( delta ) > 0.0000f ) {
					side = delta < 0.0000f ? 1 : -1;
				}

				g_cl.m_anim_data.UpdateLayers( g_cl.m_local, { 0.f, data.m_body_yaw, 0.f }, &data, g_cl.m_curtime, false );

				const float best_angle = data.m_foot_yaw + ( 35.f * side );

				if ( g_cl.m_curtime > data.m_body_update )
					g_cl.m_cmd->m_view_angles.y = best_angle;
			}
		} break;
		case AntiAimMode::WALK: {
			if ( !g_cl.m_ground && !g_cl.m_in_jump && g_menu.main.antiaim.avoid_lby.get( ) == 2 )
				g_cl.m_cmd->m_view_angles.y = fake_data->m_foot_yaw + 180.f + ( 15.f * side );
		} break;
		case AntiAimMode::AIR: {
			if (!g_menu.main.antiaim.no_avoid_lby_in_air.get() && (g_cl.m_lagcomp))
			{
				const float deltas[ ] = {
					math::NormalizedAngle( g_cl.m_body - fake_data->m_foot_yaw ),
					math::NormalizedAngle( g_cl.m_body - data.m_foot_yaw )
				};

				if ( fabs( deltas[ 0 ] ) > 0.000f )
					side = deltas[ 0 ] > 0.000f ? 1 : -1;
				else if ( fabs( deltas[ 1 ] ) > 0.000f )
					side = deltas[ 1 ] > 0.000f ? -1 : 1;

				if ( g_menu.main.antiaim.avoid_lby.get( ) == 2 )
					g_cl.m_cmd->m_view_angles.y = data.m_body_yaw + ( m_avoid_air * side );
				else
					g_cl.m_cmd->m_view_angles.y = data.m_body_yaw + m_avoid_air;
			}
		} break;
		}
	}
}

void HVH::DoFakeAntiAim() {
	switch (g_menu.main.antiaim.fake_yaw.get()) {

		// default.
	case 1:
		// set base to opposite of direction.
		g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

		// apply 45 degree jitter.
		g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat(-90.f, 90.f);
		break;

		// relative.
	case 2:
		// set base to opposite of direction.
		g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

		// apply offset correction.
		g_cl.m_cmd->m_view_angles.y += g_menu.main.antiaim.fake_relative.get();
		break;

		// relative jitter.
	case 3: {
		// get fake jitter range from menu.
		float range = g_menu.main.antiaim.fake_jitter_range.get() / 2.f;

		// set base to opposite of direction.
		g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

		// apply jitter.
		g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat(-range, range);
		break;
	}

		  // rotate.
	case 4:
		g_cl.m_cmd->m_view_angles.y = m_direction + 90.f + std::fmod(g_csgo.m_globals->m_curtime * 360.f, 180.f);
		break;

		// random.
	case 5:
		g_cl.m_cmd->m_view_angles.y = g_csgo.RandomFloat(-180.f, 180.f);
		break;

		// local view.
	case 6:
		g_cl.m_cmd->m_view_angles.y = g_cl.m_view_angles.y;
		break;

	default:
		break;
	}

	// normalize fake angle.
	math::NormalizeAngle(g_cl.m_cmd->m_view_angles.y);
}

void HVH::AntiAim( ) {
	g_cl.m_flick = false;

	if ( !g_menu.main.antiaim.enable.get( ) )
		return;

	// disable conditions.
	if ( g_csgo.m_gamerules->m_bFreezePeriod( ) || ( g_cl.m_flags & FL_FROZEN ) || g_cl.m_round_end || ( g_cl.m_cmd->m_buttons & IN_USE ) || g_hvh.cumming_cock == true || g_cl.m_shot )
		return;

	m_mode = AntiAimMode::STAND;

	if ( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
		m_mode = AntiAimMode::AIR;

	else if ( g_cl.m_state_velocity.length_2d( ) > 0.1f )
		m_mode = AntiAimMode::WALK;

	switch ( m_mode ) {
	case AntiAimMode::STAND:
		m_pitch = g_menu.main.antiaim.pitch_stand.get( );
		m_yaw = g_menu.main.antiaim.yaw_stand.get( );
		m_jitter_range = g_menu.main.antiaim.jitter_range_stand.get( );
		m_rot_range = g_menu.main.antiaim.rot_range_stand.get( );
		m_rot_speed = g_menu.main.antiaim.rot_speed_stand.get( );
		m_rand_update = g_menu.main.antiaim.rand_update_stand.get( );
		m_dir = g_menu.main.antiaim.dir_stand.get( );
		m_dir_custom = g_menu.main.antiaim.dir_custom_stand.get( );
		m_base_angle = g_menu.main.antiaim.base_angle_stand.get( );
		m_auto_time = g_menu.main.antiaim.dir_time_stand.get( );

		if ( g_menu.main.antiaim.avoid_lby.get( ) == 2 )
			m_avoid_air = g_csgo.RandomFloat( 75.f, 135.f );
		else
			m_avoid_air = 180.f + g_csgo.RandomFloat( -90.f, 90.f );
		break;
	case AntiAimMode::WALK:
		m_pitch = g_menu.main.antiaim.pitch_walk.get( );
		m_yaw = g_menu.main.antiaim.yaw_walk.get( );
		m_jitter_range = g_menu.main.antiaim.jitter_range_walk.get( );
		m_rot_range = g_menu.main.antiaim.rot_range_walk.get( );
		m_rot_speed = g_menu.main.antiaim.rot_speed_walk.get( );
		m_rand_update = g_menu.main.antiaim.rand_update_walk.get( );
		m_dir = g_menu.main.antiaim.dir_walk.get( );
		m_dir_custom = g_menu.main.antiaim.dir_custom_walk.get( );
		m_base_angle = g_menu.main.antiaim.base_angle_walk.get( );
		m_auto_time = g_menu.main.antiaim.dir_time_walk.get( );

		if ( g_menu.main.antiaim.avoid_lby.get( ) == 2 )
			m_avoid_air = g_csgo.RandomFloat( 75.f, 135.f );
		else
			m_avoid_air = 180.f + g_csgo.RandomFloat( -90.f, 90.f );
		break;
	case AntiAimMode::AIR:
		m_pitch = g_menu.main.antiaim.pitch_air.get( );
		m_yaw = g_menu.main.antiaim.yaw_air.get( );
		m_jitter_range = g_menu.main.antiaim.jitter_range_air.get( );
		m_rot_range = g_menu.main.antiaim.rot_range_air.get( );
		m_rot_speed = g_menu.main.antiaim.rot_speed_air.get( );
		m_rand_update = g_menu.main.antiaim.rand_update_air.get( );
		m_dir = g_menu.main.antiaim.dir_air.get( );
		m_dir_custom = g_menu.main.antiaim.dir_custom_air.get( );
		m_base_angle = g_menu.main.antiaim.base_angle_air.get( );
		m_auto_time = g_menu.main.antiaim.dir_time_air.get( );
		break;
	}

	// set pitch.
	AntiAimPitch( );
	// set direction.
	GetAntiAimDirection( );

	if ( g_cl.m_lag ) {
		DoFakeAntiAim( );
		return;
	}

	const rebuilt_animstate_t* fake_data = &g_cl.m_anim_data.m_fake_data;
	g_cl.m_anim_data.UpdateLayers( g_cl.m_local, g_cl.m_angle, &m_exploit_data, g_cl.m_curtime, false );

	DoRealAntiAim( );
}

int HVH::ChokePeek() {
	int damage = 0;
	LagRecord* record = nullptr;

	vec3_t backup_shoot_pos = g_cl.m_shoot_pos;

	if (!g_cl.m_weapon || !g_cl.m_weapon_info)
		return 0;

	if (g_cl.m_local->m_vecVelocity().length() <= 30.f)
		return 0;

	auto m_settings = g_menu.main.aimbot.general;

	switch (g_cl.m_weapon_type) {
	case WEAPONTYPE_SNIPER_RIFLE: {
		if (g_cl.m_weapon_id == G3SG1 || g_cl.m_weapon_id == SCAR20)
			m_settings = g_menu.main.aimbot.auto_sniper;
		else if (g_cl.m_weapon_id == AWP)
			m_settings = g_menu.main.aimbot.awp;
		else if (g_cl.m_weapon_id == SSG08)
			m_settings = g_menu.main.aimbot.scout;
	} break;
	case WEAPONTYPE_PISTOL:
	{
		if (g_cl.m_weapon_id == DEAGLE || g_cl.m_weapon_id == REVOLVER)
			m_settings = g_menu.main.aimbot.heavy_pistol;
		else
			m_settings = g_menu.main.aimbot.pistol;
	} break;
	}

	float max_speed = (g_cl.m_weapon && g_cl.m_weapon_info) ? (g_cl.m_local->m_bIsScoped() ? g_cl.m_weapon_info->m_max_player_speed_alt : g_cl.m_weapon_info->m_max_player_speed) : 260.f;

	g_cl.m_shoot_pos += g_cl.m_local->m_vecVelocity() * (g_csgo.m_globals->m_interval * 4.f * (260.f / max_speed));

	std::vector<int> hitboxes;

	hitboxes.emplace_back(HITBOX_HEAD);
	hitboxes.emplace_back(HITBOX_THORAX);
	hitboxes.emplace_back(HITBOX_BODY);

	// iterate all targets.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player || !player->alive() || player->dormant() || player->m_bIsLocalPlayer())
			continue;

		AimPlayer* data = &g_aimbot.m_players[i - 1];
		if (!data)
			continue;

		if (data->m_records.empty())
			continue;

		// this player broke lagcomp.
		// his bones have been resetup by our lagcomp.
		// therfore now only the front record is valid.

		LagRecord* ideal = &data->m_records.front();


		if (!ideal)
			continue;

		ideal->cache();	

		for (int hb : hitboxes) {
			const model_t* model = player->GetModel();
			if (!model)
				continue;

			studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
			if (!hdr)
				continue;

			mstudiohitboxset_t* set = hdr->GetHitboxSet(player->m_nHitboxSet());
			if (!set)
				continue;

			mstudiobbox_t* bbox = set->GetHitbox(hb);
			if (!bbox)
				continue;

			vec3_t p;

			if (bbox->m_radius) {
				p = (bbox->m_maxs + bbox->m_mins) / 2.f;
				math::VectorTransform(p, ideal->m_bones[bbox->m_bone], p);
			}
			else {
				matrix3x4_t rot_matrix;
				g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

				// apply the rotation to the entity input space (local).
				matrix3x4_t matrix;
				math::ConcatTransforms(ideal->m_bones[bbox->m_bone], rot_matrix, matrix);

				// extract origin from matrix.
				vec3_t origin = matrix.GetOrigin();

				// compute raw center point.
				p = (bbox->m_mins + bbox->m_maxs) / 2.f;

				p = { p.dot(matrix[0]), p.dot(matrix[1]), p.dot(matrix[2]) };

				// transform point to world space.
				p += origin;
			}

			penetration::PenetrationInput_t in;

			int wanted_dmg = g_aimbot.m_damage_override ? g_menu.main.aimbot.dmg_ov.get() : m_settings.minimal_damage.get();

			if (m_settings.minimal_damage_hp.get() && wanted_dmg > player->m_iHealth())
				wanted_dmg = player->m_iHealth();

			in.m_damage = wanted_dmg;
			in.m_damage_pen = wanted_dmg;
			in.m_can_pen = true;
			in.m_target = player;
			in.m_from = g_cl.m_local;
			in.m_pos = p;

			penetration::PenetrationOutput_t out;

			// we can hit p!
			if (penetration::run(&in, &out)) {
				record = ideal;
				damage = out.m_damage;
			}
		}
	}

	g_cl.m_shoot_pos = backup_shoot_pos;

	static bool reset_choke = true;

	if (record && damage) {
		if (reset_choke) {
			reset_choke = false;
			return 1;
		}
		return 2;
	}
	else
		reset_choke = true;

	return 0;
}
void HVH::SendPacket( ) {
	static float last_ground_time = 0.f;
	if ( !g_cl.m_in_jump )
		last_ground_time = g_cl.m_curtime;

	// indicates wether to lag or not.
	bool active{ false };
	int  choke{ 1 };

	if ( g_cl.m_old_shot && !g_cl.m_old_packet ) {
		g_cl.m_packet = true;
		return;
	}

	// fake-lag enabled.
	if ( g_menu.main.antiaim.lag_enable.get( ) && !g_csgo.m_gamerules->m_bFreezePeriod( ) && !( g_cl.m_flags & FL_FROZEN ) ) {
		// limit of lag.
		const int limit = std::min( ( int )g_menu.main.antiaim.lag_limit.get( ), g_cl.m_max_lag );

		// get current origin.
		const vec3_t& cur = g_cl.m_local->m_vecOrigin( );

		// get prevoius origin.
		const vec3_t& prev = g_cl.m_net_pos.empty( ) ? g_cl.m_local->m_vecOrigin( ) : g_cl.m_net_pos.front( ).m_pos;

		// delta between the current origin and the last sent origin.
		const float delta = ( cur - prev ).length_sqr( );

		const bool moving = g_cl.m_local->m_vecVelocity( ).length_2d( ) > ( g_menu.main.antiaim.fake_desync.get( ) ? 25.f : 0.1f ) && ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND );

		const auto& activation = g_menu.main.antiaim.lag_active.GetActiveIndices( );

		for ( auto it = activation.begin( ); it != activation.end( ); it++ ) {
			// move.
			if ( *it == 0 && moving ) {
				active = true;
				break;
			}

			// air.
			if ( *it == 1 && !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) ) {
				active = true;
				break;
			}

			// stand.
			if ( *it == 2 && !moving && ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) ) {
				active = true;
				break;
			}

			// crouch.
			if ( *it == 3 && g_cl.m_local->m_bDucking( ) ) {
				active = true;
				break;
			}
		}

		if ( active ) {
			int mode = g_menu.main.antiaim.lag_mode.get( );

			switch ( mode ) {
			case 0: {
				active = true;
				choke = limit;
			} break;
			case 1: {
				if ( delta <= 4096.f ) {
					active = true;
					choke = limit;
				}
			} break;
			case 2: {
				if ( g_cl.m_lag >= m_random_lag )
					m_random_lag = g_csgo.RandomInt( 1, limit );
				// factor not met, keep choking.
				else {
					active = true;
					choke = limit;
				}
			} break;
			case 3: {
				if ( delta <= 4096.f ) {
					active = true;
					choke = g_cl.m_max_lag;
				}
				else {
					active = true;
					choke = m_fl_switch ? limit : g_cl.m_max_lag;
				}
			} break;
			}
		}
	}

	if ( g_menu.main.antiaim.lag_active.get( 4 ) && !g_cl.m_fakewalk ) {
		int peek = ChokePeek( );

		switch ( peek ) {
		case 1:
			active = true;
			choke = 1;
			break;
		case 2:
			active = true;
			choke = g_cl.m_max_lag;
			break;
		}
	}

	const rebuilt_animstate_t* data = &g_cl.m_anim_data.m_data[ g_cl.m_local->index( ) - 1 ].m_server_data;

	if ( g_menu.main.antiaim.enable.get( ) && g_menu.main.antiaim.body_fake_stand.get( ) ) {
		if ( !g_cl.m_in_jump && g_cl.m_local->m_vecVelocity( ).length_2d( ) <= 0.1f ) {
			if ( ( g_cl.m_curtime + g_csgo.m_globals->m_interval * 3.f ) > data->m_body_update && g_cl.m_curtime <= data->m_body_update ) {
				g_cl.m_packet = true;
				return;
			}
		}
	}

	// force fake-lag to 14 when fakelagging.
	if ( g_input.GetKeyState( g_menu.main.movement.fakewalk.get( ) ) && !g_menu.main.antiaim.fake_desync.get( ) ) {
		active = true;
		choke = g_cl.m_max_lag;
	}

	if ( ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) && !( g_cl.m_flags & FL_ONGROUND ) ) {
		active = true;
		choke = g_cl.m_max_lag;
	}

	//if ( fabs( g_cl.m_curtime - last_ground_time ) < ( g_menu.main.antiaim.lag_airdelay.get( ) / 1000.f ) && g_cl.m_in_jump ) {
	//	active = true;
	//	choke = std::min( choke, 1 );
	//}

	if ( !g_cl.m_in_jump && data->m_landing && g_menu.main.antiaim.enable.get( ) && g_menu.main.antiaim.avoid_lby.get( ) == 2 ) {
		active = true;
		choke = std::min( choke, 1 );
	}

	if ( CLMove::Enabled ) {
		active = true; 
		choke = std::min( choke, g_cl.m_max_lag - CLMove::WantedTicks );
	}

	if (active && !(g_hvh.m_exploit && (g_cl.m_local->m_fFlags() & FL_ONGROUND))) {
		g_cl.m_packet = g_cl.m_lag >= choke;
		return;
	}

	if ( g_menu.main.antiaim.enable.get( ) && g_menu.main.antiaim.fake_yaw.get( ) )
		g_cl.m_packet = g_cl.m_lag >= 1;
}