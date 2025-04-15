#include "includes.h"

float smoothstep_bounds( float edge0, float edge1, float x )
{
	x = std::clamp( ( x - edge0 ) / ( edge1 - edge0 ), 0.f, 1.f );
	return x * x * ( 3 - 2 * x );
}

void animstate_pose_param_cache_t::SetValue( Player* player, float flValue )
{
}

int AnimLayers::LookupSequence( Player* e, const char* label )
{
	if ( !e || !label )
		return -1;

	typedef int( __thiscall* fnLookupSequence )( void*, const char* );
	static auto lookup_sequnece_adr = pattern::find( g_csgo.m_client_dll, XOR( "E8 ? ? ? ? 5E 83 F8 FF" ) ).rel32( 1 ).as<fnLookupSequence>( );

	return lookup_sequnece_adr( e, label );
}

int AnimLayers::GetLayerActivity( Player* pPlayer, C_AnimationLayer* pLayer )
{
	if ( !pPlayer || !pLayer )
		return ACT_INVALID;

	return pPlayer->GetSequenceActivity( pLayer->m_sequence );
}

void AnimLayers::IncrementLayerCycle( float increment, C_AnimationLayer* pLayer, bool bAllowLoop )
{
	if ( !pLayer )
		return;

	if ( abs( pLayer->m_playback_rate ) <= 0 )
		return;

	float flCurrentCycle = pLayer->m_cycle;
	flCurrentCycle += increment * pLayer->m_playback_rate;

	if ( !bAllowLoop && flCurrentCycle >= 1 )
	{
		flCurrentCycle = 0.999f;
	}

	pLayer->m_cycle = math::ClampCycle( flCurrentCycle );
}

void AnimLayers::IncrementLayerWeight( float increment, C_AnimationLayer* pLayer )
{
	if ( !pLayer )
		return;

	if ( abs( pLayer->m_weight_delta_rate ) <= 0.f )
		return;

	float flCurrentWeight = pLayer->m_weight;
	flCurrentWeight += increment * pLayer->m_weight_delta_rate;
	flCurrentWeight = std::clamp( flCurrentWeight, 0.f, 1.f );

	pLayer->m_weight = flCurrentWeight;
}


void AnimLayers::UpdateAnimLayer( C_AnimationLayer* pLayer, int nSequence, float flPlaybackRate, float flWeight, float flCycle )
{
	if ( nSequence > 1 )
	{
		pLayer->m_sequence = nSequence;
		pLayer->m_playback_rate = flPlaybackRate;
		pLayer->m_cycle = std::clamp( flCycle, 0.f, 1.f );

		pLayer->m_weight = std::clamp( flWeight, 0.f, 1.f );
	}
}

mstudioseqdesc_t* SeqDesc( CStudioHdr* mdl, int seq )
{
	if ( !mdl || !seq )
		return nullptr;

	// fix this shit
	static auto target = pattern::find( g_csgo.m_client_dll, "55 8B EC 56 8B 75 08 57 8B F9 85 F6 78 18" ).as<mstudioseqdesc_t * ( __thiscall* )( CStudioHdr*, int )>( );
	return target( mdl, seq );
}

float AnimLayers::GetLayerIdealWeightFromSeqCycle( Player* pPlayer, C_AnimationLayer* pLayer )
{
	if ( !pPlayer || !pLayer )
		return 0.f;

	auto model = pPlayer->GetModelPtr( );
	if ( !model )
		return 0.f;

	auto seqdesc = SeqDesc( model, pLayer->m_sequence );
	if ( !seqdesc )
		return 0.f;

	float flCycle = pLayer->m_cycle;
	if ( flCycle >= 0.999f )
		flCycle = 1;

	float flEaseIn = seqdesc->fadeintime; // seqdesc.fadeintime;
	float flEaseOut = seqdesc->fadeouttime; // seqdesc.fadeouttime;
	float flIdealWeight = 1;

	if ( flEaseIn > 0 && flCycle < flEaseIn )
	{
		flIdealWeight = smoothstep_bounds( 0, flEaseIn, flCycle );
	}
	else if ( flEaseOut < 1 && flCycle > flEaseOut )
	{
		flIdealWeight = smoothstep_bounds( 1.0f, flEaseOut, flCycle );
	}

	if ( flIdealWeight < 0.0015f )
		return 0.f;

	return std::clamp( flIdealWeight, 0.f, 1.f );
}

bool AnimLayers::IsLayerSequenceCompleted( float increment, C_AnimationLayer* pLayer )
{
	if ( !pLayer )
		return false;

	return ( pLayer->m_cycle + ( increment * pLayer->m_playback_rate ) ) >= 1;
}

