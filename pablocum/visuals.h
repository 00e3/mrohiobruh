#pragma once

struct ImpactData_t {
    vec3_t pos = vec3_t(0.f, 0.f, 0.f);
    float time;
    bool handled = false;
};

struct OffScreenDamageData_t {
    float m_time, m_color_step;
    Color m_color;

    __forceinline OffScreenDamageData_t( ) : m_time{ 0.f }, m_color{ colors::white } {}
    __forceinline OffScreenDamageData_t( float time, float m_color_step, Color color ) : m_time{ time }, m_color{ color } {}
};

struct ResolveInfo {
    std::string label;
    Color color;
};
class Visuals {
public:
	std::array< std::pair<bool, Rect>, 64 > m_box;
	std::array< float, 64 >                 m_alive_time;
	std::array< float, 2048 >               m_opacities;
    std::array< OffScreenDamageData_t, 64 > m_offscreen_damage;
	vec2_t                                  m_crosshair;
	bool                                    m_thirdperson;

    // info about planted c4.
    bool        m_c4_planted;
    Entity      *m_planted_c4;
    float       m_planted_c4_explode_time;
    vec3_t      m_planted_c4_explosion_origin;
    float       m_planted_c4_damage;
    float       m_planted_c4_radius;
    float       m_planted_c4_radius_scaled;
    std::string m_last_bombsite;

	IMaterial* smoke1;
	IMaterial* smoke2;
	IMaterial* smoke3;
	IMaterial* smoke4;

    std::unordered_map< int, char > m_weapon_icons = {
    	{ DEAGLE, 'F' },
    	{ ELITE, 'S' },
    	{ FIVESEVEN, 'U' },
    	{ GLOCK, 'C' },
    	{ AK47, 'B' },
    	{ AUG, 'E' },
    	{ AWP, 'R' },
    	{ FAMAS, 'T' },
    	{ G3SG1, 'I' },
    	{ GALIL, 'V' },
    	{ M249, 'Z' },
    	{ M4A4, 'W' },
    	{ MAC10, 'L' },
    	{ P90, 'M' },
    	{ UMP45, 'Q' },
    	{ XM1014, ']' },
    	{ BIZON, 'D' },
    	{ MAG7, 'K' },
    	{ NEGEV, 'Z' },
    	{ SAWEDOFF, 'K' },
    	{ TEC9, 'C' },
    	{ ZEUS, 'Y' },
    	{ P2000, 'Y' },
    	{ MP7, 'X' },
    	{ MP9, 'D' },
    	{ NOVA, 'K' },
    	{ P250, 'Y' },
    	{ SCAR20, 'I' },
    	{ SG553, '[' },
    	{ SSG08, 'N' },
    	{ KNIFE_CT, 'J' },
    	{ FLASHBANG, 'G' },
    	{ HEGRENADE, 'H' },
    	{ SMOKE, 'P' },
    	{ MOLOTOV, 'H' },
    	{ DECOY, 'G' },
    	{ FIREBOMB, 'H' },
    	{ C4, '\\' },
    	{ KNIFE_T, 'J' },
    	{ M4A1S, 'W' },
    	{ USPS, 'Y' },
    	{ CZ75A, 'Y' },
    	{ REVOLVER, 'F' },
    	{ KNIFE_BAYONET, 'J' },
    	{ KNIFE_FLIP, 'J' },
    	{ KNIFE_GUT, 'J' },
    	{ KNIFE_KARAMBIT, 'J' },
    	{ KNIFE_M9_BAYONET, 'J' },
    	{ KNIFE_HUNTSMAN, 'J' },
    	{ KNIFE_FALCHION, 'J' },
    	{ KNIFE_BOWIE, 'J' },
    	{ KNIFE_BUTTERFLY, 'J' },
    	{ KNIFE_SHADOW_DAGGERS, 'J' },
    };

public:

    // In visuals.h
    ResolveInfo ResolveConfidence(const int& index);

    render::Font& GetEspFont(bool small_font);

    static void ModulateWorld( );
    static void Skybox( );

	void PlayerAngleLines( );
	void RenderOverrideArrow( );
	void ThirdpersonThink( );
	void Hitmarker( );
	void NoSmoke( );
	void think( );
	void Spectators( );
	void ClientImpacts( );
	void StatusIndicators( );
	void SpreadCrosshair( );
    void PenetrationCrosshair( );
    void draw(Entity* ent);
    void DrawPlantedC4();
	void DrawProjectile( Weapon* ent );
	void DrawItem( Weapon* item );
	void OffScreen( Player* player, int alpha );
	void DrawPlayer( Player* player );
	bool GetPlayerBoxRect( Player* player, Rect& box );
	void DrawHistorySkeleton( Player* player, float opacity );
	void DrawSkeleton( Player* player, float opacity );
	void RenderGlow( );
	void DrawHitboxMatrix( LagRecord* record, Color col, float time );
    void DrawBeams( );
	void DebugAimbotPoints( Player* player );
};
extern Visuals g_visuals;