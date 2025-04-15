#include "includes.h"

Visuals g_visuals{ };;

render::Font& Visuals::GetEspFont( bool small_font )
{
	return small_font ? render::esp_small : render::esp;
}

void Visuals::Skybox( )
{
	static auto sv_skyname = g_csgo.m_cvar->FindVar( HASH( "sv_skyname" ) );
	switch ( g_menu.main.visuals.sky_box.get( ) ) {
	case 1: //Tibet
		sv_skyname->SetValue( XOR( "cs_tibet" ) );
		break;
	case 2: //Embassy
		sv_skyname->SetValue( XOR( "embassy" ) );
		break;
	case 3: //Italy
		sv_skyname->SetValue( XOR( "italy" ) );
		break;
	case 4: //Daylight 1
		sv_skyname->SetValue( XOR( "sky_cs15_daylight01_hdr" ) );
		break;
	case 5: //Cloudy
		sv_skyname->SetValue( XOR( "sky_csgo_cloudy01" ) );
		break;
	case 6: //Night 1
		sv_skyname->SetValue( XOR( "sky_csgo_night02" ) );
		break;
	case 7: //Night 2
		sv_skyname->SetValue( XOR( "sky_csgo_night02b" ) );
		break;
	case 8: //Night Flat
		sv_skyname->SetValue( XOR( "sky_csgo_night_flat" ) );
		break;
	case 9: //Day HD
		sv_skyname->SetValue( XOR( "sky_day02_05_hdr" ) );
		break;
	case 10: //Day
		sv_skyname->SetValue( XOR( "sky_day02_05" ) );
		break;
	case 11: //Rural
		sv_skyname->SetValue( XOR( "sky_l4d_rural02_ldr" ) );
		break;
	case 12: //Vertigo HD
		sv_skyname->SetValue( XOR( "vertigo_hdr" ) );
		break;
	case 13: //Vertigo Blue HD
		sv_skyname->SetValue( XOR( "vertigoblue_hdr" ) );
		break;
	case 14: //Vertigo
		sv_skyname->SetValue( XOR( "vertigo" ) );
		break;
	case 15: //Vietnam
		sv_skyname->SetValue( XOR( "vietnam" ) );
		break;
	case 16: //Dusty Sky
		sv_skyname->SetValue( XOR( "sky_dust" ) );
		break;
	case 17: //Jungle
		sv_skyname->SetValue( XOR( "jungle" ) );
		break;
	case 18: //Nuke
		sv_skyname->SetValue( XOR( "nukeblank" ) );
		break;
	case 19: //Office
		sv_skyname->SetValue( XOR( "office" ) );
		break;
	}
}

void Visuals::ModulateWorld( )
{
	Skybox( );

	std::vector< IMaterial* > world, props, sky;

	// iterate material handles.
	for ( uint16_t h{ g_csgo.m_material_system->FirstMaterial( ) }; h != g_csgo.m_material_system->InvalidMaterial( ); h = g_csgo.m_material_system->NextMaterial( h ) )
	{
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial( h );
		if ( !mat )
			continue;

		// store world materials.
		if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "World textures" ) )
			world.push_back( mat );

		// store props.
		else if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "StaticProp textures" ) )
			props.push_back( mat );

		// store skybox
		else if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "SkyBox textures" ) )
			sky.push_back( mat );
	}

	// night
	Color& ref = g_menu.main.visuals.world_color.get( );
	Color& ref2 = g_menu.main.visuals.sky_color.get( );

	const int r = ref.r( ) * ( ref.a( ) / 255.f );
	const int g = ref.g( ) * ( ref.a( ) / 255.f );
	const int b = ref.b( ) * ( ref.a( ) / 255.f );
	const int a = ref.a( ) * ( ref.a( ) / 255.f );

	const int r2 = ref2.r( ) * ( ref2.a( ) / 255.f );
	const int g2 = ref2.g( ) * ( ref2.a( ) / 255.f );
	const int b2 = ref2.b( ) * ( ref2.a( ) / 255.f );

	Color world_clr = { ( r * r ) / 255, ( g * g ) / 255, ( b * b ) / 255, 255 };
	Color props_clr = { r, g, b, 255 };
	Color sky_clr = { r2, g2, b2, 255 };

	const float transparency = 1.f - ( g_menu.main.visuals.prop_transparency.get( ) / 100.f );

	if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 )
	{
		g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
	}

	if ( g_menu.main.visuals.world.get( 0 ) ) {
		for ( const auto& p : props )
		{
			p->ColorModulate( props_clr );
			p->AlphaModulate( transparency );
		}

		for ( const auto& w : world )
			w->ColorModulate( world_clr );
	}
	else {
		for ( const auto& p : props )
		{
			p->ColorModulate( { 255, 255, 255, 255 } );
			p->AlphaModulate( transparency );
		}

		for ( const auto& w : world )
			w->ColorModulate( { 255, 255, 255, 255 } );
	}

	if ( g_menu.main.visuals.world.get( 2 ) ) {
		for ( const auto& s : sky )
			s->ColorModulate( sky_clr );
	}
	else {
		for ( const auto& s : sky )
			s->ColorModulate( { 255, 255, 255, 255 } );
	}

	static ConVar* cl_csm_shadows = g_csgo.m_cvar->FindVar( HASH( "cl_csm_shadows" ) );
	if ( cl_csm_shadows )
		cl_csm_shadows->SetValue( g_menu.main.visuals.world.get( 3 ) ? 0 : 1 );
}