void AnimLayers::IncrementLayerCycleWeightRateGeneric( float increment, C_AnimationLayer* pLayer )
{
	if ( !pLayer )
		return;

	float flWeightPrevious = pLayer->m_weight;
	IncrementLayerCycle( increment, pLayer, false );
	pLayer->m_weight = GetLayerIdealWeightFromSeqCycle( ( Player* )pLayer->m_owner, pLayer );
	pLayer->m_weight_delta_rate = ( pLayer->m_weight - flWeightPrevious ) / increment;
}

void AnimLayers::SetLayerSequence( CCSGOPlayerAnimState* state, C_AnimationLayer* layer, int act, int overrideseq )
{
	if ( !state || !layer )
		return;

	int32_t sequence = state->select_sequence_from_acitivty_modifier( act );

	if ( overrideseq != -1 )
		sequence = overrideseq;

	if ( sequence < 0 )
		return;

	layer->m_cycle = 0.0f;
	layer->m_weight = 0.0f;
	layer->m_sequence = sequence;
	layer->m_playback_rate = state->m_player->get_layer_seq_cycle_rate( layer, sequence );
}

int IndexFromAnimTagName( const char* szName )
{
	for ( int i = 1; i < ANIMTAG_COUNT; i++ )
	{
		const animtaglookup_t* pAnimTag = &g_AnimTagLookupTable[ i ];
		if ( !std::strcmp( szName, pAnimTag->szName ) )
		{
			return pAnimTag->nIndex;
		}
	}
	return ANIMTAG_INVALID;
}

float AnimLayers::GetFirstSequenceAnimTag( Player* player, int sequence, int nDesiredTag, float flStart, float flEnd )
{
	CStudioHdr* pstudiohdr = player->GetModelPtr( );

	if ( !pstudiohdr/* || sequence >= pstudiohdr->GetNumSeq()*/ )
		return flStart;

	mstudioseqdesc_t* seqdesc = SeqDesc( pstudiohdr, sequence );
	if ( seqdesc->numanimtags == 0 )
		return flStart;

	mstudioanimtag_t* panimtag = NULL;

	for ( int index = 0; index < ( int )seqdesc->numanimtags; index++ )
	{
		panimtag = seqdesc->pAnimTag( index );

		if ( panimtag->tag == ANIMTAG_INVALID )
			continue;

		if ( panimtag->tag == ANIMTAG_UNINITIALIZED )
		{
			panimtag->tag = IndexFromAnimTagName( panimtag->pszTagName( ) );
		}

		if ( panimtag->tag == nDesiredTag && panimtag->cycle >= flStart && panimtag->cycle < flEnd )
		{
			return panimtag->cycle;
		}
	}

	return flStart;
}

