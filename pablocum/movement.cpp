#include "includes.h"

Movement g_movement{ };;

float Player::get_max_speed( )
{
	Weapon* weapon = GetActiveWeapon( );
	if ( !weapon )
		return 260.f;

	WeaponInfo* weapon_info = weapon->GetWpnData( );
	if ( !weapon_info )
		return 260.f;

	return m_bIsScoped( ) ? weapon_info->m_max_player_speed_alt : weapon_info->m_max_player_speed;
}

void Movement::JumpRelated( )
{
	if ( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP )
		return;

	if ( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
	{
		// bhop.
		if ( g_menu.main.movement.bhop.get( ) )
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;

		// duck jump ( crate jump ).
		if ( g_menu.main.movement.airduck.get( ) )
			g_cl.m_cmd->m_buttons |= IN_DUCK;
	}
}

void Rotate( float angle )
{
	const auto rot = math::deg_to_rad( g_cl.m_cmd->m_view_angles.y - angle );
	const auto forward = std::cos( rot ) * g_cl.m_cmd->m_forward_move - std::sin( rot ) * g_cl.m_cmd->m_side_move;
	g_cl.m_cmd->m_side_move = std::sin( rot ) * g_cl.m_cmd->m_forward_move + std::cos( rot ) * g_cl.m_cmd->m_side_move;
	g_cl.m_cmd->m_forward_move = forward;
}

void Movement::Strafe( )
{
	// don't strafe while noclipping or on ladders..
	if ( !g_cl.m_local || g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	if ( ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) && !( g_cl.m_cmd->m_buttons & IN_JUMP ) )
		return;

	if ( g_cl.m_cmd->m_buttons & IN_SPEED )
	{
		FullStop( );
		return;
	}

	const vec3_t& velocity = g_cl.m_local->m_vecVelocity( );

	float yaw = std::remainderf( g_cl.m_view_angles.y, 360.f );

	float offset = 0.f;
	if ( g_cl.m_cmd->m_buttons & IN_MOVELEFT )
		offset += 90.f;
	if ( g_cl.m_cmd->m_buttons & IN_MOVERIGHT )
		offset -= 90.f;
	if ( g_cl.m_cmd->m_buttons & IN_FORWARD )
		offset *= .5f;
	else if ( g_cl.m_cmd->m_buttons & IN_BACK )
		offset = ( -offset * .5f ) + 180.f;

	yaw += offset;

	float velocity_angle = math::rad_to_deg( std::atan2f( velocity.y, velocity.x ) );
	if ( velocity_angle < 0.f )
		velocity_angle += 360.f;

	velocity_angle -= roundf( velocity_angle / 360.f ) * 360.f;

	const auto speed = velocity.length_2d( );
	const auto ideal = speed > 0.f ? std::clamp( math::rad_to_deg( std::atan2f( 15.f, speed ) ), 0.f, 45.f ) : 0.f;

	const auto correct = ( 100.f /*- g_menu.main.movement.strafe_smooth.get( )*/ ) * .01f * ( ideal * 2.0f );
	g_cl.m_cmd->m_forward_move = 0.f;
	const auto velocity_delta = math::NormalizedAngle( yaw - velocity_angle );

	if ( ( fabsf( velocity_delta ) > 170.f || velocity_delta > correct ) && speed > 80.f )
	{
		yaw = correct + velocity_angle;
		g_cl.m_cmd->m_side_move = -450.f;
		Rotate( math::NormalizedAngle( yaw ) );
		return;
	}

	m_invert = !m_invert;
	if ( -correct <= velocity_delta || speed <= 80.f )
	{
		if ( m_invert )
		{
			yaw -= ideal;
			g_cl.m_cmd->m_side_move = -450.f;
		}
		else
		{
			yaw += ideal;
			g_cl.m_cmd->m_side_move = 450.f;
		}
		Rotate( math::NormalizedAngle( yaw ) );
	}
	else
	{
		yaw = velocity_angle - correct;
		g_cl.m_cmd->m_side_move = 450.f;
		Rotate( math::NormalizedAngle( yaw ) );
	}
}

// onetap v2
void Movement::AutoStop( )
{
	const vec3_t& velocity = g_inputpred.m_velocity;

	const float& max_speed = std::floorf( g_cl.m_local->get_max_speed( ) * 0.33f );

	vec2_t tmp_move{ g_cl.m_cmd->m_forward_move, g_cl.m_cmd->m_side_move };
	const float& speed2d = std::floorf( velocity.length_2d( ) );

	g_cl.m_cmd->m_buttons &= ~IN_SPEED;

	if ( speed2d <= max_speed )
	{
		const float cmd_move = tmp_move.length( );

		if ( cmd_move > 0.0f )
		{
			tmp_move.x = ( tmp_move.x / cmd_move ) * max_speed;
			tmp_move.y = ( tmp_move.y / cmd_move ) * max_speed;
		}
	}
	else
	{
		static ang_t angle;
		math::VectorAngles( velocity, angle );
		angle.y = g_cl.m_view_angles.y - angle.y;
		static vec3_t direction;
		math::AngleVectors( angle, &direction );
		tmp_move.x = direction.x * -450.f;
		tmp_move.y = direction.y * -450.f;
	}

	g_cl.m_cmd->m_forward_move = std::clamp( tmp_move.x, -450.f, 450.f );
	g_cl.m_cmd->m_side_move = std::clamp( tmp_move.y, -450.f, 450.f );
}

void Movement::FullStop( )
{
	const float& max_speed = g_cl.m_local->get_max_speed( );
	const float& friction = g_csgo.sv_friction->GetFloat( ) * g_cl.m_local->m_surfaceFriction( );

	const vec3_t& velocity = g_inputpred.m_velocity;

	if ( ( max_speed * friction * g_csgo.m_globals->m_interval ) >= velocity.length_2d( ) ) {
		g_cl.m_cmd->m_forward_move = 0.f;
		g_cl.m_cmd->m_side_move = 0.f;
		return;
	}

	static ang_t angle;
	math::VectorAngles( velocity, angle );
	angle.y = g_cl.m_view_angles.y - angle.y;

	static vec3_t direction;
	math::AngleVectors( angle, &direction );

	g_cl.m_cmd->m_forward_move = std::clamp( direction.x * -450.f, -450.f, 450.f );
	g_cl.m_cmd->m_side_move = std::clamp( direction.y * -450.f, -450.f, 450.f );
}
void Movement::QuickStop() {
	// convert velocity to angular momentum.
	ang_t angle;
	math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);

	// get our current speed of travel.
	float speed = g_cl.m_local->m_vecVelocity().length();

	// fix direction by factoring in where we are looking.
	angle.y = g_cl.m_view_angles.y - angle.y;

	// convert corrected angle back to a direction.
	vec3_t direction;
	math::AngleVectors(angle, &direction);

	vec3_t stop = direction * -speed;

	if (g_cl.m_speed > 13.f) {
		g_cl.m_cmd->m_forward_move = stop.x;
		g_cl.m_cmd->m_side_move = stop.y;
	}
	else {
		g_cl.m_cmd->m_forward_move = 0.f;
		g_cl.m_cmd->m_side_move = 0.f;
	}
}
void Movement::FixMove( CUserCmd* cmd )
{
	vec3_t  move, dir;
	float   delta, len;
	ang_t   move_angle;

	// convert movement to vector.
	move = { cmd->m_forward_move, cmd->m_side_move, 0.f };

	// get move length and ensure we're using a unit vector ( vector with length of 1 ).
	len = move.normalize( );
	if ( !len )
	{
		g_cl.m_fixed_movement = true;
		return;
	}

	// convert move to an angle.
	math::VectorAngles( move, move_angle );

	// calculate yaw delta.
	delta = ( cmd->m_view_angles.y - g_cl.m_strafe_angles.y );

	// accumulate yaw delta.
	move_angle.y += delta;

	// calculate our new move direction.
	// dir = move_angle_forward * move_length
	math::AngleVectors( move_angle, &dir );

	// scale to og movement.
	dir *= len;

	// fix ladder and noclip.
	if ( g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
	{
		// invert directon for up and down.
		if ( cmd->m_view_angles.x >= 45.f && g_cl.m_strafe_angles.x < 45.f && std::abs( delta ) <= 65.f )
			dir.x = -dir.x;

		// write to movement.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;
	}

	// we are moving normally.
	else
	{
		// we must do this for pitch angles that are out of bounds.
		if ( cmd->m_view_angles.x < -90.f || cmd->m_view_angles.x > 90.f )
			dir.x = -dir.x;

		// set move.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;
	}

	cmd->m_buttons &= ~( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT );

	if ( g_menu.main.movement.slidewalk.get( ) && g_cl.m_local->m_MoveType( ) != MOVETYPE_LADDER ) {
		if ( cmd->m_forward_move > 0.f )
			cmd->m_buttons |= IN_BACK;

		else if ( cmd->m_forward_move < 0.f )
			cmd->m_buttons |= IN_FORWARD;

		if ( cmd->m_side_move > 0.f )
			cmd->m_buttons |= IN_MOVELEFT;

		else if ( cmd->m_side_move < 0.f )
			cmd->m_buttons |= IN_MOVERIGHT;
	}
	else {
		if ( cmd->m_forward_move > 0.f )
			cmd->m_buttons |= IN_FORWARD;

		else if ( cmd->m_forward_move < 0.f )
			cmd->m_buttons |= IN_BACK;

		if ( cmd->m_side_move > 0.f )
			cmd->m_buttons |= IN_MOVERIGHT;

		else if ( cmd->m_side_move < 0.f )
			cmd->m_buttons |= IN_MOVELEFT;
	}

	g_cl.m_fixed_movement = true;
}

void Movement::AutoPeek() {
	m_autopeek = g_input.GetKeyState(g_menu.main.movement.autopeek.get());

	if (!m_autopeek) {
		m_target_pos = g_cl.m_local->m_vecOrigin(); // 현재 위치 저장
		m_reached_target = false;
		g_cl.m_pre_autopeek_pos = g_cl.m_local->m_vecOrigin(); // autopeek 위치 저장
		std::memcpy(g_cl.m_pre_autopeek_bones, g_cl.m_local->m_BoneCache().m_pCachedBones, sizeof(matrix3x4_t) * g_cl.m_local->m_BoneCache().m_CachedBoneCount);
		return;
	}

	if (g_cl.m_old_shot)
		m_reached_target = false;

	vec3_t dist = g_cl.m_pre_autopeek_pos - g_cl.m_local->m_vecOrigin(); // 기존 위치로부터의 거리 계산

	if (dist.length_2d() <= 5.f) {
		if (!m_reached_target)
			QuickStop();
		m_reached_target = true;
	}

	if (m_reached_target)
		return;

	g_cl.m_cmd->m_forward_move = 450.f;
	g_cl.m_cmd->m_side_move = 0.f;

	math::VectorAngles(dist, g_cl.m_strafe_angles);

}

void Movement::FakeWalk( )
{
	vec3_t velocity{ g_cl.m_local->m_vecVelocity( ) };
	int    ticks{ }, max{ 16 };

	static vec3_t previousOrigin = g_cl.m_local->m_vecOrigin( );

	if ( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
	{
		previousOrigin = g_cl.m_local->m_vecOrigin( );
		return;
	}

	if ( ( g_menu.main.antiaim.fake_desync.get( ) || g_hvh.m_exploit ) && ( g_cl.m_fakewalk || ( g_cl.m_cmd->m_buttons & IN_SPEED ) ) )
	{
		previousOrigin = g_cl.m_local->m_vecOrigin( );

		g_cl.m_cmd->m_buttons &= ~IN_SPEED;
		if ( g_hvh.m_exploit )
		{
			g_cl.m_cmd->m_side_move *= 0.2f;
			g_cl.m_cmd->m_forward_move *= 0.2f;
		}
		else
		{
			g_cl.m_cmd->m_side_move *= 0.05f;
			g_cl.m_cmd->m_forward_move *= 0.05f;
		}

		return;
	}

	if ( !g_cl.m_fakewalk )
	{
		previousOrigin = g_cl.m_local->m_vecOrigin( );
		return;
	}

	if ( g_cl.m_flick ) {
		FullStop( );
	}

	/* data */
	float serverSurfaceFriction = g_csgo.sv_friction->GetFloat( ) * g_cl.m_local->m_surfaceFriction( );
	vec3_t playerVelocity = g_cl.m_local->m_vecVelocity( );

	float maxSpeed = g_cl.m_local->get_max_speed( ); /* max speed in units */

	if ( g_cl.m_cmd->m_buttons & IN_DUCK )
	{
		maxSpeed *= 0.34f;
	}

	const int maxSimulatedTicks = 15;
	float speedDropPerTick = maxSpeed * serverSurfaceFriction * g_csgo.m_globals->m_interval; /* how much velocity can drop per tick */

	int ticksSimulated = maxSimulatedTicks - g_cl.m_lag;
	bool shouldStopMoving = ticksSimulated <= ( ( ( int )roundf( playerVelocity.length_2d( ) / speedDropPerTick ) ) + 1 );
	bool isChoking = g_cl.m_lag > 0;
	if ( !isChoking )
	{
		FullStop( );
		previousOrigin = g_cl.m_local->m_vecOrigin( );
		return;
	}

	rebuilt_animstate_t* data = &g_cl.m_anim_data.m_data[ g_cl.m_local->index( ) - 1 ].m_server_data;
	if ( g_menu.main.antiaim.enable.get( ) && g_menu.main.antiaim.body_fake_stand.get( ) &&
		( g_cl.m_curtime + 0.15f ) >= data->m_body_update )
	{
		FullStop( );
		return;
	}

	if ( !shouldStopMoving )
		return;

	FullStop( );
}

void Movement::run( )
{
	g_movement.Strafe( );
	g_movement.JumpRelated( );
	g_movement.FakeWalk( );
	g_movement.AutoPeek( );

	if ( g_aimbot.m_stop )
		g_movement.AutoStop( );

	bool should_quickstop = g_cl.m_cmd->m_forward_move == 0.f && g_cl.m_cmd->m_side_move == 0.f && ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND );
	if ( should_quickstop )
	{
		if ( g_menu.main.antiaim.fake_desync.get( ) == 2 && g_cl.m_local->m_vecVelocity( ).length_2d( ) < 25.f )
		{
			const float speed = 16.f * ( 260.f / g_cl.m_local->get_max_speed( ) );
			static bool flip = false;

			if ( !m_did_move && !g_cl.m_lag )
			{
				flip = !flip;

				g_cl.m_cmd->m_forward_move += speed * ( flip ? 1.f : -1.f );
				g_cl.m_cmd->m_side_move += speed * ( flip ? 1.f : -1.f );

				m_did_move = true;
			}
		}
		else if ( g_menu.main.movement.quickstop.get( ) && !( g_cl.m_cmd->m_buttons & IN_JUMP ) )
			FullStop( );
	}
}