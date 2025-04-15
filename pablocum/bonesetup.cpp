#include "includes.h"

Bones g_bones{};;

void Bones::SetupCorrectionMatrix( Player* target, BoneArray* in, int size ) {
	SetupBones::BoneOrigin[ target->index( ) - 1 ] = target->GetAbsOrigin( );
	std::memcpy( SetupBones::BoneCache[ target->index( ) - 1 ], in, sizeof( BoneArray ) * size );
}

bool Bones::Setup( Player* target, int mask, BoneArray* out, int size, bool rebuilt, bool setup_cache ) {
	if ( !target || !target->alive( ) )
		return false;

	static alignas( 16 ) vec3_t		  pos[ 128 ];
	static alignas( 16 ) quaternion_t q[ 128 ];

	CStudioHdr* hdr = target->m_studioHdr( );

	if ( !hdr )
		return false;

	int& jiggle_bones_player = *reinterpret_cast< int* >( uintptr_t( target ) + 0x291C );

	const float lean = target->m_AnimOverlay( )[ 12 ].m_weight;
	const float curtime = g_csgo.m_globals->m_curtime;
	const float frametime = g_csgo.m_globals->m_frametime;
	const int effects = target->m_fEffects( );
	const int eflags = target->m_iEFlags( );
	const int computed_lod_frame = target->m_nComputedLODframe( );
	const int bone_jiggle = jiggle_bones_player;

	g_csgo.m_globals->m_curtime = target->m_flSimulationTime( );
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	// stop player interpolation.
	if ( target != g_cl.m_local || !g_menu.main.misc.animations.get( 4 ) )
		target->AddEffect( EF_NOINTERP );

	target->m_nComputedLODframe( ) = 0;
	target->InvalidateBoneCache( );

	jiggle_bones_player = 0;

	SetupBones::AllowSetup = true;

	/* for local we want client bone setup for interp and for teammates we want it for nametag */
	if ( rebuilt ) {
		target->m_iEFlags( ) |= EFL_SETTING_UP_BONES;

		// first we setup needed shit for bones
		target->StandardBlendingRules( hdr, pos, q, g_csgo.m_globals->m_curtime, mask );

		// build chain.
		static int32_t chain[ 128 ] = {};
		const auto chain_length = hdr->m_studio_hdr->m_num_bones;
		for ( auto i = 0; i < chain_length; i++ )
			chain[ chain_length - i - 1 ] = i;

		// build transformations.
		// (this actually set the bones up)
		static matrix3x4_t rotation;
		math::AngleMatrix( target->GetAbsAngles( ), target->GetAbsOrigin( ), rotation );
		for ( auto j = chain_length - 1; j >= 0; j-- )
		{
			const auto i = chain[ j ];
			const auto parent = hdr->m_bone_parent.Count( ) > i ? &hdr->m_bone_parent[ i ] : nullptr;

			if ( !parent )
				continue;

			static matrix3x4_t qua;
			qua = math::QuaternionMatrix( q[ i ], pos[ i ] );

			if ( *parent == -1 )
				math::ConcatTransforms( rotation, qua, out[ i ] );
			else
				math::ConcatTransforms( out[ *parent ], qua, out[ i ] );
		}
	}
	else {
		AttachmentHelper::Allow = false;
		SetupBones::original( target->renderable( ), nullptr, out, size, mask, g_csgo.m_globals->m_curtime );
		AttachmentHelper::Allow = true;
	}

	SetupBones::AllowSetup = false;

	// start interpolation again.
	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = frametime;
	target->m_fEffects( ) = effects;
	target->m_iEFlags( ) = eflags;
	target->m_nComputedLODframe( ) = computed_lod_frame;
	jiggle_bones_player = bone_jiggle;

	if ( setup_cache )
		SetupCorrectionMatrix( target, out, size );

	return true;
}