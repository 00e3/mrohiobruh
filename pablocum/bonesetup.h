#pragma once

class Bones {
public:
	void SetupCorrectionMatrix(Player* target, BoneArray* in, int size = 128);
	bool Setup( Player* target, int mask, BoneArray* out, int size = 128, bool rebuilt = true, bool setup_cache = false);
};

extern Bones g_bones;