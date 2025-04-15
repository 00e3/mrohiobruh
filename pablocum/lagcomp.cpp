#include "includes.h"

LagCompensation g_lagcomp{};;

bool LagCompensation::ValidRecord(float simtime) {
	const float dead_time = std::floorf(game::TICKS_TO_TIME(g_cl.m_arrival_tick) - g_csgo.sv_maxunlag->GetFloat());
	if (simtime < dead_time)
		return false;

	// use prediction curtime for this.
	const float curtime = g_cl.m_local && g_cl.m_local->alive() ? g_cl.m_curtime : g_csgo.m_globals->m_curtime;

	// correct is the amount of time we have to correct game time,
	float correct = g_cl.m_lerp;

	// stupid fake latency goes into the incoming latency.
	if (g_csgo.m_net) {
		correct += g_csgo.m_net->GetLatency(INetChannel::FLOW_OUTGOING);
		correct += g_csgo.m_net->GetLatency(INetChannel::FLOW_INCOMING);
	}

	// check bounds [ 0, sv_maxunlag ]
	math::clamp(correct, 0.f, g_csgo.sv_maxunlag->GetFloat());

	// calculate difference between tick sent by player and our latency based tick.
	// ensure this record isn't too old.
	return std::fabs(correct - (curtime - simtime)) <= 0.2f;
}

LagRecord* LagCompensation::StartPrediction(AimPlayer* data) {
	if (data->m_records.size() <= 1)
		return nullptr;

	LagRecord* current = &data->m_records[0];

	if (!current)
		return nullptr;

	if (current->immune())
		return nullptr;

	if (current->m_record_flags & RecordFlags::SHIFTING)
		return nullptr;

	if (!(current->m_record_flags & RecordFlags::BREAKINGLC))
		return nullptr;

	if (!current->valid_origin_time(true))
		return current;

	return nullptr;
}

void LagCompensation::PlayerMove(LagRecord* record) {
	vec3_t                start, end, normal;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// define trace start.
	start = record->m_origin;

	// move trace end one tick into the future using predicted velocity.
	end = start + (record->m_velocity * g_csgo.m_globals->m_interval);

	// trace.
	g_csgo.m_engine_trace->TraceRay(Ray(start, end, record->m_mins, record->m_maxs), CONTENTS_SOLID, &filter, &trace);

	// we hit shit
	// we need to fix hit.
	if (trace.m_fraction != 1.f) {

		// fix sliding on planes.
		for (int i{}; i < 2; ++i) {
			record->m_velocity -= trace.m_plane.m_normal * record->m_velocity.dot(trace.m_plane.m_normal);

			float adjust = record->m_velocity.dot(trace.m_plane.m_normal);
			if (adjust < 0.f)
				record->m_velocity -= (trace.m_plane.m_normal * adjust);

			start = trace.m_endpos;
			end = start + (record->m_velocity * (g_csgo.m_globals->m_interval * (1.f - trace.m_fraction)));

			g_csgo.m_engine_trace->TraceRay(Ray(start, end, record->m_mins, record->m_maxs), CONTENTS_SOLID, &filter, &trace);
			if (trace.m_fraction == 1.f)
				break;
		}
	}

	// set new final origin.
	start = end = record->m_origin = trace.m_endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	g_csgo.m_engine_trace->TraceRay(Ray(start, end, record->m_mins, record->m_maxs), CONTENTS_SOLID, &filter, &trace);

	// strip onground flag.
	record->m_flags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f)
		record->m_flags |= FL_ONGROUND;
}