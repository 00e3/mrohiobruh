#include "includes.h"

void Hooks::OnRenderStart( ) {
	// call og.
	g_hooks.m_view_render.GetOldMethod< OnRenderStart_t >( CViewRender::ONRENDERSTART )( this );

	if( g_menu.main.visuals.fov.get( ) ) {
		if( g_cl.m_local && g_cl.m_local->m_bIsScoped( ) ) {
			auto* weapon = g_cl.m_local->GetActiveWeapon();
			if (g_cl.m_weapon_id != Weapons_t::SG553 && g_cl.m_weapon_id != Weapons_t::AUG) {

				if (weapon) {
					float percent = g_menu.main.visuals.fov_scoped1.get() / 100.f;
					float base_fov = g_menu.main.visuals.fov_amt.get();

					// Adjust FOV based on zoom level
					if (weapon->m_zoomLevel() != 2) {
						if (g_csgo.m_view_render) {
							g_csgo.m_view_render->m_view.m_fov = base_fov / (1.f + 1.25f * percent);
						}
					}
					else {
						if (g_csgo.m_view_render) {
							g_csgo.m_view_render->m_view.m_fov = base_fov / (1.f + 2.5f * percent);
						}
					}
				}
			}
		}

		else g_csgo.m_view_render->m_view.m_fov = g_menu.main.visuals.fov_amt.get( );
	}

	if( g_menu.main.visuals.viewmodel_fov.get( ) )
		g_csgo.m_view_render->m_view.m_viewmodel_fov = g_menu.main.visuals.viewmodel_fov_amt.get( );
}

void Hooks::RenderView(const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw) {
	//

	if (g_csgo.viewmodel_offset_x)
		g_csgo.viewmodel_offset_x->SetValue(g_menu.main.visuals.viewmodel_x.get() / 2.f);
	if (g_csgo.viewmodel_offset_y)
		g_csgo.viewmodel_offset_y->SetValue(g_menu.main.visuals.viewmodel_y.get() / 2.f);
	if (g_csgo.viewmodel_offset_z)
		g_csgo.viewmodel_offset_z->SetValue(g_menu.main.visuals.viewmodel_z.get() / 2.f);

	g_hooks.m_view_render.GetOldMethod< RenderView_t >(CViewRender::RENDERVIEW)(this, view, hud_view, clear_flags, what_to_draw);
}

void Hooks::Render2DEffectsPostHUD( const CViewSetup &setup ) {
	if( !g_menu.main.visuals.removals.get(4) )
		g_hooks.m_view_render.GetOldMethod< Render2DEffectsPostHUD_t >( CViewRender::RENDER2DEFFECTSPOSTHUD )( this, setup );
}

void Hooks::RenderSmokeOverlay( bool unk ) {
	// do not render smoke overlay.
	if( !g_menu.main.visuals.removals.get(2) )
		g_hooks.m_view_render.GetOldMethod< RenderSmokeOverlay_t >( CViewRender::RENDERSMOKEOVERLAY )( this, unk );
}
