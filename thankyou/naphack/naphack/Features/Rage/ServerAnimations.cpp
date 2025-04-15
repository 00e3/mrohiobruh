#include "ServerAnimations.hpp"
#include "../../SDK/Displacement.hpp"
#include "../Rage/BoneSetup.hpp"
#include "../Scripting/Scripting.hpp"
#include "../Scripting/Wrappers/entity.h"
#include "../Visuals/EventLogger.hpp"
#include "../Rage/EnginePrediction.hpp"
#include "TickbaseShift.hpp"

ServerAnimations g_ServerAnimations;

void ServerAnimations::SetLayerSequence( C_CSPlayer *pEntity, C_AnimationLayer *pLayer, int32_t activity, /*CUtlVector<uint16_t> modifiers,*/ int nOverrideSequence ) {

	//m_pMDLCache->BeginLock( );
	//int nSequence = SelectWeightedSequenceFromModifiers( pEntity, activity, modifiers );
	//m_pMDLCache->EndLock( );

	if( nOverrideSequence >= 2 ) {
		g_pMDLCache->BeginLock( );
		if( pLayer ) {
			pLayer->m_nSequence = nOverrideSequence;
			pLayer->m_flPlaybackRate = pEntity->GetLayerSequenceCycleRate( pLayer, nOverrideSequence );
			pLayer->m_flCycle = 0.0f;
			pLayer->m_flWeight = 0.0f;

			// todo: maybe some other day, i don't think it's needed
			// UpdateLayerOrderPreset( 0.0f, layer, sequence ); 
		}
		g_pMDLCache->EndLock( );
	}
}

bool ServerAnimations::HandleLayerSeparately( int nLayer ) {
	auto pLocal = C_CSPlayer::GetLocalPlayer( );
	if( !pLocal ) {
		return false;
	}

	bool bDontHandle = false;
	static float flSpawnTime = 0.f;
	if( ( g_pGameRules->m_bFreezePeriod( ) || pLocal->m_fFlags( ) & 0x40 || flSpawnTime != pLocal->m_flSpawnTime( ) ) ) {
		bDontHandle = true;
		flSpawnTime = pLocal->m_flSpawnTime( );
	}

	if( bDontHandle )
		return false;

	return ( nLayer == ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB || nLayer == ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ) || nLayer == ANIMATION_LAYER_MOVEMENT_MOVE;
}

