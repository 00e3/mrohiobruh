#include "includes.h"
static ConVar* ff_damage_reduction_bullets;
static ConVar* ff_damage_bullet_penetration;
static ConVar* mp_damage_scale_ct_head;
static ConVar* mp_damage_scale_ct_body;
static ConVar* mp_damage_scale_t_head;
static ConVar* mp_damage_scale_t_body;
pen_data_t c_auto_wall::get(const vec3_t& shoot_pos, const vec3_t& point, const Player* enemy) {
	const auto who_tf = point - shoot_pos;
	ang_t angle = {};
	math::VectorAngles(who_tf, angle);

	vec3_t dir = {};
	math::AngleVectors(angle, &dir);

	dir.normalize();

	auto dmg{ 0.f };
	auto hitbox{ -1 };
	auto remain_pen{ -1 };
	auto hit_group{ -1 };
	bool dont_care{};
	if (fire_bullet(g_cl.m_local->GetActiveWeapon(), dir, dont_care, dmg, remain_pen, hit_group, hitbox, (Entity*)enemy, 0.0f, shoot_pos))
		return pen_data_t(static_cast <int> (dmg), hitbox, hit_group, remain_pen);
	else
		return pen_data_t(-1, -1, -1, -1);
}

bool c_auto_wall::handle_bullet_penetration(
	WeaponInfo* wpn_data, CGameTrace& enter_trace, vec3_t& eye_pos, const vec3_t& direction, int& possible_hits_remain,
	float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration
) {
	if (wpn_data->m_penetration <= 0.0f)
		return false;

	if (possible_hits_remain <= 0)
		return false;

	auto contents_grate = enter_trace.m_contents & CONTENTS_GRATE;
	auto surf_nodraw = enter_trace.m_surface.m_flags & SURF_NODRAW;

	auto enter_surf_data = g_csgo.m_phys_props->GetSurfaceData(enter_trace.m_surface.m_surface_props);
	auto enter_material = enter_surf_data->m_game.m_material;

	auto is_solid_surf = enter_trace.m_contents >> 3 & CONTENTS_SOLID;
	auto is_light_surf = enter_trace.m_surface.m_flags >> 7 & 0x0001;

	CGameTrace exit_trace;

	if (!trace_to_exit(enter_trace.m_endpos, direction, enter_trace, exit_trace)
		&& !(g_csgo.m_engine_trace->GetPointContents(enter_trace.m_endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL))
		return false;

	auto enter_penetration_modifier = enter_surf_data->m_game.m_penetration_modifier;
	auto exit_surface_data = g_csgo.m_phys_props->GetSurfaceData(exit_trace.m_surface.m_surface_props);

	if (!exit_surface_data)
		return false;

	auto exit_material = exit_surface_data->m_game.m_material;
	auto exit_penetration_modifier = exit_surface_data->m_game.m_penetration_modifier;

	auto combined_damage_modifier = 0.16f;
	auto combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;

	if (enter_material == 'Y' || enter_material == 'G')
	{
		combined_penetration_modifier = 3.0f;
		combined_damage_modifier = 0.05f;
	}
	else if (contents_grate || surf_nodraw)
		combined_penetration_modifier = 1.0f;
	else if (enter_material == 'F' && ((Player*)enter_trace.m_entity)->m_iTeamNum() == g_cl.m_local->m_iTeamNum() && ff_damage_reduction_bullets)
	{
		if (!ff_damage_bullet_penetration)
			return false;

		combined_penetration_modifier = ff_damage_bullet_penetration;
		combined_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == 'W' || exit_material == 'U')
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 'L')
			combined_penetration_modifier = 2.0f;
	}

	auto penetration_modifier = std::fmaxf(0.0f, 1.0f / combined_penetration_modifier);
	auto penetration_distance = (exit_trace.m_endpos - enter_trace.m_endpos).length();

	penetration_distance = penetration_distance * penetration_distance * penetration_modifier * 0.041666668f;

	auto damage_modifier = std::max(0.0f, 3.0f / wpn_data->m_penetration * 1.25f) * penetration_modifier * 3.0f + cur_dmg * combined_damage_modifier + penetration_distance;
	auto damage_lost = std::max(0.0f, damage_modifier);

	if (damage_lost > cur_dmg)
		return false;

	cur_dmg -= damage_lost;

	if (cur_dmg < 1.0f)
		return false;

	eye_pos = exit_trace.m_endpos;
	--possible_hits_remain;

	return true;
}

