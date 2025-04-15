#include "../Hooked.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "../../Utils/InputSys.hpp"
#include "../../SDK/Classes/PropManager.hpp"
#include "../../Features/Miscellaneous/Miscellaneous.hpp"

void __fastcall Hooked::OverrideView( void* ECX, int EDX, CViewSetup* vsView ) {
	if( !vsView ) {
		return oOverrideView( ECX, vsView );
	}

	g_Vars.globals.szLastHookCalled = XorStr( "18" );
	C_CSPlayer *const pLocal = C_CSPlayer::GetLocalPlayer( );

	bool bOk = vsView && pLocal != nullptr && g_pEngine->IsInGame( );

	if( bOk ) {
		if( !pLocal->IsDead( ) ) {
			static bool bFakeducking = false;

			// show viewmodel in scope
			auto weapon = ( C_WeaponCSBaseGun * )pLocal->m_hActiveWeapon( ).Get( );
			if( weapon ) {
				auto weaponData = weapon->GetCSWeaponData( );
				const bool m_bHideViewModelZoomed = *( bool * )( ( uintptr_t )weaponData.Xor( ) + 0x01BD ) = !g_Vars.misc.show_viewmodel_in_scope;
			}
		}

		g_Misc.ThirdPerson( );
	}

	g_Misc.OverrideFOV( vsView );

	g_Vars.globals.m_vecCameraPosition = vsView->origin;
	g_Vars.globals.m_angCameraAngles = vsView->angles;

	oOverrideView( ECX, vsView );
}