void ServerAnimations::HandleAnimationEvents( C_CSPlayer *pLocal, CCSGOPlayerAnimState *pState, C_AnimationLayer *layers, /*CUtlVector< uint16_t > uModifiers,*/ CUserCmd *cmd ) {
	if( !pLocal || !pState || !cmd )
		return;

	if( pLocal->IsDead( ) )
		return;

	if( !pLocal->m_PlayerAnimState( ) || !g_pEngine->IsInGame( ) || !g_pEngine->IsConnected( ) || !g_pGameRules.Xor( ) )
		return;

	auto pWeapon = ( C_WeaponCSBaseGun * )pLocal->m_hActiveWeapon( ).Get( );
	if( !pWeapon )
		return;

	MDLCACHE_CRITICAL_SECTION( );

	// setup layers that we want to use/fix
	C_AnimationLayer *ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB = &layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];
	C_AnimationLayer *ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL = &layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ];

	if( !ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB || !ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL )
		return;

	int fFlags = g_Prediction.get_initial_vars( )->flags;

	Vector vecStart = pLocal->m_vecOrigin( );
	Vector vecEnd = vecStart;
	Vector vecVelocity = pLocal->m_vecVelocity( );

	vecVelocity.z -= ( g_Vars.sv_gravity->GetFloat( ) * g_pGlobalVars->interval_per_tick );
	vecEnd += ( vecVelocity );
	vecEnd.z -= 2.f;

	CTraceFilterWorldOnly filter;
	CGameTrace trace;
	g_pEngineTrace->TraceRay( Ray_t( vecStart, vecEnd ), MASK_SOLID, &filter, &trace );

	fFlags &= ~FL_ONGROUND;
	if( trace.fraction != 1.f && trace.plane.normal.z > 0.7f )
		fFlags |= FL_ONGROUND;

	// setup ground and flag stuff
	static bool bOnGround = false;
	bool bWasOnGround = bOnGround;
	bOnGround = ( fFlags & 1 );

	static float m_flLadderWeight = 0.f, m_flLadderSpeed = 0.f;

	// not sure about these two, they don't seem wrong though
	bool bLeftTheGroundThisFrame = bWasOnGround && !bOnGround;
	bool bLandedOnGroundThisFrame = !bWasOnGround && bOnGround;

	// ladder stuff
	static bool bOnLadder = false;
	bool bPreviouslyOnLadder = bOnLadder;
	bOnLadder = !bOnGround && pLocal->m_MoveType( ) == MOVETYPE_LADDER;
	bool bStartedLadderingThisFrame = ( !bPreviouslyOnLadder && bOnLadder );
	bool bStoppedLadderingThisFrame = ( bPreviouslyOnLadder && !bOnLadder );

	// TODO: fix the rest of the layers, for now I'm only fixing the land and jump layer
	// because until I get this working without any fuckery, there is no point to continue
	// trying to fix every other layer.
	// see: CSGOPlayerAnimState::SetUpMovement
	int nSequence = 0;

	static bool bDucking = false;
	static float bRunning = false;

	float flAirTime = pState->m_flDurationInAir;

	// fix ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB
	if( bOnLadder ) {
		if( bStartedLadderingThisFrame ) {
			nSequence = 13;
			SetLayerSequence( pLocal, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, -1, /*uModifiers,*/ nSequence );
		}
	}
	// fix ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL
	else {
		m_flLadderSpeed = m_flLadderWeight = 0.f;

		// TODO: bStoppedLadderingThisFrame
		if( bLandedOnGroundThisFrame ) {
			// setup the sequence responsible for landing
			nSequence = 20;
			if( ( bRunning > .25f ) )
				nSequence = 22;

			if( ( bDucking ) )
				nSequence = ( bRunning > .25f ) ? 19 : 21;

			if( ( flAirTime > 1.0f ) )
				nSequence = ( bDucking ) ? 24 : 23;

			SetLayerSequence( pLocal, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, -1, /*uModifiers,*/ nSequence );
			//printf( "land anim\n" );
		}
		else if( bLeftTheGroundThisFrame ) {
			bDucking = pState->m_flAnimDuckAmount > .55f;
			bRunning = pState->m_flSpeedAsPortionOfWalkTopSpeed;

			// setup the sequence responsible for jumping
			if( ( bRunning > .25f ) ) {
				nSequence = ( bDucking ) ? 18 : 16;
			}
			else {
				nSequence = ( bDucking ) ? 17 : 15;
			}

			SetLayerSequence( pLocal, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, -1, /*uModifiers,*/ nSequence );
		}
	}

	// apply our changes
	layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ] = *ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB;
	layers[ animstate_layer_t::ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ] = *ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL;
}

void ServerAnimations::SetLayerInactive( C_AnimationLayer *layers, int idx ) {
	if( !layers )
		return;

	layers[ idx ].m_flCycle = 0.f;
	layers[ idx ].m_nSequence = 0.f;
	layers[ idx ].m_flWeight = 0.f;
	layers[ idx ].m_flPlaybackRate = 0.f;
}

