#include "includes.h"

bool Hooks::ShouldDrawParticles( ) {
	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawParticles_t >( IClientMode::SHOULDDRAWPARTICLES )( this );
}

bool Hooks::ShouldDrawFog( ) {
	// remove fog.
	if ( g_menu.main.visuals.removals.get(3) )
		return false;

	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawFog_t >( IClientMode::SHOULDDRAWFOG )( this );
}

void Hooks::OverrideView( CViewSetup* view ) {
	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );


	if ( g_menu.main.visuals.removals.get(5) ) {
		static uintptr_t blurtest_target = pattern::find( g_csgo.m_client_dll, XOR( "0F 11 05 ? ? ? ? 0F 10 87" ) ).add( 0x3 ).as<uintptr_t>( );

		if ( blurtest_target ) {
			static post_process_parameters_t* blurtest = *reinterpret_cast< post_process_parameters_t** >( blurtest_target );

			if ( g_cl.m_weapon && g_cl.m_weapon->m_zoomLevel( ) != 0 )
			{
				if ( blurtest->m_parmeters[ PPPN_VIGNETTE_BLUR_STRENGTH ] != 0.f )
					blurtest->m_parmeters[ PPPN_VIGNETTE_BLUR_STRENGTH ] = 0.f;
			}
		}
	}

	g_visuals.ThirdpersonThink( );

	// call original.
	g_hooks.m_client_mode.GetOldMethod< OverrideView_t >( IClientMode::OVERRIDEVIEW )( this, view );
}
//
//bool Hooks::CreateMove( float time, CUserCmd* cmd ) {
//	Stack   stack;
//	bool    ret;
//
//	// let original run first.
//	ret = g_hooks.m_client_mode.GetOldMethod< CreateMove_t >( IClientMode::CREATEMOVE )( this, time, cmd );
//
//	// called from CInput::ExtraMouseSample -> return original.
//	if( !cmd->m_command_number )
//		return ret;
//
//	// if we arrived here, called from -> CInput::CreateMove
//	// call EngineClient::SetViewAngles according to what the original returns.
//	if( ret )
//		g_csgo.m_engine->SetViewAngles( cmd->m_view_angles );
//
//	// random_seed isn't generated in ClientMode::CreateMove yet, we must set generate it ourselves.
//	cmd->m_random_seed = g_csgo.MD5_PseudoRandom( cmd->m_command_number ) & 0x7fffffff;
//
//	// get bSendPacket off the stack.
//	g_cl.m_packet = stack.next( ).local( 0x1c ).as< bool* >( );
//
//	// get bFinalTick off the stack.
//	g_cl.m_final_packet = stack.next( ).local( 0x1b ).as< bool* >( );
//
//	// invoke move function.
//	g_cl.OnTick( cmd );
//
//	return false;
//}

bool Hooks::DoPostScreenSpaceEffects( CViewSetup* setup ) {
	g_visuals.RenderGlow( );

	return g_hooks.m_client_mode.GetOldMethod< DoPostScreenSpaceEffects_t >( IClientMode::DOPOSTSPACESCREENEFFECTS )( this, setup );
}