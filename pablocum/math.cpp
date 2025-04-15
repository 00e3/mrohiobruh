#include "includes.h"

matrix3x4_t math::QuaternionMatrix( const quaternion_t& q )
{
	matrix3x4_t result{ };

	result[ 0 ][ 0 ] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
	result[ 1 ][ 0 ] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
	result[ 2 ][ 0 ] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;

	result[ 0 ][ 1 ] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
	result[ 1 ][ 1 ] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
	result[ 2 ][ 1 ] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

	result[ 0 ][ 2 ] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
	result[ 1 ][ 2 ] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
	result[ 2 ][ 2 ] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

	result[ 0 ][ 3 ] = 0.0f;
	result[ 1 ][ 3 ] = 0.0f;
	result[ 2 ][ 3 ] = 0.0f;
	return result;
}

matrix3x4_t math::QuaternionMatrix( const quaternion_t& q, const vec3_t& pos )
{
	matrix3x4_t result{ };

	result = QuaternionMatrix( q );

	result[ 0 ][ 3 ] = pos.x;
	result[ 1 ][ 3 ] = pos.y;
	result[ 2 ][ 3 ] = pos.z;
	return result;
}

vec3_t math::CalcAngle(const vec3_t& vecSource, const vec3_t& vecDestination) {
	vec3_t vAngle;
	vec3_t delta((vecSource.x - vecDestination.x), (vecSource.y - vecDestination.y), (vecSource.z - vecDestination.z));
	double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

	vAngle.x = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
	vAngle.y = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
	vAngle.z = 0.0f;

	if (delta.x >= 0.0)
		vAngle.y += 180.0f;

	return vAngle;
}

float math::SegmentToSegment(const vec3_t& s1, const vec3_t& s2, const vec3_t& k1, const vec3_t& k2) {
	static auto constexpr epsilon = 0.00000001;

	auto u = s2 - s1;
	auto v = k2 - k1;
	const auto w = s1 - k1;

	const auto a = u.dot(u);
	const auto b = u.dot(v);
	const auto c = v.dot(v);
	const auto d = u.dot(w);
	const auto e = v.dot(w);
	const auto D = a * c - b * b;
	float sn, sd = D;
	float tn, td = D;

	if (D < epsilon)
	{
		sn = 0.0;
		sd = 1.0;
		tn = e;
		td = c;
	}
	else
	{
		sn = b * e - c * d;
		tn = a * e - b * d;

		if (sn < 0.0)
		{
			sn = 0.0;
			tn = e;
			td = c;
		}
		else if (sn > sd)
		{
			sn = sd;
			tn = e + b;
			td = c;
		}
	}

	if (tn < 0.0)
	{
		tn = 0.0;

		if (-d < 0.0)
			sn = 0.0;
		else if (-d > a)
			sn = sd;
		else
		{
			sn = -d;
			sd = a;
		}
	}
	else if (tn > td)
	{
		tn = td;

		if (-d + b < 0.0)
			sn = 0;
		else if (-d + b > a)
			sn = sd;
		else
		{
			sn = -d + b;
			sd = a;
		}
	}

	const float sc = abs(sn) < epsilon ? 0.0 : sn / sd;
	const float tc = abs(tn) < epsilon ? 0.0 : tn / td;

	m128 n;
	auto dp = w + u * sc - v * tc;
	n.f[0] = dp.dot(dp);
	const auto calc = sqrt_ps(n.v);
	return reinterpret_cast<const m128*>(&calc)->f[0];
}