#include "AntiAim.hpp"
void ServerAnimations::HandleServerAnimation( ) {
	auto pLocal = C_CSPlayer::GetLocalPlayer( );
	if( !pLocal )
		return;

	if( !m_pCmd )
		return;

	// perform basic sanity checks
	if( pLocal->IsDead( ) )
		return;

	auto pState = pLocal->m_PlayerAnimState( );
	if( !pState )
		return;

	m_bHoldingSpace = m_pCmd->buttons & IN_JUMP;
	if( g_pClientState->m_nChokedCommands( ) )
		return;

	static QAngle vecThirdpersonAngle;

	// don't animate lby flick/pre flick
	if( !g_ServerAnimations.m_uRenderAnimations.m_bDoingRealFlick && !g_ServerAnimations.m_uRenderAnimations.m_bDoingPreFlick )
		vecThirdpersonAngle = m_pCmd->viewangles;

	// set thirdperson angles
	pLocal->pl( ).v_angle = vecThirdpersonAngle;

	// allow re-animating in the same frame
	if( pState->m_nLastUpdateFrame == g_pGlobalVars->framecount )
		pState->m_nLastUpdateFrame = g_pGlobalVars->framecount - 1;

	// prevent C_BaseEntity::CalcAbsoluteVelocity being called
	pLocal->m_iEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;

	// snap to footyaw, instead of approaching
	pState->m_flMoveWeight = 0.f;

	const bool bBackupClientSideAnimation = pLocal->m_bClientSideAnimation( );

	pLocal->m_bClientSideAnimation( ) = true;
	pLocal->UpdateClientSideAnimation( );
	pLocal->m_bClientSideAnimation( ) = bBackupClientSideAnimation;

	// store here as people might fuck with them in lua
	std::memcpy( m_uServerAnimations.m_pPoseParameters.data( ), pLocal->m_flPoseParameter( ), sizeof( float ) * 20 );

	if( pState->m_bLanding && ( g_Prediction.get_initial_vars( )->flags & FL_ONGROUND ) && ( pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
		float flPitch = -10.f;

		// -10.f pitch
		pLocal->m_flPoseParameter( )[ AIM_BLEND_CROUCH_IDLE ] = ( flPitch + 90.f ) / 180.f;
	}

	// note - michal;
	// might want to make some storage for constant anim variables
	constexpr float CSGO_ANIM_LOWER_REALIGN_DELAY{ 1.1f };

	static bool bWasMoving = false;

	static int nBrokenTicks = 0;

	// rebuild server CCSGOPlayerAnimState::SetUpVelocity
	// predict m_flLowerBodyYawTarget
	const float flServerTime = TICKS_TO_TIME( pLocal->m_nTickBase( ) );
	static bool bPrevGround = pState->m_bOnGround;
	if( !bPrevGround && pState->m_bOnGround ) {
		m_uServerAnimations.m_flLowerBodyRealignTimer = flServerTime;
		m_uRenderAnimations.m_flLowerBodyRealignTimer = g_pGlobalVars->realtime;
	}
	else if( pState->m_bOnGround ) {
		if( pState->m_flVelocityLengthXY > 0.1f ) {
			m_uServerAnimations.m_flLowerBodyRealignTimer = flServerTime + 0.22f;
			m_uRenderAnimations.m_flLowerBodyRealignTimer = g_pGlobalVars->realtime + 0.22f;
			m_uServerAnimations.m_bFirstFlick = bWasMoving = true;
		}
		else if( flServerTime >= m_uServerAnimations.m_flLowerBodyRealignTimer /*&& abs( Math::AngleDiff( pState->m_flFootYaw, pState->m_flEyeYaw ) ) > ( 35.0f )*/ ) {
			m_uServerAnimations.m_flLowerBodyRealignTimer = flServerTime + 1.1f;
			m_uRenderAnimations.m_flLowerBodyRealignTimer = g_pGlobalVars->realtime + 1.1f;
		}
	}
	bPrevGround = pState->m_bOnGround;

	if( g_AntiAim.m_bWaitForBreaker && g_ServerAnimations.m_uRenderAnimations.m_bDoingRealFlick ) {
		if( abs( Math::AngleDiff( pState->m_flFootYaw, pState->m_flEyeYaw ) ) > ( 35.0f ) )
			nBrokenTicks++;

		if( nBrokenTicks > 3 )
			g_AntiAim.m_bWaitForBreaker = false;
	}

	if( m_pCmd->buttons & IN_USE || ( pLocal->m_vecVelocity( ).Length2D( ) >= 0.1f && !g_Vars.globals.m_bFakeWalking ) ) {
		g_AntiAim.m_bWaitForBreaker = true;
		g_ServerAnimations.m_uRenderAnimations.m_bDoingRealFlick = false;
	}

	// lby isn't broken and hasn't been broken for 
	// a good while now, notify our anti-aim of this
	/*const float flLastValidDeltaTime = std::abs( m_uServerAnimations.m_flLastValidDelta - flServerTime );
	if( flLastValidDeltaTime > ( CSGO_ANIM_LOWER_REALIGN_DELAY * 0.5f ) && flDelta < 35.f ) {
		m_uServerAnimations.m_nFailedDeltaTicks++;
	}*/

	for( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
		g_ServerAnimations.m_uVisualAnimations.m_pAnimOverlays[ i ] = pLocal->m_AnimOverlay( )[ i ];
	}

	// handle animation events on client
	HandleAnimationEvents( pLocal, pState, pLocal->m_AnimOverlay( ).Base( ), /*modifiers,*/ m_pCmd );

	pLocal->m_AnimOverlay( )[ animstate_layer_t::ANIMATION_LAYER_LEAN ].m_flWeight = 0.f;

	// save ALL layers from this animation update
	for( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
		m_uServerAnimations.m_pAnimOverlays[ i ] = pLocal->m_AnimOverlay( )[ i ];
	}

#ifdef LUA_SCRIPTING
	//if( pLocal ) {
	//	Scripting::Script::DoCallback( hash_32_fnv1a_const( XorStr( "post_anim_update" ) ), Wrappers::Entity::CEntity( pLocal ) );
	//}
#endif

	// use this matrix for rendering
	matrix3x4_t pBackupMatrix[ MAXSTUDIOBONES ];
	std::memcpy( pBackupMatrix, pLocal->m_CachedBoneData( ).Base( ), pLocal->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );

	c_bone_builder m_pServerBones;
	m_pServerBones.store( pLocal, m_uServerAnimations.m_pMatrix, 0x7FF00 );
	m_pServerBones.setup( );

	if( g_ServerAnimations.m_uRenderAnimations.m_bDoingRealFlick ) {
		// angle between our non-flicked angle and our flicked angle
		const float flAngleDelta = Math::AngleDiff( vecThirdpersonAngle.y, m_pCmd->viewangles.y );

		// since we don't animate our lby flick, we gotta rotate the matrix
		// towards our lby angle, else it will just render on our regular matrix
		Vector vecBonePos, vecRotatedPos;
		for( int i = 0; i < MAXSTUDIOBONES; i++ ) {
			// create a matrix based around our wish rotation
			Math::AngleMatrix( QAngle( 0.f, flAngleDelta, 0.f ), m_uBodyAnimations.m_pMatrix[ i ] );
			Math::MatrixMultiply( m_uBodyAnimations.m_pMatrix[ i ], m_uServerAnimations.m_pMatrix[ i ] );

			// get the bone positions of our server matrix
			vecBonePos = Vector( m_uServerAnimations.m_pMatrix[ i ][ 0 ][ 3 ], m_uServerAnimations.m_pMatrix[ i ][ 1 ][ 3 ], m_uServerAnimations.m_pMatrix[ i ][ 2 ][ 3 ] );

			// get rid of the matrix origin, we don't want to 
			// rotate the 3d origin too xD
			vecBonePos -= pLocal->GetAbsOrigin( );

			// apply rotation to our matrix
			Math::VectorRotate( vecBonePos, QAngle( 0.f, flAngleDelta, 0.f ), vecRotatedPos );

			// add back our world position
			vecRotatedPos += pLocal->GetAbsOrigin( );

			// finally append our bone changes to our matrix which we will render lel
			m_uBodyAnimations.m_pMatrix[ i ][ 0 ][ 3 ] = vecRotatedPos.x;
			m_uBodyAnimations.m_pMatrix[ i ][ 1 ][ 3 ] = vecRotatedPos.y;
			m_uBodyAnimations.m_pMatrix[ i ][ 2 ][ 3 ] = vecRotatedPos.z;
		}

		g_ServerAnimations.m_uBodyAnimations.m_flBodyAlpha = 1.f;
	}

	for( int i = 0; i < MAXSTUDIOBONES; ++i ) for( int n = 0; n <= 2; ++n )
		g_ServerAnimations.m_uServerAnimations.m_pMatrix[ i ].m[ n ][ 3 ] -= pLocal->GetAbsOrigin( )[ n ];

	if( !g_Vars.globals.m_bShotWhileHiding ) {
		// save real rotation
		m_uServerAnimations.m_flFootYaw = pState->m_flFootYaw;
	}

	m_uServerAnimations.m_flEyeYaw = pState->m_flEyeYaw;
	m_uServerAnimations.m_flLowerBodyYawTarget = pLocal->m_flLowerBodyYawTarget( );

	std::memcpy( pLocal->m_flPoseParameter( ), m_uServerAnimations.m_pPoseParameters.data( ), sizeof( float ) * 20 );
}

void ServerAnimations::HandleAnimations( bool *bSendPacket, CUserCmd *cmd ) {
	if( !bSendPacket || !cmd )
		return;

	C_CSPlayer *pLocal = C_CSPlayer::GetLocalPlayer( );
	if( !pLocal )
		return;

	if( pLocal->IsDead( ) )
		return;

	auto pState = Encrypted_t<CCSGOPlayerAnimState>( pLocal->m_PlayerAnimState( ) );
	if( !pState.IsValid( ) )
		return;

	bool bFrozen = pLocal->m_fFlags( ) & 0x40;
	if( g_pGameRules.IsValid( ) ) {
		if( g_pGameRules->m_bFreezePeriod( ) ) {
			bFrozen = true;
		}
	}

	std::array<C_AnimationLayer, 13> pAnimOverlays;
	//std::array<float, 20> pPoseParameters;
	std::memcpy( pAnimOverlays.data( ), pLocal->m_AnimOverlay( ).Base( ), 13 * sizeof( C_AnimationLayer ) );
	//std::memcpy( pPoseParameters.data( ), pLocal->m_flPoseParameter( ), sizeof( float ) * 20 );

	matrix3x4_t pBackupMatrix[ MAXSTUDIOBONES ];
	std::memcpy( pBackupMatrix, pLocal->m_CachedBoneData( ).Base( ), pLocal->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );

	// restore ONLY layers we modified previous animation update
	// prevents the game from touching them, lets us update them 24/7
	for( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
		if( !HandleLayerSeparately( i ) )
			continue;

		// YO FUCK OFF GAME
		m_uServerAnimations.m_pAnimOverlays[ i ].m_pOwner = pLocal;
		m_uServerAnimations.m_pAnimOverlays[ i ].m_pStudioHdr = pLocal->m_pStudioHdr( );

		if( m_uServerAnimations.m_flSpawnTime != pLocal->m_flSpawnTime( ) || bFrozen ) {
			m_uServerAnimations.m_pAnimOverlays[ i ].m_flCycle = 0.f;
			m_uServerAnimations.m_pAnimOverlays[ i ].m_flWeight = 0.f;
			m_uServerAnimations.m_pAnimOverlays[ i ].m_flPlaybackRate = 0.f;

			if( !bFrozen ) {
				pState->m_pPlayer = pLocal;
				pState->Reset( );
			}
		}

		pLocal->m_AnimOverlay( )[ i ] = m_uServerAnimations.m_pAnimOverlays[ i ];
	}

	m_uServerAnimations.m_flSpawnTime = pLocal->m_flSpawnTime( );

	// handle server animations
	m_pCmd = cmd;
	HandleServerAnimation( );

	std::memcpy( pLocal->m_CachedBoneData( ).Base( ), pBackupMatrix, pLocal->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );
	std::memcpy( pLocal->m_AnimOverlay( ).Base( ), pAnimOverlays.data( ), 13 * sizeof( C_AnimationLayer ) );
	//std::memcpy( pLocal->m_flPoseParameter( ), pPoseParameters.data( ), sizeof( float ) * 20 );
}