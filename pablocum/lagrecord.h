#pragma once

// pre-declare.
class LagRecord;

class BackupRecord {
public:
	BoneArray  m_bones[ 128 ];
	vec3_t     m_origin, m_abs_origin;
	vec3_t     m_mins;
	vec3_t     m_maxs;
	ang_t      m_abs_ang;

public:
	__forceinline void store( Player* player ) {
		player->GetBones( m_bones );
		m_origin = player->m_vecOrigin( );
		m_mins = player->m_vecMins( );
		m_maxs = player->m_vecMaxs( );
		m_abs_origin = player->GetAbsOrigin( );
		m_abs_ang = player->GetAbsAngles( );
	}

	__forceinline void restore( Player* player ) {
		player->SetBones( m_bones );

		player->m_vecOrigin( ) = m_origin;
		player->m_vecMins( ) = m_mins;
		player->m_vecMaxs( ) = m_maxs;
		player->SetAbsAngles( m_abs_ang, false );
		player->SetAbsOrigin( m_origin, false );
	}
};

enum RecordFlags {
	INVALID = ( 1 << 0 ),
	BREAKINGLC = ( 1 << 1 ),
	SHIFTING = ( 1 << 2 ),
	PREDICTED = ( 1 << 3 )
};

enum Modes : size_t
{
	RESOLVE_NONE = 0,
	RESOLVE_WALK,
	RESOLVE_DESYNC,
	RESOLVE_STAND,
	RESOLVE_STAND_STOPPEDMOVING,
	RESOLVE_STAND_NOUPDATE,
	RESOLVE_STAND_LOGIC,
	RESOLVE_STAND_TWIST,
	RESOLVE_STAND_ROTATE,
	RESOLVE_STAND_BREAKMOVE,
	RESOLVE_AIR,
	RESOLVE_UPDATE,
};

enum StandBrute : size_t
{
	NORMAL,
	LOGIC,
	NOUPDATE,
	TWIST,
	ROTATE,
	BREAKMOVE,
	STOPPEDMOVING,
	MAX
};

class LagRecord {
public:
	// data.
	Player* m_player;
	float   m_immune;
	int     m_lag;
	bool        m_dormant;
	bool m_delayed;

	// netvars.
	float  m_lerp_time;
	float  m_sim_time;
	int    m_sv_tick;
	int    m_flags;
	vec3_t m_origin;
	vec3_t m_velocity;
	vec3_t m_mins;
	vec3_t m_maxs;
	ang_t  m_eye_angles;
	ang_t  m_abs_ang;
	float  m_body;
	float  m_duck;
	float  m_duck_speed;
	int    m_move_state;
	int    m_move_type;
	vec3_t      m_old_origin;

	// anim stuff.
	C_AnimationLayer m_layers[ 13 ];
	C_AnimationLayer m_foot_layers[ 3 ][ 13 ];
	float            m_poses[ 24 ];
	vec3_t           m_anim_velocity, m_anim_origin;
	int              m_anim_flags;
	float            m_anim_duck;
	float            m_anim_duck_speed;
	float  m_pred_time;
	int     m_tick;

	// bone stuff.
	bool       m_setup;
	BoneArray  m_bones[ 128 ];
	BoneArray  m_safe_bones[ 8 ][ 128 ];
	int        m_safe_matrice_count;
	bool    m_extrapolated;
	// resolver stuff.
	size_t m_mode;
	bool   m_shot;
	float  m_away;
	float  m_anim_time;
	vec3_t m_pred_origin;
	int m_record_flags;
	bool   m_broke_lc;
	BoneArray m_extrap_bones[128];

	vec3_t m_pred_velocity;
	int    m_pred_flags;
	Entity* m_ground_entity;
public:

	// default ctor.
	__forceinline LagRecord( ) :
		m_setup{ false },
		m_record_flags{ 0 },
		m_broke_lc{ false },

		m_shot{ false },
		m_sv_tick{ 0 },
		m_safe_matrice_count{ 0 },
		m_lag{ 1 } {}