bool math::IntersectBBHitbox(const vec3_t& start, const vec3_t& direction, const vec3_t& min, const vec3_t& max) {
	// Calculate the inverse direction to avoid division during comparisons
	vec3_t invDir = { 1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z };

	// Compute intersection t-values for each pair of parallel planes on x, y, and z
	float t1 = (min.x - start.x) * invDir.x;
	float t2 = (max.x - start.x) * invDir.x;
	float t3 = (min.y - start.y) * invDir.y;
	float t4 = (max.y - start.y) * invDir.y;
	float t5 = (min.z - start.z) * invDir.z;
	float t6 = (max.z - start.z) * invDir.z;

	// Find the farthest entry point and the closest exit point
	float t_min = std::max({ std::min(t1, t2), std::min(t3, t4), std::min(t5, t6) });
	float t_max = std::min({ std::max(t1, t2), std::max(t3, t4), std::max(t5, t6) });

	// If the farthest entry is before the closest exit, we have an intersection
	if (t_max < 0 || t_min > t_max) {
		return false;
	}

	// Ray intersects the bounding box
	return true;
}

std::vector<int>& math::GetAllHitboxesInHitboxHitgroup(const int hitbox) {
	static std::vector<int> hitboxes;
	hitboxes.clear();

	switch (hitbox) {
	case HITBOX_HEAD:
	case HITBOX_NECK:
		hitboxes.push_back(HITBOX_HEAD);
		hitboxes.push_back(HITBOX_NECK);
		break;
	case HITBOX_PELVIS:
	case HITBOX_BODY:
		hitboxes.push_back(HITBOX_PELVIS);
		hitboxes.push_back(HITBOX_BODY);
		break;
	case HITBOX_LOWER_NECK:
	case HITBOX_THORAX:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		hitboxes.push_back(HITBOX_LOWER_NECK);
		hitboxes.push_back(HITBOX_THORAX);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
		break;
	case HITBOX_L_UPPER_ARM:
	case HITBOX_L_FOREARM:
	case HITBOX_L_HAND:
		hitboxes.push_back(HITBOX_L_UPPER_ARM);
		hitboxes.push_back(HITBOX_L_FOREARM);
		hitboxes.push_back(HITBOX_L_HAND);
		break;
	case HITBOX_R_UPPER_ARM:
	case HITBOX_R_FOREARM:
	case HITBOX_R_HAND:
		hitboxes.push_back(HITBOX_R_UPPER_ARM);
		hitboxes.push_back(HITBOX_R_FOREARM);
		hitboxes.push_back(HITBOX_R_HAND);
		break;

	case HITBOX_L_THIGH:
	case HITBOX_L_CALF:
	case HITBOX_L_FOOT:
		hitboxes.push_back(HITBOX_L_THIGH);
		hitboxes.push_back(HITBOX_L_CALF);
		hitboxes.push_back(HITBOX_L_FOOT);
		break;

	case HITBOX_R_THIGH:
	case HITBOX_R_CALF:
	case HITBOX_R_FOOT:
		hitboxes.push_back(HITBOX_R_THIGH);
		hitboxes.push_back(HITBOX_R_CALF);
		hitboxes.push_back(HITBOX_R_FOOT);
		break;
	}

	return hitboxes;
}

int math::HitboxToHitgroup(const int hitbox) {
	switch (hitbox) {
	case HITBOX_HEAD:
	case HITBOX_NECK:
		return HITGROUP_HEAD;
	case HITBOX_PELVIS:
	case HITBOX_BODY:
		return HITGROUP_STOMACH;
	case HITBOX_LOWER_NECK:
	case HITBOX_THORAX:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		return HITGROUP_CHEST;
	case HITBOX_L_UPPER_ARM:
	case HITBOX_L_FOREARM:
	case HITBOX_L_HAND:
		return HITGROUP_LEFTARM;
	case HITBOX_R_UPPER_ARM:
	case HITBOX_R_FOREARM:
	case HITBOX_R_HAND:
		return HITGROUP_RIGHTARM;
	case HITBOX_L_THIGH:
	case HITBOX_L_CALF:
	case HITBOX_L_FOOT:
		return HITGROUP_LEFTLEG;
	case HITBOX_R_THIGH:
	case HITBOX_R_CALF:
	case HITBOX_R_FOOT:
		return HITGROUP_RIGHTLEG;
	}

	return HITGROUP_GENERIC;
}

