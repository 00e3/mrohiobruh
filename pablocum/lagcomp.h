#pragma once

class AimPlayer;

class LagCompensation {
public:
	LagRecord* m_pred_record[64];


	bool ValidRecord(float simtime);

	LagRecord* StartPrediction(AimPlayer* player);
	void PlayerMove(LagRecord* record);

};

extern LagCompensation g_lagcomp;
