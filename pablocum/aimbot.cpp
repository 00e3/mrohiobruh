#include "includes.h"
#include <random>

Aimbot g_aimbot{ };

inline WeaponCfg m_settings;

void Aimbot::UpdateSettings()
{
	static int last_weapon_id = -1;

	auto GetWeaponName = [](int id) -> const char* {
		switch (id) {
		case G3SG1:     return "auto";
		case SCAR20:    return "auto";
		case AWP:       return "AWP";
		case SSG08:     return "scout";
		case DEAGLE:    return "heavy pistol";
		case REVOLVER:  return "heavy pistol";
		case ZEUS:      return "zeus";
		default:        return "default";
		}
		};

	if (g_cl.m_weapon && g_cl.m_weapon_id != ZEUS)
	{
		if (g_cl.m_weapon_id != last_weapon_id)
		{
			last_weapon_id = g_cl.m_weapon_id;

			const char* weapon_name = GetWeaponName(g_cl.m_weapon_id);

			g_csgo.m_hud_chat->ChatPrintf(
				tfm::format(" %s[tap] %sLoaded %s%s%s config",
					"\x10", "\x01", "\x04", weapon_name, "\x01").c_str());
		}

		m_settings = g_menu.main.aimbot.general;

		switch (g_cl.m_weapon_type)
		{
		case WEAPONTYPE_SNIPER_RIFLE:
			if (g_cl.m_weapon_id == G3SG1 || g_cl.m_weapon_id == SCAR20)
				m_settings = g_menu.main.aimbot.auto_sniper;
			else if (g_cl.m_weapon_id == AWP)
				m_settings = g_menu.main.aimbot.awp;
			else if (g_cl.m_weapon_id == SSG08)
				m_settings = g_menu.main.aimbot.scout;
			break;

		case WEAPONTYPE_PISTOL:
			if (g_cl.m_weapon_id == DEAGLE || g_cl.m_weapon_id == REVOLVER)
				m_settings = g_menu.main.aimbot.heavy_pistol;
			else
				m_settings = g_menu.main.aimbot.pistol;
			break;
		}

		return;
	}

	m_settings.hitchance_amount.set(55);
	m_settings.accuracy_boost.set(35);

	m_settings.hitbox.clear();
	m_settings.hitbox.select(1);
	m_settings.hitbox.select(2);

	m_settings.hitbox_air = m_settings.hitbox;
	m_settings.minimal_damage.set(15);
	m_settings.minimal_damage_hp.set(true);
	m_settings.penetrate.set(true);

	m_settings.air_scale.set(50);
	m_settings.scale.set(75);
	m_settings.body_scale.set(75);

	m_settings.baim1.clear();
	m_settings.baim2.clear();
	m_settings.prefer_safety.set(true);
	m_settings.force_safety.clear();
	for (int i = 0; i < 4; i++)
		m_settings.force_safety.select(i);
	m_settings.baim_hp.set(100);

	m_settings.auto_stop.clear();
}


void AimPlayer::SyncPlayerVelocity( LagRecord& current, LagRecord* previous )
{
	if ( !( current.m_anim_flags & FL_ONGROUND ) )
		return;

	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState( );
	if ( !state )
		return;

	Weapon* weapon = m_player->GetActiveWeapon( );
	if ( !weapon )
		return;

	const float max_speed = std::fmin( weapon->GetMaxSpeed( ), 0.001f );
	const float& length_2d = std::fmin( current.m_anim_velocity.length_2d( ), max_speed );
	if ( length_2d <= 0.0f )
		return;

	const float z_velocity = current.m_anim_velocity.z;

	// the weight can only be 0 if the player has been standing still for at least 1 tick.
	if ( current.m_layers[ ANIMATION_LAYER_MOVEMENT_MOVE ].m_weight == 0.f )
	{
		current.m_anim_velocity = { 0.f, 0.f, z_velocity };
		return;
	}

	// return if playback rate is too low.
	if ( current.m_layers[ ANIMATION_LAYER_MOVEMENT_MOVE ].m_playback_rate < 0.001f )
	{
		current.m_anim_velocity = { 0.f, 0.f, z_velocity };
		return;
	}

	const C_AnimationLayer& movement = current.m_layers[ ANIMATION_LAYER_MOVEMENT_MOVE ];
	if ( previous )
	{
		const C_AnimationLayer& alive_loop = current.m_layers[ ANIMATION_LAYER_ALIVELOOP ];
		const C_AnimationLayer& old_alive_loop = previous->m_layers[ ANIMATION_LAYER_ALIVELOOP ];

		if ( alive_loop.m_weight > 0.55f && alive_loop.m_weight < 0.9f &&
			alive_loop.m_playback_rate == old_alive_loop.m_playback_rate &&
			alive_loop.m_sequence == old_alive_loop.m_sequence &&
			weapon == state->m_weapon )
		{
			float m_flSpeedAsPortionOfRunTopSpeed = ( 1.f - alive_loop.m_weight ) * ( 0.9f - 0.55f );
			if ( m_flSpeedAsPortionOfRunTopSpeed > 0.f ) {
				m_flSpeedAsPortionOfRunTopSpeed += 0.55f;

				float newSpeed = m_flSpeedAsPortionOfRunTopSpeed * length_2d;
				current.m_anim_velocity = current.m_anim_velocity.normalized( ) * newSpeed;
				current.m_anim_velocity.z = z_velocity;
				return;
			}
		}
	}

	/* alive loop failed :( */
	if ( movement.m_weight > 0.f && movement.m_weight < 1.f )
	{
		float newSpeed = math::Lerp( current.m_anim_duck, max_speed, max_speed * 0.34f ) * movement.m_weight;
		current.m_anim_velocity = current.m_anim_velocity.normalized( ) * newSpeed;
		current.m_anim_velocity.z = z_velocity;
	}
}