bool math::RayIntersectHitbox(Player* player, const vec3_t& start, const vec3_t& end, matrix3x4_t* bone_matrix, const int& hitbox, bool scan_hitgroup) {
	const model_t* model = player->GetModel();
	if (!model)
		return false;

	const studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	const mstudiohitboxset_t* set = hdr->GetHitboxSet(player->m_nHitboxSet());
	if (!set)
		return false;

	static std::vector<int> hitboxes;
	if (!scan_hitgroup) {
		hitboxes.clear();
		hitboxes.emplace_back(hitbox);
	}
	else
		hitboxes = GetAllHitboxesInHitboxHitgroup(hitbox);

	for (const int& hb : hitboxes) {
		mstudiobbox_t* studio_box = set->GetHitbox(hitbox);
		if (!studio_box)
			continue;

		static vec3_t min{ }, max{ }, start_scaled{ }, delta{ };

		if (studio_box->m_radius > 0.f) {
			VectorTransform(studio_box->m_mins, bone_matrix[studio_box->m_bone], min);
			VectorTransform(studio_box->m_maxs, bone_matrix[studio_box->m_bone], max);

			if (SegmentToSegment(start, end, min, max) <= studio_box->m_radius) {
				//g_csgo.m_debug_overlay->AddLineOverlay(start, end, 255, 255, 255, true, 0.1f);
				return true;
			}
		}
		/* we will assume boxes are safe since we do not have a method of intersecting them */
		else {
			static matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(studio_box->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			static matrix3x4_t matrix;
			math::ConcatTransforms(bone_matrix[studio_box->m_bone], rot_matrix, matrix);

			min = { studio_box->m_mins.dot(matrix[0]), studio_box->m_mins.dot(matrix[1]), studio_box->m_mins.dot(matrix[2]) };
			min += matrix.GetOrigin();

			max = { studio_box->m_maxs.dot(matrix[0]), studio_box->m_maxs.dot(matrix[1]), studio_box->m_maxs.dot(matrix[2]) };
			max += matrix.GetOrigin();

			if (IntersectBBHitbox(start, end - start, min, max))
				return true;
		}
	}

	return false;
}

vec3_t math::VectorRotate(vec3_t in1, matrix3x4_t in2)
{
	return { in1.dot(in2[0]), in1.dot(in2[1]), in1.dot(in2[2]) };
}
//--------------------------------------------------------------------------------
vec3_t math::VectorRotate(const vec3_t& in1, const ang_t& in2)
{
	matrix3x4_t mat;
	math::AngleMatrix(in2, in1, mat);
	return VectorRotate(in1, mat);
}

float math::AngleDiff(float destAngle, float srcAngle) {
	float delta = fmodf(destAngle - srcAngle, 360.0f);
	if (destAngle > srcAngle)
	{
		if (delta >= 180)
			delta -= 360;
	}
	else
	{
		if (delta <= -180)
			delta += 360;
	}

	return delta;
}

vec3_t math::Approach(vec3_t target, vec3_t value, float speed)
{
	vec3_t diff = (target - value);
	float delta = diff.length();

	if (delta > speed)
		value += diff.normalized() * speed;
	else if (delta < -speed)
		value -= diff.normalized() * speed;
	else
		value = target;

	return value;
}


float math::Approach(float target, float value, float speed) {
	float delta = target - value;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

float math::RemapValClamped(float val, float A, float B, float C, float D)
{
	if (A == B)
		return fsel(val - B, D, C);

	float cVal = (val - A) / (B - A);
	cVal = std::clamp<float>(cVal, 0.0f, 1.0f);

	return C + (D - C) * cVal;
}

float math::ReverseRemapValClamped(float result, float A, float B, float C, float D)
{
	if (C == D) // Handle edge case where the range [C, D] is invalid
		return fsel(result - D, B, A);

	// Inverse of the remapping process
	float cVal = (result - C) / (D - C); // Normalize result into [0, 1] of the second range
	cVal = std::clamp<float>(cVal, 0.0f, 1.0f); // Clamp to ensure valid output
	return A + (B - A) * cVal; // Map back to the original range [A, B]
}

float math::SimpleSplineRemapValClamped(float val, float A, float B, float C, float D)
{
	if (A == B)
		return val >= B ? D : C;
	float cVal = (val - A) / (B - A);
	cVal = std::clamp(cVal, 0.0f, 1.0f);
	return C + (D - C) * SimpleSpline(cVal);
}


void math::AngleMatrix(const ang_t& ang, const vec3_t& pos, matrix3x4_t& out) {
	g_csgo.AngleMatrix(ang, out);
	out.SetOrigin(pos);
}

void math::NormalizeAngle(float& angle) {
	float rot;

	// bad number.
	if (!std::isfinite(angle)) {
		angle = 0.f;
		return;
	}

	// no need to normalize this angle.
	if (angle >= -180.f && angle <= 180.f)
		return;

	// get amount of rotations needed.
	rot = std::round(std::abs(angle / 360.f));

	// normalize.
	angle = (angle < 0.f) ? angle + (360.f * rot) : angle - (360.f * rot);
}

float math::ApproachAngle(float target, float value, float speed) {
	float delta;

	target = AngleMod(target);
	value = AngleMod(value);
	delta = target - value;

	// speed is assumed to be positive.
	speed = std::abs(speed);

	math::NormalizeAngle(delta);

	if (delta > speed)
		value += speed;

	else if (delta < -speed)
		value -= speed;

	else
		value = target;

	return value;
}

void math::VectorAngles(const vec3_t& forward, ang_t& angles, vec3_t* up) {
	vec3_t  left;
	float   len, up_z, pitch, yaw, roll;

	// get 2d length.
	len = forward.length_2d();

	if (up && len > 0.001f) {
		pitch = rad_to_deg(std::atan2(-forward.z, len));
		yaw = rad_to_deg(std::atan2(forward.y, forward.x));

		// get left direction vector using cross product.
		left = (*up).cross(forward).normalized();

		// calculate up_z.
		up_z = (left.y * forward.x) - (left.x * forward.y);

		// calculate roll.
		roll = rad_to_deg(std::atan2(left.z, up_z));
	}

	else {
		if (len > 0.f) {
			// calculate pitch and yaw.
			pitch = rad_to_deg(std::atan2(-forward.z, len));
			yaw = rad_to_deg(std::atan2(forward.y, forward.x));
			roll = 0.f;
		}

		else {
			pitch = (forward.z > 0.f) ? -90.f : 90.f;
			yaw = 0.f;
			roll = 0.f;
		}
	}

	// set out angles.
	angles = { pitch, yaw, roll };
}

void math::AngleVectors(const ang_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up) {
	float cp = std::cos(deg_to_rad(angles.x)), sp = std::sin(deg_to_rad(angles.x));
	float cy = std::cos(deg_to_rad(angles.y)), sy = std::sin(deg_to_rad(angles.y));
	float cr = std::cos(deg_to_rad(angles.z)), sr = std::sin(deg_to_rad(angles.z));

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
		right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
		right->z = -1.f * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

float math::GetFOV(const ang_t& view_angles, const vec3_t& start, const vec3_t& end) {
	vec3_t dir, fw;

	// get direction and normalize.
	dir = (end - start).normalized();

	// get the forward direction vector of the view angles.
	AngleVectors(view_angles, &fw);

	// get the angle between the view angles forward directional vector and the target location.
	return std::max(rad_to_deg(std::acos(fw.dot(dir))), 0.f);
}

void math::VectorTransform(const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out) {
	out = {
		in.dot(vec3_t(matrix[0][0], matrix[0][1], matrix[0][2])) + matrix[0][3],
		in.dot(vec3_t(matrix[1][0], matrix[1][1], matrix[1][2])) + matrix[1][3],
		in.dot(vec3_t(matrix[2][0], matrix[2][1], matrix[2][2])) + matrix[2][3]
	};
}

void math::VectorITransform(const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out) {
	vec3_t diff;

	diff = {
		in.x - matrix[0][3],
		in.y - matrix[1][3],
		in.z - matrix[2][3]
	};

	out = {
		diff.x * matrix[0][0] + diff.y * matrix[1][0] + diff.z * matrix[2][0],
		diff.x * matrix[0][1] + diff.y * matrix[1][1] + diff.z * matrix[2][1],
		diff.x * matrix[0][2] + diff.y * matrix[1][2] + diff.z * matrix[2][2]
	};
}

void math::MatrixAngles(const matrix3x4_t& matrix, ang_t& angles) {
	vec3_t forward, left, up;

	// extract the basis vectors from the matrix. since we only need the z
	// component of the up vector, we don't get x and y.
	forward = { matrix[0][0], matrix[1][0], matrix[2][0] };
	left = { matrix[0][1], matrix[1][1], matrix[2][1] };
	up = { 0.f, 0.f, matrix[2][2] };

	float len = forward.length_2d();

	// enough here to get angles?
	if (len > 0.001f) {
		angles.x = rad_to_deg(std::atan2(-forward.z, len));
		angles.y = rad_to_deg(std::atan2(forward.y, forward.x));
		angles.z = rad_to_deg(std::atan2(left.z, up.z));
	}

	else {
		angles.x = rad_to_deg(std::atan2(-forward.z, len));
		angles.y = rad_to_deg(std::atan2(-left.x, left.y));
		angles.z = 0.f;
	}
}

void math::MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out) {
	std::memcpy(out.Base(), in.Base(), sizeof(matrix3x4_t));
}

void math::ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out) {
	if (&in1 == &out) {
		matrix3x4_t in1b;
		MatrixCopy(in1, in1b);
		ConcatTransforms(in1b, in2, out);
		return;
	}

	if (&in2 == &out) {
		matrix3x4_t in2b;
		MatrixCopy(in2, in2b);
		ConcatTransforms(in1, in2b, out);
		return;
	}

	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];

	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];

	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

