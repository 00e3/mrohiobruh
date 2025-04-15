#pragma once

class ShotRecord {
public:
	ShotRecord( ) {
		m_hitgroup = -1;
		m_impact_group = -1;
		std::memset( &m_record, 0, sizeof( LagRecord ) );
		m_tick = -1;
		m_lat = 0.f;
		m_range = 8912.f;
		m_start = { 0, 0, 0 };
		m_end = { 0, 0, 0 };
		m_matched = false;
		m_safe = false;
	}

public:
	int        m_tick, m_hitgroup, m_impact_group;
	LagRecord  m_record;
	float      m_lat, m_range;
	vec3_t     m_start, m_end;
	bool       m_matched, m_safe;
};

class VisualImpactData_t {
public:
	vec3_t m_impact_pos, m_shoot_pos;
	int    m_tickbase;
	bool   m_ignore, m_hit_player;

public:
	__forceinline VisualImpactData_t( const vec3_t& impact_pos, const vec3_t& shoot_pos, int tickbase ) :
		m_impact_pos{ impact_pos }, m_shoot_pos{ shoot_pos }, m_tickbase{ tickbase }, m_ignore{ false }, m_hit_player{ false } {}
};

class ImpactRecord {
public:
	ImpactRecord( ) {
		m_hitgroup = -1;
		m_tick = -1;
		m_miss = -1;
		m_hurt = false;
		m_scanned = false;
		m_shot = nullptr;
		m_pos = { 0, 0, 0 };
	}

public:
	int         m_hitgroup;
	int         m_tick;
	int         m_miss;
	bool        m_hurt;
	bool        m_scanned;
	ShotRecord* m_shot;
	vec3_t      m_pos;
};

class HitRecord {
public:
	__forceinline HitRecord( ) : m_impact{}, m_group{ -1 }, m_damage{} {}

public:
	ImpactRecord* m_impact;
	int           m_group;
	float         m_damage;
};

class Shots {
public:
	std::array< std::string, 9 > m_groups = {
	XOR( "body" ),
	XOR( "head" ),
	XOR( "chest" ),
	XOR( "stomach" ),
	XOR( "left arm" ),
	XOR( "right arm" ),
	XOR( "left leg" ),
	XOR( "right leg" ),
	XOR( "neck" )
	};

	void OnShotFire( Player* target, const int& damage, LagRecord* record, const vec3_t& end_pos, const int& hitgroup, const bool& safety );
	void OnImpact( IGameEvent* evt );
	void OnHurt( IGameEvent* evt );
	void HandleMisses( AimPlayer* data );

public:
	std::deque< ShotRecord >          m_shots;
	std::vector< VisualImpactData_t > m_vis_impacts;
	std::deque< ImpactRecord >        m_impacts;
	std::deque< HitRecord >           m_hits;
};

extern Shots g_shots;