bool c_auto_wall::fire_bullet(Weapon* wpn, vec3_t& direction, bool& visible, float& cur_dmg, int& remaining_pen, int& hit_group,
	int& hitbox, Entity* entity, float length, const vec3_t& pos)
{
	static CTraceFilterSkipTwoEntities_game filter{};
	if (!wpn)
		return false;

	auto wpn_data = wpn->GetWpnData();

	if (!wpn_data)
		return false;

	CGameTrace enter_trace;

	cur_dmg = (float)wpn_data->m_damage;

	auto eye_pos = pos;
	auto cur_dist = 0.0f;
	auto max_range = wpn_data->m_range;
	auto pen_dist = 3000.0f;
	auto pen_power = wpn_data->m_penetration;
	auto possible_hit_remain = 4;
	remaining_pen = 4;
	filter.SetPassEntity(g_cl.m_local);
	filter.SetPassEntity2(nullptr);
	while (cur_dmg > 0.f)
	{
		max_range -= cur_dist;
		auto end = eye_pos + direction * max_range;

		g_csgo.m_engine_trace->TraceRay(Ray(eye_pos, end), MASK_SHOT, (ITraceFilter*)&filter, &enter_trace);

		if (entity)
			penetration::ClipTraceToPlayer(eye_pos, end + (direction * 40.f), MASK_SHOT, &enter_trace, (Player*)entity, -60.f);

		auto enter_surf_data = g_csgo.m_phys_props->GetSurfaceData(enter_trace.m_surface.m_surface_props);
		auto enter_surf_pen_mod = enter_surf_data->m_game.m_penetration_modifier;
		auto enter_mat = enter_surf_data->m_game.m_material;

		if (enter_trace.m_fraction == 1.0f)
			break;

		cur_dist += enter_trace.m_fraction * max_range;
		cur_dmg *= pow(wpn_data->m_range_modifier, cur_dist / 500.0f);

		auto hit_player = static_cast <Player*> (enter_trace.m_entity);

		if (cur_dist > pen_dist && wpn_data->m_penetration || enter_surf_pen_mod < 0.1f)
			break;

		auto can_do_dmg = enter_trace.m_hitgroup != HITGROUP_GEAR && enter_trace.m_hitgroup != HITGROUP_GENERIC;
		auto is_player = ((Player*)enter_trace.m_entity)->IsPlayer();
		auto is_enemy = ((Player*)enter_trace.m_entity)->m_iTeamNum() != g_cl.m_local->m_iTeamNum();

		if (can_do_dmg
			&& is_player
			&& is_enemy
			&& hit_player
			&& hit_player->IsPlayer())
		{
			scale_dmg(hit_player, enter_trace, wpn_data, cur_dmg, static_cast <std::ptrdiff_t> (enter_trace.m_hitgroup));
			hitbox = static_cast <int> (enter_trace.m_hitbox);
			hit_group = static_cast<int> (enter_trace.m_hitgroup);
			return true;
		}

		if (!possible_hit_remain)
			break;

		static auto dmg_reduction_bullets = g_csgo.m_cvar->FindVar(HASH("ff_damage_reduction_bullets"));
		static auto dmg_bullet_pen = g_csgo.m_cvar->FindVar(HASH("ff_damage_bullet_penetration"));

		if (!handle_bullet_penetration(wpn_data, enter_trace, eye_pos, direction,
			possible_hit_remain, cur_dmg, pen_power, dmg_reduction_bullets->GetFloat(), dmg_bullet_pen->GetFloat())) {
			remaining_pen = possible_hit_remain;
			break;
		}

		remaining_pen = possible_hit_remain;

		visible = false;
	}

	return false;
}