void AnimLayers::UpdateLayers( Player* player, const ang_t& angle, rebuilt_animstate_t* data, const float& curtime, bool apply_state )
{
	if ( !player )
		return;

	auto state = player->m_PlayerAnimState( );
	if ( !state )
		return;

	static auto GetWeaponPrefix = pattern::find( g_csgo.m_client_dll, XOR( "E8 ? ? ? ? 50 8D 44 24 54" ) ).resolve_rip( ).as<const char* ( __thiscall* )( CCSGOPlayerAnimState* )>( );

	// setup layers that we want to use/fix
	C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];
	C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ];
	//C_AnimationLayer* ANIMATION_LAYER_ADJUST = &data->m_layers[animstate_layer_t::ANIMATION_LAYER_ADJUST];
	C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_MOVE = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_MOVE ];
	C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_STRAFECHANGE = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ];
	C_AnimationLayer* ANIMATION_LAYER_LEAN = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_LEAN ];
	C_AnimationLayer* ANIMATION_LAYER_ALIVELOOP = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_ALIVELOOP ];
	C_AnimationLayer* ANIMATION_LAYER_AIMMATRIX = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_AIMMATRIX ];

	ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_owner =
		ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_owner =
		//ANIMATION_LAYER_ADJUST->m_owner =
		ANIMATION_LAYER_MOVEMENT_MOVE->m_owner =
		ANIMATION_LAYER_MOVEMENT_STRAFECHANGE->m_owner =
		ANIMATION_LAYER_LEAN->m_owner =
		ANIMATION_LAYER_AIMMATRIX->m_owner =
		ANIMATION_LAYER_ALIVELOOP->m_owner = player;

	// setup ground and flag stuff
	if (data->m_spawn_time != player->m_flSpawnTime())
	{
		ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight =
			ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight =
			//ANIMATION_LAYER_ADJUST->m_weight =
			ANIMATION_LAYER_MOVEMENT_MOVE->m_weight =
			ANIMATION_LAYER_MOVEMENT_STRAFECHANGE->m_weight =
			ANIMATION_LAYER_AIMMATRIX->m_weight =
			ANIMATION_LAYER_ALIVELOOP->m_weight = 0.f;

		data->reset();

		data->m_spawn_time = player->m_flSpawnTime();
	}
	else if (player == g_cl.m_local)
		SetLayers(player, data);

	// copy the angle.
	data->m_angle = angle;

	player->GetPoseParameters( data->m_poses );

	data->m_increment = std::max( 0.0f, curtime - data->m_last_update );

	data->m_old_weapon = data->m_weapon;
	data->m_weapon = player->GetActiveWeapon( );

	data->m_old_pos = data->m_pos;
	data->m_pos = player->m_vecOrigin( );

	data->m_abs_vel = player->m_vecAbsVelocity( );
	data->m_vel_z = data->m_abs_vel.z;
	data->m_abs_vel.z = 0.f;

	data->m_vel = math::Approach( data->m_abs_vel, player->m_vecVelocity( ), data->m_increment * 2000 );

	data->m_vel_xy = std::min( data->m_vel.length_2d( ), 260.f );
	data->m_vel_normalized = data->m_vel.normalized( );

	if ( data->m_vel_xy > 0 )
		data->m_vel_normalized_non_zero = data->m_vel_normalized;

	data->m_old_moving = data->m_moving;
	data->m_moving = data->m_vel.length_2d( ) > 0.1f;

	data->m_old_ground = data->m_ground;
	data->m_ground = player->m_fFlags( ) & FL_ONGROUND;

	data->m_old_ladder = data->m_ladder;
	data->m_ladder = !data->m_ground && player->m_MoveType( ) == MOVETYPE_LADDER;

	float flMaxSpeedRun = data->m_weapon ? std::max( data->m_weapon->GetMaxSpeed( ), 0.001f ) : 260.f;

	data->m_speed_portion_run = std::clamp( data->m_vel_xy / flMaxSpeedRun, 0.f, 1.f );
	data->m_speed_portion_walk = data->m_vel_xy / ( flMaxSpeedRun * 0.52f );
	data->m_speed_portion_crouch = data->m_vel_xy / ( flMaxSpeedRun * 0.34f );

	data->m_duck_amount = std::clamp( math::Approach( std::clamp( player->m_flDuckAmount( ) + data->m_duck_additive, 0.f, 1.f ), data->m_duck_amount, data->m_increment * 6.0f ), 0.f, 1.f );

	data->m_foot_yaw = std::clamp( data->m_foot_yaw, -360.f, 360.f );
	float flEyeFootDelta = math::AngleDiff( angle.y, data->m_foot_yaw );

	// narrow the available aim matrix width as speed increases
	data->m_max_yaw_multiplier = math::Lerp( std::clamp( data->m_speed_portion_walk, 0.f, 1.f ), 1.0f, math::Lerp( data->m_walk_to_run, 0.8f, 0.5f ) );

	if ( data->m_duck_amount > 0 )
	{
		data->m_max_yaw_multiplier = math::Lerp( data->m_duck_amount * std::clamp( data->m_speed_portion_crouch, 0.f, 1.f ), data->m_max_yaw_multiplier, 0.5f );
	}

	float flTempYawMax = 58.f * data->m_max_yaw_multiplier;
	float flTempYawMin = -58.f * data->m_max_yaw_multiplier;

	if ( flEyeFootDelta > flTempYawMax )
	{
		data->m_foot_yaw = angle.y - abs( flTempYawMax );
	}
	else if ( flEyeFootDelta < flTempYawMin )
	{
		data->m_foot_yaw = angle.y + abs( flTempYawMin );
	}

	data->m_foot_yaw = math::NormalizedAngle( data->m_foot_yaw );

	if ( data->m_ground )
	{
		if ( data->m_vel_xy > 0.1f )
		{
			data->m_foot_yaw = math::ApproachAngle( angle.y, data->m_foot_yaw, data->m_increment * ( 30.0f + 20.0f * data->m_walk_to_run ) );

			data->m_body_update = curtime + ( 1.1f * 0.2f );
			data->m_body_yaw = angle.y;
		}
		else
		{
			data->m_foot_yaw = math::ApproachAngle( data->m_body_yaw, data->m_foot_yaw, data->m_increment * 100.f );

			if ( curtime > data->m_body_update && std::abs( math::AngleDiff( data->m_foot_yaw, angle.y ) ) > 35.0f )
			{
				data->m_body_update = curtime + 1.1f;
				data->m_body_yaw = angle.y;
			}
		}
	}

	if ( data->m_walk_to_run > 0 && data->m_walk_to_run < 1 )
	{
		//currently transitioning between walk and run
		if ( data->m_walk_to_run_state == 0 )
		{
			data->m_walk_to_run += data->m_increment * 2.0f;
		}
		else // m_bWalkToRunTransitionState == 1
		{
			data->m_walk_to_run -= data->m_increment * 2.0f;
		}
		data->m_walk_to_run = std::clamp( data->m_walk_to_run, 0.f, 1.f );
	}

	if ( data->m_vel_xy > 135.2f && data->m_walk_to_run_state == 1 )
	{
		//crossed the walk to run threshold
		data->m_walk_to_run_state = 0;
		data->m_walk_to_run = std::max( 0.01f, data->m_walk_to_run );
	}
	else if ( data->m_vel_xy < 135.2f && data->m_walk_to_run_state == 0 )
	{
		//crossed the run to walk threshold
		data->m_walk_to_run_state = 1;
		data->m_walk_to_run = std::min( 0.99f, data->m_walk_to_run );
	}

	if ( player->m_iMoveState( ) != data->m_move_state )
	{
		data->m_stutter_step += 10;
	}

	data->m_move_state = player->m_iMoveState( );
	data->m_stutter_step = std::clamp( math::Approach( 0, data->m_stutter_step, data->m_increment * 40 ), 0.f, 100.f );

	/* adjust */

	bool started_moving = data->m_moving && !data->m_old_moving;
	bool stopped_moving = !data->m_moving && data->m_old_moving;

	if ( data->m_vel_xy > 0 && data->m_ground )
	{
		// convert horizontal velocity vec to angular yaw
		float flRawYawIdeal = ( atan2( -data->m_vel.y, -data->m_vel.x ) * 180 / math::pi );
		if ( flRawYawIdeal < 0 )
			flRawYawIdeal += 360;

		data->m_move_yaw_ideal = math::NormalizedAngle( math::AngleDiff( flRawYawIdeal, data->m_foot_yaw ) );
	}

	// delta between current yaw and ideal velocity derived target (possibly negative!)
	data->m_move_yaw_to_ideal = math::NormalizedAngle( math::AngleDiff( data->m_move_yaw_ideal, data->m_move_yaw ) );

	if ( started_moving && data->m_move_weight <= 0 )
	{
		data->m_move_yaw = data->m_move_yaw_ideal;

		// select a special starting cycle that's set by the animator in content
		int nMoveSeq = ANIMATION_LAYER_MOVEMENT_MOVE->m_sequence;
		if ( nMoveSeq != -1 )
		{
			mstudioseqdesc_t* seqdesc = SeqDesc( player->GetModelPtr( ), nMoveSeq );
			if ( seqdesc && seqdesc->numanimtags > 0 )
			{
				if ( std::abs( math::AngleDiff( data->m_move_yaw, 180 ) ) <= 22.5f ) //N
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_N, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, 135 ) ) <= 22.5f ) //NE
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_NE, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, 90 ) ) <= 22.5f ) //E
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_E, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, 45 ) ) <= 22.5f ) //SE
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_SE, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, 0 ) ) <= 22.5f ) //S
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_S, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, -45 ) ) <= 22.5f ) //SW
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_SW, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, -90 ) ) <= 22.5f ) //W
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_W, 0, 1 );
				}
				else if ( std::abs( math::AngleDiff( data->m_move_yaw, -135 ) ) <= 22.5f ) //NW
				{
					data->m_primary_cycle = GetFirstSequenceAnimTag( player, nMoveSeq, ANIMTAG_STARTCYCLE_NW, 0, 1 );
				}
			}
		}
	}
	else
	{
		if ( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE->m_weight >= 1 )
		{
			data->m_move_yaw = data->m_move_yaw_ideal;
		}
		else
		{

			float flMoveWeight = math::Lerp( data->m_duck_amount, std::clamp( data->m_speed_portion_walk, 0.f, 1.f ), std::clamp( data->m_speed_portion_crouch, 0.f, 1.f ) );
			float flRatio = math::Bias( flMoveWeight, 0.18f ) + 0.1f;

			data->m_move_yaw = math::NormalizedAngle( data->m_move_yaw + ( data->m_move_yaw_to_ideal * flRatio ) );
		}
	}

	data->m_poses[ animstate_pose_param_idx_t::POSE_MOVE_YAW ] = ( data->m_move_yaw / 360 ) + 0.5f;

	float flAimYaw = (math::AngleDiff( angle.y, data->m_foot_yaw ) / 58.f) * 60.f;

	data->m_poses[ animstate_pose_param_idx_t::POSE_BODY_YAW ] = std::clamp( ( flAimYaw / 120.f ) + 0.5f, 0.f, 1.f );

	/* aim matrix */
	char szTransitionStandAimMatrix[ 64 ];
	std::sprintf( szTransitionStandAimMatrix, "%s_aim", GetWeaponPrefix( state ) );
	int nSeqStand = LookupSequence( player, szTransitionStandAimMatrix );

	UpdateAnimLayer( ANIMATION_LAYER_AIMMATRIX, nSeqStand, 0, 1, 0 );

	/* move */	
	char szWeaponMoveSeq[ 64 ];
	std::sprintf( szWeaponMoveSeq, "move_%s", GetWeaponPrefix( state ) );

	int nWeaponMoveSeq = LookupSequence( player, szWeaponMoveSeq );
	if ( nWeaponMoveSeq == -1 )
	{
		nWeaponMoveSeq = LookupSequence( player, "move" );
	}

	// see: CSGOPlayerAnimState::SetUpMovement
	data->m_target_move_weight = math::Lerp( data->m_duck_amount, std::clamp( data->m_speed_portion_walk, 0.f, 1.f ), std::clamp( data->m_speed_portion_crouch, 0.f, 1.f ) );

	if ( data->m_move_weight <= data->m_target_move_weight )
	{
		data->m_move_weight = data->m_target_move_weight;
	}
	else
	{
		data->m_move_weight = math::Approach( data->m_target_move_weight, data->m_move_weight, data->m_increment * math::RemapValClamped( data->m_stutter_step, 0.0f, 100.0f, 2, 20 ) );
	}

	vec3_t vecMoveYawDir;
	math::AngleVectors( ang_t( 0, math::NormalizedAngle( data->m_foot_yaw + data->m_move_yaw + 180 ), 0 ), &vecMoveYawDir );
	float flYawDeltaAbsDot = abs( data->m_vel_normalized_non_zero.dot( vecMoveYawDir ) );
	data->m_move_weight *= math::Bias( flYawDeltaAbsDot, 0.2 );

	float flMoveWeightWithAirSmooth = data->m_move_weight * data->m_air_smooth;

	// dampen move weight for landings
	flMoveWeightWithAirSmooth *= std::max( ( 1.0f - ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight ), 0.55f );

	float flMoveCycleRate = 0;
	if ( data->m_vel_xy > 0 )
	{
		flMoveCycleRate = player->get_layer_seq_cycle_rate( player->GetModelPtr( ), nWeaponMoveSeq );
		float flSequenceGroundSpeed = std::max( player->get_seq_move_dist( player->GetModelPtr( ), nWeaponMoveSeq, data->m_poses ) / ( 1.0f / flMoveCycleRate ), 0.001f );
		flMoveCycleRate *= data->m_vel_xy / flSequenceGroundSpeed;

		flMoveCycleRate *= math::Lerp( data->m_walk_to_run, 1.0f, 0.85f );
	}

	float flLocalCycleIncrement = ( flMoveCycleRate * data->m_increment );
	data->m_primary_cycle = math::ClampCycle( data->m_primary_cycle + flLocalCycleIncrement );

	flMoveWeightWithAirSmooth = std::clamp( flMoveWeightWithAirSmooth, 0.f, 1.f );
	UpdateAnimLayer( ANIMATION_LAYER_MOVEMENT_MOVE, nWeaponMoveSeq, flLocalCycleIncrement, flMoveWeightWithAirSmooth, data->m_primary_cycle );

	/* strafe change */

	const float fix = ( g_cl.m_local == player && !g_cl.m_fixed_movement ) ? math::NormalizedAngle( math::NormalizedAngle( angle.y ) - math::NormalizedAngle( g_cl.m_view_angles.y ) ) : 0.f;

	vec3_t vecForward;
	vec3_t vecRight;
	math::AngleVectors( ang_t( 0, data->m_foot_yaw - fix, 0 ), &vecForward, &vecRight );
	vecRight = vecRight.normalized( );

	float flVelToRightDot = data->m_vel_normalized_non_zero.dot( vecRight );
	float flVelToForwardDot = data->m_vel_normalized_non_zero.dot( vecForward );

	// We're interested in if the player's desired direction (indicated by their held buttons) is opposite their current velocity.
	// This indicates a strafing direction change in progress.

	if ( player == g_cl.m_local && g_cl.m_fixed_movement )
	{
		bool moveRight = ( data->m_buttons & ( IN_MOVERIGHT ) ) != 0;
		bool moveLeft = ( data->m_buttons & ( IN_MOVELEFT ) ) != 0;
		bool moveForward = ( data->m_buttons & ( IN_FORWARD ) ) != 0;
		bool moveBackward = ( data->m_buttons & ( IN_BACK ) ) != 0;

		bool bStrafeRight = ( data->m_speed_portion_walk >= 0.73f && moveRight && !moveLeft && flVelToRightDot < -0.63f );
		bool bStrafeLeft = ( data->m_speed_portion_walk >= 0.73f && moveLeft && !moveRight && flVelToRightDot > 0.63f );
		bool bStrafeForward = ( data->m_speed_portion_walk >= 0.65f && moveForward && !moveBackward && flVelToForwardDot < -0.55f );
		bool bStrafeBackward = ( data->m_speed_portion_walk >= 0.65f && moveBackward && !moveForward && flVelToForwardDot > 0.55f );

		player->m_bStrafing( ) = ( bStrafeRight || bStrafeLeft || bStrafeForward || bStrafeBackward );
	}

	if ( player->m_bStrafing( ) )
	{
		if ( !data->m_strafe_changed )
		{
			data->m_strafe_duration = 0;
		}

		data->m_strafe_changed = true;

		data->m_strafe_weight = math::Approach( 1, data->m_strafe_weight, data->m_increment * 20 );
		data->m_strafe_cycle = math::Approach( 0, data->m_strafe_cycle, data->m_increment * 10 );

		data->m_poses[ animstate_pose_param_idx_t::POSE_STRAFE_YAW ] = ( math::NormalizedAngle( data->m_move_yaw ) / 360.f ) + 0.5f;
	}
	else if ( data->m_strafe_weight > 0 )
	{
		data->m_strafe_duration += data->m_increment;

		if ( data->m_strafe_duration > 0.08f )
			data->m_strafe_weight = math::Approach( 0, data->m_strafe_weight, data->m_increment * 5 );

		data->m_strafe_sequence = LookupSequence( player, "strafe" );
		float flRate = player->get_layer_seq_cycle_rate( player->GetModelPtr( ), data->m_strafe_sequence );
		data->m_strafe_cycle = std::clamp( data->m_strafe_cycle + data->m_increment * flRate, 0.f, 1.f );
	}

	if ( data->m_strafe_weight <= 0 )
	{
		data->m_strafe_changed = false;
	}

	bool jumped = !data->m_ground && data->m_old_ground;
	bool landed = data->m_ground && !data->m_old_ground;

	float flDistanceFell = 0;
	if ( jumped )
	{
		data->m_ground_height = data->m_pos.z;
	}

	if ( landed )
	{
		flDistanceFell = abs( data->m_ground_height - data->m_pos.z );
		float flDistanceFallNormalizedBiasRange = math::Bias( math::RemapValClamped( flDistanceFell, 12.0f, 72.0f, 0.0f, 1.0f ), 0.4f );

		//Msg( "Fell %f units, ratio is %f. ", flDistanceFell, flDistanceFallNormalizedBiasRange );
		//Msg( "Fell for %f secs, multiplier is %f\n", m_flDurationInAir, m_flLandAnimMultiplier );

		data->m_landing_multiplier = std::clamp( math::Bias( data->m_time_in_air, 0.3f ), 0.1f, 1.0f );
		data->m_duck_additive = std::max( data->m_landing_multiplier, flDistanceFallNormalizedBiasRange );

		//Msg( "m_flDuckAdditional is %f\n", m_flDuckAdditional );
	}
	else
	{
		data->m_duck_additive = math::Approach( 0, data->m_duck_additive, data->m_increment * 2 );
	}

	data->m_air_smooth = math::Approach( data->m_ground ? 1 : 0, data->m_air_smooth, math::Lerp( data->m_duck_amount, 8.f, 16.f ) * data->m_increment );
	data->m_air_smooth = std::clamp( data->m_air_smooth, 0.f, 1.f );

	data->m_strafe_weight *= ( 1.0f - data->m_duck_amount );
	data->m_strafe_weight *= data->m_air_smooth;
	data->m_strafe_weight = std::clamp( data->m_strafe_weight, 0.f, 1.f );

	if ( data->m_strafe_sequence != -1 )
		UpdateAnimLayer( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE, data->m_strafe_sequence, 0, data->m_strafe_weight, data->m_strafe_cycle );

	/* jump_fall and land_climb  */
	bool started_laddering = !data->m_old_ladder && data->m_ladder;
	bool stopped_laddering = data->m_old_ladder && !data->m_ladder;

	if ( data->m_ladder_weight > 0 || data->m_ladder )
	{
		if ( started_laddering )
		{
			SetLayerSequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, ACT_CSGO_CLIMB_LADDER, -1 );
		}

		if ( std::abs( data->m_vel_z ) > 100 )
		{
			data->m_ladder_speed = math::Approach( 1, data->m_ladder_speed, data->m_increment * 10.0f );
		}
		else
		{
			data->m_ladder_speed = math::Approach( 0, data->m_ladder_speed, data->m_increment * 10.0f );
		}
		data->m_ladder_speed = std::clamp( data->m_ladder_speed, 0.f, 1.f );

		if ( data->m_ladder )
		{
			data->m_ladder_weight = math::Approach( 1, data->m_ladder_weight, data->m_increment * 5.0f );
		}
		else
		{
			data->m_ladder_weight = math::Approach( 0, data->m_ladder_weight, data->m_increment * 10.0f );
		}
		data->m_ladder_weight = std::clamp( data->m_ladder_weight, 0.f, 1.f );

		float flLadderClimbCycle = ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_cycle;
		flLadderClimbCycle += ( data->m_pos.z - data->m_old_pos.z ) * math::Lerp( data->m_ladder_speed, 0.010f, 0.004f );

		if ( GetLayerActivity( player, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) == ACT_CSGO_CLIMB_LADDER )
		{
			ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight = data->m_ladder_weight;
		}

		ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_cycle = flLadderClimbCycle;

		// fade out jump if we're climbing
		if ( data->m_ladder )
		{
			float flIdealJumpWeight = 1.0f - data->m_ladder_weight;
			if ( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight > flIdealJumpWeight )
			{
				ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight = flIdealJumpWeight;
			}
		}
	}
	else
	{
		data->m_ladder_speed = 0;
	}

	if ( data->m_ground )
	{
		if ( !data->m_landing && ( landed || stopped_laddering ) )
		{
			SetLayerSequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, data->m_time_in_air > 1 ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT, -1 );
			ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight = 0.f;
			data->m_landing = true;
		}
		data->m_time_in_air = 0;

		if ( data->m_landing && GetLayerActivity( player, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) != ACT_CSGO_CLIMB_LADDER )
		{
			data->m_jumping = false;

			IncrementLayerCycle( data->m_increment, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, false );
			IncrementLayerCycle( data->m_increment, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );

			data->m_poses[ animstate_pose_param_idx_t::POSE_JUMP_FALL ] = 0.f;

			if ( IsLayerSequenceCompleted( data->m_increment, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) )
			{
				data->m_landing = false;
				ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight = 0.f;
				//SetLayerRate( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 1.0f );
				ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight = 0.f;
				data->m_landing_multiplier = 1.0f;
			}
			else
			{
				float flLandWeight = GetLayerIdealWeightFromSeqCycle( player, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) * data->m_landing_multiplier;

				// if we hit the ground crouched, reduce the land animation as a function of crouch, since the land animations move the head up a bit ( and this is undesirable )
				flLandWeight *= std::clamp( ( 1.0f - data->m_duck_amount ), 0.2f, 1.0f );

				ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight = flLandWeight;

				// fade out jump because land is taking over
				float flCurrentJumpFallWeight = ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight;
				if ( flCurrentJumpFallWeight > 0 )
				{
					flCurrentJumpFallWeight = math::Approach( 0, flCurrentJumpFallWeight, data->m_increment * 10.0f );
					ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight = flCurrentJumpFallWeight;
				}
			}
		}

		if ( !data->m_landing && !data->m_jumping && data->m_ladder_weight <= 0 )
		{
			ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight = 0.f;
		}
	}
	else if ( !data->m_ladder )
	{
		data->m_landing = false;

		// we're in the air
		if ( jumped || stopped_laddering )
		{
			// If entered the air by jumping, then we already set the jump activity.
			// But if we're in the air because we strolled off a ledge or the floor collapsed or something,
			// we need to set the fall activity here.
			if ( !data->m_jumping )
			{
				SetLayerSequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, ACT_CSGO_FALL, -1 );
			}
			data->m_time_in_air = 0;
		}

		data->m_time_in_air += data->m_increment;

		IncrementLayerCycle( data->m_increment, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );

		// increase jump weight
		float flJumpWeight = ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight;
		float flNextJumpWeight = GetLayerIdealWeightFromSeqCycle( player, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL );
		if ( flNextJumpWeight > flJumpWeight )
		{
			ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL->m_weight = flNextJumpWeight;
		}

		// bash any lingering land weight to zero
		float flLingeringLandWeight = ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight;
		if ( flLingeringLandWeight > 0 )
		{
			flLingeringLandWeight *= smoothstep_bounds( 0.2f, 0.0f, data->m_time_in_air );
			ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB->m_weight = flLingeringLandWeight;
		}

		data->m_poses[ animstate_pose_param_idx_t::POSE_JUMP_FALL ] = std::clamp( smoothstep_bounds( 0.72f, 1.52f, data->m_time_in_air ), 0.f, 1.f );
	}

	/* aliveloop */
	if ( GetLayerActivity( player, ANIMATION_LAYER_ALIVELOOP ) != ACT_CSGO_ALIVE_LOOP )
	{
		// first time init
		SetLayerSequence( state, ANIMATION_LAYER_ALIVELOOP, ACT_CSGO_ALIVE_LOOP, -1 );
		ANIMATION_LAYER_ALIVELOOP->m_cycle = g_csgo.RandomFloat( 0, 1 );
		float flNewRate = player->get_layer_seq_cycle_rate( player->GetModelPtr( ), ANIMATION_LAYER_ALIVELOOP->m_sequence );
		flNewRate *= g_csgo.RandomFloat( 0.8f, 1.1f );
		ANIMATION_LAYER_ALIVELOOP->m_playback_rate = flNewRate;
	}
	else
	{
		if ( data->m_weapon && data->m_weapon != data->m_old_weapon )
		{
			//re-roll act on weapon change
			float flRetainCycle = ANIMATION_LAYER_ALIVELOOP->m_cycle;
			SetLayerSequence( state, ANIMATION_LAYER_ALIVELOOP, ACT_CSGO_ALIVE_LOOP, -1 );
			ANIMATION_LAYER_ALIVELOOP->m_cycle = flRetainCycle;
		}
		else if ( IsLayerSequenceCompleted( data->m_increment, ANIMATION_LAYER_ALIVELOOP ) )
		{
			float flNewRate = player->get_layer_seq_cycle_rate( player->GetModelPtr( ), ANIMATION_LAYER_ALIVELOOP->m_sequence );
			flNewRate *= g_csgo.RandomFloat( 0.8f, 1.1f );
			ANIMATION_LAYER_ALIVELOOP->m_playback_rate = flNewRate;
		}
		else
		{
			float flWeightOutPoseBreaker = math::RemapValClamped( data->m_speed_portion_run, 0.55f, 0.9f, 1.0f, 0.0f );
			ANIMATION_LAYER_ALIVELOOP->m_weight = flWeightOutPoseBreaker;
		}
	}

	IncrementLayerCycle( data->m_increment, ANIMATION_LAYER_ALIVELOOP, true );

	/* setup lean */

	// lean the body into velocity derivative (acceleration) to simulate maintaining a center of gravity
	float flInterval = curtime - data->m_vel_test_time;
	if ( flInterval > 0.025f )
	{
		flInterval = std::min( flInterval, 0.1f );
		data->m_vel_test_time = curtime;

		data->m_target_acceleration = ( player->m_vecVelocity( ) - data->m_vel_last ) / flInterval;
		data->m_target_acceleration.z = 0;

		data->m_vel_last = player->m_vecVelocity( );
	}

	data->m_acceleration = math::Approach( data->m_target_acceleration, data->m_acceleration, data->m_increment * 800.0f );

	//ang_t temp;
	//math::VectorAngles(data->m_acceleration, temp);

	data->m_acceleration_weight = std::clamp( ( data->m_acceleration.length( ) / 260.f ) * data->m_speed_portion_run, 0.f, 1.f );
	data->m_acceleration_weight *= ( 1.0f - data->m_ladder_weight );

	//data->m_poses[animstate_pose_param_idx_t::POSE_LEAN_YAW] = math::NormalizedAngle(data->m_foot_yaw - temp.y);

	if ( ANIMATION_LAYER_LEAN->m_sequence <= 0 )
	{
		SetLayerSequence( state, ANIMATION_LAYER_LEAN, -1, LookupSequence( player, "lean" ) );
	}

	ANIMATION_LAYER_LEAN->m_weight = data->m_acceleration_weight;

	data->m_poses[ animstate_pose_param_idx_t::POSE_SPEED ] = std::clamp( data->m_speed_portion_run, 0.0f, 1.0f );
	data->m_poses[ animstate_pose_param_idx_t::POSE_STAND ] = std::clamp( 1.0f - ( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE->m_weight * data->m_duck_amount ), 0.0f, 1.0f );

	data->m_poses[ animstate_pose_param_idx_t::POSE_MOVE_BLEND_WALK ] = ( 1.0f - data->m_walk_to_run ) * ( 1.0f - data->m_duck_amount );
	data->m_poses[ animstate_pose_param_idx_t::POSE_MOVE_BLEND_RUN ] = data->m_walk_to_run * ( 1.0f - data->m_duck_amount );
	data->m_poses[ animstate_pose_param_idx_t::POSE_MOVE_BLEND_CROUCH ] = data->m_duck_amount;

	data->m_last_update = curtime;
	/* apply changes to player animstate aswell */
	if ( apply_state )
		data->correct( state );
}