void AimPlayer::UpdateAnimations( bool predicted )
{
	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState( );
	if ( !state )
		return;

	const float curtime = g_csgo.m_globals->m_curtime;
	const float frametime = g_csgo.m_globals->m_frametime;

	const bool update_anims = m_player->m_bClientSideAnimation( );
	const int  eflags = m_player->m_iEFlags( );
	const vec3_t abs_origin = m_player->m_vecAbsOrigin( );
	const vec3_t abs_velocity = m_player->m_vecAbsVelocity( );

	// initialize records.
	LagRecord* previous = m_records.empty( ) ? nullptr : &m_records.front( );
	LagRecord& current = m_records.emplace_front( );
	current.store( m_player );

	current.m_mode = Modes::RESOLVE_NONE;

	// player respawned.
	if ( m_player->m_flSpawnTime( ) != m_spawn )
	{
		// reset animation state.
		game::ResetAnimationState( state );
		for ( auto& data : m_anim_data )
			data.reset( );

		// note new spawn time.
		m_spawn = m_player->m_flSpawnTime( );
	}

	state->m_last_update_frame = 0;

	m_player->m_bClientSideAnimation( ) =
		UpdateClientAnimations::InUpdate = true;

	// we have a previous record.
	if ( previous ) {
		m_player->m_bStrafing( ) = current.m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_weight > previous->m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_weight ||
			current.m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_weight >= 1.f;

		if ( ( current.m_origin - previous->m_origin ).length_sqr( ) > 4096.f )
			current.m_record_flags |= RecordFlags::BREAKINGLC;

		for ( int i = 1; i < m_records.size( ); i++ ) {
			LagRecord* record = &m_records[ i ];
			if ( !record )
				continue;

			if ( current.m_sim_time <= record->m_sim_time ) {
				current.m_record_flags |= RecordFlags::SHIFTING;
				break;
			}
		}

		current.m_lag = game::TIME_TO_TICKS( current.m_sim_time - previous->m_sim_time );

		m_player->m_flLowerBodyYawTarget( ) = previous->m_body;
		m_player->SetAnimLayers( previous->m_layers );

		// fix some animstate variables.
		state->m_strafe_change_cycle = previous->m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_cycle;
		state->m_strafe_change_weight = previous->m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_weight;
		state->m_strafe_sequence = previous->m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_sequence;
		state->m_primary_cycle = previous->m_layers[ ANIMATION_LAYER_MOVEMENT_MOVE ].m_cycle;

		// recalculate velocity and set animtime
		if ( current.m_lag >= 1 ) {
			current.m_velocity = ( current.m_origin - previous->m_origin ) / game::TICKS_TO_TIME( current.m_lag );
			current.m_anim_time = previous->m_sim_time + g_csgo.m_globals->m_interval;
		}

		// player is choking.
		if ( current.m_lag > 1 && !predicted ) {
			m_player->m_MoveType( ) = previous->m_move_type;
			m_player->m_iMoveState( ) = previous->m_move_state;

			// set previous flags.
			current.m_anim_flags = previous->m_flags;

			const float frac = std::min( std::max( g_csgo.m_globals->m_interval / game::TICKS_TO_TIME( current.m_lag ), 1.f ), 0.f );

			current.m_anim_duck = math::Lerp( frac, previous->m_duck, current.m_duck );
			current.m_anim_duck_speed = math::Lerp( frac, previous->m_duck_speed, current.m_duck_speed );
			current.m_anim_velocity = math::Lerp( frac, previous->m_velocity, current.m_velocity );
			current.m_anim_origin = math::Lerp( frac, previous->m_origin, current.m_origin );

			// fix flags.
			if ( current.m_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_cycle > previous->m_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_cycle && m_player->m_MoveType( ) != MOVETYPE_LADDER )
				current.m_anim_flags |= FL_ONGROUND;

			if ( current.m_layers[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_cycle > previous->m_layers[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_cycle && m_player->m_MoveType( ) != MOVETYPE_LADDER )
				current.m_anim_flags &= FL_ONGROUND;

			const bool has_ground_ent = current.m_anim_flags & FL_ONGROUND;

			/* flag and duck amt fix */
			if ( current.m_anim_duck > previous->m_anim_duck )
			{
				if ( current.m_anim_duck >= 1.f || !has_ground_ent )
				{
					current.m_anim_flags |= ( FL_ANIMDUCKING | FL_DUCKING );
					current.m_anim_duck = 1.f;
				}

				if ( current.m_anim_duck >= 0.1f && !( current.m_anim_flags & FL_ANIMDUCKING ) )
				{
					current.m_anim_flags |= FL_ANIMDUCKING;
				}
			}
			else if ( current.m_anim_duck < previous->m_anim_duck )
			{
				float duckSpeed = std::max( 1.5f, current.m_anim_duck_speed );
				if ( m_player->m_bIsDefusing( ) )
				{
					duckSpeed *= 0.4f;
				}

				if ( current.m_anim_duck <= 0.f || !has_ground_ent )
				{
					current.m_anim_flags &= ~( FL_DUCKING | FL_ANIMDUCKING );
					current.m_anim_duck = 0.f;
				}

				if ( current.m_anim_duck <= 0.75f && current.m_anim_flags & ( FL_ANIMDUCKING | FL_DUCKING ) )
				{
					current.m_anim_flags &= ~( FL_ANIMDUCKING | FL_DUCKING );
				}
			}

			g_resolver.MatchShot( this, current, previous );
		}
		else
			current.init_anim_data( );
	}
	else {
		current.init_anim_data( );
	}

	SyncPlayerVelocity( current, previous );

	state->m_weapon = m_player->GetActiveWeapon( );

	m_player->m_iEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;
	m_player->m_fFlags( ) = current.m_anim_flags;
	m_player->m_flDuckAmount( ) = current.m_anim_duck;
	m_player->m_flDuckSpeed( ) = current.m_anim_duck_speed;
	m_player->m_vecOrigin( ) = current.m_anim_origin;
	m_player->m_vecAbsVelocity( ) = m_player->m_vecVelocity( ) = current.m_anim_velocity;

	g_resolver.ResolveAngles( m_player, current, previous );

	const bool unsafe = current.m_mode != Modes::RESOLVE_NONE && current.m_mode != Modes::RESOLVE_WALK && current.m_mode != Modes::RESOLVE_UPDATE;
	if ( !predicted )
	{
		bool old_unsafe = previous ?
			( previous->m_mode != Modes::RESOLVE_NONE && previous->m_mode != Modes::RESOLVE_WALK && previous->m_mode != Modes::RESOLVE_UPDATE ) :
			true;

		int id = 0;
		for ( auto& data : m_anim_data )
		{
			if ( unsafe || old_unsafe ) {
				switch ( id )
				{
				case 1:
					data.m_foot_yaw = math::NormalizedAngle( current.m_eye_angles.y + 58.f );
					break;
				case 2:
					data.m_foot_yaw = math::NormalizedAngle( current.m_eye_angles.y - 58.f );
					break;
				}
			}

			if ( previous )
				std::memcpy( data.m_layers, previous->m_layers, sizeof( C_AnimationLayer ) * 13 );

			g_cl.m_anim_data.UpdateLayers( m_player, current.m_eye_angles, &data, current.m_anim_time, false );

			std::memcpy( current.m_foot_layers[ id ], data.m_layers, sizeof( C_AnimationLayer ) );

			++id;
		}

		g_resolver.CorrectFootYaw( current, previous, state );
	}

	g_csgo.m_globals->m_curtime = current.m_anim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	/* setup intersection matrices */
	static CCSGOPlayerAnimState backup_state;
	m_player->GetAnimState( &backup_state );

	static C_AnimationLayer backup_layers[ 13 ];
	m_player->GetAnimLayers( backup_layers );

	static std::vector<anim_backup_data_t> safe_data;
	safe_data.clear( );

	for ( int i = 0; i < ( unsafe ? 7 : 2 ); i++ )
	{
		if ( unsafe )
		{
			state->m_abs_yaw = math::NormalizedAngle( current.m_eye_angles.y + ( ( i + 1 ) * 45.f ) );

			game::UpdateAnimationState( state, { current.m_eye_angles.x, state->m_abs_yaw, 0.f } );
		}
		else
		{
			if ( predicted )
			{
				switch ( i )
				{
				case 0:
					state->m_abs_yaw = math::NormalizedAngle( current.m_eye_angles.y + 58.f );
					break;
				case 1:
					state->m_abs_yaw = math::NormalizedAngle( current.m_eye_angles.y - 58.f );
					break;
				case 2:
					state->m_abs_yaw = math::NormalizedAngle( current.m_eye_angles.y );
					break;
				}
			}

			game::UpdateAnimationState( state, current.m_eye_angles );

			if ( !predicted )
			{
				switch ( i )
				{
				case 0:
					state->m_abs_yaw = m_anim_data[ 1 ].m_foot_yaw;
					g_cl.m_anim_data.SetPoses( m_player, &m_anim_data[ 1 ] );
					break;
				case 1:
					state->m_abs_yaw = m_anim_data[ 2 ].m_foot_yaw;
					g_cl.m_anim_data.SetPoses( m_player, &m_anim_data[ 2 ] );
					break;
				}
			}
		}

		safe_data.emplace_back( ).backup_data( state->m_abs_yaw, m_player->m_AnimOverlay( ), m_player->m_flPoseParameter( ) );

		m_player->SetAnimLayers( backup_layers );
		m_player->SetAnimState( &backup_state );
	}

	game::UpdateAnimationState( state, current.m_eye_angles );

	m_player->GetPoseParameters( current.m_poses );

	m_player->SetAbsOrigin( current.m_origin, true );
	m_player->SetAnimLayers( current.m_layers );

	const float abs_yaw_backup = state->m_abs_yaw;

	/* make sure the count is 0 */
	current.m_safe_matrice_count = 0;

	for ( auto& data : safe_data )
	{
		m_player->SetPoseParameters( data.m_poses );
		m_player->SetAbsAngles( { 0, data.m_abs_yaw, 0 }, true );

		g_bones.Setup( m_player, BONE_USED_BY_ANYTHING, current.m_safe_bones[ current.m_safe_matrice_count ], 128, true, false );

		++current.m_safe_matrice_count;
	}

	m_player->SetPoseParameters( current.m_poses );
	m_player->SetAbsAngles( { 0, abs_yaw_backup, 0 }, true );

	current.m_setup = g_bones.Setup( m_player, BONE_USED_BY_ANYTHING, current.m_bones, 128, m_player->enemy( g_cl.m_local ), !predicted );

	UpdateClientAnimations::InUpdate = false;
	m_player->m_bClientSideAnimation( ) = update_anims;

	m_player->m_vecVelocity( ) = current.m_velocity;
	m_player->m_MoveType( ) = current.m_move_type;
	m_player->m_iMoveState( ) = current.m_move_state;
	m_player->m_fFlags( ) = current.m_flags;
	m_player->m_flDuckAmount( ) = current.m_duck;
	m_player->m_flDuckSpeed( ) = current.m_duck_speed;
	m_player->m_flLowerBodyYawTarget( ) = current.m_body;
	m_player->m_vecAbsVelocity( ) = abs_velocity;
	m_player->m_vecAbsOrigin( ) = abs_origin;

	m_player->m_iEFlags( ) = eflags;

	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = frametime;
}

void AimPlayer::OnNetUpdate( Player* player )
{
	if ( m_player != player )
		m_records.clear( );

	m_player = player;

	g_shots.HandleMisses( this );

	if ( !player->alive( ) || player->dormant( ) )
	{
		if ( !player->alive( ) )
		{
			m_update_count = 0;
			g_resolver.ResetMisses( player->index( ) );
		}

		m_records.clear( );
		return;
	}

	/* update player ptr */
	m_player = player;

	g_cl.GetCheat( player->index( ), true );

	static bool update = false;

	if ( !m_records.empty( ) )
	{
		LagRecord* previous = &m_records.front( );

		/* check if the player did a fake update */
		update = player->m_AnimOverlay( )[ 11 ].m_cycle != previous->m_layers[ 11 ].m_cycle;

		/* fix loop issue */
		if ( !update && ( player->m_flSimulationTime( ) - previous->m_sim_time ) >= 1.f )
			update = true;
	}
	else
		update = true;
	/* we recieved a update */
	if ( update )
		UpdateAnimations( );

	/* no need to store more records than tickrate */
	while ( m_records.size( ) > ( int )( 1.f / g_csgo.m_globals->m_interval ) )
		m_records.pop_back( );
}

void AimPlayer::OnRoundStart( Player* player )
{
	m_player = player;
	m_shots = 0;
	m_missed_shots = 0;

	// reset stand and body index.
	for ( auto& stand_index : m_stand_index )
		stand_index = 0;
	m_air_index = 0;
	m_body_index = 0;

	m_walk_record = MoveData_t{ };
	m_moved = false;
	m_body_delta = 0.f;

	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

void Aimbot::StripAttack( )
{
	if ( g_cl.m_weapon_id == REVOLVER )
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK2;

	else
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
}

void Aimbot::GetAimRecords( AimPlayer* data, target_data_t& info )
{
	info.m_records.clear( );

	if ( data->m_records.empty( ) )
		return;

	LagRecord* first_valid = nullptr;
	LagRecord* last_valid = nullptr;
	LagRecord* ideal = nullptr;

	for ( LagRecord& record : data->m_records )
	{
		/* check if this record is lagcompensated */
		if ( record.m_record_flags & RecordFlags::BREAKINGLC )
			break;

		if ( record.m_record_flags & RecordFlags::SHIFTING )
			continue;

		/* do not target records where the player is immune */
		if ( record.immune( ) )
			continue;

		/* check if we can even hit the record and if we have setuped the bone matrix */
		if ( !g_lagcomp.ValidRecord( record.m_sim_time ) || !record.m_setup )
			continue;

		if ( !first_valid )
			first_valid = &record;
		else
			last_valid = &record;

		switch ( record.m_mode )
		{
		case Modes::RESOLVE_NONE:
		case Modes::RESOLVE_WALK:
		case Modes::RESOLVE_DESYNC:
		case Modes::RESOLVE_UPDATE:
			if ( !ideal )
				ideal = &record;
			break;
		}
	}

	if ( first_valid ) // if there's no first_valid, it means there's no records valid at all.
	{
		if ( ideal && ideal != first_valid )
			info.m_records.emplace_back( ideal );

		info.m_records.emplace_back( first_valid );

		if ( last_valid && last_valid != first_valid && last_valid != ideal )
			info.m_records.emplace_back( last_valid );
	}
	else
	{
		LagRecord* predicted = g_lagcomp.StartPrediction( data );

		if ( predicted )
			info.m_records.emplace_back( predicted );
	}

	LagRecord copy = data->m_records[ 0 ];
	if ( copy.m_anim_flags & FL_ONGROUND ) {
		info.m_predicted_land = false;
		return;
	}

	const int old_flags = copy.m_flags;

	g_lagcomp.PlayerMove( &copy );

	info.m_predicted_land = ( copy.m_flags & FL_ONGROUND ) || ( old_flags & FL_ONGROUND );
}

bool Aimbot::ShouldForceSafety( const int& mode )
{
	switch ( mode )
	{
	case Modes::RESOLVE_NONE:
		return true;

	case Modes::RESOLVE_WALK:
	case Modes::RESOLVE_DESYNC:
		return m_settings.force_safety.get( 0 );

	case Modes::RESOLVE_AIR:
		return m_settings.force_safety.get( 2 );

	case Modes::RESOLVE_UPDATE:
		return m_settings.force_safety.get( 3 );

	default:
		return m_settings.force_safety.get( 1 );
	}

	return false;
}

bool Aimbot::PreferBodyAim( const int& hp, const pt_data_t& point ) {
	if ( point.m_lethality )
		return true;

	if ( m_settings.baim1.get( 0 ) // always
		|| m_settings.baim1.get( 1 ) && !( point.m_record->m_anim_flags & FL_ONGROUND )
		|| m_settings.baim1.get( 2 ) && ( point.m_damage * 2 ) >= hp ) {
		return true;
	}

	return false;
}

bool Aimbot::SelectBestPoint( target_data_t& target, const pt_data_t& point )
{
	// we have a previous target!
	if ( target.best_point.m_damage > 0 ) {
		// this point is just... perfect.
		if ( point.m_lethality && point.m_safe ) {
			target.best_point = point;
			return point.m_body;
		}

		if ( !target.best_point.m_lethality || target.best_point.m_safe ) {
			if ( PreferBodyAim( target.m_player_health, point ) ) {
				target.best_point = point;
				return point.m_safe;
			}
		}

		// we found nothing better.
		return false;
	}

	// for now our current point will do.
	target.best_point = point;

	return false;
}

bool Aimbot::ScanPoint( const target_data_t& target, pt_data_t& point )
{
	static penetration::PenetrationInput_t input;
	static penetration::PenetrationOutput_t output;

	input.m_from = g_cl.m_local;
	input.m_target = target.m_player;
	input.m_start = g_cl.m_shoot_pos;
	input.m_pos = point.m_point;
	input.m_damage = input.m_damage_pen = std::min( ( int )( m_damage_override ? g_menu.main.aimbot.dmg_ov.get( ) : m_settings.minimal_damage.get( ) ), target.m_player_health );
	input.m_can_pen = m_settings.penetrate.get( );

	point.m_record->cache( );

	/* we cannot deal our minimum damage */
	if ( !penetration::run( &input, &output ) )
		return false;

	if ( point.m_hitbox == HITBOX_HEAD && output.m_hitgroup != HITGROUP_HEAD )
		return false;

	const vec3_t extrapolated_point = g_cl.m_shoot_pos + ( point.m_point - g_cl.m_shoot_pos ).normalized( ) * 8192.f;

	// check intersections within matricies.
	int intersection_count = point.m_record->m_mode == Modes::RESOLVE_DESYNC ? point.m_record->m_safe_matrice_count : 0;
	for ( int i = 0; i < point.m_record->m_safe_matrice_count; i++ )
	{
		if ( !math::RayIntersectHitbox(
			point.m_record->m_player,
			g_cl.m_shoot_pos,
			extrapolated_point,
			point.m_record->m_safe_bones[ i ],
			point.m_hitbox ) )
			continue;

		++intersection_count;
	}

	point.m_safe = intersection_count >= point.m_record->m_safe_matrice_count;

	if ( m_settings.force_safety.get( 5 ) && target.m_predicted_land && !point.m_safe )
		return false;

	if ( ShouldForceSafety( point.m_record->m_mode ) && !point.m_safe )
		return false;

	point.m_damage = output.m_damage;
	point.m_lethality = point.m_damage >= target.m_player_health;
	point.m_body = output.m_hitgroup == HITGROUP_STOMACH || output.m_hitgroup == HITGROUP_CHEST;// ensure out hitbox is body.
	return true;
}

void Aimbot::Think( )
{
	m_stop = false;
	m_aimbot_targets.clear( );
	m_best_target = pt_data_t( );

	// sanity.
	if ( !g_cl.m_weapon || ( g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS ) )
		return;

	// no grenades or bomb.
	if ( g_cl.m_weapon_type == WEAPONTYPE_GRENADE || g_cl.m_weapon_type == WEAPONTYPE_C4 )
		return;

	// we have no aimbot enabled.
	if ( !g_menu.main.aimbot.enable.get( ) )
		return;

	if (g_menu.main.config.config_logs.get())
		UpdateSettings( );

	vec3_t enemy_pos;
	bool whitelisted;

	// setup bones for all valid targets.
	for ( AimPlayer& data : m_players )
	{
		// we have no records.
		if ( data.m_records.empty( ) )
			continue;

		// the player is null.
		if ( !data.m_player )
			continue;

		// the player isnt a valid target.
		if ( !IsValidTarget( data.m_player ) )
			continue;

		const int& id = data.m_player->index( );

		whitelisted = false;
		for ( const int& wl : m_whitelisted_ids ) {
			if ( wl == id ) {
				whitelisted = true;
				break;
			}
		}

		if ( whitelisted )
			continue;

		target_data_t info;
		GetAimRecords( &data, info );
		if ( info.m_records.empty( ) )
			continue;

		data.m_player->GetEyePos( &enemy_pos );

		info.m_player = data.m_player;
		info.m_player_index = id;
		info.m_player_health = data.m_player->m_iHealth( );
		info.m_lethal = data.m_player->ScaleDamage(
			g_cl.m_weapon_info->m_armor_ratio,
			HITGROUP_STOMACH,
			g_cl.m_weapon_info->m_damage )
			>= info.m_player_health;

		info.m_fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, enemy_pos );
		info.best_point = { };

		m_aimbot_targets.emplace_back( info );
	}

	// no targets alive/enemy
	if ( m_aimbot_targets.empty( ) )
		return;

	// sort by lowest FOV.
	std::sort( m_aimbot_targets.begin( ), m_aimbot_targets.end( ), [ ]( const target_data_t& a, const target_data_t& b ) {
		return a.m_fov < b.m_fov;
		} );

	while ( m_aimbot_targets.size( ) > 2 ) {
		std::random_device rd;  // a seed source for the random number engine
		std::mt19937 gen( rd( ) ); // mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<> distrib( 1, m_aimbot_targets.size( ) - 1 );

		m_aimbot_targets.erase( m_aimbot_targets.begin( ) + distrib( gen ) );
	}

	std::vector< pt_data_t > pts;
	bool has_lethal_body;
	bool has_safepoint;

	// run hitscan etc.
	for ( auto& target : m_aimbot_targets )
	{
		// reset data each target iteration.
		has_lethal_body = false;
		has_safepoint = false;

		if ( !pts.empty( ) )
			pts.clear( );

		// create points for EVERY record, and iterate them AFTER we got all the points for each record.
		// except, you'd want to do bestrecord scan in the record loop so you prevent unneeded loops when you already have lethal stomach or w/e criteria to force hitbox.
		for ( LagRecord* record : target.m_records )
		{
			// create hitboxes for mps
			CreateHitboxes( record );

			if ( m_hitboxes.empty( ) )
				continue;

			// create points to scan.
			for ( const int& hitbox_index : m_hitboxes )
				CreatePoints( pts, hitbox_index, record );
		}

		if ( pts.empty( ) )
			continue;

		// scan the points we created.
		for ( int i = 0; i < pts.size( ); i++ )
		{
			pt_data_t& point = pts[ i ];

			// erase points we cannot shoot.
			if ( !ScanPoint( target, point ) ) {
				pts.erase( pts.begin( ) + i );
				continue;
			}

			if ( point.m_safe )
				has_safepoint = true;

			// we found a lethal point!!!
			if ( point.m_lethality && point.m_body ) {
				has_lethal_body = true;

				if ( point.m_safe )
					break;
			}
		}

		// we cant hit any points.
		if ( pts.empty( ) )
			continue;

		std::sort( pts.begin( ), pts.end( ), [ ]( const pt_data_t& a, const pt_data_t& b ) {
			return a.m_damage > b.m_damage;
			} );

		for ( const pt_data_t& point : pts )
		{
			// we found a lethal point ignore the rest.
			if ( has_lethal_body && !point.m_lethality )
				continue;

			// we found a safepoint ignore the rest.
			if ( has_safepoint && !point.m_safe && m_settings.prefer_safety.get( ) )
				continue;

			// force safety on potentially lethal players.
			if ( target.m_lethal && !point.m_safe && m_settings.force_safety.get( 4 ) )
				continue;

			// select best point.
			if ( SelectBestPoint( target, point ) )
				break;
		}

		if ( !target.best_point.m_record )
			continue;

		if ( target.best_point.m_damage > m_best_target.m_damage )
			m_best_target = target.best_point;
	}

	if ( !m_best_target.m_record )
		return;

	if ( m_settings.auto_stop.get( 0 ) || g_cl.m_weapon_fire )
	{
		if ( m_settings.auto_stop.get( 2 ) || !g_cl.m_in_jump )
		{
			m_stop = true;
		}
	}

	// push the traces.
	m_best_target.m_record->cache( );

	if ( !g_cl.m_weapon_fire )
		return;

	math::VectorAngles( m_best_target.m_point - g_cl.m_shoot_pos, m_aim_angle );

	if ( !CheckHitchance( m_best_target, m_aim_angle ) )
		return;

	if ( g_menu.main.antiaim.enable.get( ) && g_menu.main.antiaim.fake_yaw.get( ) && !g_cl.m_lag )
		return;

	g_cl.m_cmd->m_buttons |= IN_ATTACK;
	g_cl.m_cmd->m_view_angles = m_aim_angle - g_cl.m_local->m_aimPunchAngle( ) * g_csgo.weapon_recoil_scale->GetFloat( );

	if ( !( m_best_target.m_record->m_record_flags & BREAKINGLC ) )
		g_cl.m_cmd->m_tick = game::TIME_TO_TICKS( m_best_target.m_record->m_sim_time + g_cl.m_lerp );

	g_shots.OnShotFire( m_best_target.m_record->m_player, m_best_target.m_damage, m_best_target.m_record, m_best_target.m_point, math::HitboxToHitgroup( m_best_target.m_hitbox ), m_best_target.m_safe );

	if ( g_menu.main.aimbot.aim_matrix.get( ) )
		g_visuals.DrawHitboxMatrix( m_best_target.m_record, g_menu.main.aimbot.aim_matrix_col.get( ), 4.f );

	g_movement.m_auto_peek = true;
}

void Aimbot::CreateHitboxes( const LagRecord* record )
{
	auto& hitboxes = ( !( record->m_anim_flags & FL_ONGROUND ) && !m_settings.hitbox_air.empty( ) ) ?
		m_settings.hitbox_air :
		m_settings.hitbox;

	m_hitboxes.clear( );

	if ( hitboxes.get( 0 ) && !m_force_body )
	{
		m_hitboxes.emplace_back( HITBOX_HEAD );
	}

	// scan stomach first since its the second biggest damage hitbox.
	if ( hitboxes.get( 2 ) )
	{
		m_hitboxes.push_back( HITBOX_BODY );
	}

	if ( hitboxes.get( 1 ) )
	{
		m_hitboxes.push_back( HITBOX_THORAX );
		m_hitboxes.push_back( HITBOX_UPPER_CHEST );
	}

	if ( hitboxes.get( 3 ) )
	{
		m_hitboxes.push_back( HITBOX_L_UPPER_ARM );
		m_hitboxes.push_back( HITBOX_R_UPPER_ARM );
		m_hitboxes.push_back( HITBOX_L_FOREARM );
		m_hitboxes.push_back( HITBOX_R_FOREARM );
	}

	if ( hitboxes.get( 4 ) )
	{
		m_hitboxes.push_back( HITBOX_L_THIGH );
		m_hitboxes.push_back( HITBOX_R_THIGH );
		m_hitboxes.push_back( HITBOX_L_CALF );
		m_hitboxes.push_back( HITBOX_R_CALF );
		m_hitboxes.push_back( HITBOX_L_FOOT );
		m_hitboxes.push_back( HITBOX_R_FOOT );
	}
}

static vec3_t vertice_list_top[ ] = {
		vec3_t( 0.f, 0.f, 0.f ), // top
		vec3_t( 0.f, 0.f, 0.65f ), // top
		vec3_t( 0.f, 0.f, 1.3f ), // top
		vec3_t( 0.25f, 0.f, 1.25f ), // top
		vec3_t( -0.25f, 0.f, 1.25f ), // top
		vec3_t( 0.5f, 0.f, 1.15f ), // top
		vec3_t( -0.5f, 0.f, 1.15f ), // top
		vec3_t( 0.7f, 0.f, 1.f ), // top
		vec3_t( -0.7f, 0.f, 1.f ), // top
		vec3_t( 0.85f, 0.f, 0.85f ), // top
		vec3_t( -0.85f, 0.f, 0.85f ), // top
		vec3_t( 1.f, 0.f, 0.5f ), // center left
		vec3_t( -1.f, 0.f, 0.5f ), // center right
};

void Aimbot::CreatePoints( std::vector< pt_data_t >& points, const int& index, LagRecord* record )
{
	const model_t* model = record->m_player->GetModel( );
	if ( !model )
		return;

	const studiohdr_t* studio_model = g_csgo.m_model_info->GetStudioModel( model );
	if ( !studio_model )
		return;

	const mstudiohitboxset_t* hitbox_set = studio_model->GetHitboxSet( record->m_player->m_nHitboxSet( ) );
	if ( !hitbox_set )
		return;

	const mstudiobbox_t* bbox = hitbox_set->GetHitbox( index );
	if ( !bbox )
		return;

	static std::vector<vec3_t> world_points;
	world_points.clear( );

	// bands up
	float hitbox_scale = index == HITBOX_HEAD ? m_settings.scale.get( ) : m_settings.body_scale.get( );
	if ( !( record->m_anim_flags & FL_ONGROUND ) )
		hitbox_scale *= m_settings.air_scale.get( ) * 0.01f;

	hitbox_scale *= 0.01f;

	if ( bbox->m_radius > 0.f ) {
		static vec3_t world_mins, world_maxs;
		math::VectorTransform( bbox->m_maxs, record->m_bones[ bbox->m_bone ], world_maxs );
		math::VectorTransform( bbox->m_mins, record->m_bones[ bbox->m_bone ], world_mins );

		const vec3_t& center = ( world_mins + world_maxs ) * 0.5f;

		const vec3_t& forward = ( center - g_cl.m_shoot_pos ).normalized( );
		const vec3_t& right = forward.cross( { 0.0f, 0.0f, 1.0f } ).normalized( );
		const vec3_t& up = right.cross( forward ).normalized( );

		world_points.emplace_back( center );

		float scale = bbox->m_radius * hitbox_scale;

		// not enough scale, too close.
		if ( hitbox_scale < 0.01f )
			return;

		// move it to the top for head so it is not centered.
		if ( index == HITBOX_HEAD )
		{
			for ( const auto& vertices : vertice_list_top )
			{
				// transform the point using the calculated basis
				const vec3_t point = ( ( right * vertices.x ) + ( up * vertices.z ) + ( forward * vertices.y ) ) * scale;

				world_points.emplace_back( center + point );
			}
		}
		else {
			if ( index >= HITBOX_PELVIS && index <= HITBOX_UPPER_CHEST ) {
				if ( index == HITBOX_UPPER_CHEST ) {
					world_points.emplace_back( center + up * scale * 0.8f );

					world_points.emplace_back( center - ( right - up ) * scale * 0.5f );
					world_points.emplace_back( center + ( right + up ) * scale * 0.5f );
				}
				else {
					world_points.emplace_back( center - right * scale * 0.5f );
					world_points.emplace_back( center + right * scale * 0.5f );
				}

				world_points.emplace_back( center - right * scale );
				world_points.emplace_back( center + right * scale );
			}
			else {
				world_points.emplace_back( center - right * scale * 0.75f );
				world_points.emplace_back( center + right * scale * 0.75f );
			}
		}
	}
	else {
		static matrix3x4_t rot_matrix;
		g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

		// apply the rotation to the entity input space (local).
		static matrix3x4_t matrix;
		math::ConcatTransforms( record->m_bones[ bbox->m_bone ], rot_matrix, matrix );

		// extract origin from matrix.
		const vec3_t& origin = matrix.GetOrigin( );
		const vec3_t center = ( bbox->m_maxs + bbox->m_mins ) / 2.f;
		const vec3_t dist = bbox->m_maxs - center;

		// the feet hiboxes have a side, heel and the toe.
		if ( index == HITBOX_R_FOOT || index == HITBOX_L_FOOT )
		{
			float d1 = dist.z * 0.5f;
			float d2 = dist.x * 0.5f;

			// invert.
			if ( index == HITBOX_L_FOOT )
				d1 *= -1.f;

			world_points.emplace_back( vec3_t( center.x + d2, center.y, center.z ) );
			world_points.emplace_back( vec3_t( center.x - d2, center.y, center.z ) );
		}

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for ( auto& p : world_points )
		{
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p = { p.dot( matrix[ 0 ] ), p.dot( matrix[ 1 ] ), p.dot( matrix[ 2 ] ) };

			// transform point to world space.
			p += origin;
		}
	}

	for ( const auto& point : world_points )
		points.emplace_back( point, record, index );
}

bool Aimbot::CheckHitchance( const pt_data_t& target, const ang_t& angle )
{
	if ( !target.m_record )
		return false;

	if ( !m_settings.hitchance_amount.get( ) )
		return true;

	constexpr int MAX_SEEDS = 238;
	const int ACCURACY_SEEDS = static_cast< int >( MAX_SEEDS * ( m_settings.accuracy_boost.get( ) / 100.f ) );

	static vec3_t     fwd, right, up;
	size_t	   total_hits{ };

	// get needed directional vectors.
	math::AngleVectors( angle, &fwd, &right, &up );

	const float& inaccuracy = g_cl.m_weapon->GetInaccuracy( );
	const float& spread = g_cl.m_weapon->GetSpread( );

	const int& health = target.m_record->m_player->m_iHealth( );

	static penetration::PenetrationInput_t input;
	static penetration::PenetrationOutput_t output;

	// init our seeds.
	if ( m_seed_table.empty( ) )
	{
		for ( int i = 0; i < MAX_SEEDS; i++ )
		{
			g_csgo.RandomSeed( i );

			hc_seed_t& seed = m_seed_table.emplace_front( );

			seed.radius_curve_tens0 = g_csgo.RandomFloat( 0.f, 1.0f );
			seed.theta0 = g_csgo.RandomFloat( 0.0f, math::pi_2 );
			seed.radius_curve_tens1 = g_csgo.RandomFloat( 0.f, 1.0f );
			seed.theta1 = g_csgo.RandomFloat( 0.0f, math::pi_2 );
		}
	}

	// iterate all possible seeds.
	for ( int i = 0; i < MAX_SEEDS; i++ )
	{
		const hc_seed_t& seed = m_seed_table[ i ];

		const float& radius_curve_density = seed.radius_curve_tens0;
		const float& theta0 = seed.theta0;

		const float radius0 = radius_curve_density * inaccuracy;
		const float x0 = radius0 * cosf( theta0 );
		const float y0 = radius0 * sinf( theta0 );

		const float& spread_curve_density = seed.radius_curve_tens1;
		const float& theta1 = seed.theta1;

		const float radius1 = spread_curve_density * spread;
		const float x1 = radius1 * cosf( theta1 );
		const float y1 = radius1 * sinf( theta1 );

		const float spread_x = x0 + x1;
		const float spread_y = y0 + y1;

		// get spread direction
		vec3_t dir = ( fwd + ( right * spread_x ) + ( up * spread_y ) ).normalized( );

		// get trace end point
		const vec3_t& end = g_cl.m_shoot_pos + ( dir * g_cl.m_weapon_info->m_range );

		// account for damage accuracy(accuracy boost) for X seeds.
		if ( i <= ACCURACY_SEEDS &&
			!math::RayIntersectHitbox( target.m_record->m_player,
				g_cl.m_shoot_pos,
				end,
				target.m_record->m_bones,
				target.m_hitbox,
				false ) )
			continue;

		input.m_from = g_cl.m_local;
		input.m_target = target.m_record->m_player;
		input.m_start = g_cl.m_shoot_pos;
		input.m_pos = end;
		input.m_damage = input.m_damage_pen = 1;
		input.m_can_pen = g_cl.m_weapon_id != ZEUS;

		// scan point damage.
		if ( !penetration::run( &input, &output ) ) // damage accuracy check failed.
			continue;

		++total_hits;

		// reached enough hitchance, stop iterating for optimization.
		if ( ( static_cast< float >( total_hits ) / static_cast< float >( MAX_SEEDS ) ) >= m_settings.hitchance_amount.get( ) * 0.01f )
			return true;
	}

	return false;
}