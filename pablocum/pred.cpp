#include "includes.h"
#include "pred.h"

InputPrediction g_inputpred { };;
RestoreData m_pRestoreData { };

void RestoreData::Setup( Player* player )
{
	info.m_aimPunchAngle = player->m_aimPunchAngle( );
	info.m_aimPunchAngleVel = player->m_aimPunchAngleVel( );
	info.m_viewPunchAngle = player->m_viewPunchAngle( );

	info.m_vecViewOffset = player->m_vecViewOffset( );
	info.m_vecBaseVelocity = player->m_vecBaseVelocity( );
	info.m_vecVelocity = player->m_vecVelocity( );
	info.m_vecOrigin = player->m_vecOrigin( );

	info.m_flFallVelocity = player->m_flFallVelocity( );
	info.m_flVelocityModifier = player->m_flVelocityModifier( );
	info.m_flDuckAmount = player->m_flDuckAmount( );
	info.m_flDuckSpeed = player->m_flDuckSpeed( );

	info.m_surfaceFriction = player->m_surfaceFriction( );
	info.m_GroundEntity = player->m_hGroundEntity( );
	info.m_nMoveType = player->m_MoveType( );
	info.m_nFlags = player->m_fFlags( );
	info.m_nTickBase = player->m_nTickBase( );

	Weapon* weapon = ( Weapon* )player->GetActiveWeapon( );
	if ( weapon )
	{
		info.m_fAccuracyPenalty = weapon->m_fAccuracyPenalty( );
		info.m_flRecoilIndex = weapon->m_flRecoilIndex( );
	}

	info.is_filled = true;
}

void RestoreData::Apply( Player* player )
{
	if ( !info.is_filled )
		return;

	player->m_aimPunchAngle( ) = info.m_aimPunchAngle;
	player->m_aimPunchAngleVel( ) = info.m_aimPunchAngleVel;
	player->m_viewPunchAngle( ) = info.m_viewPunchAngle;

	player->m_vecViewOffset( ) = info.m_vecViewOffset;
	player->m_vecBaseVelocity( ) = info.m_vecBaseVelocity;
	player->m_vecVelocity( ) = info.m_vecVelocity;
	player->m_vecOrigin( ) = info.m_vecOrigin;

	player->m_flFallVelocity( ) = info.m_flFallVelocity;
	player->m_flVelocityModifier( ) = info.m_flVelocityModifier;
	player->m_flDuckAmount( ) = info.m_flDuckAmount;
	player->m_flDuckSpeed( ) = info.m_flDuckSpeed;

	player->m_surfaceFriction( ) = info.m_surfaceFriction;
	player->m_hGroundEntity( ) = info.m_GroundEntity;
	player->m_MoveType( ) = info.m_nMoveType;
	player->m_fFlags( ) = info.m_nFlags;
	player->m_nTickBase( ) = info.m_nTickBase;

	Weapon* weapon = ( Weapon* )player->GetActiveWeapon( );
	if ( weapon )
	{
		weapon->m_fAccuracyPenalty( ) = info.m_fAccuracyPenalty;
		weapon->m_flRecoilIndex( ) = info.m_flRecoilIndex;
	}
}

void InputPrediction::update_viewmodel( bool store ) {
	if (!g_cl.m_local || !g_cl.m_local->alive())
		return;

	CBaseHandle& viewmodel_handle = g_cl.m_local->m_hViewModel();
	if (viewmodel_handle == 0xFFFFFFFF)
		return;

	Entity* viewmodel = g_csgo.m_entlist->GetClientEntityFromHandle(viewmodel_handle);

	if (!viewmodel)
		return;

	if (store) {
		m_anim_time = viewmodel->m_flAnimTime();
		m_cycle = viewmodel->m_flCycle();
		return;
	}

	viewmodel->m_flAnimTime() = m_anim_time;
	viewmodel->m_flCycle() = m_cycle;
}

void InputPrediction::start_command( Player* player, CUserCmd* pCmd )
{
	player->m_pCurrentCommand( ) = pCmd;
	//m_LastCmd shouldnt be needed if we run engine prediction
	*g_csgo.m_nPredictionRandomSeed = pCmd ? g_cl.m_cmd->m_random_seed : -1;
	g_csgo.m_pPredictionPlayer = g_cl.m_local;
}

void InputPrediction::finish_command( Player* player )
{
	player->m_pCurrentCommand( ) = NULL;
	*g_csgo.m_nPredictionRandomSeed = NULL;
	g_csgo.m_pPredictionPlayer = NULL;
}

void InputPrediction::run_prethink( Player* player )
{
	if ( !player->PhysicsRunThink( 0 ) )
		return;

	player->PreThink( );
}

void InputPrediction::run_think( Player* player )
{
	int thinktick = player->nextThinkTick( );
	if ( thinktick <= 0 || thinktick > player->m_nTickBase( ) )
		return;

	player->nextThinkTick( ) = -1;
	//sub_10179C60(0); // ????
	player->Think( );
}