void AnimLayers::SetLayers( Player* player, rebuilt_animstate_t* data )
{
	for ( int i = 0; i < 13; i++ )
	{
		if ( !data->m_layers[ i ].m_owner )
			continue;

		player->m_AnimOverlay( )[ i ] = data->m_layers[ i ];
	}
}

void AnimLayers::SetPoses( Player* player, rebuilt_animstate_t* data )
{
	for ( int i = 0; i < 24; i++ )
	{
		if ( i != POSE_SPEED && i != POSE_STAND &&
			i != POSE_MOVE_BLEND_WALK && i != POSE_MOVE_BLEND_RUN && i != POSE_MOVE_BLEND_CROUCH &&
			i != POSE_JUMP_FALL &&
			i != POSE_STRAFE_YAW && i != POSE_BODY_YAW && i != POSE_MOVE_YAW )
			continue;

		player->m_flPoseParameter( )[ i ] = data->m_poses[ i ];
	}
}

void AnimLayers::DoAnimationEvent( Player* player, Entity* ground_entity, rebuilt_animstate_t* data )
{
	if ( !player )
		return;

	auto state = player->m_PlayerAnimState( );
	if ( !state )
		return;

	data->m_old_buttons = data->m_buttons;
	data->m_buttons = g_cl.m_cmd->m_buttons;

	if ( !( data->m_buttons & IN_JUMP ) )
		return;

	if ( ground_entity == NULL )
	{
		data->m_old_buttons |= IN_JUMP;
		return;
	}

	C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL = &data->m_layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ];

	data->m_jumping = true;
	SetLayerSequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, ACT_CSGO_JUMP, -1 );
}