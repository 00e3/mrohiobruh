#pragma once

class VoiceDataWrapper {
public: /* basically useless but this class would store our data */
	uint32_t xuidLow;
	uint32_t xuidHigh;
	int32_t sequenceBytes;
	uint32_t sectionNumber;
	uint32_t uncompressedSampleOffset;
};

class CustomVoiceData {
public:
	char packetIdentifier[ 20 ];

	/* packet data */
	const char* cheatName = XOR( "" );
	int testInt = 0;
	float testFloat = 0.f;
	ang_t testAng = ang_t( 0.f, 0.f, 0.f );
};

class CSVCMsg_VoiceData {
public:
	char pad_0000[ 8 ]; //0x0000
	int32_t client; //0x0008
	int32_t audible_mask; //0x000C
	uint32_t xuid_low{ };
	uint32_t xuid_high{ };
	void* voice_data; //0x0018
	int32_t proximity; //0x001C
	//int32_t caster; //0x0020
	int32_t format; //0x0020
	int32_t sequence_bytes; //0x0024
	uint32_t section_number; //0x0028
	uint32_t uncompressed_sample_offset; //0x002C
public:
	VoiceDataWrapper GetData( ) const {
		VoiceDataWrapper customData{};
		customData.xuidLow = this->xuid_low;
		customData.xuidHigh = this->xuid_high;
		customData.sequenceBytes = this->sequence_bytes;
		customData.sectionNumber = this->section_number;
		customData.uncompressedSampleOffset = this->uncompressed_sample_offset;

		return customData;
	}
};

using AttachmentHelperFn = void( __thiscall* )( Entity*, CStudioHdr* );

namespace FireEvents {
	void __fastcall Hook( );
	using fn = void( __fastcall* )( );
	inline fn original;
}

namespace MaintainSequenceTransitions {
	void __fastcall Hook( void* _this, void* boneSetup, float flCycle, void* pos, void* q );
	using fn = void( __fastcall* )( void* _this, void* boneSetup, float flCycle, void* pos, void* q );
	inline fn original;
}

namespace VoiceData {
	void __fastcall Hook( void* ecx, void* edx, const CSVCMsg_VoiceData& msg );
	using fn = void( __fastcall* )( void* ecx, void* edx, const CSVCMsg_VoiceData& msg );
	inline fn original;
}

namespace SendDataGram {
	int __fastcall Hook( void* ecx, void* edx, void* data );
	using fn = int( __fastcall* )( void*, void*, void* );
	inline fn original = nullptr;
}

namespace ProcessPacket {
	void __fastcall Hook( void* ecx, int edx, void* packet, bool bHasHeader );

	using fn = void( __fastcall* )( void* ecx, int edx, void* packet, bool bHasHeader );
	inline fn original = nullptr;
}

namespace CreateMove {
	using fn = void* ( __fastcall* )( void*, int, int, float, bool );

	void __stdcall           CreateMove( int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket );
	void __fastcall          Hook( void* _this, int, int sequence_number, float input_sample_frametime, bool active );


	inline fn original = nullptr;
}

namespace AttachmentHelper {
	inline bool Allow = false;

	void __fastcall Hook( void* ecx, void* edx, void* studiohdr );
	using fn = void( __fastcall* )( void* ecx, void* edx, void* studiohdr );
	inline fn original = nullptr;
}

namespace SkipAnimationFrame {
	bool __fastcall Hook( void* ecx, void* edx );
	using fn = bool( __fastcall* )( void* ecx, void* edx );
	inline fn original = nullptr;
}

namespace SetupBones {
	inline bool AllowSetup;
	inline vec3_t BoneOrigin[ 64 ];
	inline BoneArray BoneCache[ 64 ][ 0x100 ];

	void CorrectMatrix( int i, int size, matrix3x4_t* matrix, vec3_t origin );

	bool __fastcall Hook( void* ecx, void* edx, BoneArray* bone_to_world_out, int max_bones, int bone_mask, float curtime );
	using fn = bool( __fastcall* )( void*, void*, BoneArray*, int, int, float );
	inline fn original = nullptr;
}

namespace InterpolateServerEntities {
	inline vec3_t Origin[ 64 ];

	void __fastcall Hook( );
	using fn = void( __fastcall* )( );
	inline fn original = nullptr;
}