bool math::IntersectRayWithBox(const vec3_t& start, const vec3_t& delta, const vec3_t& mins, const vec3_t& maxs, float tolerance, BoxTraceInfo_t* out_info) {
	int   i;
	float d1, d2, f;

	for (i = 0; i < 6; ++i) {
		if (i >= 3) {
			d1 = start[i - 3] - maxs[i - 3];
			d2 = d1 + delta[i - 3];
		}

		else {
			d1 = -start[i] + mins[i];
			d2 = d1 - delta[i];
		}

		// if completely in front of face, no intersection.
		if (d1 > 0.f && d2 > 0.f) {
			out_info->m_startsolid = false;

			return false;
		}

		// completely inside, check next face.
		if (d1 <= 0.f && d2 <= 0.f)
			continue;

		if (d1 > 0.f)
			out_info->m_startsolid = false;

		// crosses face.
		if (d1 > d2) {
			f = std::max(0.f, d1 - tolerance);

			f = f / (d1 - d2);
			if (f > out_info->m_t1) {
				out_info->m_t1 = f;
				out_info->m_hitside = i;
			}
		}

		// leave.
		else {
			f = (d1 + tolerance) / (d1 - d2);
			if (f < out_info->m_t2)
				out_info->m_t2 = f;
		}
	}

	return out_info->m_startsolid || (out_info->m_t1 < out_info->m_t2 && out_info->m_t1 >= 0.f);
}