bool c_auto_wall::trace_to_exit(
	const vec3_t& src, const vec3_t& dir,
	const CGameTrace& enter_trace, CGameTrace& exit_trace)
{
	static CTraceFilterSimple_game filter{};
	float dist{};
	int first_contents{};

	constexpr auto k_step_size = 4.f;
	constexpr auto k_max_dist = 90.f;

	while (dist <= k_max_dist) {
		dist += k_step_size;

		const auto out = src + (dir * dist);

		const auto cur_contents = g_csgo.m_engine_trace->GetPointContents(out, MASK_SHOT | CONTENTS_HITBOX);

		if (!first_contents)
			first_contents = cur_contents;

		if (cur_contents & MASK_SHOT_HULL
			&& (!(cur_contents & CONTENTS_HITBOX) || cur_contents == first_contents))
			continue;

		g_csgo.m_engine_trace->TraceRay({ out, out - dir * k_step_size }, MASK_SHOT, nullptr, &exit_trace);

		if (exit_trace.m_startsolid
			&& exit_trace.m_surface.m_flags & SURF_HITBOX) {
			filter.SetPassEntity(exit_trace.m_entity);

			g_csgo.m_engine_trace->TraceRay(Ray(out, src), MASK_SHOT_HULL, (ITraceFilter*)&filter, &exit_trace);

			if (exit_trace.hit() && !exit_trace.m_startsolid) {
				return true;
			}

			continue;
		}

		if (!exit_trace.hit()
			|| exit_trace.m_startsolid) {
			if (enter_trace.m_entity
				&& enter_trace.m_entity->index()
				&& game::IsBreakable(enter_trace.m_entity)) {
				exit_trace = enter_trace;
				exit_trace.m_endpos = src + dir;

				return true;
			}

			continue;
		}

		if (exit_trace.m_surface.m_flags & SURF_NODRAW) {
			if (game::IsBreakable(exit_trace.m_entity)
				&& game::IsBreakable(enter_trace.m_entity))
				return true;

			if (!(enter_trace.m_surface.m_flags & SURF_NODRAW))
				continue;
		}

		if (exit_trace.m_plane.m_normal.dot(dir) <= 1.f)
			return true;
	}

	return false;
}

__forceinline bool has_armor(Player* player, int hit_group) {
	const bool has_helmet = player->m_bHasHelmet();
	const bool has_heavy_armor = player->m_bHasHeavyArmor();
	const float armor_val = player->m_ArmorValue();

	if (armor_val > 0.f) {
		switch (hit_group) {
		case 2:
		case 3:
		case 4:
		case 5:
			return true;
			break;
		case 1:
			return has_helmet || has_heavy_armor;
			break;
		default:
			return has_heavy_armor;
			break;
		}
	}

	return false;
}
void c_auto_wall::scale_dmg(Player* player, CGameTrace& trace, WeaponInfo* wpn_info, float& dmg, const int hit_group) {
	if (!player->IsPlayer())
		return;

	const auto team = player->m_iTeamNum();

	static auto mp_dmg_scale_ct_head = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_ct_head"));
	static auto mp_dmg_scale_t_head = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_t_head"));

	static auto mp_dmg_scale_ct_body = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_ct_body"));
	static auto mp_dmg_scale_t_body = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_t_body"));

	float head_dmg_scale = team == 3 ? mp_dmg_scale_ct_head->GetFloat() : mp_dmg_scale_t_head->GetFloat();
	const float body_dmg_scale = 3 ? mp_dmg_scale_ct_body->GetFloat() : mp_dmg_scale_t_body->GetFloat();

	const auto armored = has_armor(player, hit_group);
	const bool has_heavy_armor = player->m_bHasHeavyArmor();
	const bool is_zeus = g_cl.m_local->GetActiveWeapon() ? g_cl.m_local->GetActiveWeapon()->m_iItemDefinitionIndex() == ZEUS : false;

	const auto armor_val = static_cast <float> (player->m_ArmorValue());

	if (has_heavy_armor)
		head_dmg_scale *= 0.5f;

	if (!is_zeus) {
		switch (hit_group) {
		case 1:
			dmg = (dmg * 4.f) * head_dmg_scale;
			break;
		case 3:
			dmg = (dmg * 1.25f) * body_dmg_scale;
			break;
		case 6:
		case 7:
			dmg = (dmg * 0.75f) * body_dmg_scale;
			break;
		default:
			break;
		}
	}

	if (!g_cl.m_local
		|| !g_cl.m_local->GetActiveWeapon()
		|| !g_cl.m_local->GetActiveWeapon()->GetWpnData())
		return;

	const auto armor_ratio = g_cl.m_local->GetActiveWeapon()->GetWpnData()->m_armor_ratio;

	if (armored) {
		float armor_scale = 1.f;
		float armor_bonus_ratio = 0.5f;
		float armor_ratio_calced = armor_ratio * 0.5f;
		float dmg_to_health = 0.f;

		if (has_heavy_armor) {
			armor_ratio_calced = armor_ratio * 0.25f;
			armor_bonus_ratio = 0.33f;
			armor_scale = 0.33f;

			dmg_to_health = (dmg * armor_ratio_calced) * 0.85f;
		}
		else
			dmg_to_health = dmg * armor_ratio_calced;

		float dmg_to_armor = (dmg - dmg_to_health) * (armor_scale * armor_bonus_ratio);

		if (dmg_to_armor > armor_val)
			dmg_to_health = dmg - (armor_val / armor_bonus_ratio);

		dmg = dmg_to_health;
	}

	dmg = std::floor(dmg);
}