namespace ModifyEyePosition {
	void __fastcall Hook( void* ecx, uintptr_t, vec3_t& position );
	using fn = void( __fastcall* )( void*, uintptr_t, vec3_t& );
	inline fn original = nullptr;
}

namespace UpdateClientAnimations {
	inline bool InUpdate;
	void DoUpdate( Player* player );

	void __fastcall Hook( void* ecx );
	using fn = void( __fastcall* )( void* );
	inline fn original = nullptr;
}

namespace ExtraBonesProcessing {
	void __fastcall Hook( void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7 );
	using fn = void( __fastcall* )( void*, void*, int, int, int, int, int, int );
	inline fn original = nullptr;
}

namespace InterpolatePart {
	int __fastcall Hook( Entity* ent, void* edx, float& curtime, vec3_t& old_origin, ang_t& old_angles, int& no_more_changes );
	using fn = int( __fastcall* )( Entity*, void*, float&, vec3_t&, ang_t&, int& );
	inline fn original = nullptr;
}

namespace GetEyeAngles {
	ang_t* __fastcall Hook( void* ecx, void* edx );
	using fn = ang_t * ( __fastcall* )( void*, void* );
	inline fn original = nullptr;
}

namespace CLMove {
	inline bool Enabled = false;
	inline int WantedTicks = 8;

	inline int ChargedTicks;
	inline float NextDTTime;

	inline bool ShiftedTick;

	void __cdecl Hook( float fSamples, bool bFinalTick );
	using fn = void( __cdecl* )( float, bool );
	inline fn original = nullptr;
}

class Hooks {
public:
	void init( );
	void uninit( );
public:
	// forward declarations
	class IRecipientFilter;