	// ctor.
	__forceinline LagRecord( Player* player ) :
		m_setup{ false },
		m_record_flags{ 0 },
		m_broke_lc{ false },
		m_shot{ false },
		m_sv_tick{ 0 },
		m_safe_matrice_count{ 0 },
		m_lag{ 1 } {

		store( player );
	}

	// function: allocates memory for SetupBones and stores relevant data.
	__forceinline void store( Player* player ) {
		// player data.
		m_player = player;
		m_immune = player->m_fImmuneToGunGameDamageTime( );
		m_pred_origin = m_origin = player->m_vecOrigin();
		m_pred_velocity = m_velocity = player->m_vecVelocity();
		m_pred_time = m_sim_time = player->m_flSimulationTime();
		m_tick = g_csgo.m_cl->m_server_tick;

		// netvars.
		m_lerp_time = m_anim_time = m_sim_time = player->m_flSimulationTime( );
		m_flags = player->m_fFlags( );
		m_origin = player->m_vecOrigin( );
		m_eye_angles = player->m_angEyeAngles( );
		m_abs_ang = player->GetAbsAngles( );
		m_body = player->m_flLowerBodyYawTarget( );
		m_mins = player->m_vecMins( );
		m_maxs = player->m_vecMaxs( );
		m_duck_speed = player->m_flDuckSpeed( );
		m_duck = player->m_flDuckAmount( );
		m_velocity = player->m_vecVelocity( );
		m_ground_entity = player->GetGroundEntity( );
		m_move_state = player->m_iMoveState( );
		m_move_type = player->m_MoveType( );

		m_sv_tick = g_cl.m_server_tick;

		// save networked animlayers.
		player->GetAnimLayers( m_layers );
	}
	__forceinline bool brokelc() {
		return (this->m_origin - this->m_old_origin).length_sqr() > 4096.f;
	}
	__forceinline void init_anim_data( ) {
		// initialize animtime later.
		m_anim_duck = m_duck;
		m_anim_duck_speed = m_duck_speed;
		m_anim_flags = m_flags;
		m_anim_origin = m_origin;
		m_anim_velocity = m_velocity;
	}
	__forceinline void predict() {
		m_broke_lc = false;
		m_extrapolated = false;
		m_pred_origin = m_origin;
		m_pred_velocity = m_velocity;
		m_pred_time = m_sim_time;
		m_pred_flags = m_flags;

		// memcpy bones
		if (m_setup)
			memcpy(m_extrap_bones, m_bones, m_player->m_BoneCache().m_CachedBoneCount * sizeof(matrix3x4_t));

	}
	// function: writes current record to bone cache.
	__forceinline void cache( ) {
		if ( !m_setup )
			return;

		if (m_setup) {
			m_player->InvalidateBoneCache();

			if (!m_extrapolated)
				memcpy(m_player->m_BoneCache().m_pCachedBones, m_bones, m_player->m_BoneCache().m_CachedBoneCount * sizeof(matrix3x4_t));
			else
				memcpy(m_player->m_BoneCache().m_pCachedBones, m_extrap_bones, m_player->m_BoneCache().m_CachedBoneCount * sizeof(matrix3x4_t));
		}


		m_player->SetBones( m_bones );

		m_player->m_vecOrigin( ) = m_origin;
		m_player->m_vecMins( ) = m_mins;
		m_player->m_vecMaxs( ) = m_maxs;

		m_player->SetAbsAngles( m_abs_ang, false );
		m_player->SetAbsOrigin( m_origin, false );
	}
	__forceinline bool dormant() {
		return m_dormant;
	}
	__forceinline bool immune( ) const {
		return m_immune > 0.f;
	}

	__forceinline bool valid_origin_time( bool strict = false ) const {
		// calculate the delta in ticks between the current server tick and the record's tick.
		const int updatedelta = g_cl.m_server_tick - m_sv_tick;
		int lag = m_lag;

		if ( strict )
			lag /= 2;

		// calculate the valid window as the difference between their lag and your ping.
		const int valid_window = lag - game::TIME_TO_TICKS( g_cl.m_latency );

		// if the valid window is negative or zero, no valid time remains.
		if ( valid_window <= 0 )
			return false;

		// check if the delta is within the valid window.
		return std::abs( updatedelta ) <= valid_window;
	}
};