void Visuals::ThirdpersonThink( )
{
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if ( !g_csgo.m_engine->IsInGame( ) )
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive( );

	// camera should be in thirdperson.
	if ( m_thirdperson )
	{

		// if alive and not in thirdperson already switch to thirdperson.
		if ( alive && !g_csgo.m_input->CAM_IsThirdPerson( ) )
			g_csgo.m_input->CAM_ToThirdPerson( );

		// if dead and spectating in firstperson switch to thirdperson.
		else if ( g_cl.m_local->m_iObserverMode( ) == 4 )
		{

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if ( g_csgo.m_input->CAM_IsThirdPerson( ) )
			{
				g_csgo.m_input->CAM_ToFirstPerson( );
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode( ) = 5;
		}
	}

	// camera should be in firstperson.
	else if ( g_csgo.m_input->CAM_IsThirdPerson( ) )
	{
		g_csgo.m_input->CAM_ToFirstPerson( );
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if ( g_csgo.m_input->CAM_IsThirdPerson( ) )
	{
		// get camera angles.
		g_csgo.m_engine->GetViewAngles( offset );

		// get our viewangle's forward directional vector.
		math::AngleVectors( offset, &forward );

		// cam_idealdist convar.
		offset.z = g_menu.main.visuals.thirdperson_distance.get( );

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity( g_cl.m_local );

		g_csgo.m_engine_trace->TraceRay(
			Ray( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
			MASK_NPCWORLDSTATIC,
			( ITraceFilter* )&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp( tr.m_fraction, 0.f, 1.f );
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::Hitmarker( )
{
	auto RenderHitmarker = [ & ]( vec2_t vecCenter, const int nPaddingFromCenter, const int nSize, Color color ) {
		render::line( vecCenter - nPaddingFromCenter, vecCenter - nPaddingFromCenter - nSize, color );
		render::line( vecCenter + nPaddingFromCenter, vecCenter + nPaddingFromCenter + nSize, color );

		render::line( vecCenter - vec2_t( -nPaddingFromCenter, nPaddingFromCenter ), vecCenter - vec2_t( -nPaddingFromCenter - nSize, nPaddingFromCenter + nSize ), color );
		render::line( vecCenter + vec2_t( -nPaddingFromCenter, nPaddingFromCenter ), vecCenter + vec2_t( -nPaddingFromCenter - nSize, nPaddingFromCenter + nSize ), color );
		};

	const float curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	const float& duration = g_menu.main.misc.hitmarker_time.get( );

	if ( duration <= 0.f )
		return;

	for ( size_t i = 0; i < g_shots.m_impacts.size( ); i++ )
	{
		const auto& impact = g_shots.m_impacts.at( i );

		/* check if we hit the target*/
		if ( !impact.m_hurt )
			continue;

		/* check if we should render the impact */
		const float complete[ ] = {
			( curtime - game::TICKS_TO_TIME( impact.m_tick ) ),
			( curtime - game::TICKS_TO_TIME( impact.m_tick ) ) / duration
		};

		if ( complete[ 1 ] < 0.f || complete[ 1 ] > 1.f )
			continue;

		/* crosshair hitmarker */
		if ( i == 0 && g_menu.main.misc.hitmarker.get( 0 ) && complete[ 0 ] >= 0.f && complete[ 0 ] <= 1.f )
		{
			const int x = g_cl.m_width / 2,
				y = g_cl.m_height / 2;

			const int alpha = ( 1.f - complete[ 0 ] ) * 240;

			RenderHitmarker( vec2_t( x, y ), 4, 4, Color( 255, 255, 255, alpha ) );
		}

		/* world hitmarker */
		if ( g_menu.main.misc.hitmarker.get( 1 ) )
		{
			vec2_t pos;
			if ( !render::WorldToScreen( impact.m_pos, pos ) )
				continue;

			const int alpha = ( 1.f - complete[ 1 ] ) * 240;

			RenderHitmarker( pos, 4, 4, Color( 255, 255, 255, alpha ) );
		}
	}
}

void Visuals::NoSmoke( )
{
	if ( !smoke1 )
		smoke1 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_fire" ), XOR( "Other textures" ) );

	if ( !smoke2 )
		smoke2 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_smokegrenade" ), XOR( "Other textures" ) );

	if ( !smoke3 )
		smoke3 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods" ), XOR( "Other textures" ) );

	if ( !smoke4 )
		smoke4 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ), XOR( "Other textures" ) );

	if ( g_menu.main.visuals.removals.get(2) )
	{
		if ( !smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, true );
	}

	else
	{
		if ( smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, false );
	}
}

std::string Visuals::ResolveConfidence( const int& index ) {
	AimPlayer* data = &g_aimbot.m_players[ index - 1 ];
	if ( !data || data->m_records.empty( ) )
		return "";

	LagRecord* current = &data->m_records[ 0 ];
	if ( !current )
		return "";

	switch ( current->m_mode ) {
	case Modes::RESOLVE_NONE:
	case Modes::RESOLVE_DESYNC:
		return "VHIGH";
	case Modes::RESOLVE_WALK:
	case Modes::RESOLVE_STAND_STOPPEDMOVING:
		return "HIGH";
	case Modes::RESOLVE_STAND:
	case Modes::RESOLVE_STAND_LOGIC:
	case Modes::RESOLVE_UPDATE:
		return "MEDIUM";
	}

	return "LOW";
}

void Visuals::RenderOverrideArrow( )
{
	if ( !g_cl.m_correction_override || !g_resolver.m_override.m_player )
		return;

	AimPlayer* data = &g_aimbot.m_players[ g_resolver.m_override.m_player->index( ) - 1 ];
	if ( !data || data->m_records.empty( ) )
		return;

	LagRecord* record = &data->m_records.front( );
	if ( !record )
		return;

	Color clr = record->m_mode == Modes::RESOLVE_UPDATE ? Color( 255, 255, 255, 205 ) : Color( 168, 230, 69, 205 );

	// here we do arrows
	vec2_t screen_start, screen_end;

	if ( render::WorldToScreen( g_resolver.m_override.m_start, screen_start ) )
	{
		if ( render::WorldToScreen( g_resolver.m_override.m_end, screen_end ) )
		{
			vec2_t delta = screen_end - screen_start;

			ang_t dir;
			math::VectorAngles( g_resolver.m_override.m_end - g_resolver.m_override.m_start, dir );

			render::line( screen_start, screen_end, clr );

			float ang = math::NormalizedAngle( g_resolver.m_override.m_yaw );

			if ( fabs( ang ) <= 45.f )
				render::triangle( screen_end + vec2_t( 0, 16 ), screen_end + vec2_t( -8, 0 ), screen_end + vec2_t( 8, 0 ), clr );

			else if ( fabs( ang ) >= 135.f )
				render::triangle( screen_end + vec2_t( 0, -16 ), screen_end + vec2_t( 8, 0 ), screen_end + vec2_t( -8, 0 ), clr );

			else if ( delta.x < 0 )
				render::triangle( screen_end + vec2_t( -16, 0 ), screen_end + vec2_t( 0, -8 ), screen_end + vec2_t( 0, 8 ), clr );

			else if ( delta.x > 0 )
				render::triangle( screen_end + vec2_t( 16, 0 ), screen_end + vec2_t( 0, 8 ), screen_end + vec2_t( 0, -8 ), clr );
		}
	}
}

void Visuals::think( )
{
	// don't run anything if our local player isn't valid.
	if ( !g_cl.m_local )
		return;

	if ( g_menu.main.visuals.removals.get(5)
		&& g_cl.m_local->alive( )
		&& g_cl.m_local->GetActiveWeapon( )
		&& g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped( ) )
	{

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt( );

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if ( size > 1 )
		{
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		// draw our lines.
		render::rect_filled( 0, y, w, size, colors::black );
		render::rect_filled( x, 0, size, h, colors::black );
	}

	//PlayerAngleLines( );

	// draw esp on ents.
	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i )
	{
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		draw( ent );
	}

	if ( g_cl.m_local->alive( ) )
		RenderOverrideArrow( );

	// draw everything else.
	ClientImpacts( );
	SpreadCrosshair( );
	StatusIndicators( );
	Spectators( );
	PenetrationCrosshair( );
	Hitmarker( );
	DrawPlantedC4( );

	auto& predicted_nades = g_grenade_warning.get_list( );

	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		if ( ent->dormant( ) )
			continue;

		if ( !ent->is( HASH( "CMolotovProjectile" ) )
			&& !ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) )
			continue;

		if ( ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) ) {
			const auto studio_model = ent->GetModel( );
			if ( !studio_model
				|| std::string_view( studio_model->m_name ).find( "fraggrenade" ) == std::string::npos )
				continue;
		}

		const auto handle = reinterpret_cast< Player* >( ent )->GetRefEHandle( );

		if ( ent->m_fEffects( ) & EF_NODRAW ) {
			predicted_nades.erase( handle );

			continue;
		}

		Player* thrower = reinterpret_cast< Player* >( g_csgo.m_entlist->GetClientEntityFromHandle( ent->m_hThrower( ) ) );

		if ( predicted_nades.find( handle ) == predicted_nades.end( ) ) {
			predicted_nades.emplace(
				std::piecewise_construct,
				std::forward_as_tuple( handle ),
				std::forward_as_tuple(
					thrower,
					ent->is( HASH( "CMolotovProjectile" ) ) ? MOLOTOV : HEGRENADE,
					ent->m_vecOrigin( ), ent->m_vecVelocity( ), ent->m_flSpawnTimeGrenade( ),
					game::TIME_TO_TICKS( reinterpret_cast< Player* >( ent )->m_flSimulationTime( ) - ent->m_flSpawnTimeGrenade( ) )
				)
			);
		}

		if ( predicted_nades.at( handle ).draw( ) )
			continue;

		predicted_nades.erase( handle );
	}

	if ( g_menu.main.visuals.grenade_trajectory.get( ) )
		g_grenade_warning.m_data.draw( );
}

void Visuals::Spectators( )
{
	if ( !g_menu.main.visuals.spectators.get( ) )
		return;

	std::vector< std::string > spectators{ XOR( "spectators" ) };
	int h = render::menu_shade.m_size.m_height;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i )
	{
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !player )
			continue;

		if ( player->m_bIsLocalPlayer( ) )
			continue;

		if ( player->dormant( ) )
			continue;

		if ( player->m_lifeState( ) == LIFE_ALIVE )
			continue;

		if ( player->GetObserverTarget( ) != g_cl.m_local )
			continue;

		player_info_t info;
		if ( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
			continue;

		spectators.push_back( std::string( info.m_name ).substr( 0, 24 ) );
	}

	size_t total_size = spectators.size( ) * ( h - 1 );

	for ( size_t i{ }; i < spectators.size( ); ++i )
	{
		const std::string& name = spectators[ i ];

		render::menu_shade.string( g_cl.m_width - 20, ( g_cl.m_height / 2 ) - ( total_size / 2 ) + ( i * ( h - 1 ) ),
			{ 255, 255, 255, 179 }, name, render::ALIGN_RIGHT );
	}
}

