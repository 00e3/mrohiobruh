#pragma once

class InputPrediction {
public:
    float m_curtime;
    float m_frametime;
    vec3_t m_velocity;
    CMoveData m_MoveData;
    bool m_first_time_predicted;
    bool m_in_prediction;
    Entity* m_old_ground_ent;

    float m_cycle;
    float m_anim_time;

public:
    void update_viewmodel(bool store);
    void post_think(Player* player);
    void start_command(Player* player, CUserCmd* pCmd);
    void finish_command(Player* player);
    void run_prethink(Player* player);
    void run_think(Player* player);

    void update(bool store = false);

    void repredict();
    void run();
    void restore();
};

extern InputPrediction g_inputpred;


struct RestoreData
{
    struct RestoreInfo_t
    {
        void Reset()
        {
            m_aimPunchAngle = { 0.f, 0.f, 0.f };
            m_aimPunchAngleVel = { 0.f, 0.f, 0.f };
            m_viewPunchAngle = { 0.f, 0.f, 0.f };

            m_vecViewOffset = { 0.f, 0.f, 0.f };
            m_vecBaseVelocity = { 0.f, 0.f, 0.f };
            m_vecVelocity = { 0.f, 0.f, 0.f };
            m_vecAnimVelocity = { 0.f, 0.f, 0.f };
            m_vecOrigin = { 0.f, 0.f, 0.f };

            m_flFallVelocity = 0.0f;
            m_flVelocityModifier = 0.0f;
            m_flDuckAmount = 0.0f;
            m_flDuckSpeed = 0.0f;
            m_surfaceFriction = 0.0f;

            m_fAccuracyPenalty = 0.0f;
            m_flRecoilIndex = 0.f;

            m_GroundEntity = 0;
            m_nMoveType = 0;
            m_nFlags = 0;
            m_nTickBase = 0;
        }

        bool is_filled = false;

        CMoveData m_MoveData;

        ang_t m_aimPunchAngle = { };
        ang_t m_aimPunchAngleVel = { };
        ang_t m_viewPunchAngle = { };

        vec3_t m_vecViewOffset = { };
        vec3_t m_vecBaseVelocity = { };
        vec3_t m_vecVelocity = { };
        vec3_t m_vecAnimVelocity = { };
        vec3_t m_vecOrigin = { };

        float m_flFallVelocity = 0.0f;
        float m_flVelocityModifier = 0.0f;
        float m_flDuckAmount = 0.0f;
        float m_flDuckSpeed = 0.0f;
        float m_surfaceFriction = 0.0f;

        float m_fAccuracyPenalty = 0.0f;
        float m_flRecoilIndex = 0;

        EHANDLE m_GroundEntity = 0;
        int m_nMoveType = 0;
        int m_nFlags = 0;
        int m_nTickBase = 0;
    };

    RestoreInfo_t info = { };

    void Setup(Player* player);

    void Apply(Player* player);
};