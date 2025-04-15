#include "includes.h"

bool Hooks::IsConnected( ) {
	return g_hooks.m_engine.GetOldMethod< IsConnected_t >( IVEngineClient::ISCONNECTED )( this );
}

bool Hooks::IsHLTV( ) {
	if (SetupBones::AllowSetup)
		return true;

	if (UpdateClientAnimations::InUpdate)
		return true;

	return g_hooks.m_engine.GetOldMethod< IsHLTV_t >( IVEngineClient::ISHLTV )( this );
}

void Hooks::EmitSound( IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, float flAttenuation, int nSeed, int iFlags, int iPitch, const vec3_t* pOrigin, const vec3_t* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity ) {
	return g_hooks.m_engine_sound.GetOldMethod<EmitSound_t>( IEngineSound::EMITSOUND )( this, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, flAttenuation, nSeed, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity );
}