void InputPrediction::update( bool store )
{
	static bool done = false;
	if ( !done )
	{
		auto cl_move_clamp = pattern::find( g_csgo.m_engine_dll, XOR( "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ) ) + 1;
		unsigned long protect = 0;

		VirtualProtect( ( void* )cl_move_clamp, 4, PAGE_EXECUTE_READWRITE, &protect );
		*( std::uint32_t* )cl_move_clamp = 62;
		VirtualProtect( ( void* )cl_move_clamp, 4, protect, &protect );
		done = true;
	}

	if ( store )
	{
		m_curtime = g_csgo.m_globals->m_curtime;
		m_frametime = g_csgo.m_globals->m_frametime;

		m_first_time_predicted = g_csgo.m_prediction->m_first_time_predicted;
		m_in_prediction = g_csgo.m_prediction->m_in_prediction;
	}

	if ( g_csgo.m_cl->m_delta_tick <= 0 )
		return;

	g_csgo.m_prediction->Update( g_csgo.m_cl->m_delta_tick, true, g_csgo.m_cl->m_last_command_ack, g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands );
}

void InputPrediction::post_think( Player* player )
{
	static auto PostThinkVPhysics = pattern::find( g_csgo.m_client_dll, "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB ? ? ? ? ? 75 50 8B 0D" ).as<bool( __thiscall* )( Entity* )>( );
	static auto SimulatePlayerSimulatedEntities = pattern::find( g_csgo.m_client_dll, "56 8B F1 57 8B BE ? ? ? ? 83 EF 01 78 72 90 8B 86" ).as<void( __thiscall* )( Entity* )>( );

	if ( player && player->alive( ) )
	{
		using UpdateCollisionBoundsFn = void( __thiscall* )( void* );
		util::get_method<UpdateCollisionBoundsFn>( player, 329 )( player );

		if ( player->m_fFlags( ) & FL_ONGROUND )
			*reinterpret_cast< uintptr_t* >( uintptr_t( player ) + 0x3004 ) = 0;

		if ( *reinterpret_cast< int* >( uintptr_t( player ) + 0x28AC ) == -1 )
		{
			using SetSequenceFn = void( __thiscall* )( void*, int );
			util::get_method<SetSequenceFn>( player, 213 )( player, 0 );
		}

		using StudioFrameAdvanceFn = void( __thiscall* )( void* );
		util::get_method<StudioFrameAdvanceFn>( player, 214 )( player );

		PostThinkVPhysics( player );
	}
	SimulatePlayerSimulatedEntities( player );
}

void InputPrediction::run( )
{
	std::memset( &m_MoveData, 0, sizeof( CMoveData ) );

	start_command( g_cl.m_local, g_cl.m_cmd );

	g_csgo.m_prediction->m_first_time_predicted = false;
	g_csgo.m_prediction->m_in_prediction = true;

	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	g_csgo.m_globals->m_frametime = g_csgo.m_prediction->m_engine_paused ? 0.f : g_csgo.m_globals->m_interval;

	g_cl.m_cmd->m_buttons |= g_cl.m_local->m_afButtonForced( );

	g_csgo.m_move_helper->SetHost( g_cl.m_local );
	g_csgo.m_game_movement->StartTrackPredictionErrors( g_cl.m_local );

	// Latch in impulse.
	if ( g_cl.m_cmd->m_impulse )
	{
		// Discard impulse commands unless the vehicle allows them.
		// FIXME: UsingStandardWeapons seems like a bad filter for this. 
		// The flashlight is an impulse command, for example.
		//if ( !pVehicle || player->UsingStandardWeaponsInVehicle( ) )
		//{
		g_cl.m_local->m_nImpulse( ) = g_cl.m_cmd->m_impulse;
		//}
	}

	g_cl.m_local->UpdateButtonState( g_cl.m_cmd->m_buttons );

	g_csgo.m_prediction->CheckMovingGround( g_cl.m_local, g_csgo.m_globals->m_frametime );

	run_prethink( g_cl.m_local );
	run_think( g_cl.m_local );

	m_old_ground_ent = g_cl.m_local->GetGroundEntity( );

	g_csgo.m_prediction->SetupMove( g_cl.m_local, g_cl.m_cmd, g_csgo.m_move_helper, &m_MoveData );
	m_MoveData.m_flForwardMove = g_cl.m_cmd->m_forward_move;
	m_MoveData.m_flSideMove = g_cl.m_cmd->m_side_move;
	m_MoveData.m_flUpMove = g_cl.m_cmd->m_up_move;
	m_MoveData.m_nButtons = g_cl.m_cmd->m_buttons;
	m_MoveData.m_vecOldAngles.y = g_cl.m_cmd->m_view_angles.x;
	m_MoveData.m_vecOldAngles.z = g_cl.m_cmd->m_view_angles.y;
	m_MoveData.m_outStepHeight = g_cl.m_cmd->m_view_angles.z;
	m_MoveData.m_vecViewAngles = g_cl.m_cmd->m_view_angles;
	m_MoveData.m_nImpulseCommand = g_cl.m_cmd->m_impulse;

	m_MoveData.m_bGameCodeMovedPlayer = false;
	m_MoveData.m_vecVelocity = g_cl.m_local->m_vecVelocity( );
	m_MoveData.m_vecAbsOrigin = g_cl.m_local->m_vecOrigin( );

	std::memcpy( &m_pRestoreData.info.m_MoveData, &m_MoveData, sizeof( CMoveData ) );
	m_pRestoreData.Setup( g_cl.m_local );

	g_csgo.m_game_movement->ProcessMovement( g_cl.m_local, &m_MoveData );

	g_csgo.m_prediction->FinishMove( g_cl.m_local, g_cl.m_cmd, &m_MoveData );
	g_csgo.m_move_helper->ProcessImpacts( );
	g_csgo.m_game_movement->Reset( );

	post_think( g_cl.m_local );

	finish_command( g_cl.m_local );

	auto weapon = g_cl.m_local->GetActiveWeapon( );
	if ( weapon )
		weapon->UpdateAccuracyPenalty( );

	update_viewmodel( true );
}

void InputPrediction::repredict( )
{
	m_pRestoreData.Apply( g_cl.m_local );

	g_inputpred.m_old_ground_ent = g_cl.m_local->GetGroundEntity();

	update( );

	start_command( g_cl.m_local, g_cl.m_cmd );

	g_csgo.m_prediction->m_first_time_predicted = false;
	g_csgo.m_prediction->m_in_prediction = true;

	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	g_csgo.m_globals->m_frametime = g_csgo.m_prediction->m_engine_paused ? 0.f : g_csgo.m_globals->m_interval;

	g_cl.m_cmd->m_buttons |= g_cl.m_local->m_afButtonForced( );

	g_csgo.m_move_helper->SetHost( g_cl.m_local );
	g_csgo.m_game_movement->StartTrackPredictionErrors( g_cl.m_local );

	// Latch in impulse.
	if ( g_cl.m_cmd->m_impulse )
	{
		// Discard impulse commands unless the vehicle allows them.
		// FIXME: UsingStandardWeapons seems like a bad filter for this. 
		// The flashlight is an impulse command, for example.
		//if ( !pVehicle || player->UsingStandardWeaponsInVehicle( ) )
		//{
		g_cl.m_local->m_nImpulse( ) = g_cl.m_cmd->m_impulse;
		//}
	}

	g_cl.m_local->UpdateButtonState( g_cl.m_cmd->m_buttons );

	g_csgo.m_prediction->CheckMovingGround( g_cl.m_local, g_csgo.m_globals->m_frametime );

	run_prethink( g_cl.m_local );
	run_think( g_cl.m_local );

	std::memcpy( &m_MoveData, &m_pRestoreData.info.m_MoveData, sizeof( CMoveData ) );
	m_MoveData.m_flForwardMove = g_cl.m_cmd->m_forward_move;
	m_MoveData.m_flSideMove = g_cl.m_cmd->m_side_move;
	m_MoveData.m_flUpMove = g_cl.m_cmd->m_up_move;
	m_MoveData.m_nButtons = g_cl.m_cmd->m_buttons;
	m_MoveData.m_vecOldAngles.y = g_cl.m_cmd->m_view_angles.x;
	m_MoveData.m_vecOldAngles.z = g_cl.m_cmd->m_view_angles.y;
	m_MoveData.m_outStepHeight = g_cl.m_cmd->m_view_angles.z;
	m_MoveData.m_vecViewAngles = g_cl.m_cmd->m_view_angles;
	m_MoveData.m_nImpulseCommand = g_cl.m_cmd->m_impulse;

	m_MoveData.m_bGameCodeMovedPlayer = false;
	m_MoveData.m_vecVelocity = g_cl.m_local->m_vecVelocity( );
	m_MoveData.m_vecAbsOrigin = g_cl.m_local->m_vecOrigin( );
	m_pRestoreData.Setup( g_cl.m_local );

	g_csgo.m_game_movement->ProcessMovement( g_cl.m_local, &m_MoveData );

	g_csgo.m_prediction->FinishMove( g_cl.m_local, g_cl.m_cmd, &m_MoveData );
	g_csgo.m_move_helper->ProcessImpacts( );
	g_csgo.m_game_movement->Reset( );

	post_think( g_cl.m_local );

	finish_command( g_cl.m_local );

	auto weapon = g_cl.m_local->GetActiveWeapon( );
	if ( weapon )
		weapon->UpdateAccuracyPenalty( );
}

void InputPrediction::restore( )
{
	// restore globals.
	g_csgo.m_globals->m_curtime = m_curtime;
	g_csgo.m_globals->m_frametime = m_frametime;

	g_csgo.m_prediction->m_first_time_predicted = m_first_time_predicted;
	g_csgo.m_prediction->m_in_prediction = m_in_prediction;

	m_pRestoreData.Apply( g_cl.m_local );

	g_csgo.m_game_movement->FinishTrackPredictionErrors( g_cl.m_local );
	g_csgo.m_move_helper->SetHost( nullptr );
}