float penetration::scale(Player* player, float damage, float armor_ratio, int hitgroup) {
	bool  has_heavy_armor;
	int   armor;
	float heavy_ratio, bonus_ratio, ratio, new_damage;

	static auto is_armored = [](Player* player, int armor, int hitgroup) {
		// the player has no armor.
		if (armor <= 0)
			return false;

		// if the hitgroup is head and the player has a helment, return true.
		// otherwise only return true if the hitgroup is not generic / legs / gear.
		if (hitgroup == HITGROUP_HEAD && player->m_bHasHelmet())
			return true;

		else if (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM)
			return true;

		return false;
		};

	// check if the player has heavy armor, this is only really used in operation stuff.
	has_heavy_armor = player->m_bHasHeavyArmor();

	// scale damage based on hitgroup.
	switch (hitgroup) {
	case HITGROUP_HEAD:
		if (has_heavy_armor)
			damage = (damage * 4.f) * 0.5f;
		else
			damage *= 4.f;
		break;

	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= 0.75f;
		break;

	default:
		break;
	}

	// grab amount of player armor.
	armor = player->m_ArmorValue();

	// check if the ent is armored and scale damage based on armor.
	if (is_armored(player, armor, hitgroup)) {
		heavy_ratio = 1.f;
		bonus_ratio = 0.5f;
		ratio = armor_ratio * 0.5f;

		// player has heavy armor.
		if (has_heavy_armor) {
			// calculate ratio values.
			bonus_ratio = 0.33f;
			ratio = armor_ratio * 0.25f;
			heavy_ratio = 0.33f;

			// calculate new damage.
			new_damage = (damage * ratio) * 0.85f;
		}

		// no heavy armor, do normal damage calculation.
		else
			new_damage = damage * ratio;

		if (((damage - new_damage) * (heavy_ratio * bonus_ratio)) > armor)
			new_damage = damage - (armor / bonus_ratio);

		damage = new_damage;
	}

	return std::floor(damage);
}

bool Player::IsArmored(int nHitGroup)
{
	bool bApplyArmor = false;

	if (m_ArmorValue() > 0)
	{
		switch (nHitGroup)
		{
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
			break;
		case HITGROUP_HEAD:
			if (m_bHasHelmet())
			{
				return true;
			}
			break;
		default:
			break;
		}
	}

	return false;
}

float Player::ScaleDamage(float flWpnArmorRatio, int group, float fDamage) {
	float flArmorBonus = 0.5f;
	float fDamageToHealth = fDamage;
	float fDamageToArmor = 0;
	float fHeavyArmorBonus = 1.0f;
	float flArmorRatio = 0.5f;
	flArmorRatio *= flWpnArmorRatio;

	if (!mp_damage_scale_ct_body)
		mp_damage_scale_ct_body = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_ct_body"));

	if (!mp_damage_scale_ct_head)
		mp_damage_scale_ct_head = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_ct_head"));

	if (!mp_damage_scale_t_body)
		mp_damage_scale_t_body = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_t_body"));

	if (!mp_damage_scale_t_head)
		mp_damage_scale_t_head = g_csgo.m_cvar->FindVar(HASH("mp_damage_scale_t_head"));

	float flBodyDamageScale = (m_iTeamNum() == TEAM_COUNTERTERRORISTS) ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();
	float flHeadDamageScale = (m_iTeamNum() == TEAM_COUNTERTERRORISTS) ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();

	if (m_bHasHeavyArmor()) {
		flArmorRatio *= 0.5f;
		flArmorBonus = 0.33f;
		fHeavyArmorBonus = 0.33f;

		// heavy armor reduces headshot damage by have of what it is, so it does x2 instead of x4
		flHeadDamageScale = flHeadDamageScale * 0.5f;
	}

	switch (group) {
	case HITGROUP_GENERIC:
		break;

	case HITGROUP_HEAD:
		fDamage *= 4.f;
		fDamage *= flHeadDamageScale;
		break;

	case HITGROUP_CHEST:
		fDamage *= 1.f;
		fDamage *= flBodyDamageScale;
		break;

	case HITGROUP_STOMACH:
		fDamage *= 1.25f;
		fDamage *= flBodyDamageScale;
		break;

	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		fDamage *= 1.0f;
		fDamage *= flBodyDamageScale;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		fDamage *= 0.75f;
		fDamage *= flBodyDamageScale;
		break;

	default:
		break;
	}

	// deal with Armour
	if (m_ArmorValue() > 0.f && IsArmored(group)) {
		fDamageToHealth = fDamage * flArmorRatio;
		fDamageToArmor = (fDamage - fDamageToHealth) * (flArmorBonus * fHeavyArmorBonus);

		int armorValue = m_ArmorValue();

		// Does this use more armor than we have?
		if (fDamageToArmor > armorValue)
			fDamageToHealth = fDamage - armorValue / flArmorBonus;

		return fDamageToHealth;
	}

	return fDamage;
}