bool math::IntersectRayWithBox(const vec3_t& start, const vec3_t& delta, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr, float* fraction_left_solid) {
	BoxTraceInfo_t box_tr;

	// note - dex; this is Collision_ClearTrace.
	out_tr->m_startpos = start;
	out_tr->m_endpos = start;
	out_tr->m_endpos += delta;
	out_tr->m_startsolid = false;
	out_tr->m_allsolid = false;
	out_tr->m_fraction = 1.f;
	out_tr->m_contents = 0;

	if (IntersectRayWithBox(start, delta, mins, maxs, tolerance, &box_tr)) {
		out_tr->m_startsolid = box_tr.m_startsolid;

		if (box_tr.m_t1 < box_tr.m_t2 && box_tr.m_t1 >= 0.f) {
			out_tr->m_fraction = box_tr.m_t1;

			// VectorMA( pTrace->startpos, trace.t1, vecRayDelta, pTrace->endpos );

			out_tr->m_contents = CONTENTS_SOLID;
			out_tr->m_plane.m_normal = vec3_t{};

			if (box_tr.m_hitside >= 3) {
				box_tr.m_hitside -= 3;

				out_tr->m_plane.m_dist = maxs[box_tr.m_hitside];
				out_tr->m_plane.m_normal[box_tr.m_hitside] = 1.f;
				out_tr->m_plane.m_type = box_tr.m_hitside;
			}

			else {
				out_tr->m_plane.m_dist = -mins[box_tr.m_hitside];
				out_tr->m_plane.m_normal[box_tr.m_hitside] = -1.f;
				out_tr->m_plane.m_type = box_tr.m_hitside;
			}

			return true;
		}

		if (out_tr->m_startsolid) {
			out_tr->m_allsolid = (box_tr.m_t2 <= 0.f) || (box_tr.m_t2 >= 1.f);
			out_tr->m_fraction = 0.f;

			if (fraction_left_solid)
				*fraction_left_solid = box_tr.m_t2;

			out_tr->m_endpos = out_tr->m_startpos;
			out_tr->m_contents = CONTENTS_SOLID;
			out_tr->m_plane.m_dist = out_tr->m_startpos.x;
			out_tr->m_plane.m_normal = { 1.f, 0.f, 0.f };
			out_tr->m_plane.m_type = 0;
			out_tr->m_startpos = start + (box_tr.m_t2 * delta);

			return true;
		}
	}

	return false;
}