	// prototypes.
	using PaintTraverse_t = void( __thiscall* )( void*, VPANEL, bool, bool );
	using DoPostScreenSpaceEffects_t = bool( __thiscall* )( void*, CViewSetup* );
	//using CreateMove_t                 = bool( __thiscall* )( void*, float, CUserCmd* );
	using LevelInitPostEntity_t = void( __thiscall* )( void* );
	using LevelShutdown_t = void( __thiscall* )( void* );
	using LevelInitPreEntity_t = void( __thiscall* )( void*, const char* );
	using IN_KeyEvent_t = int( __thiscall* )( void*, int, int, const char* );
	using FrameStageNotify_t = void( __thiscall* )( void*, Stage_t );
	using UpdateClientSideAnimation_t = void( __thiscall* )( void* );
	using DoExtraBoneProcessing_t = void( __thiscall* )( void*, int, int, int, int, int, int );
	using CalcViewModelView_t = void( __thiscall* )( void*, vec3_t&, ang_t& );
	using InPrediction_t = bool( __thiscall* )( void* );
	using OverrideView_t = void( __thiscall* )( void*, CViewSetup* );
	using LockCursor_t = void( __thiscall* )( void* );
	using RunCommand_t = void( __thiscall* )( void*, Player*, CUserCmd*, IMoveHelper* );
	// using CanPacket_t                = bool( __thiscall* )( void* );
	using PlaySound_t = void( __thiscall* )( void*, const char* );
	using GetScreenSize_t = void( __thiscall* )( void*, int&, int& );
	using Push3DView_t = void( __thiscall* )( void*, CViewSetup&, int, void*, void* );
	using SceneEnd_t = void( __thiscall* )( void* );
	using DrawModelExecute_t = void( __thiscall* )( void*, uintptr_t, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t* );
	using ComputeShadowDepthTextures_t = void( __thiscall* )( void*, const CViewSetup&, bool );
	using GetInt_t = int( __thiscall* )( void* );
	using GetBool_t = bool( __thiscall* )( void* );
	using IsConnected_t = bool( __thiscall* )( void* );
	using IsHLTV_t = bool( __thiscall* )( void* );
	using OnEntityCreated_t = void( __thiscall* )( void*, Entity* );
	using OnEntityDeleted_t = void( __thiscall* )( void*, Entity* );
	using RenderSmokeOverlay_t = void( __thiscall* )( void*, bool );
	using ShouldDrawFog_t = bool( __thiscall* )( void* );
	using ShouldDrawParticles_t = bool( __thiscall* )( void* );
	using Render2DEffectsPostHUD_t = void( __thiscall* )( void*, const CViewSetup& );
	using OnRenderStart_t = void( __thiscall* )( void* );
	using RenderView_t = void( __thiscall* )( void*, const CViewSetup&, const CViewSetup&, int, int );
	using GetMatchSession_t = CMatchSessionOnlineHost * ( __thiscall* )( void* );
	using OnScreenSizeChanged_t = void( __thiscall* )( void*, int, int );
	using OverrideConfig_t = bool( __thiscall* )( void*, MaterialSystem_Config_t*, bool );
	using PostDataUpdate_t = void( __thiscall* )( void*, DataUpdateType_t );
	using TempEntities_t = bool( __thiscall* )( void*, void* );
	using EmitSound_t = void( __thiscall* )( void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, float, int, int, int, const vec3_t*, const vec3_t*, void*, bool, float, int );
	// using PreDataUpdate_t            = void( __thiscall* )( void*, DataUpdateType_t );
	using PacketStart_t = void( __thiscall* )( void*, int, int );

public:
	void                     PacketStart( int incoming_sequence, int outgoing_acknowledged );
	bool                     TempEntities( void* msg );
	void                     PaintTraverse( VPANEL panel, bool repaint, bool force );
	bool                     DoPostScreenSpaceEffects( CViewSetup* setup );
	//bool                     CreateMove( float input_sample_time, CUserCmd* cmd );
	void                     LevelInitPostEntity( );
	void                     LevelShutdown( );
	//int                      IN_KeyEvent( int event, int key, const char* bind );
	void                     LevelInitPreEntity( const char* map );
	void                     FrameStageNotify( Stage_t stage );
	bool                     InPrediction( );
	bool                     ShouldDrawParticles( );
	bool                     ShouldDrawFog( );
	void                     OverrideView( CViewSetup* view );
	void                     LockCursor( );
	void                     PlaySound( const char* name );
	void                     OnScreenSizeChanged( int oldwidth, int oldheight );
	void                     RunCommand( Player* ent, CUserCmd* cmd, IMoveHelper* movehelper );
	//void                     GetScreenSize( int& w, int& h );
	void                     SceneEnd( );
	void                     DrawModelExecute( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone );
	void                     ComputeShadowDepthTextures( const CViewSetup& view, bool unk );
	int                      DebugSpreadGetInt( );
	bool                     NetShowFragmentsGetInt( );
	bool                     ExtrapolateGetInt( );
	bool                     IsConnected( );
	bool                     IsHLTV( );
	void                     EmitSound( IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, float flAttenuation, int nSeed, int iFlags, int iPitch, const vec3_t* pOrigin, const vec3_t* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity );
	void                     RenderSmokeOverlay( bool unk );
	void                     OnRenderStart( );
	void                     RenderView( const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw );
	void                     Render2DEffectsPostHUD( const CViewSetup& setup );
	CMatchSessionOnlineHost* GetMatchSession( );
	bool                     OverrideConfig( MaterialSystem_Config_t* config, bool update );
	void                     PostDataUpdate( DataUpdateType_t type );

	static LRESULT WINAPI WndProc( HWND wnd, uint32_t msg, WPARAM wp, LPARAM lp );

public:
	// vmts.
	VMT m_panel;
	VMT m_client_mode;
	VMT m_client;
	VMT m_client_state;
	VMT m_engine;
	VMT m_engine_sound;
	VMT m_prediction;
	VMT m_surface;
	VMT m_render;
	VMT m_render_view;
	VMT m_model_render;
	VMT m_shadow_mgr;
	VMT m_view_render;
	VMT m_match_framework;
	VMT m_material_system;
	VMT m_fire_bullets;
	VMT m_net_show_fragments;
	VMT m_cl_extrapolate;

	// player shit.
	std::array< VMT, 64 > m_player;

	// cvars
	VMT m_debug_spread;

	// wndproc old ptr.
	WNDPROC m_old_wndproc;

	// netvar proxies.
	RecvVarProxy_t m_Pitch_original;
	RecvVarProxy_t m_Force_original;
};

// note - dex; these are defined in player.cpp.
class CustomEntityListener : public IEntityListener {
public:
	virtual void OnEntityCreated( Entity* ent );
	virtual void OnEntityDeleted( Entity* ent );

	__forceinline void init( ) {
		g_csgo.AddListenerEntity( this );
	}

	__forceinline void uninit( ) {
		( *g_csgo.m_eventListeners ).FindAndRemove( this );
	}
};

extern Hooks                g_hooks;
extern CustomEntityListener g_custom_entity_listener;