bool penetration::TraceToExit(const vec3_t& start, const vec3_t& dir, vec3_t& out, CGameTrace* enter_trace, CGameTrace* exit_trace) {
	static CTraceFilterSimple_game filter{};

	float  dist{};
	vec3_t new_end;
	int    contents, first_contents{};

	// max pen distance is 90 units.
	while (dist <= 90.f) {
		// step forward a bit.
		dist += 4.f;

		// set out pos.
		out = start + (dir * dist);

		if (!first_contents)
			first_contents = g_csgo.m_engine_trace->GetPointContents(out, MASK_SHOT, nullptr);

		contents = g_csgo.m_engine_trace->GetPointContents(out, MASK_SHOT, nullptr);

		if ((contents & MASK_SHOT_HULL) && (!(contents & CONTENTS_HITBOX) || (contents == first_contents)))
			continue;

		// move end pos a bit for tracing.
		new_end = out - (dir * 4.f);

		// do first trace aHR0cHM6Ly9zdGVhbWNvbW11bml0eS5jb20vaWQvc2ltcGxlcmVhbGlzdGlj.
		g_csgo.m_engine_trace->TraceRay(Ray(out, new_end), MASK_SHOT, nullptr, exit_trace);

		// note - dex; this is some new stuff added sometime around late 2017 ( 10.31.2017 update? ).
		if (g_csgo.sv_clip_penetration_traces_to_players->GetInt())
			game::UTIL_ClipTraceToPlayers(out, new_end, MASK_SHOT, nullptr, exit_trace, -60.f);

		// we hit an ent's hitbox, do another trace.
		if (exit_trace->m_startsolid && (exit_trace->m_surface.m_flags & SURF_HITBOX)) {
			filter.SetPassEntity(exit_trace->m_entity);

			g_csgo.m_engine_trace->TraceRay(Ray(out, start), MASK_SHOT_HULL, (ITraceFilter*)&filter, exit_trace);

			if (exit_trace->hit() && !exit_trace->m_startsolid) {
				out = exit_trace->m_endpos;
				return true;
			}

			continue;
		}

		if (!exit_trace->hit() || exit_trace->m_startsolid) {
			if (game::IsBreakable(enter_trace->m_entity)) {
				*exit_trace = *enter_trace;
				exit_trace->m_endpos = start + dir;
				return true;
			}

			continue;
		}

		if ((exit_trace->m_surface.m_flags & SURF_NODRAW)) {
			// note - dex; ok, when this happens the game seems to not ignore world?
			if (game::IsBreakable(exit_trace->m_entity) && game::IsBreakable(enter_trace->m_entity)) {
				out = exit_trace->m_endpos;
				return true;
			}

			if (!(enter_trace->m_surface.m_flags & SURF_NODRAW))
				continue;
		}

		if (exit_trace->m_plane.m_normal.dot(dir) <= 1.f) {
			out -= (dir * (exit_trace->m_fraction * 4.f));
			return true;
		}
	}

	return false;
}

void penetration::ClipTraceToPlayer(const vec3_t& start, const vec3_t& end, uint32_t mask, CGameTrace* tr, Player* player, float min) {
	vec3_t     pos, to, dir, on_ray;
	float      len, range_along, range;
	Ray        ray;
	CGameTrace new_trace;

	// reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

	// set some local vars.
	pos = player->m_vecOrigin() + ((player->m_vecMins() + player->m_vecMaxs()) * 0.5f);
	to = pos - start;
	dir = start - end;
	len = dir.normalize();
	range_along = dir.dot(to);

	// off start point.
	if (range_along < 0.f)
		range = -(to).length();

	// off end point.
	else if (range_along > len)
		range = -(pos - end).length();

	// within ray bounds.
	else {
		on_ray = start + (dir * range_along);
		range = (pos - on_ray).length();
	}

	if ( /*min <= range &&*/ range <= 60.f) {
		// clip to player.
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), mask, player, &new_trace);

		if (tr->m_fraction > new_trace.m_fraction)
			*tr = new_trace;
	}
}

