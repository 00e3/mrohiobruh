#pragma once

class Movement {
public:
	bool m_autopeek;

	float  m_speed;
	float  m_ideal;
	float  m_ideal2;
	vec3_t m_mins;
	vec3_t m_maxs;
	vec3_t m_origin;
	float  m_switch_value = 1.f;
	int    m_strafe_index;
	float  m_old_yaw;
	float  m_circle_yaw;
	bool   m_did_move;
	bool   m_auto_peek;
	bool   m_invert;
	bool m_reached_target;
	vec3_t m_target_pos;

public:
	void run( );
	void JumpRelated( );
	void Strafe( );
	void AutoStop( );
	void FullStop( );
	void QuickStop();
	void FixMove( CUserCmd* cmd );
	void AutoPeek( );
	void FakeWalk( );
};

extern Movement g_movement;