void Visuals::ClientImpacts( )
{
	if ( g_menu.main.visuals.bullet_impacts.get( 0 ) )
	{
		Color& color = g_menu.main.visuals.client_impact_color.get( );

		static auto last_count = 0;

		const float size = g_menu.main.visuals.impact_size.get( ) / 10.f;
		auto& client_impact_list = *( CUtlVector< ClientHitVerify_T >* )( ( uintptr_t )g_cl.m_local + 0xBA84 );

		for ( auto i = client_impact_list.Count( ); i > last_count; i-- )
		{
			g_csgo.m_debug_overlay->AddBoxOverlay( client_impact_list[ i - 1 ].pos,
				{ -size, -size, -size },
				{ size, size, size },
				{ 0, 0, 0 },
				color.r( ), color.g( ), color.b( ), color.a( ),
				4.f );
		}

		if ( client_impact_list.Count( ) != last_count )
			last_count = client_impact_list.Count( );
	}

	if ( g_menu.main.visuals.bullet_impacts.get( 1 ) )
	{
		Color& color = g_menu.main.visuals.server_impact_color.get( );

		const float size = g_menu.main.visuals.impact_size.get( ) / 10.f;

		for ( auto& impact : g_shots.m_vis_impacts )
		{
			if ( impact.m_ignore )
				continue;

			impact.m_ignore = true;

			g_csgo.m_debug_overlay->AddBoxOverlay( impact.m_impact_pos,
				{ -size, -size, -size },
				{ size, size, size },
				{ 0, 0, 0 },
				color.r( ), color.g( ), color.b( ), color.a( ),
				4.f );
		}
	}
}

void Visuals::PlayerAngleLines( )
{
	//if ( !g_cl.m_local->alive( ) )
	//	return;

	//static vec3_t src3D, dst3D, forward;
	//static vec2_t src, dst;
	//const Color& color_real = g_menu.main.visuals.color_real.get( );
	//const Color& color_lby = g_menu.main.visuals.color_lby.get( );

	//src3D = g_cl.m_local->GetAbsOrigin( );

	////REAL
	//if ( g_menu.main.visuals.local_aa_lines.get( 0 ) ) {
	//	math::AngleVectors( { 0, g_cl.m_angle.y, 0 }, &forward );
	//	dst3D = src3D + ( forward * 50.f );

	//	if ( !render::WorldToScreen( src3D, src ) || !render::WorldToScreen( dst3D, dst ) )
	//		return;
	//	//src.y += 5;
	//	//dst.y += 5;

	//	render::line( src.x, src.y, dst.x, dst.y, color_real );
	//	render::esp_small.string( ( int )dst.x, ( int )dst.y, color_real, "REAL", render::ALIGN_CENTER );
	//}

	////LBY
	//if ( g_menu.main.visuals.local_aa_lines.get( 1 ) ) {
	//	math::AngleVectors( { 0, g_cl.m_body, 0 }, &forward );
	//	dst3D = src3D + ( forward * 50.f );

	//	if ( !render::WorldToScreen( src3D, src ) || !render::WorldToScreen( dst3D, dst ) )
	//		return;

	//	//src.y += 5;
	//	//dst.y += 5;

	//	render::line( src.x, src.y, dst.x, dst.y, color_lby );
	//	render::esp_small.string( ( int )dst.x, ( int )dst.y, color_lby, "LBY", render::ALIGN_CENTER );
	//}

}

void Visuals::StatusIndicators( )
{
	// dont do if dead.
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	int offset = 50;

	int x = g_cl.m_width / 2, y = g_cl.m_height / 2;

	Color color = g_cl.m_auto_peek ? g_menu.main.movement.autopeek_color.get() : g_menu.main.antiaim.manual_color.get();

	float multiplier = 0.90f;
	int size[ ] = { 65 * multiplier, 45 * multiplier, 10 * multiplier };

	if ( g_hvh.m_left )
		render::triangle( vec2_t( x - size[ 0 ], y ), vec2_t( x - size[ 1 ], y - size[ 2 ] ), vec2_t( x - size[ 1 ], y + size[ 2 ] ), color );

	if ( g_hvh.m_right )
		render::triangle( vec2_t( x + size[ 0 ], y ), vec2_t( x + size[ 1 ], y + size[ 2 ] ), vec2_t( x + size[ 1 ], y - size[ 2 ] ), color );

	if ( g_hvh.m_back )
		render::triangle( vec2_t( x, y + size[ 0 ] ), vec2_t( x - size[ 2 ], y + size[ 1 ] ), vec2_t( x + size[ 2 ], y + size[ 1 ] ), color );

	if ( g_hvh.m_front )
		render::triangle( vec2_t( x, y - size[ 0 ] ), vec2_t( x + size[ 2 ], y - size[ 1 ] ), vec2_t( x - size[ 2 ], y - size[ 1 ] ), color );

	// compute hud size.
	// int size = ( int )std::round( ( g_cl.m_height / 17.5f ) * g_csgo.hud_scaling->GetFloat( ) );

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	// LC
	if ( g_menu.main.visuals.indicators.get( 1 ) )
	{
		if ( g_cl.m_local->m_vecVelocity( ).length_2d( ) > 270.f || g_cl.m_lagcomp )
		{
			Indicator_t ind{ };
			ind.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
			ind.text = XOR( "LC" );

			indicators.push_back( ind );
		}
	}

	// LBY
	if ( g_menu.main.visuals.indicators.get( 0 ) )
	{
		Indicator_t ind{ };
		if ( g_hvh.m_exploit && g_menu.main.antiaim.enable.get( ) )
			ind.color = { 255, 255, 35 };
		else
			ind.color = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) ) > 35.f ? 0xff15c27b : 0xff0000ff;

		ind.text = XOR( "LBY" );
		indicators.push_back( ind );
	}

	// PING
	if ( g_menu.main.visuals.indicators.get( 2 ) && ( g_aimbot.m_fake_latency || g_menu.main.misc.fake_latency.get( ) ) )
	{
		Indicator_t ind{ };
		ind.color = g_aimbot.m_fake_latency ? 0xffffffff : 0xff15c27b;
		ind.text = XOR( "PING" );

		if ( g_csgo.m_net ) {
			const float target_latency = ( g_aimbot.m_fake_latency ? g_menu.main.misc.secondary_latency_amt.get( ) : g_menu.main.misc.fake_latency_amt.get( ) ) / 1000.f;

			const float latency = g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING ) + g_csgo.m_net->GetLatency( INetChannel::FLOW_INCOMING );
			const float& frac = std::clamp( 1.f - ( latency / target_latency ), 0.f, 1.f );

			ind.color = {
				math::Lerp( frac, ( int )ind.color.r( ), 255 ),
				math::Lerp( frac, ( int )ind.color.g( ), 0 ),
				math::Lerp( frac, ( int )ind.color.b( ), 0 ),
				255
			};
		}

		indicators.push_back( ind );
	}

	// DMG
	if ( g_menu.main.visuals.indicators.get( 3 ) && g_aimbot.m_damage_override )
	{
		Indicator_t ind{ };
		ind.color = 0xffffffff;
		ind.text = XOR( "DMG" );

		indicators.push_back( ind );
	}

	// FORCEBAIM
	if ( g_menu.main.visuals.indicators.get( 4 ) && g_aimbot.m_force_body )
	{
		Indicator_t ind{ };
		ind.color = 0xffffffff;
		ind.text = XOR( "BAIM" );

		indicators.push_back( ind );
	}

	if ( indicators.empty( ) )
		return;

	// iterate and draw indicators.
	for ( size_t i{ }; i < indicators.size( ); ++i )
	{
		auto& indicator = indicators[ i ];

		render::indicator.string( 20, g_cl.m_height - 80 - ( 30 * i ), indicator.color, indicator.text );
	}
}