bool math::IntersectRayWithOBB(const vec3_t& start, const vec3_t& delta, const matrix3x4_t& obb_to_world, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr) {
	vec3_t box_extents, box_center, extent{}, uextent, segment_center, cross, new_start, tmp_end;
	float  coord, tmp, cextent, sign;

	// note - dex; this is Collision_ClearTrace.
	out_tr->m_startpos = start;
	out_tr->m_endpos = start;
	out_tr->m_endpos += delta;
	out_tr->m_startsolid = false;
	out_tr->m_allsolid = false;
	out_tr->m_fraction = 1.f;
	out_tr->m_contents = 0;

	// compute center in local space and transform to world space.
	box_extents = (mins + maxs) / 2.f;
	VectorTransform(box_extents, obb_to_world, box_center);

	// calculate extents from local center.
	box_extents = maxs - box_extents;

	// save the extents of the ray.
	segment_center = start + delta - box_center;

	// check box axes for separation.
	for (int i = 0; i < 3; ++i) {
		extent[i] = delta.x * obb_to_world[0][i] + delta.y * obb_to_world[1][i] + delta.z * obb_to_world[2][i];
		uextent[i] = std::abs(extent[i]);

		coord = segment_center.x * obb_to_world[0][i] + segment_center.y * obb_to_world[1][i] + segment_center.z * obb_to_world[2][i];
		coord = std::abs(coord);
		if (coord > (box_extents[i] + uextent[i]))
			return false;
	}

	// now check cross axes for separation.
	cross = delta.cross(segment_center);

	cextent = cross.x * obb_to_world[0][0] + cross.y * obb_to_world[1][0] + cross.z * obb_to_world[2][0];
	cextent = std::abs(cextent);
	tmp = box_extents.y * uextent.z + box_extents.z * uextent.y;
	if (cextent > tmp)
		return false;

	cextent = cross.x * obb_to_world[0][1] + cross.y * obb_to_world[1][1] + cross.z * obb_to_world[2][1];
	cextent = std::abs(cextent);
	tmp = box_extents.x * uextent.z + box_extents.z * uextent.x;
	if (cextent > tmp)
		return false;

	cextent = cross.x * obb_to_world[0][2] + cross.y * obb_to_world[1][2] + cross.z * obb_to_world[2][2];
	cextent = std::abs(cextent);
	tmp = box_extents.x * uextent.y + box_extents.y * uextent.x;
	if (cextent > tmp)
		return false;

	// we hit this box, compute intersection point and return.
	// compute ray start in bone space.
	VectorITransform(start, obb_to_world, new_start);

	// extent is ray.m_Delta in bone space, recompute delta in bone space.
	extent *= 2.f;

	// delta was prescaled by the current t, so no need to see if this intersection is closer.
	if (!IntersectRayWithBox(start, extent, mins, maxs, tolerance, out_tr))
		return false;

	// fix up the start/end pos and fraction
	VectorTransform(out_tr->m_endpos, obb_to_world, tmp_end);

	out_tr->m_endpos = tmp_end;
	out_tr->m_startpos = start;
	out_tr->m_fraction *= 2.f;

	// fix up the plane information
	sign = out_tr->m_plane.m_normal[out_tr->m_plane.m_type];

	out_tr->m_plane.m_normal.x = sign * obb_to_world[0][out_tr->m_plane.m_type];
	out_tr->m_plane.m_normal.y = sign * obb_to_world[1][out_tr->m_plane.m_type];
	out_tr->m_plane.m_normal.z = sign * obb_to_world[2][out_tr->m_plane.m_type];
	out_tr->m_plane.m_dist = out_tr->m_endpos.dot(out_tr->m_plane.m_normal);
	out_tr->m_plane.m_type = 3;

	return true;
}

