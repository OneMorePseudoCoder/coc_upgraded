#pragma once
////////////////////////////////////////////////////////////////////////
// Effector controlling class
////////////////////////////////////////////////////////////////////////
#include "pp_effector_custom.h"

class CPPEffectorPsyDogAura : public CPPEffectorCustom
{
    typedef CPPEffectorCustom inherited;

    enum
    {
        eStateFadeIn,
        eStateFadeOut,
        eStatePermanent
    } m_effector_state;

    u32 m_time_state_started;
    u32 m_time_to_fade;

public:
    CPPEffectorPsyDogAura(const SPPInfo& ppi, u32 time_to_fade);
    virtual BOOL update();
    void switch_off();
};

class CPsyDog;
class CActor;

class CPsyDogAura : public CPPEffectorCustomController<CPPEffectorPsyDogAura>
{
    CPsyDog* m_object;
	
    u32 m_time_actor_saw_phantom;
    u32 m_time_phantom_saw_actor;
    CActor* m_actor;

public:
    CPsyDogAura(CPsyDog* dog) : m_object(dog), m_time_actor_saw_phantom(0), m_time_phantom_saw_actor(0), m_actor(nullptr) {}
    void reinit();
    void on_death();
    void update_schedule();
};
