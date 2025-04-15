#pragma once

class ShotRecord;

struct OverrideAngle
{
	float m_yaw;
	vec3_t m_start, m_end;
	Player* m_player;
};

class Resolver
{
public:

	OverrideAngle m_override;

	float GetAwayAngle( Player* record );

	float AutoDirection( LagRecord& record, float multiplier );
	void MatchShot( AimPlayer* data, LagRecord& record, LagRecord* previous );

	void ResolveMove(AimPlayer* data, LagRecord* current, LagRecord* previous);

	void Override( );
	void ResolveDesync(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void OnBodyUpdate( AimPlayer* data, LagRecord& record, LagRecord* previous );
	void SetResolverMode( AimPlayer* data, LagRecord& record, LagRecord* previous, const int& act );
	void ResolveAngles( Player* player, LagRecord& record, LagRecord* previous );
	void ResolveStand( AimPlayer* data, LagRecord& record, LagRecord* previous, const int& act );
	void StandNS( AimPlayer* data, LagRecord& record );
	void ResolveAir( AimPlayer* data, LagRecord& record, LagRecord* previous, CCSGOPlayerAnimState* state );
	void CorrectFootYaw( LagRecord& record, LagRecord* previous, CCSGOPlayerAnimState* state );
	//void ResolveFootYaw(Player* player, LagRecord& record);
	//float MaxFootDelta(serveranimation_data_t* data);

	void AirNS( AimPlayer* data, LagRecord& record );

	void ResetMisses( int index );
public:
	std::array< vec3_t, 64 > m_impacts;
};

extern Resolver g_resolver;