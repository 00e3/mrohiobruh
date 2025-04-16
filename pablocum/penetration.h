#pragma once

struct pen_data_t {
	__forceinline pen_data_t() = default;
	__forceinline pen_data_t(int dmg, int hitbox, int group, int pen) : m_dmg{ dmg }, m_hitbox{ hitbox }, m_hit_group{ group }, m_remaining_pen{ pen } {}

	int m_dmg{}, m_hitbox{}, m_hit_group{}, m_remaining_pen{};
};

namespace penetration {
	struct PenetrationInput_t {
		Player* m_from;
		Player* m_target;
		vec3_t  m_pos;
		float	m_damage;
		float   m_damage_pen;
		bool	m_can_pen;
		vec3_t start;
		Player* m_notargetbutstillignore;
	};

	struct PenetrationOutput_t {
		Player* m_target;
		float   m_damage;
		int     m_hitgroup;
		bool    m_pen;

		__forceinline PenetrationOutput_t() : m_target{ nullptr }, m_damage{ 0.f }, m_hitgroup{ -1 }, m_pen{ false } {}
	};

	float scale(Player* player, float damage, float armor_ratio, int hitgroup);
	bool  TraceToExit(const vec3_t& start, const vec3_t& dir, vec3_t& out, CGameTrace* enter_trace, CGameTrace* exit_trace);
	void  ClipTraceToPlayer(const vec3_t& start, const vec3_t& end, uint32_t mask, CGameTrace* tr, Player* player, float min);
	bool  run(PenetrationInput_t* in, PenetrationOutput_t* out);
	bool run_modified(PenetrationInput_t* in, PenetrationOutput_t* out);
	static bool is_visible(const vec3_t& start, const vec3_t& end, PenetrationInput_t* start_ent = nullptr, PenetrationOutput_t* end_ent = nullptr);
}

class c_auto_wall {
private:
public:
	pen_data_t get(const vec3_t& shoot_pos, const vec3_t& point, const Player* enemy);
	void scale_dmg(Player* player, CGameTrace& trace, WeaponInfo* wpn_info, float& cur_dmg, const int hit_group);

	bool trace_to_exit(const vec3_t& src, const vec3_t& dir,
		const CGameTrace& enter_trace, CGameTrace& exit_trace);

	bool handle_bullet_penetration(
		WeaponInfo* wpn_data, CGameTrace& enter_trace, vec3_t& eye_pos, const vec3_t& direction, int& possible_hits_remain,
		float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration
	);

	bool fire_bullet(Weapon* wpn, vec3_t& direction, bool& visible, float& cur_dmg, int& remaining_pen, int& hit_group,
		int& hitbox, Entity* e = nullptr, float length = 0.f, const vec3_t& pos = { 0.f,0.f,0.f });
};

inline const auto g_auto_wall = std::make_unique < c_auto_wall >();