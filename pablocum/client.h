#pragma once

class Sequence {
public:
	float m_time;
	int   m_state;
	int   m_seq;

public:
	__forceinline Sequence( ) : m_time{}, m_state{}, m_seq{} {};
	__forceinline Sequence( float time, int state, int seq ) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
};

class NetPos {
public:
	float  m_time;
	vec3_t m_pos;

public:
	__forceinline NetPos( ) : m_time{}, m_pos{} {};
	__forceinline NetPos( float time, vec3_t pos ) : m_time{ time }, m_pos{ pos } {};
};

class AnimLayers {
public:
	void SetLayerSequence( CCSGOPlayerAnimState* state, C_AnimationLayer* layer, int act, int overrideseq );

	float GetLayerIdealWeightFromSeqCycle( Player* pPlayer, C_AnimationLayer* pLayer );
	bool IsLayerSequenceCompleted( float increment, C_AnimationLayer* pLayer );
	int GetLayerActivity( Player* pPlayer, C_AnimationLayer* pLayer );
	int LookupSequence( Player* e, const char* label );
	void IncrementLayerCycle( float increment, C_AnimationLayer* pLayer, bool bAllowLoop );
	void IncrementLayerWeight( float increment, C_AnimationLayer* pLayer );
	void IncrementLayerCycleWeightRateGeneric( float increment, C_AnimationLayer* pLayer );
	void UpdateAnimLayer( C_AnimationLayer* pLayer, int nSequence, float flPlaybackRate, float flWeight, float flCycle );
	float GetFirstSequenceAnimTag( Player* player, int sequence, int nDesiredTag, float flStart, float flEnd );

	struct PlayerData {
		rebuilt_animstate_t m_server_data;
	};
	PlayerData m_data[ 64 ];

	rebuilt_animstate_t m_fake_data;

	void UpdateLayers( Player* player, const ang_t& angle, rebuilt_animstate_t* data, const float& curtime, bool apply_state = true );
	void SetLayers( Player* player, rebuilt_animstate_t* data );
	void SetPoses( Player* player, rebuilt_animstate_t* data );
	void DoAnimationEvent( Player* player, Entity* ground_entity, rebuilt_animstate_t* data );
};

struct ServerNetVars {
	vec3_t m_origin;
	int    m_tickbase;
};


class Client {
public:
	// hack thread.
	static ulong_t __stdcall init( void* arg );

	void StartMove( CUserCmd* cmd );
	void EndMove( CUserCmd* cmd );
	void BackupPlayers( bool restore );
	void DoMove( );
	void DrawHUD( );
	void UpdateInformation( );
	void SetAngles( rebuilt_animstate_t* data );
	void HandleAnimationCopy( rebuilt_animstate_t* data, rebuilt_animstate_t& copy );
	void UpdateShootPosition( );
	void KillFeed( );
	void DrawServerHitboxes( );
	int  GetCheat( int i, bool update );

	void OnPaint( );
	void OnMapload( );
	void OnTick( CUserCmd* cmd );

	// debugprint function.
	void print( const std::string text, ... );

	// check if we are able to fire this tick.
	bool CanFireWeapon( );
	void UpdateRevolverCock( );
	void UpdateIncomingSequences( );
public:
	C_AnimationLayer m_server_layers[ 2 ][ 13 ];
	float  m_speed;

	int              m_taps;
	vec3_t m_pre_autopeek_pos;
	matrix3x4_t m_pre_autopeek_bones[128];

	std::deque<int>  m_sent_cmds;
	// local player variables.
	AnimLayers       m_anim_data;
	BoneArray        m_shot_bones[ 128 ];
	Player* m_local;
	int	             m_flags;
	vec3_t	         m_shoot_pos, m_origin;
	bool	         m_player_fire;
	bool	         m_shot;
	bool	         m_old_shot;
	float            m_curtime;

	bool             m_flick;

	bool             m_fake_bones_setup;
	BoneArray        m_fake_bones[ 128 ];
	vec3_t           m_fake_bone_origin;

	bool m_correction_override;
	bool m_update_override_correction;
	ang_t m_override_correction_ang;
	int      m_latency_ticks;
	// active weapon variables.
	Weapon* m_weapon;
	int         m_weapon_id;
	WeaponInfo* m_weapon_info;
	int         m_weapon_type;
	bool        m_weapon_fire;

	// revolver variables.
	int	 m_revolver_cock;
	int	 m_revolver_query;
	bool m_revolver_fire;

	// general game varaibles.
	bool     m_round_end;
	Stage_t	 m_stage;
	int	     m_max_lag;
	int      m_lag;
	int	     m_old_lag;
	bool     m_packet;
	//bool*    m_final_packet;
	bool	 m_old_packet;
	float	 m_lerp;
	float    m_latency;
	int      m_server_tick;
	int      m_arrival_tick;
	int      m_width, m_height;

	// usercommand variables.
	CUserCmd* m_cmd;
	int	      m_tick;
	int       m_old_buttons;
	ang_t     m_view_angles;
	ang_t	  m_strafe_angles;
	vec3_t	  m_forward_dir;
	bool      m_fixed_movement;

	penetration::PenetrationOutput_t m_pen_data;

	std::deque< Sequence > m_sequences;
	std::deque< NetPos >   m_net_pos;

	// animation variables.
	ang_t  m_abs_angle;
	ang_t  m_angle;
	ang_t  m_radar;
	float  m_move_yaw;
	float  m_body;
	float  m_body_pred;
	float  m_anim_time;
	float  m_anim_frame;
	bool   m_anim_shot;
	float  m_shot_pitch;
	bool   m_old_lagcomp;
	bool   m_lagcomp;
	int    m_lby_increment;
	bool   m_ground;

	vec3_t m_state_velocity;

	bool m_fakewalk, m_in_jump;
	bool m_auto_peek;
	rebuilt_animstate_t m_rebuilt_state;

	// hack username.
	std::string m_user;
};

extern Client g_cl;