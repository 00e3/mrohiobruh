#include "includes.h"

void Hooks::DrawModelExecute( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {

	static bool last_update = false;
	static IMaterial* blurfilterx_nohdr = nullptr;
	static IMaterial* blurfiltery_nohdr = nullptr;
	static IMaterial* scope_bluroverlay = nullptr;

	if ( last_update != g_menu.main.visuals.removals.get(5) )
	{
		if ( !blurfilterx_nohdr )
			blurfilterx_nohdr = g_csgo.m_material_system->FindMaterial( ( "dev/blurfilterx_nohdr" ), nullptr );

		if ( !blurfiltery_nohdr )
			blurfiltery_nohdr = g_csgo.m_material_system->FindMaterial( ( "dev/blurfiltery_nohdr" ), nullptr );

		if ( !scope_bluroverlay )
			scope_bluroverlay = g_csgo.m_material_system->FindMaterial( ( "dev/scope_bluroverlay" ), nullptr );

		if ( blurfilterx_nohdr )
			blurfilterx_nohdr->SetFlag( MATERIAL_VAR_NO_DRAW, g_menu.main.visuals.removals.get(5) );

		if ( blurfiltery_nohdr )
			blurfiltery_nohdr->SetFlag( MATERIAL_VAR_NO_DRAW, g_menu.main.visuals.removals.get(5) );

		if ( scope_bluroverlay )
			scope_bluroverlay->SetFlag( MATERIAL_VAR_NO_DRAW, g_menu.main.visuals.removals.get(5) );

		last_update = g_menu.main.visuals.removals.get(5);
	}

	// do chams.
	if ( g_chams.DrawModel( ctx, state, info, bone ) ) {
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( this, ctx, state, info, bone );
	}

	// disable material force for next call.
	//g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
}