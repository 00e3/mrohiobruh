#include "includes.h"

Chams g_chams{ };;

Chams::model_type_t Chams::GetModelType( const ModelRenderInfo_t& info ) {
	// model name.
	//const char* mdl = info.m_model->m_name;

	std::string mdl{ info.m_model->m_name };

	//static auto int_from_chars = [ mdl ]( size_t index ) {
	//	return *( int* )( mdl + index );
	//};

	// little endian.
	//if( int_from_chars( 7 ) == 'paew' ) { // weap
	//	if( int_from_chars( 15 ) == 'om_v' && int_from_chars( 19 ) == 'sled' )
	//		return model_type_t::arms;
	//
	//	if( mdl[ 15 ] == 'v' )
	//		return model_type_t::view_weapon;
	//}

	//else if( int_from_chars( 7 ) == 'yalp' ) // play
	//	return model_type_t::player;

	if ( mdl.find( XOR( "player" ) ) != std::string::npos && info.m_index >= 1 && info.m_index <= 64 )
		return model_type_t::player;

	return model_type_t::invalid;
}

bool Chams::IsInViewPlane( const vec3_t& world ) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix( );

	w = matrix[ 3 ][ 0 ] * world.x + matrix[ 3 ][ 1 ] * world.y + matrix[ 3 ][ 2 ] * world.z + matrix[ 3 ][ 3 ];

	return w > 0.001f;
}

void Chams::SetColor( Color col, IMaterial* mat ) {
	if ( mat ) {
		mat->ColorModulate( col );
		auto pVar = mat->GetVar( "$envmaptint" );
		if ( pVar )
			( *( void( __thiscall** )( int, float, float, float ) )( *( DWORD* )pVar + 44 ) )( ( uintptr_t )pVar, col.r( ) / 255.f, col.g( ) / 255.f, col.b( ) / 255.f );
	}
	else
		g_csgo.m_render_view->SetColorModulation( col );
}

void Chams::SetAlpha( float alpha, IMaterial* mat ) {
	if ( mat )
		mat->AlphaModulate( alpha );

	else
		g_csgo.m_render_view->SetBlend( alpha );
}

void Chams::SetupMaterial( IMaterial* mat, Color col, bool z_flag ) {
	SetColor( col, mat );

	// mat->SetFlag( MATERIAL_VAR_HALFLAMBERT, flags );
	mat->SetFlag( MATERIAL_VAR_ZNEARER, z_flag );
	mat->SetFlag( MATERIAL_VAR_NOFOG, z_flag );
	mat->SetFlag( MATERIAL_VAR_IGNOREZ, z_flag );

	g_csgo.m_studio_render->ForcedMaterialOverride( mat );
}

void Chams::init( ) {
	// find stupid materials.
	std::ofstream( "csgo\\materials\\sap_shine.vmt" ) << R"#("VertexLitGeneric" 
	{
					"$basetexture"				"vgui/white_additive"
					"$ignorez"					"0"
					"$phong"					"1"
					"$BasemapAlphaPhongMask"    "1"
					"$phongexponent"			"15"
					"$normalmapalphaenvmask"	"1"
					"$envmap"					"env_cubemap"
					"$envmaptint"				"[0.0 0.0 0.0]"
					"$phongboost"				"[0.6 0.6 0.6]"
					"phongfresnelranges"		"[0.5 0.5 1.0]"
					"$nofog"					"1"
					"$model"					"1"
					"$nocull"					"0"
					"$selfillum"				"1"
					"$halflambert"				"1"
					"$znearer"					"0"
					"$flat"						"0"	
					"$rimlight"					"1"
					"$rimlightexponent"			"2"
					"$rimlightboost"			"0"
		})#";

	std::ofstream( "csgo/materials/sap_glowoverlay.vmt" ) << R"#("VertexLitGeneric" 
		{
		"$additive" "1"
		"$envmap" "models/effects/cube_white"
		"$envmaptint" "[1 1 1]"
		"$envmapfresnel" "1"
		"$envmapfresnelminmaxexp" "[0 1 2]"
		"$alpha" "1"
		})#";

	metallic = g_csgo.m_material_system->FindMaterial( XOR( "sap_shine" ), XOR( "Model textures" ) );
	metallic->IncrementReferenceCount( );

	overlay = g_csgo.m_material_system->FindMaterial( XOR( "sap_glowoverlay" ), nullptr );
	overlay->IncrementReferenceCount( );

	debugambientcube = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugambientcube" ), XOR( "Model textures" ) );
	debugambientcube->IncrementReferenceCount( );

	debugdrawflat = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugdrawflat" ), XOR( "Model textures" ) );
	debugdrawflat->IncrementReferenceCount( );
}

bool Chams::OverridePlayer( int index ) {
	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !player || !player->IsPlayer( ) || !player->alive( ) || player->dormant( ) )
		return false;

	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	if ( player == g_cl.m_local )
		return true;

	if ( enemy )
		return g_menu.main.players.chams_enemy.get( 0 ) || g_menu.main.players.glow_enemy_enable.get( 2 );

	return g_menu.main.players.chams_friendly.get( 0 ) || g_menu.main.players.glow_friendly_enable.get( 2 );
}

