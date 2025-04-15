#include "includes.h"

void CustomEntityListener::OnEntityCreated( Entity *ent ) {
    if( ent ) {
        // player created.
        if( ent->IsPlayer( ) ) {
		    Player* player = ent->as< Player* >( );

		    // access out player data stucture and reset player data.
		    AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
            if( data )
		        data->reset( );

		    // get ptr to vmt instance and reset tables.
		    VMT* vmt = &g_hooks.m_player[ player->index( ) - 1 ];
            if( vmt ) {
                // init vtable with new ptr.
		        vmt->reset( );
		        vmt->init( player );
            }
        }
	}
}

void CustomEntityListener::OnEntityDeleted( Entity *ent ) {
    // note; IsPlayer doesn't work here because the ent class is CBaseEntity.
	if( ent && ent->index( ) >= 1 && ent->index( ) <= 64 ) {
		Player* player = ent->as< Player* >( );

		// access out player data stucture and reset player data.
		AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
        if( data )
		    data->reset( );

		// get ptr to vmt instance and reset tables.
		VMT* vmt = &g_hooks.m_player[ player->index( ) - 1 ];
		if (vmt) {
			vmt->reset();
		}
	}
}