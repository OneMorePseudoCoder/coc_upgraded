#include "stdafx.h"
#include "radioactivezone.h"
#include "Level.h"
#include "xrmessages.h"
#include "xrCore/Animation/Bone.hpp"
#include "actor.h"
#include "game_base_space.h"
#include "Hit.h"
#include "xrEngine/xr_collide_form.h"

CRadioactiveZone::CRadioactiveZone(void) {}
CRadioactiveZone::~CRadioactiveZone(void) {}
void CRadioactiveZone::Load(LPCSTR section) { inherited::Load(section); }
bool CRadioactiveZone::BlowoutState()
{
    bool result = inherited::BlowoutState();
    if (!result)
        UpdateBlowout();
    return result;
}

void CRadioactiveZone::Affect(SZoneObjectInfo* O)
{
    float one = 0.1f;
    float tg = Device.fTimeGlobal;

    if (!O->object || O->f_time_affected + one > Device.fTimeGlobal)
        return;

    clamp(O->f_time_affected, tg - (one * 3), tg);

    Fvector pos;
    XFORM().transform_tiny(pos, GetCForm()->getSphere().P);

    Fvector dir = {0, 0, 0};
    float power = Power(O->object->Position().distance_to(pos), nearest_shape_radius(O));

    float impulse = 0.0f;
    if (power < EPS)
    {
        O->f_time_affected = tg;
        return;
    }

    float send_power = power * one;

    while (O->f_time_affected + one < tg)
    {
        CreateHit(O->object->ID(), ID(), dir, send_power, BI_NONE, Fvector().set(0.0f, 0.0f, 0.0f), impulse, m_eHitTypeBlowout);
        O->f_time_affected += one;
    }
}

void CRadioactiveZone::feel_touch_new(IGameObject* O)
{
    inherited::feel_touch_new(O);
};

#include "actor.h"
bool CRadioactiveZone::feel_touch_contact(IGameObject* O)
{
    CActor* A = smart_cast<CActor*>(O);
    if (A)
    {
        if (!((CCF_Shape*)GetCForm())->Contact(O))
            return false;
        return A->feel_touch_on_contact(this);
    }
    else
        return false;
}

void CRadioactiveZone::UpdateWorkload(u32 dt)
{
    inherited::UpdateWorkload(dt);
}

float CRadioactiveZone::nearest_shape_radius(SZoneObjectInfo* O)
{
    CCF_Shape* Sh = (CCF_Shape*)GetCForm();

    if (Sh->Shapes().size() == 1)
    {
        return Radius();
    }
    else
    {
        xr_vector<CCF_Shape::shape_def>& Shapes = Sh->Shapes();
        CCF_Shape::shape_def& s = Shapes[0];
        return s.data.sphere.R;
    }
}
