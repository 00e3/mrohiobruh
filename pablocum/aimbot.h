#pragma once

struct AnimationBackup_t
{
	vec3_t           m_origin, m_abs_origin;
	vec3_t           m_velocity, m_abs_velocity;
	int              m_flags, m_eflags;
	float            m_duck, m_duck_speed, m_body;
	C_AnimationLayer m_layers[ 13 ];
};

enum Cheats
{
	UNK,
	RAX,
	FAMILY,
	MONEYBOT
};

struct MoveData_t
{
	float m_body;
	vec3_t m_origin;
	float m_anim_time;
};

class AimPlayer
{
public:
	// essential data.
	Player* m_player;
	float	  m_freestand[ 2 ];
	float	  m_spawn;
	std::deque< LagRecord > m_records;

	float m_fov;
	bool m_delayed;
	// resolve data.
	int       m_shots;
	int       m_missed_shots;
	MoveData_t m_walk_record;

	std::deque <float> m_body_data;
	float     m_body_delta;
	float     m_body_update;
	bool      m_moved;

	int m_stand_index[ StandBrute::MAX ];
	int m_body_index;
	int m_air_index;

	int m_update_count;
	rebuilt_animstate_t m_anim_data[ 3 ];

	int m_cheat;


public:
	struct anim_backup_data_t
	{
		float                m_abs_yaw;
		C_AnimationLayer     m_layers[ 13 ];
		float                m_poses[ 24 ];

		void backup_data( const float& angle, C_AnimationLayer* layers, float* poses )
		{
			m_abs_yaw = angle;
			std::memcpy( m_layers, layers, sizeof( C_AnimationLayer ) * 13 );
			std::memcpy( m_poses, poses, sizeof( float ) * 24 );
		}
	};

	void SyncPlayerVelocity( LagRecord& current, LagRecord* previous );
	void UpdateAnimations( bool predicted = false );
	void OnNetUpdate( Player* player );
	void OnRoundStart( Player* player );

public:
	void reset( )
	{
		m_player = nullptr;
		m_spawn = 0.f;
		m_shots = 0;

		m_missed_shots = 0;
		m_body_index = 0;

		for ( auto& stand_index : m_stand_index )
			stand_index = 0;

		m_body_delta = 0;

		m_records.clear( );
		m_body_data.clear( );

		m_fov = 0.f;
		m_cheat = -1;
	}
};

class Aimbot
{
private:

public:
	bool m_delayed;
	bool m_stop;
	int  m_minimum_damage;
	bool m_damage_override;
	bool m_force_body;

	std::array< AimPlayer, 64 > m_players;
	std::vector< AimPlayer* >   m_targets;

	BackupRecord m_backup[ 64 ];

	// fake latency stuff.
	bool       m_fake_latency;

public:
	__forceinline void reset( )
	{
		// reset all players data.
		for ( auto& p : m_players )
			p.reset( );
	}

	__forceinline bool IsValidTarget( Player* player )
	{
		if ( !player )
			return false;

		if ( !player->IsPlayer( ) )
			return false;

		if ( !player->alive( ) )
			return false;

		if ( player->m_bIsLocalPlayer( ) )
			return false;

		if ( g_cl.m_local && !player->enemy( g_cl.m_local ) )
			return false;

		return true;
	}

public:
	struct hc_seed_t
	{
	public:
		float theta0;
		float theta1;
		float radius_curve_tens0;
		float radius_curve_tens1;
	};

	struct pt_data_t
	{
		bool m_body = false;
		bool m_lethality = false;
		bool m_safe = false;
		int m_hitbox = -1;
		int m_damage = -1;
		vec3_t m_point = vec3_t( );
		LagRecord* m_record = nullptr; // better be saving it as a ptr to save some heap bruw

		// ctor for multipoints when will add proper fkin system bruh
		pt_data_t( const vec3_t& point, LagRecord* record, const int& hitbox )
		{
			m_body = false;
			m_lethality = false;
			m_safe = 0;
			m_hitbox = hitbox;
			m_damage = -1;
			m_point = point;
			m_record = record;
		}

		__forceinline bool safe_record( ) const {
			switch ( m_record->m_mode ) {
			case Modes::RESOLVE_NONE:
			case Modes::RESOLVE_WALK:
			case Modes::RESOLVE_DESYNC:
			case Modes::RESOLVE_UPDATE:
				return true;
			}

			return false;
		}

		pt_data_t( )
		{
			m_body = false;
			m_lethality = false;
			m_safe = false;
			m_hitbox = -1;
			m_damage = -1;
			m_point = {};
			m_record = nullptr;
		}
	};

	struct target_data_t
	{
		pt_data_t best_point; // best record will contain record/damage/point data.
		int m_player_index;
		int m_player_health;
		bool m_lethal;
		bool m_predicted_land;

		std::vector< LagRecord* > m_records;

		float m_fov; // will be used for target limit
		Player* m_player;

		target_data_t( )
		{

		}
	};

	std::deque< hc_seed_t > m_seed_table;
	std::vector< target_data_t > m_aimbot_targets{ };
	std::vector< int > m_hitboxes{ };
	std::vector< int > m_whitelisted_ids;

	ang_t     m_aim_angle;
	pt_data_t m_best_target;

	bool ShouldForceSafety( const int& mode );

	bool PreferBodyAim( const int& hp, const pt_data_t& point );
	bool SelectBestPoint( target_data_t& target, const pt_data_t& point );

	void GetAimRecords( AimPlayer* data, target_data_t& info );

	void UpdateSettings( );
	void CreatePoints( std::vector< pt_data_t >& points, const int& index, LagRecord* record );
	void CreateHitboxes( const LagRecord* record );

	bool ScanPoint( const target_data_t& target, pt_data_t& point );

	// aimbot.
	void Think( );
	void StripAttack( );
	bool CheckHitchance( const pt_data_t& target, const ang_t& angle );
	bool OGCheckHitchance(Player* player, const ang_t& angle);

};

extern Aimbot g_aimbot;