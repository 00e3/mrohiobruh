#pragma once

class AdaptiveAngle {
public:
	float m_yaw, m_add;
	float m_dist;

public:
	// ctor.
	__forceinline AdaptiveAngle( float yaw, float add ) {
		// set yaw.
		m_yaw = math::NormalizedAngle( yaw );
		m_add = math::NormalizedAngle( add );

		// init distance.
		m_dist = 0.f;
	}
};

enum AntiAimMode : size_t {
	STAND = 0,
	WALK,
	AIR,
};

class HVH {
public:
	size_t m_mode;
	int    m_pitch;
	int    m_yaw;
	float  m_jitter_range;
	float  m_rot_range;
	float  m_rot_speed;
	float  m_rand_update;
	int    m_dir;
	float  m_dir_custom;
	size_t m_base_angle;
	float  m_auto_time;

	bool cumming_cock;

	bool   m_step_switch;
	int    m_random_lag;
	float  m_next_random_update;
	float  m_random_angle;
	float  m_direction;
	float  m_auto;
	float  m_auto_dist;
	float  m_auto_last;
	float  m_view;

	bool   m_front;
	bool   m_back;
	bool   m_right;
	bool   m_left;

	bool   m_extend_desync;
	bool   m_old_desync;
	bool   m_desync;
	bool   m_inverter;
	bool   m_exploit;
	int    m_twist_side;

	bool   m_fl_switch;
	float  m_break_yaw;
	bool   m_fakelag;
	float  m_desync_yaw;
	bool   m_balance_desync;

	rebuilt_animstate_t m_exploit_data;
	float  m_avoid_air;
	bool   m_jitter;

	Player* m_target;
	float   m_at_target;
	float  m_twist_yaw;
	bool   m_twist_flick;
	bool   m_twist_invert;
public:
	int ChokePeek();
	void AntiLastMove();
	void IdealPitch( );
	void AntiAimPitch( );
	void AutoDirection( );
	void GetAntiAimDirection( );
    bool DoEdgeAntiAim( Player *player, ang_t &out );
	void DoRealAntiAim( );
	bool HandleLby( rebuilt_animstate_t& data, const int& body_mode );
	void DoFakeAntiAim( );
	void AntiAim( );
	void SendPacket( );
};

extern HVH g_hvh;