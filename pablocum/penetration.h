#pragma once

namespace penetration {
	struct PenetrationInput_t {
		Player* m_from;
		Player* m_target;
		vec3_t  m_start;
		vec3_t  m_pos;
		float	m_damage;
		float   m_damage_pen;
		bool	m_can_pen;
	};

	struct PenetrationOutput_t {
		int     m_damage;
		int     m_hitgroup;
		int     m_pen;

        __forceinline PenetrationOutput_t() : m_damage{ 0 }, m_hitgroup{ -1 }, m_pen{ 0 } {}
	};

    bool  run( PenetrationInput_t* in, PenetrationOutput_t* out );
}