bool penetration::run(PenetrationInput_t* in, PenetrationOutput_t* out) {
	static CTraceFilterSkipTwoEntities_game filter{};

	int			  pen{ 4 }, enter_material, exit_material;
	float		  damage, penetration, penetration_mod, player_damage, remaining, trace_len{}, total_pen_mod, damage_mod, modifier, damage_lost;
	surfacedata_t* enter_surface, * exit_surface;
	bool		  nodraw, grate;
	vec3_t		  start, dir, end, pen_end;
	CGameTrace	  trace, exit_trace;
	Weapon* weapon;
	WeaponInfo* weapon_info;

	// if we are tracing from our local player perspective.
	if (in->m_from->m_bIsLocalPlayer()) {
		weapon = g_cl.m_weapon;
		weapon_info = g_cl.m_weapon_info;
		start = g_cl.m_shoot_pos;
	}

	// not local player.
	else {
		weapon = in->m_from->GetActiveWeapon();
		if (!weapon)
			return false;

		// get weapon info.
		weapon_info = weapon->GetWpnData();
		if (!weapon_info)
			return false;

		// set trace start.
		start = in->m_from->GetShootPosition();
	}

	// get some weapon data.
	damage = (float)weapon_info->m_damage;
	penetration = weapon_info->m_penetration;

	// used later in calculations.
	penetration_mod = std::max(0.f, (3.f / penetration) * 1.25f);

	// get direction to end point.
	dir = (in->m_pos - start).normalized();

	// setup trace filter for later.
	filter.SetPassEntity(in->m_from);
	filter.SetPassEntity2(nullptr);

	while (damage > 0.f) {
		// calculating remaining len.
		remaining = weapon_info->m_range - trace_len;

		// set trace end.
		end = start + (dir * remaining);

		// setup ray and trace.
		// TODO; use UTIL_TraceLineIgnoreTwoEntities?
		g_csgo.m_engine_trace->TraceRay(Ray(start, end), MASK_SHOT, (ITraceFilter*)&filter, &trace);

		// check for player hitboxes extending outside their collision bounds.
		// if no target is passed we clip the trace to a specific player, otherwise we clip the trace to any player.
		if (in->m_target)
			ClipTraceToPlayer(start, end + (dir * 40.f), MASK_SHOT, &trace, in->m_target, -60.f);

		else
			game::UTIL_ClipTraceToPlayers(start, end + (dir * 40.f), MASK_SHOT, (ITraceFilter*)&filter, &trace, -60.f);

		// we didn't hit anything.
		if (trace.m_fraction == 1.f)
			return false;

		// calculate damage based on the distance the bullet traveled.
		trace_len += trace.m_fraction * remaining;
		damage *= std::pow(weapon_info->m_range_modifier, trace_len / 500.f);

		// if a target was passed.
		if (in->m_target) {

			// validate that we hit the target we aimed for.
			if (trace.m_entity && trace.m_entity == in->m_target && game::IsValidHitgroup(trace.m_hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == ZEUS) ? HITBOX_BODY : trace.m_hitgroup;

				// scale damage based on the hitgroup we hit.
				player_damage = scale(in->m_target, damage, weapon_info->m_armor_ratio, group);

				// set result data for when we hit a player.
				out->m_pen = pen != 4;
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = in->m_target;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}
		}

		// no target was passed, check for any player hit or just get final damage done.
		else {
			out->m_pen = pen != 4;

			// todo - dex; team checks / other checks / etc.
			if (trace.m_entity && trace.m_entity->IsPlayer() && game::IsValidHitgroup(trace.m_hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == ZEUS) ? HITBOX_BODY : trace.m_hitgroup;

				player_damage = scale(trace.m_entity->as< Player* >(), damage, weapon_info->m_armor_ratio, group);

				// set result data for when we hit a player.
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = trace.m_entity->as< Player* >();

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}

			// if we've reached here then we didn't hit a player yet, set damage and hitgroup.
			out->m_damage = damage;
		}

		// don't run pen code if it's not wanted.
		if (!in->m_can_pen)
			return false;

		// get surface at entry point.
		enter_surface = g_csgo.m_phys_props->GetSurfaceData(trace.m_surface.m_surface_props);

		// this happens when we're too far away from a surface and can penetrate walls or the surface's pen modifier is too low.
		if ((trace_len > 3000.f && penetration) || enter_surface->m_game.m_penetration_modifier < 0.1f)
			return false;

		// store data about surface flags / contents.
		nodraw = (trace.m_surface.m_flags & SURF_NODRAW);
		grate = (trace.m_contents & CONTENTS_GRATE);

		// get material at entry point.
		enter_material = enter_surface->m_game.m_material;

		// note - dex; some extra stuff the game does.
		if (!pen && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
			return false;

		// no more pen.
		if (penetration <= 0.f || pen <= 0)
			return false;

		// try to penetrate object.
		if (!TraceToExit(trace.m_endpos, dir, pen_end, &trace, &exit_trace)) {
			if (!(g_csgo.m_engine_trace->GetPointContents(pen_end, MASK_SHOT_HULL) & MASK_SHOT_HULL))
				return false;
		}

		// get surface / material at exit point.
		exit_surface = g_csgo.m_phys_props->GetSurfaceData(exit_trace.m_surface.m_surface_props);
		exit_material = exit_surface->m_game.m_material;

		// todo - dex; check for CHAR_TEX_FLESH and ff_damage_bullet_penetration / ff_damage_reduction_bullets convars?
		//             also need to check !isbasecombatweapon too.
		if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS) {
			total_pen_mod = 3.f;
			damage_mod = 0.05f;
		}

		else if (nodraw || grate) {
			total_pen_mod = 1.f;
			damage_mod = 0.16f;
		}

		else {
			total_pen_mod = (enter_surface->m_game.m_penetration_modifier + exit_surface->m_game.m_penetration_modifier) * 0.5f;
			damage_mod = 0.16f;
		}

		// thin metals, wood and plastic get a penetration bonus.
		if (enter_material == exit_material) {
			if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
				total_pen_mod = 3.f;

			else if (exit_material == CHAR_TEX_PLASTIC)
				total_pen_mod = 2.f;
		}

		// set some local vars.
		trace_len = (exit_trace.m_endpos - trace.m_endpos).length();
		modifier = std::max(0.f, 1.f / total_pen_mod);
		damage_lost = ((modifier * 3.f) * penetration_mod + (damage * damage_mod)) + (((trace_len * trace_len) * modifier) / 24.f);


		damage -= std::max(0.f, damage_lost);
		if (damage < 1.f)
			return false;

		// set new start pos for successive trace.
		start = exit_trace.m_endpos;

		// decrement pen.
		--pen;
	}

	return false;
}