bool Chams::GenerateLerpedMatrix( int index, BoneArray* out ) {
	AimPlayer* data = &g_aimbot.m_players[ index - 1 ];
	if ( !data || !data->m_player )
		return false;

	if ( data->m_records.size( ) < 2 )
		return false;

	const vec3_t& abs_origin = data->m_player->GetAbsOrigin( );

	for ( int i = data->m_records.size( ) - 1; i > 1; i-- ) {
		LagRecord* next_record = &data->m_records.at( i - 1 );
		LagRecord* record = &data->m_records.at( i );

		if ( !g_lagcomp.ValidRecord( record->m_sim_time ) )
			continue;

		float flDelta = ( g_csgo.m_globals->m_curtime - next_record->m_lerp_time ) / ( next_record->m_sim_time - record->m_sim_time );

		if ( flDelta < 0.f || flDelta > 1.f )
			next_record->m_lerp_time = g_csgo.m_globals->m_curtime;

		flDelta = ( g_csgo.m_globals->m_curtime - next_record->m_lerp_time ) / ( next_record->m_sim_time - record->m_sim_time );

		flDelta = std::clamp( flDelta, 0.f, 1.f );

		const vec3_t& interp_pos = math::Lerp( flDelta, record->m_origin, next_record->m_origin );

		if ( ( abs_origin - interp_pos ).length_2d( ) <= 10.f )
			continue;

		std::memcpy( out, record->m_bones, sizeof( BoneArray ) * 128 );
		for ( int i = 0; i < 128; i++ ) {
			vec3_t pos1 = record->m_bones[ i ].GetOrigin( ) - record->m_origin;

			out[ i ].SetOrigin( pos1 + interp_pos );
		}

		return true;
	}

	return false;
}

void Chams::RenderHistoryChams( int index ) {
	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !player )
		return;

	if ( !g_aimbot.IsValidTarget( player ) )
		return;

	AimPlayer* data = &g_aimbot.m_players[ index - 1 ];
	if ( !data || data->m_records.empty( ) )
		return;

	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );
	if ( !enemy )
		return;

	// was the matrix properly setup?
	static BoneArray backup[ 128 ];
	static BoneArray arr[ 128 ];

	if ( GenerateLerpedMatrix( index, arr ) ) {
		player->GetBones( backup );

		// override blend.
		SetAlpha( g_menu.main.players.chams_enemy_history_col.get( ).a( ) / 255.f );

		// set material and color.
		SetupMaterial( debugdrawflat, g_menu.main.players.chams_enemy_history_col.get( ), true );

		player->SetBones( arr );

		// manually draw the model.
		player->DrawModel( );

		player->SetBones( backup );
	}

	//player->GetBones( backup );

	//LagRecord& current = data->m_records[ 0 ];
	//for ( int i = 0; i < current.m_safe_matrice_count; i++ ) {
	//	// override blend.
	//	SetAlpha( g_menu.main.players.chams_enemy_history_col.get( ).a( ) / 255.f );

	//	// set material and color.
	//	SetupMaterial( debugdrawflat, g_menu.main.players.chams_enemy_history_col.get( ), true );

	//	player->SetBones( current.m_safe_bones[ i ] );

	//	// manually draw the model.
	//	player->DrawModel( );
	//}

	//player->SetBones( backup );
}

bool Chams::DrawModel( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {
	// store and validate model type.
	model_type_t type = GetModelType( info );
	if ( type == model_type_t::invalid )
		return true;

	// is a valid player.
	if ( type == model_type_t::player ) {
		// do not cancel out our own calls from SceneEnd
		// also do not cancel out calls from the glow.
		if ( !m_running && !g_csgo.m_studio_render->m_pForcedMaterial && OverridePlayer( info.m_index ) )
			return false;
	}

	return true;
}

void Chams::SceneEnd( ) {
	// store and sort ents by distance.
	if ( SortPlayers( ) ) {
		// iterate each player and render them.
		for ( const auto& p : m_players )
			RenderPlayer( p );
	}

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );
}