bool math::IntersectRayWithOBB(const vec3_t& start, const vec3_t& delta, const vec3_t& box_origin, const ang_t& box_rotation, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr) {
	// todo - dex; https://github.com/pmrowla/hl2sdk-csgo/blob/master/public/collisionutils.cpp#L1400
	return false;
}

bool math::IntersectInfiniteRayWithSphere(const vec3_t& start, const vec3_t& delta, const vec3_t& sphere_center, float radius, float* out_t1, float* out_t2) {
	vec3_t sphere_to_ray;
	float  a, b, c, discrim, oo2a;

	sphere_to_ray = start - sphere_center;
	a = delta.dot(delta);

	// this would occur in the case of a zero-length ray.
	if (!a) {
		*out_t1 = 0.f;
		*out_t2 = 0.f;

		return sphere_to_ray.length_sqr() <= (radius * radius);
	}

	b = 2.f * sphere_to_ray.dot(delta);
	c = sphere_to_ray.dot(sphere_to_ray) - (radius * radius);

	discrim = b * b - 4.f * a * c;
	if (discrim < 0.f)
		return false;

	discrim = std::sqrt(discrim);
	oo2a = 0.5f / a;

	*out_t1 = (-b - discrim) * oo2a;
	*out_t2 = (-b + discrim) * oo2a;

	return true;
}

bool math::IntersectRayWithSphere(const vec3_t& start, const vec3_t& delta, const vec3_t& sphere_center, float radius, float* out_t1, float* out_t2) {
	if (!IntersectInfiniteRayWithSphere(start, delta, sphere_center, radius, out_t1, out_t2))
		return false;

	if (*out_t1 > 1.0f || *out_t2 < 0.0f)
		return false;

	// clamp intersection points.
	*out_t1 = std::max(0.f, *out_t1);
	*out_t2 = std::min(1.f, *out_t2);

	return true;
}

vec3_t math::Interpolate(const vec3_t from, const vec3_t to, const float percent) {
	return to * percent + from * (1.f - percent);
}