bool penetration::run_modified(PenetrationInput_t* in, PenetrationOutput_t* out) {
	static CTraceFilterSkipTwoEntities_game filter{};

	int			  pen{ 4 }, enter_material, exit_material;
	float		  damage, penetration, penetration_mod, player_damage, remaining, trace_len{}, total_pen_mod, damage_mod, modifier, damage_lost;
	surfacedata_t* enter_surface, * exit_surface;
	bool		  nodraw, grate;
	vec3_t		  start, dir, end, pen_end;
	CGameTrace	  trace, exit_trace;
	Weapon* weapon;
	WeaponInfo* weapon_info;

	bool canpen = false;

	//if (!in->m_from->alive() || !in->m_notargetbutstillignore->alive())
	//	return false;

	// if we are tracing from our local player perspective.
	if (in->m_from->m_bIsLocalPlayer()) {
		weapon = g_cl.m_weapon;
		weapon_info = g_cl.m_weapon_info;
		start = g_cl.m_shoot_pos;
	}

	// not local player.
	else {
		weapon = in->m_from->GetActiveWeapon();
		if (!weapon)
			return false;

		// get weapon info.
		weapon_info = weapon->GetWpnData();
		if (!weapon_info)
			return false;

		// set trace start.
		start = in->start;
	}

	// get some weapon data.
	damage = (float)weapon_info->m_damage;
	penetration = weapon_info->m_penetration;

	// used later in calculations.
	penetration_mod = std::max(0.f, (3.f / penetration) * 1.25f);

	// get direction to end point.
	dir = (in->m_pos - start).normalized();

	// setup trace filter for later.
	filter.SetPassEntity(in->m_from);
	filter.SetPassEntity2(g_cl.m_local);

	while (damage > 0.f) {
		// calculating remaining len.
		remaining = weapon_info->m_range - trace_len;

		// set trace end.
		end = start + (dir * remaining);

		// setup ray and trace.
		// TODO; use UTIL_TraceLineIgnoreTwoEntities?
		g_csgo.m_engine_trace->TraceRay(Ray(start, end), MASK_SHOT, (ITraceFilter*)&filter, &trace);

		// check for player hitboxes extending outside their collision bounds.
		// if no target is passed we clip the trace to a specific player, otherwise we clip the trace to any player.
		if (in->m_target)
			ClipTraceToPlayer(start, end + (dir * 40.f), MASK_SHOT, &trace, in->m_target, -60.f);

		else
			game::UTIL_ClipTraceToPlayers(start, end + (dir * 40.f), MASK_SHOT, (ITraceFilter*)&filter, &trace, -60.f);

		// we didn't hit anything.
	//	if (trace.m_fraction == 1.f)
			//return false;

		// calculate damage based on the distance the bullet traveled.
		trace_len += trace.m_fraction * remaining;
		damage *= std::pow(weapon_info->m_range_modifier, trace_len / 500.f);

		// if a target was passed.
		if (in->m_target) {

			// validate that we hit the target we aimed for.
			if (trace.m_entity && trace.m_entity == in->m_target && game::IsValidHitgroup(trace.m_hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == ZEUS) ? HITBOX_BODY : trace.m_hitgroup;

				// scale damage based on the hitgroup we hit.
				player_damage = scale(in->m_target, damage, weapon_info->m_armor_ratio, group);

				// set result data for when we hit a player.
				out->m_pen = pen != 4;
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = in->m_target;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}
		}

		// no target was passed, check for any player hit or just get final damage done.
		else {
			//g_csgo.m_debug_overlay->AddLineOverlay(start, end, 255, 0, 0, true, 0.1f);
			out->m_pen = pen != 4;

			// todo - dex; team checks / other checks / etc.
			if (trace.m_entity && trace.m_entity->IsPlayer() && game::IsValidHitgroup(trace.m_hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == ZEUS) ? HITBOX_BODY : trace.m_hitgroup;

				player_damage = scale(trace.m_entity->as< Player* >(), damage, weapon_info->m_armor_ratio, group);
				// set result data for when we hit a player.
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = trace.m_entity->as< Player* >();

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}

			// if we've reached here then we didn't hit a player yet, set damage and hitgroup.
			player_damage = scale(g_cl.m_local, damage, weapon_info->m_armor_ratio, HITGROUP_HEAD); //scale damage based on us
			out->m_damage = player_damage;
		}

		if (trace_len > (in->m_pos - start).length()) {
			return true;
		}

		// don't run pen code if it's not wanted.
		if (!in->m_can_pen)
			return false;

		// get surface at entry point.
		enter_surface = g_csgo.m_phys_props->GetSurfaceData(trace.m_surface.m_surface_props);

		// this happens when we're too far away from a surface and can penetrate walls or the surface's pen modifier is too low.
		if ((trace_len > 3000.f && penetration) || enter_surface->m_game.m_penetration_modifier < 0.1f)
			return false;

		// store data about surface flags / contents.
		nodraw = (trace.m_surface.m_flags & SURF_NODRAW);
		grate = (trace.m_contents & CONTENTS_GRATE);

		// get material at entry point.
		enter_material = enter_surface->m_game.m_material;

		// note - dex; some extra stuff the game does.
		if (!pen && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
			return false;

		// no more pen.
		if (penetration <= 0.f || pen <= 0)
			return false;

		// try to penetrate object.
		if (!TraceToExit(trace.m_endpos, dir, pen_end, &trace, &exit_trace)) {
			if (!(g_csgo.m_engine_trace->GetPointContents(pen_end, MASK_SHOT_HULL) & MASK_SHOT_HULL))
				return false;
		}

		// get surface / material at exit point.
		exit_surface = g_csgo.m_phys_props->GetSurfaceData(exit_trace.m_surface.m_surface_props);
		exit_material = exit_surface->m_game.m_material;

		// todo - dex; check for CHAR_TEX_FLESH and ff_damage_bullet_penetration / ff_damage_reduction_bullets convars?
		//             also need to check !isbasecombatweapon too.
		if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS) {
			total_pen_mod = 3.f;
			damage_mod = 0.05f;
		}

		else if (nodraw || grate) {
			total_pen_mod = 1.f;
			damage_mod = 0.16f;
		}

		else {
			total_pen_mod = (enter_surface->m_game.m_penetration_modifier + exit_surface->m_game.m_penetration_modifier) * 0.5f;
			damage_mod = 0.16f;
		}

		// thin metals, wood and plastic get a penetration bonus.
		if (enter_material == exit_material) {
			if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
				total_pen_mod = 3.f;

			else if (exit_material == CHAR_TEX_PLASTIC)
				total_pen_mod = 2.f;
		}

		// set some local vars.
		trace_len = (exit_trace.m_endpos - trace.m_endpos).length();
		modifier = std::max(0.f, 1.f / total_pen_mod);
		damage_lost = ((modifier * 3.f) * penetration_mod + (damage * damage_mod)) + (((trace_len * trace_len) * modifier) / 24.f);

		// subtract from damage.
		damage -= std::max(0.f, damage_lost);
		if (damage < 1.f) {
			return false;
		}
		else {
			// set new start pos for successive trace.
			start = exit_trace.m_endpos;

			// decrement pen.
			--pen;
			canpen = true;
		}
	}
	if (canpen)
		return true;

	return false;
}