void Visuals::SpreadCrosshair( )
{
	// dont do if dead.
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	if ( !g_menu.main.visuals.spread_xhair.get( ) )
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon( );
	if ( !weapon )
		return;

	WeaponInfo* data = weapon->GetWpnData( );
	if ( !data )
		return;

	// do not do this on: bomb, knife, nades, and healthshot.
	CSWeaponType type = data->m_weapon_type;
	if ( type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE || type == WEAPONTYPE_HEALTHSHOT )
		return;

	// calc radius.
	float radius = ( ( weapon->GetInaccuracy( ) + weapon->GetSpread( ) ) * 320.f ) / ( std::tan( math::deg_to_rad( g_cl.m_local->GetFOV( ) ) * 0.5f ) + FLT_EPSILON );

	// scale by screen size.
	radius *= g_cl.m_height * ( 1.f / 480.f );

	// get color.
	Color col = g_menu.main.visuals.spread_xhair_col.get( );

	// modify alpha channel.
	col.a( ) = 200 * ( col.a( ) / 255.f );

	int segements = std::max( 16, ( int )std::round( radius * 0.75f ) );
	render::circle( g_cl.m_width / 2, g_cl.m_height / 2, radius, segements, col );
}

void Visuals::PenetrationCrosshair( )
{
	static int   x, y;
	static Color final_color;

	if ( !g_menu.main.visuals.pen_crosshair.get( ) || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	if ( game::IsValidHitgroup( g_cl.m_pen_data.m_hitgroup ) ) {
		render::rect( x - 1, y - 1, 3, 3, colors::transparent_yellow );
		return;
	}

	if ( g_cl.m_pen_data.m_pen < 4 ) {
		render::rect( x - 1, y - 1, 3, 3, colors::transparent_green );
		return;
	}

	render::rect( x - 1, y - 1, 3, 3, colors::transparent_red );
}

void Visuals::draw( Entity* ent )
{
	if ( ent->IsPlayer( ) )
	{
		Player* player = ent->as< Player* >( );

		if ( player->m_bIsLocalPlayer( ) )
			return;

		if ( player->alive( ) )
			m_alive_time[ player->index( ) - 1 ] = g_csgo.m_globals->m_curtime;

		// draw player esp.
		DrawPlayer( player );
	}

	else if ( g_menu.main.visuals.items.get( ) && ent->IsBaseCombatWeapon( ) && !ent->dormant( ) )
		DrawItem( ent->as< Weapon* >( ) );

	else if ( g_menu.main.visuals.proj.get( ) )
		DrawProjectile( ent->as< Weapon* >( ) );
}

void Visuals::DrawProjectile(Weapon* ent) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	if (!g_cl.m_local)
		return;

	Player* thrower = (Player*)g_csgo.m_entlist->GetClientEntityFromHandle(ent->m_hOwnerEntity());
	if (!thrower)
		return;

	bool safe_grenade_ting = true;
	if (thrower->enemy(g_cl.m_local) || thrower == g_cl.m_local) {
		safe_grenade_ting = false;
	}

	Color col = safe_grenade_ting ? g_menu.main.visuals.proj_safe_color.get() : g_menu.main.visuals.proj_dangerous_color.get();

	if (ent->is(HASH("CHostage"))) {
		std::string distance;
		int dist = (((ent->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
		if (dist < 150) {
			render::esp.string(screen.x, screen.y, colors::white, XOR("hostage"), render::ALIGN_CENTER);
		}
	}

	// find classes.
	if (ent->is(HASH("CInferno"))) {
		// get molotov info.
		const double spawn_time = *(float*)(uintptr_t(ent) + 0x20);
		const double factor = ((spawn_time + 7.031) - g_csgo.m_globals->m_curtime) / 7.031;

		if (factor < 0 || spawn_time < 0.f)
			return;

		float time_since_explosion = g_csgo.m_globals->m_interval * (g_csgo.m_globals->m_tick_count - spawn_time);

		// setup our vectors.
		vec3_t mins, maxs;

		// get molotov bounds (current radius).
		ent->GetRenderBounds(mins, maxs);

		float radius = (maxs - mins).length_2d() * 0.5;
		//radius *= (std::min(time_since_explosion, 0.1f) / 0.1f); molotov radius actually kinda matters to see so maybe dont interpolate it..

		// render the molotov range circle.
		float alpha_factor = factor <= 0.1f ? factor * 10.f : 1.f;
		col.a() = (10.f) * alpha_factor;
		col.a() = (40.f) * alpha_factor;
		render::WorldCircleOutline(origin, radius, 0.f, col);
		Color col1 = safe_grenade_ting ? g_menu.main.visuals.proj_safe_color.get() : g_menu.main.visuals.proj_dangerous_color.get();

		col1.a() = (250.f) * alpha_factor;

		// name.
		//col.a() = 255.f;
		render::esp_small.string(screen.x, screen.y, col1, XOR("FIRE"), render::ALIGN_CENTER);
	}

	else if (ent->is(HASH("CSmokeGrenadeProjectile"))) {
		// get smoke info.
		const float spawn_time = game::TICKS_TO_TIME(ent->m_nSmokeEffectTickBegin());
		const double factor = ((spawn_time + 18.041) - g_csgo.m_globals->m_curtime) / 18.041;

		// smokes are never dangerous lol.
		col = g_menu.main.visuals.proj_safe_color.get();

		// make sure the smoke effect has started
		if (spawn_time >= 0.f && factor >= 0.f) {
			float radius = 144.f;
			auto time_since_explosion = g_csgo.m_globals->m_interval * (g_csgo.m_globals->m_tick_count - ent->m_nSmokeEffectTickBegin());

			radius *= (std::min(time_since_explosion, 0.1f) / 0.1f);

			// render the smoke range circle.
			// idk why lol it was always this much too far ahead... just hard code adjust for it ig.
			float alpha_factor = (factor - 0.031812f) <= 0.1f ? (factor - 0.031812) * 10.f : 1.f;
			col.a() = (10.f) * alpha_factor;
			col.a() = (40.f) * alpha_factor;
			render::WorldCircleOutline(origin, radius, 0.f, col);
			Color col1 = g_menu.main.visuals.proj_safe_color.get();


			col1.a() = (250.f) * alpha_factor;
			render::esp_small.string(screen.x, screen.y, col1, XOR("SMOKE"), render::ALIGN_CENTER);

		}
	}
	else if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
		// get smoke info.
		//const float spawn_time = game::TICKS_TO_TIME(ent->m_nSmokeEffectTickBegin());
		//const double factor = ((spawn_time + 18.041) - g_csgo.m_globals->m_curtime) / 18.041;

		// smokes are never dangerous lol.
		//col = g_menu.main.visuals.proj_safe_color.get();


			// render the smoke range circle.
			// idk why lol it was always this much too far ahead... just hard code adjust for it ig.
			//float alpha_factor = (factor - 0.031812f) <= 0.1f ? (factor - 0.031812) * 10.f : 1.f;
			//col.a() = (80.f) * alpha_factor;
		//	render::world_circle(origin, radius, 0.f, col);
		//	col.a() = (180.f) * alpha_factor;
			//render::WorldCircleOutline(origin, radius, 0.f, col);

		//col.a() = 255.f;
		render::esp_small.string(screen.x, screen.y, col, XOR("FRAG"), render::ALIGN_CENTER);
	}
}


void Visuals::DrawItem( Weapon* item )
{
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle( item->m_hOwnerEntity( ) );
	if ( owner )
		return;

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin( );
	if ( !render::WorldToScreen( origin, screen ) )
		return;

	WeaponInfo* data = item->GetWpnData( );
	if ( !data )
		return;

	Color col = g_menu.main.visuals.item_color.get( );
	col.a( ) = 180;

	// render bomb in green.
	if ( item->is( HASH( "CC4" ) ) )
		GetEspFont( true ).string( screen.x, screen.y, { 150, 200, 60, 180 }, XOR( "BOMB" ), render::ALIGN_CENTER );

	// if not bomb
	// normal item, get its name.
	else
	{
		std::string name{ item->GetLocalizedName( ) };

		// smallfonts needs uppercase.
		std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

		GetEspFont( true ).string( screen.x, screen.y, col, name, render::ALIGN_CENTER );
	}

	if ( !g_menu.main.visuals.ammo.get( ) )
		return;

	// nades do not have ammo.
	if ( data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE )
		return;

	if ( item->m_iItemDefinitionIndex( ) == 0 || item->m_iItemDefinitionIndex( ) == C4 )
		return;

	std::string ammo = tfm::format( XOR( "(%i/%i)" ), item->m_iClip1( ), item->m_iPrimaryReserveAmmoCount( ) );
	GetEspFont( true ).string( screen.x, screen.y - GetEspFont( true ).m_size.m_height - 1, col, ammo, render::ALIGN_CENTER );
}

void Visuals::OffScreen( Player* player, int alpha )
{
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[ 3 ], verts_outline[ 3 ];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = [ ]( const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation ) {
		ang_t  view_angles( g_csgo.m_view_render->m_view.m_angles );
		vec3_t fwd, right, up( 0.f, 0.f, 1.f );
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors( view_angles, &fwd );

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize( );

		// calculate front / side positions.
		right = up.cross( fwd );
		front = delta.dot( fwd );
		side = delta.dot( right );

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg( std::atan2( out_offscreen_pos.x, out_offscreen_pos.y ) + math::pi );

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad( -out_rotation );
		sa = std::sin( yaw_rad );
		ca = std::cos( yaw_rad );

		// rotate offscreen position around.
		out_offscreen_pos.x = ( int )( ( g_cl.m_width / 2.f ) + ( radius * sa ) );
		out_offscreen_pos.y = ( int )( ( g_cl.m_height / 2.f ) - ( radius * ca ) );
		};

	if ( !g_menu.main.players.offscreen.get( ) )
		return;

	if ( !g_cl.m_local || !g_cl.m_local->enemy( player ) )
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter( );
	is_on_screen = render::WorldToScreen( target_pos, screen_pos );

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if ( !is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >( g_cl.m_width + leeway_x )
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >( g_cl.m_height + leeway_y ) )
	{

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = ( target_pos - view_origin ).normalized( );

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = 200.f * ( g_cl.m_height / 480.f );

		// get the data we need for rendering.
		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[ 0 ] = { offscreen_pos.x, offscreen_pos.y };        // 0,  0
		verts[ 1 ] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f }; // -1, 1
		verts[ 2 ] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[ 0 ] = { verts[ 0 ].m_pos.x - 1.f, verts[ 0 ].m_pos.y - 1.f };
		verts_outline[ 1 ] = { verts[ 1 ].m_pos.x - 1.f, verts[ 1 ].m_pos.y + 1.f };
		verts_outline[ 2 ] = { verts[ 2 ].m_pos.x + 1.f, verts[ 2 ].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[ 0 ] = render::RotateVertex( offscreen_pos, verts[ 0 ], offscreen_rotation );
		verts[ 1 ] = render::RotateVertex( offscreen_pos, verts[ 1 ], offscreen_rotation );
		verts[ 2 ] = render::RotateVertex( offscreen_pos, verts[ 2 ], offscreen_rotation );
		// verts_outline[ 0 ] = render::RotateVertex( offscreen_pos, verts_outline[ 0 ], offscreen_rotation );
		// verts_outline[ 1 ] = render::RotateVertex( offscreen_pos, verts_outline[ 1 ], offscreen_rotation );
		// verts_outline[ 2 ] = render::RotateVertex( offscreen_pos, verts_outline[ 2 ], offscreen_rotation );



		// render!
		color = g_menu.main.players.offscreen_color.get( ); // damage_data.m_color;
		color.a( ) = ( alpha == 255 ) ? alpha : alpha / 2;

		render::triangle( verts[ 0 ].m_pos, verts[ 2 ].m_pos, verts[ 1 ].m_pos, color );

		// g_csgo.m_surface->DrawSetColor( colors::black );
		// g_csgo.m_surface->DrawTexturedPolyLine( 3, verts_outline );
	}
}

void Visuals::DrawPlayer( Player* player )
{
	const static float MAX_DORMANT_TIME = 10.f;
	const static float DORMANT_FADE_DELTA = 2.f;
	const static float DORMANT_FADE_TIME = MAX_DORMANT_TIME - DORMANT_FADE_DELTA;

	player_info_t info;
	Color		  color;

	const bool& alive = player->alive( );

	// retarded servers that go above 100 hp..
	const float hp = std::min( 100, player->m_iHealth( ) );

	// get player index.
	const int& index = player->index( );

	// get reference to bbox
	auto& box = m_box[ index - 1 ];

	// get reference to array variable.
	float& opacity = m_opacities[ index - 1 ];

	// opacity should reach 1 in 300 milliseconds.
	const static int frequency = 1.f / 0.3f;

	// is player enemy.
	const bool& enemy = player->enemy( g_cl.m_local );
	const bool& dormant = player->dormant( );

	if ( g_menu.main.visuals.enemy_radar.get( ) && enemy && !dormant )
		player->m_bSpotted( ) = true;

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy && g_menu.main.players.dormant.get( );

	if ( !alive )
		opacity = 0.f;
	else
		opacity += g_csgo.m_globals->m_frametime * 2.f;

	// clamp the opacity.
	math::clamp( opacity, 0.f, 1.f );
	if ( !opacity )
		return;

	// stay for x seconds max.
	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime( );
	if ( dormant && ( dt > MAX_DORMANT_TIME || !dormant_esp ) )
		return;

	// calculate alpha channels.
	int alpha = ( int )( 255.f * opacity );
	int low_alpha = ( int )( 179.f * opacity );
	int bg_alpha = ( int )( 150.f * opacity );

	// get color based on enemy or not.
	color = enemy ? g_menu.main.players.box_enemy.get( ) : g_menu.main.players.box_friendly.get( );

	if ( dormant && dormant_esp )
	{
		alpha /= 2.f;
		low_alpha /= 2.f;
		bg_alpha /= 2.f;

		// fade.
		float delta = dt - DORMANT_FADE_TIME;
		if ( dt > 0.f )
		{
			alpha *= 1.f - std::clamp( delta / DORMANT_FADE_DELTA, 0.f, 1.f );
			low_alpha *= 1.f - std::clamp( delta / DORMANT_FADE_DELTA, 0.f, 1.f );
		}

		// override color.
		color = { 112, 112, 112 };
	}

	// override alpha.
	color.a( ) = alpha;

	// get player info.
	if ( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	// run offscreen ESP.
	OffScreen( player, alpha );

	// attempt to get player box.
	//if (alive)
	box.first = GetPlayerBoxRect( player, box.second );

	if ( !box.first )
		return;

	DebugAimbotPoints( player );

	if ( enemy ) {
		if ( g_menu.main.players.skeleton_enemy_enable.get( 0 ) )
			DrawSkeleton( player, opacity );

		if ( g_menu.main.players.skeleton_enemy_enable.get( 1 ) )
			DrawHistorySkeleton( player, opacity );
	}
	else {
		if ( g_menu.main.players.skeleton_friendly_enable.get( ) )
			DrawSkeleton( player, opacity );
	}

	// is box esp enabled for this player.
	bool box_esp = ( enemy && g_menu.main.players.box_enemy_enable.get( ) ) || ( !enemy && g_menu.main.players.box_friendly_enable.get( ) );

	// render box if specified.
	if ( box_esp )
		render::rect_outlined( box.second.x, box.second.y, box.second.w, box.second.h, color, { 10, 10, 10, low_alpha } );

	// is name esp enabled for this player.
	bool name_esp = ( enemy && g_menu.main.players.name_enemy.get( ) ) || ( !enemy && g_menu.main.players.name_friendly.get( ) );

	// draw name.
	if ( name_esp )
	{
		// fix retards with their namechange meme 
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		const std::string& name{ std::string( info.m_name ).substr( 0, 24 ) };

		Color clr = enemy ? g_menu.main.players.name_color_enemy.get( ) : g_menu.main.players.name_color_friendly.get( );
		// override alpha.
		clr.a( ) = low_alpha;

		GetEspFont( false ).string( box.second.x + ( box.second.w / 2 ), box.second.y - GetEspFont( false ).m_size.m_height, clr, name, render::ALIGN_CENTER );
	}

	// is health esp enabled for this player.
	bool health_esp = ( enemy && g_menu.main.players.health_enemy.get( 0 ) ) || ( !enemy && g_menu.main.players.health_friendly.get( 0 ) );

	if ( health_esp )
	{
		int y = box.second.y + 1;
		int h = box.second.h - 2;

		// get hp bar height.
		int fill = ( int )std::round( hp * h / 100.f );

		const int rgba[ ] = { math::Lerp( hp / 100.f, 255, 164 ),
			math::Lerp( hp / 100.f, 0, 218 ),
			math::Lerp( hp / 100.f, 0, 48 ),
			255 };

		Color health_color = { rgba[ 0 ], rgba[ 1 ], rgba[ 2 ], rgba[ 3 ] };

		if ( enemy && g_menu.main.players.health_enemy.get( 1 ) )
			health_color = g_menu.main.players.health_enemy_color.get( );

		if ( !enemy && g_menu.main.players.health_friendly.get( 1 ) )
			health_color = g_menu.main.players.health_friendly_color.get( );

		health_color.a( ) = alpha;

		// render background.
		render::rect_filled( box.second.x - 6, y - 1, 4, h + 2, { 10, 10, 10, low_alpha } );

		// render actual bar.
		render::rect( box.second.x - 5, y + h - fill, 2, fill, health_color );

		// if hp is below max, draw a string.
		if ( hp < 100 )
			GetEspFont( true ).string( box.second.x - 5, y + ( h - fill ) - 5, { 255, 255, 255, low_alpha }, std::to_string( ( int )hp ), render::ALIGN_CENTER );
	}

	// draw flags.
	{
		static std::vector< std::pair< std::string, Color > > flags;
		static bool whitelisted;

		flags.clear( );
		whitelisted = false;
		for ( const int& wl : g_aimbot.m_whitelisted_ids ) {
			if ( wl == index ) {
				whitelisted = true;
				break;
			}
		}

		const auto& items = enemy ? g_menu.main.players.flags_enemy.GetActiveIndices( ) : g_menu.main.players.flags_friendly.GetActiveIndices( );
		const int& cheat = g_cl.GetCheat( index, false );
		const Color& lethal_color = player->m_ArmorValue( ) > 0 ? Color{ 255, 255, 0, low_alpha } : Color{ 150, 200, 60, low_alpha };

		bool lethal = ( g_cl.m_local && g_cl.m_local->alive( ) && g_cl.m_weapon && g_cl.m_weapon_info ) ? ( player->ScaleDamage( g_cl.m_weapon_info->m_armor_ratio, HITGROUP_STOMACH, g_cl.m_weapon_info->m_damage ) >= hp ) : false;

		if ( cheat > Cheats::UNK && enemy )
		{
			static std::string name;
			switch ( cheat )
			{
			case Cheats::RAX:
				name = XOR( "RAX" );
				break;
			case Cheats::FAMILY:
				name = XOR( "FAMILY" );
				break;
			case Cheats::MONEYBOT:
				name = XOR( "MONEYBOT" );
				break;
			default:
				if ( !name.empty( ) )
					name.clear( );
				break;
			}

			if ( !name.empty( ) && g_menu.main.players.flags_enemy.get( 6 + cheat ) )
				flags.push_back( { name, whitelisted ? Color { 150, 200, 60, low_alpha } : Color { 255, 255, 255, low_alpha } } );
		}

		for ( auto it = items.begin( ); it != items.end( ); ++it )
		{

			// money.
			if ( *it == 0 )
				flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 150, 200, 60, low_alpha } } );

			// armor.
			if ( *it == 1 )
			{
				// helmet and kevlar.
				if ( player->m_bHasHelmet( ) && player->m_ArmorValue( ) > 0 )
					flags.push_back( { XOR( "HK" ), { 255, 255, 255, low_alpha } } );

				// only helmet.
				else if ( player->m_bHasHelmet( ) )
					flags.push_back( { XOR( "H" ), { 255, 255, 255, low_alpha } } );

				// only kevlar.
				else if ( player->m_ArmorValue( ) > 0 )
					flags.push_back( { XOR( "K" ), { 255, 255, 255, low_alpha } } );
			}

			// scoped.
			if ( *it == 2 && player->m_bIsScoped( ) )
				flags.push_back( { XOR( "ZOOM" ), { 60, 180, 225, low_alpha } } );

			// flashed.
			if ( *it == 3 && player->m_flFlashBangTime( ) > 0.f )
				flags.push_back( { XOR( "FLASHED" ), { 255, 255, 0, low_alpha } } );

			// reload.
			if ( *it == 4 )
			{
				// get ptr to layer 1.
				C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

				// check if reload animation is going on.
				if ( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 /* ACT_CSGO_RELOAD */ )
					flags.push_back( { XOR( "RELOAD" ), { 60, 180, 225, low_alpha } } );
			}

			// bomb.
			if ( *it == 5 && player->HasC4( ) )
				flags.push_back( { XOR( "BOMB" ), { 255, 0, 0, low_alpha } } );

			if ( *it == 6 && enemy && lethal )
				flags.push_back( { XOR( "LETHAL" ), lethal_color } );

			if ( *it == 10 )
				flags.push_back( { ResolveConfidence( index ).c_str( ), { 255, 255, 255, low_alpha } } );
		}

		// iterate flags.
		for ( size_t i{ }; i < flags.size( ); ++i )
		{
			// get flag job (pair).
			const auto& f = flags[ i ];

			if ( f.first.empty( ) )
				continue;

			int offset = i * GetEspFont( true ).m_size.m_height;

			auto size = GetEspFont( true ).size( f.first );

			// draw flag.
			GetEspFont( true ).string( box.second.x + box.second.w + 2, box.second.y + offset, f.second, f.first );
		}
	}

	// draw bottom bars.
	{
		int  offset{ 0 };

		// draw lby update bar.
		if ( enemy && g_menu.main.players.lby_update.get( ) )
		{
			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];

			// make sure everything is valid.
			if ( data && data->m_moved && data->m_records.size( ) )
			{
				// grab lag record.
				LagRecord* current = &data->m_records.front( );

				if ( current )
				{
					if ( !( current->m_anim_velocity.length_2d( ) > 0.1 ) && data->m_body_index <= 3 )
					{
						// calculate box width
						float cycle = std::clamp<float>( data->m_body_update - current->m_anim_time, 0.f, 1.0f );
						float width = ( box.second.w * cycle ) / 1.1f;

						if ( width > 0.f )
						{
							// draw.
							render::rect_filled( box.second.x, box.second.y + box.second.h + 2, box.second.w, 4, { 10, 10, 10, low_alpha } );

							Color clr = g_menu.main.players.lby_update_color.get( );
							clr.a( ) = alpha;
							render::rect( box.second.x + 1, box.second.y + box.second.h + 3, width, 2, clr );

							// move down the offset to make room for the next bar.
							offset += 5;
						}
					}
				}
			}
		}

		// draw weapon.
		if ( ( enemy && g_menu.main.players.weapon_enemy.get( ) ) || ( !enemy && g_menu.main.players.weapon_friendly.get( ) ) )
		{
			const int& weapon_mode = enemy ? g_menu.main.players.weapon_mode_enemy.get( ) : g_menu.main.players.weapon_mode_friendly.get( );

			Weapon* weapon = player->GetActiveWeapon( );
			if ( weapon )
			{
				WeaponInfo* data = weapon->GetWpnData( );
				if ( data )
				{
					int bar;
					float scale;

					// the maxclip1 in the weaponinfo
					const int& max = data->m_max_clip1;
					const int& current = weapon->m_iClip1( );

					C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

					// set reload state.
					const bool reload = ( layer1->m_weight != 0.f ) && ( player->GetSequenceActivity( layer1->m_sequence ) == 967 );

					// ammo bar.
					if ( max != -1 && g_menu.main.players.ammo.get( ) )
					{
						// check for reload.
						if ( reload )
							scale = layer1->m_cycle;

						// not reloading.
						// make the division of 2 ints produce a float instead of another int.
						else
							scale = ( float )current / max;

						// relative to bar.
						bar = ( int )std::round( ( box.second.w - 2 ) * scale );

						// draw.
						render::rect_filled( box.second.x, box.second.y + box.second.h + 2 + offset, box.second.w, 4, { 10, 10, 10, low_alpha } );

						Color clr = g_menu.main.players.ammo_color.get( );
						clr.a( ) = alpha;
						render::rect( box.second.x + 1, box.second.y + box.second.h + 3 + offset, bar, 2, clr );

						// less then a 5th of the bullets left.
						if ( current <= ( int )std::round( max / 5 ) && !reload )
							GetEspFont( true ).string( box.second.x + bar, box.second.y + box.second.h + offset, { 255, 255, 255, low_alpha }, std::to_string( current ), render::ALIGN_CENTER );

						offset += 6;
					}

					// text.
					if ( weapon_mode == 0 )
					{
						// construct std::string instance of localized weapon name.
						std::string name{ weapon->GetLocalizedName( ) };

						// smallfonts needs upper case.
						std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

						auto size = GetEspFont( true ).size( name );

						GetEspFont( true ).string( box.second.x + box.second.w / 2, box.second.y + box.second.h + offset, { 255, 255, 255, low_alpha }, name, render::ALIGN_CENTER );
					}

					// icons.
					else if ( weapon_mode == 1 )
					{
						// icons are super fat..
						// move them back up.
						offset -= 5;

						const std::string& icon = tfm::format( XOR( "%c" ), m_weapon_icons[ weapon->m_iItemDefinitionIndex( ) ] );
						render::cs.string( box.second.x + box.second.w / 2, box.second.y + box.second.h + offset, { 255, 255, 255, low_alpha }, icon, render::ALIGN_CENTER );
					}
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4( )
{
	bool        mode_2d, mode_3d, is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = [ ]( float damage, int armor_value ) {
		float new_damage, armor;

		if ( armor_value > 0 )
		{
			new_damage = damage * 0.5f;
			armor = ( damage - new_damage ) * 0.5f;

			if ( armor > ( float )armor_value )
			{
				armor = ( float )armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max( 0, ( int )std::floor( damage ) );
		};

	// store menu vars for later.
	mode_2d = g_menu.main.visuals.planted_c4.get( 0 );
	mode_3d = g_menu.main.visuals.planted_c4.get( 1 );
	if ( !mode_2d && !mode_3d )
		return;

	// bomb not currently active, do nothing.
	if ( !m_c4_planted )
		return;

	// calculate bomb damage.
	// references:
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//     https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter( );
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length( );

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp( ( dist * dist ) / ( ( m_planted_c4_radius_scaled * -2.f ) * m_planted_c4_radius_scaled ) );

		// now finally, scale the damage based on our armor (if we have any).
		final_damage = scale_damage( range_damage, g_cl.m_local->m_ArmorValue( ) );
	}

	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	time_str = tfm::format( XOR( "%.2f" ), explode_time_diff );
	damage_str = tfm::format( XOR( "%i" ), final_damage );

	// get damage color.
	damage_color = ( final_damage < g_cl.m_local->m_iHealth( ) ) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen( m_planted_c4_explosion_origin, screen_pos );

	// 'on screen (2D)'.
	if ( mode_2d )
	{
		// g_cl.m_height - 80 - ( 30 * i )
		// 80 - ( 30 * 2 ) = 20

		// render::menu_shade.string( 60, g_cl.m_height - 20 - ( render::hud_size.m_height / 2 ), 0xff0000ff, "", render::hud );

		// todo - dex; move this next to indicators?

		if ( explode_time_diff > 0.f )
			GetEspFont( false ).string( 2, 65, colors::white, time_str, render::ALIGN_LEFT );

		if ( g_cl.m_local->alive( ) )
			GetEspFont( false ).string( 2, 65 + GetEspFont( false ).m_size.m_height, damage_color, damage_str, render::ALIGN_LEFT );
	}

	// 'on bomb (3D)'.
	if ( mode_3d && is_visible )
	{
		if ( explode_time_diff > 0.f )
			GetEspFont( true ).string( screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER );

		// only render damage string if we're alive.
		if ( g_cl.m_local->alive( ) )
			GetEspFont( true ).string( screen_pos.x, ( int )screen_pos.y + GetEspFont( true ).m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER );
	}
}

bool Visuals::GetPlayerBoxRect( Player* player, Rect& box )
{
	if ( !player )
		return false;

	vec3_t min, max, out_vec;
	float left, bottom, right, top;
	matrix3x4_t& tran_frame = player->m_rgflCoordinateFrame( );

	// get hitbox bounds.
	min = player->m_vecMins( );
	max = player->m_vecMaxs( );

	vec2_t screen_boxes[ 8 ];

	// transform mins and maxes to points. 
	vec3_t points[ ] =
	{
		{ min.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, max.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z }
	};

	// transform points to 3-dimensional space.
	for ( int i = 0; i <= 7; i++ )
	{
		math::VectorTransform( points[ i ], tran_frame, out_vec );
		if ( !render::WorldToScreen( out_vec, screen_boxes[ i ] ) )
			return false;
	}

	// generate an array to clamp later.
	vec2_t box_array[ ] = {
		screen_boxes[ 3 ],
		screen_boxes[ 5 ],
		screen_boxes[ 0 ],
		screen_boxes[ 4 ],
		screen_boxes[ 2 ],
		screen_boxes[ 1 ],
		screen_boxes[ 6 ],
		screen_boxes[ 7 ]
	};

	// state the position and size of the box.
	left = screen_boxes[ 3 ].x,
		bottom = screen_boxes[ 3 ].y,
		right = screen_boxes[ 3 ].x,
		top = screen_boxes[ 3 ].y;

	// clamp the box sizes.
	for ( int i = 0; i <= 7; i++ )
	{
		if ( left > box_array[ i ].x )
			left = box_array[ i ].x;

		if ( bottom < box_array[ i ].y )
			bottom = box_array[ i ].y;

		if ( right < box_array[ i ].x )
			right = box_array[ i ].x;

		if ( top > box_array[ i ].y )
			top = box_array[ i ].y;
	}

	// state the box bounds.
	box.x = left;
	box.y = top;
	box.w = right - left;
	box.h = ( bottom - top );

	return true;

	//vec3_t origin, mins, maxs;
	//vec2_t bottom, top;

	//// get interpolated origin.
	//origin = player->GetAbsOrigin();

	//// get hitbox bounds.
	//player->ComputeHitboxSurroundingBox(&mins, &maxs);

	//// correct x and y coordinates.
	//mins = { origin.x, origin.y, mins.z };
	//maxs = { origin.x, origin.y, maxs.z + 8.f };

	//if (!render::WorldToScreen(mins, bottom) || !render::WorldToScreen(maxs, top))
	//	return false;

	//box.h = bottom.y - top.y;
	//box.w = box.h / 2.f;
	//box.x = bottom.x - (box.w / 2.f);
	//box.y = bottom.y - box.h;

	//return true;
}

void Visuals::DrawHistorySkeleton( Player* player, float opacity )
{
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if ( !g_aimbot.IsValidTarget( player ) )
		return;

	// get player's model.
	const model_t* model = player->GetModel( );
	if ( !model )
		return;

	// get studio model.
	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
	if ( !data )
		return;

	if ( data->m_records.size( ) <= 1 )
		return;

	LagRecord* record = nullptr;
	for ( int i = 1; i < data->m_records.size( ); i++ )
	{
		LagRecord* current = &data->m_records[ i ];
		if ( !current )
			continue;

		if ( current->m_record_flags & RecordFlags::BREAKINGLC )
			break;

		if ( current->m_record_flags & RecordFlags::SHIFTING )
			continue;

		if ( !g_lagcomp.ValidRecord( current->m_sim_time ) )
			continue;

		record = current;
	}

	if ( !record )
		return;

	if ( ( player->GetAbsOrigin( ) - record->m_origin ).length( ) <= 10.f )
		return;

	Color clr = g_menu.main.players.skeleton_enemy.get( );
	clr.a( ) *= opacity;

	for ( int i{ }; i < hdr->m_num_bones; ++i )
	{
		// get bone.
		mstudiobone_t* bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		record->m_bones->get_bone( bone_pos, i );
		record->m_bones->get_bone( parent_pos, parent );

		// world to screen both the bone parent bone then draw.
		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::DrawSkeleton( Player* player, float opacity )
{
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	const model_t* model = player->GetModel( );
	if ( !model )
		return;

	// get studio model.
	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	Color clr = g_menu.main.players.skeleton_enemy.get( );
	clr.a( ) *= opacity;

	CBoneCache& cache = player->m_BoneCache( );
	if ( !cache.m_pCachedBones )
		return;

	for ( int i{ }; i < hdr->m_num_bones; ++i )
	{
		// get bone.
		mstudiobone_t* bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		cache.m_pCachedBones->get_bone( bone_pos, i );
		cache.m_pCachedBones->get_bone( parent_pos, parent );

		// world to screen both the bone parent bone then draw.
		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::RenderGlow( )
{
	int render;
	Color   color;
	Player* player;

	if ( !g_cl.m_local )
		return;

	if ( !g_csgo.m_glow->m_object_definitions.Count( ) )
		return;

	for ( int i{ }; i < g_csgo.m_glow->m_object_definitions.Count( ); ++i )
	{
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[ i ];

		// skip non-players.
		if ( !obj->m_entity || !obj->m_entity->IsPlayer( ) )
			continue;

		// get player ptr.
		player = obj->m_entity->as< Player* >( );

		if ( player->m_bIsLocalPlayer( ) )
			continue;

		// get reference to array variable.
		float& opacity = m_opacities[ player->index( ) - 1 ];

		bool enemy = player->enemy( g_cl.m_local );

		if ( enemy && ( !g_menu.main.players.glow_enemy_enable.get( 0 ) && !g_menu.main.players.glow_enemy_enable.get( 1 ) ) )
			continue;

		if ( !enemy && ( !g_menu.main.players.glow_friendly_enable.get( 0 ) && !g_menu.main.players.glow_friendly_enable.get( 1 ) ) )
			continue;

		// enemy color
		if ( enemy )
		{
			color = g_menu.main.players.glow_enemy.get( );
			render = g_menu.main.players.glow_enemy_enable.get( 1 ) ? 2 : 0;
		}
		// friendly color
		else
		{
			color = g_menu.main.players.glow_friendly.get( );
			render = g_menu.main.players.glow_friendly_enable.get( 1 ) ? 2 : 0;
		}

		static IMaterial* stencil_mat;

		if ( !stencil_mat )
			stencil_mat = g_csgo.m_material_system->FindMaterial( XOR( "dev/glow_edge_highlight" ), "Other textures" );

		if ( stencil_mat && !stencil_mat->GetFlag( MATERIAL_VAR_IGNOREZ ) )
			stencil_mat->SetFlag( MATERIAL_VAR_IGNOREZ, true );

		obj->m_render_style = render;
		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_full_bloom_stencil_test_value = 999;
		obj->m_color = { ( float )color.r( ) / 255.f, ( float )color.g( ) / 255.f, ( float )color.b( ) / 255.f };
		obj->m_alpha = opacity * ( color.a( ) / 255.f );
	}
}

void Visuals::DrawHitboxMatrix( LagRecord* record, Color col, float time )
{
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel( );
	if ( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	set = hdr->GetHitboxSet( record->m_player->m_nHitboxSet( ) );
	if ( !set )
		return;

	for ( int i{ }; i < set->m_hitboxes; ++i )
	{
		bbox = set->GetHitbox( i );
		if ( !bbox )
			continue;

		// bbox.
		if ( bbox->m_radius <= 0.f )
		{
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], rot_matrix, matrix );

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles( matrix, bbox_angle );

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin( );

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r( ), col.g( ), col.b( ), 0, time );
		}

		// capsule.
		else
		{
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix( bbox->m_angle, matrix );

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], matrix, matrix );

			// get world positions from new matrix.
			math::VectorTransform( bbox->m_mins, matrix, mins );
			math::VectorTransform( bbox->m_maxs, matrix, maxs );

			g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r( ), col.g( ), col.b( ), col.a( ), time, 0, true );
		}
	}
}

void Visuals::DrawBeams( )
{
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if ( !g_cl.m_local )
		return;

	if ( !g_menu.main.visuals.impact_beams.get( ) )
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
	{
		if ( !vis_impacts->empty( ) )
			vis_impacts->clear( );
	}

	else
	{
		impact_count = vis_impacts->size( );
		if ( !impact_count )
			return;

		curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

		for ( size_t i{ impact_count }; i-- > 0; )
		{
			auto impact = &vis_impacts->operator[ ]( i );
			if ( !impact )
				continue;

			// impact is too old, erase it.
			if ( std::abs( curtime - game::TICKS_TO_TIME( impact->m_tickbase ) ) > g_menu.main.visuals.impact_beams_time.get( ) )
			{
				vis_impacts->erase( vis_impacts->begin( ) + i );

				continue;
			}

			// already rendering this impact, skip over it.
			if ( impact->m_ignore )
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if ( i == ( impact_count - 1 ) )
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if ( ( i + 1 ) < impact_count && impact->m_tickbase != vis_impacts->operator[ ]( i + 1 ).m_tickbase )
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if ( is_final_impact )
			{
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = ( impact->m_impact_pos - start ).normalized( );
				dist = ( impact->m_impact_pos - start ).length( );

				end = start + ( dir * dist );

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( XOR( "sprites/purplelaser1.vmt" ) );
				beam_info.m_pszModelName = XOR( "sprites/purplelaser1.vmt" );
				beam_info.m_flHaloScale = 0.f;
				beam_info.m_flLife = g_menu.main.visuals.impact_beams_time.get( );
				beam_info.m_flWidth = 2.f;
				beam_info.m_flEndWidth = 2.f;
				beam_info.m_flFadeLength = 0.f;
				beam_info.m_flAmplitude = 0.f;   // beam 'jitter'.
				beam_info.m_flBrightness = 255.f;
				beam_info.m_flSpeed = 0.5f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
				beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
				beam_info.m_nFlags = 0;

				if ( !impact->m_hit_player )
				{
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_color.get( ).b( );
				}

				else
				{
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_hurt_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_hurt_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_hurt_color.get( ).b( );
				}

				// attempt to render the beam.
				beam = game::CreateGenericBeam( beam_info );
				if ( beam )
				{
					g_csgo.m_beams->DrawBeam( beam );

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::DebugAimbotPoints( Player* player )
{
	if ( !g_menu.main.aimbot.debug_aim_points.get( ) )
		return;

	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	if ( !player->enemy( g_cl.m_local ) )
		return;

	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
	if ( !data || data->m_records.empty( ) )
		return;

	LagRecord* record = &data->m_records.front( );
	if ( !record || !record->m_setup )
		return;

	const model_t* model = player->GetModel( );
	if ( !model )
		return;

	const studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	const mstudiohitboxset_t* set = hdr->GetHitboxSet( player->m_nHitboxSet( ) );
	if ( !set )
		return;

	static vec2_t w2s;

	for ( auto hitbox : g_aimbot.m_hitboxes )
	{
		static std::vector<Aimbot::pt_data_t> points;
		points.clear( );

		g_aimbot.CreatePoints( points, hitbox, record );

		if ( points.empty( ) )
			continue;

		for ( auto& point : points )
		{
			if ( !render::WorldToScreen( point.m_point, w2s ) )
				continue;

			render::rect_filled( w2s.x - 2, w2s.y - 2, 4, 4, hitbox == HITBOX_HEAD ? colors::red : colors::light_blue );
		}
	}
}