void Chams::RenderPlayer( Player* player ) {
	// prevent recruisive model cancelation.
	m_running = true;

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );

	IMaterial* material = debugambientcube;

	// this is the local player.
	// we always draw the local player manually in drawmodel.
	if ( player->m_bIsLocalPlayer( ) ) {
		float multiplier = 1.f;
		if ( g_menu.main.players.chams_local_scope.get( ) && player->m_bIsScoped( ) )
			multiplier = 0.5f;

		if ( g_menu.main.players.chams_fake.get( 0 ) && g_cl.m_fake_bones_setup ) {
			// set material and color.
			if ( g_menu.main.players.chams_fake.get( 1 ) )
				material = debugdrawflat;
			else if ( g_menu.main.players.chams_fake.get( 2 ) )
				material = metallic;

			SetAlpha( ( g_menu.main.players.chams_fake_col.get( ).a( ) / 255.f ) * multiplier );

			SetupMaterial( material, g_menu.main.players.chams_fake_col.get( ), false );

			BoneArray backup[ 128 ];
			player->GetBones( backup );

			player->SetBones( g_cl.m_fake_bones );

			player->DrawModel( );

			player->SetBones( backup );

			if ( !g_menu.main.players.chams_local.get( 0 ) ) {
				g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
				g_csgo.m_render_view->SetColorModulation( colors::white );
				g_csgo.m_render_view->SetBlend( 1.f );
			}
		}

		if ( g_menu.main.players.chams_local.get( 0 ) ) {
			// set material and color.
			if ( g_menu.main.players.chams_local.get( 1 ) )
				material = debugdrawflat;
			else if ( g_menu.main.players.chams_local.get( 2 ) )
				material = metallic;

			SetAlpha( ( g_menu.main.players.chams_local_col.get( ).a( ) / 255.f ) * multiplier );

			SetupMaterial( material, g_menu.main.players.chams_local_col.get( ), false );
		}
		else
			SetAlpha( multiplier );

		// manually draw the model.
		player->DrawModel( );
		m_running = false;
		return;
	}

	// check if is an enemy.
	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	bool rendered = false;

	if ( enemy ) {
		if ( g_menu.main.players.chams_enemy_history.get( ) )
			RenderHistoryChams( player->index( ) );

		if ( g_menu.main.players.chams_enemy.get( 0 ) ) {
			if ( g_menu.main.players.chams_enemy.get( 2 ) )
				material = debugdrawflat;
			else if ( g_menu.main.players.chams_enemy.get( 3 ) )
				material = metallic;

			if ( g_menu.main.players.chams_enemy.get( 1 ) ) {

				SetAlpha( g_menu.main.players.chams_enemy_invis.get( ).a( ) / 255.f );
				SetupMaterial( material, g_menu.main.players.chams_enemy_invis.get( ), true );

				player->DrawModel( );
			}

			SetAlpha( g_menu.main.players.chams_enemy_vis.get( ).a( ) / 255.f );
			SetupMaterial( material, g_menu.main.players.chams_enemy_vis.get( ), false );

			player->DrawModel( );

			rendered = true;
		}

		if ( g_menu.main.players.glow_enemy_enable.get( 2 ) ) {
			if ( !rendered )
				player->DrawModel( );

			SetAlpha( g_menu.main.players.glow_enemy.get( ).a( ) / 255.f );
			SetupMaterial( overlay, g_menu.main.players.glow_enemy.get( ), true );

			player->DrawModel( );
		}
	}

	else {
		if ( g_menu.main.players.chams_friendly.get( 0 ) ) {
			if ( g_menu.main.players.chams_friendly.get( 2 ) )
				material = debugdrawflat;
			else if ( g_menu.main.players.chams_friendly.get( 3 ) )
				material = metallic;

			if ( g_menu.main.players.chams_friendly.get( 1 ) ) {

				SetAlpha( g_menu.main.players.chams_enemy_invis.get( ).a( ) / 255.f );
				SetupMaterial( material, g_menu.main.players.chams_friendly_invis.get( ), true );

				player->DrawModel( );
			}

			SetAlpha( g_menu.main.players.chams_enemy_vis.get( ).a( ) / 255.f );
			SetupMaterial( material, g_menu.main.players.chams_friendly_vis.get( ), false );
			player->DrawModel( );

			rendered = true;
		}

		if ( g_menu.main.players.glow_friendly_enable.get( 2 ) ) {
			if ( !rendered )
				player->DrawModel( );

			SetAlpha( g_menu.main.players.glow_friendly.get( ).a( ) / 255.f );
			SetupMaterial( overlay, g_menu.main.players.glow_friendly.get( ), true );

			player->DrawModel( );
		}
	}

	m_running = false;
}

bool Chams::SortPlayers( ) {
	// lambda-callback for std::sort.
	// to sort the players based on distance to the local-player.
	static auto distance_predicate = [ ]( Entity* a, Entity* b ) {
		vec3_t local = g_cl.m_local->GetAbsOrigin( );

		// note - dex; using squared length to save out on sqrt calls, we don't care about it anyway.
		float len1 = ( a->GetAbsOrigin( ) - local ).length_sqr( );
		float len2 = ( b->GetAbsOrigin( ) - local ).length_sqr( );

		return len1 < len2;
		};

	// reset player container.
	m_players.clear( );

	// find all players that should be rendered.
	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		// get player ptr by idx.
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		// validate.
		if ( !player || !player->IsPlayer( ) || !player->alive( ) || player->dormant( ) )
			continue;

		// do not draw players occluded by view plane.
		if ( !IsInViewPlane( player->WorldSpaceCenter( ) ) )
			continue;

		// this player was not skipped to draw later.
		// so do not add it to our render list.
		if ( !OverridePlayer( i ) )
			continue;

		m_players.push_back( player );
	}

	// any players?
	if ( m_players.empty( ) )
		return false;

	// sorting fixes the weird weapon on back flickers.
	// and all the other problems regarding Z-layering in this shit game.
	std::sort( m_players.begin( ), m_players.end( ), distance_predicate );

	return true;
}