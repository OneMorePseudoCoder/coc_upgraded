#pragma once
#include "xrEngine/CameraBase.h"
#include "xrCore/_quaternion.h"

class CCameraLook : public CCameraBase
{
    typedef CCameraBase inherited;

protected:
    Fvector2 lim_zoom;
    float dist, prev_d;

public:
    CCameraLook(IGameObject* p, u32 flags = 0);
    virtual ~CCameraLook();
    virtual void Load(LPCSTR section);
    virtual void Move(int cmd, float val = 0, float factor = 1.0f);

    virtual void OnActivate(CCameraBase* old_cam);
    virtual void Update(Fvector& point, Fvector& noise_dangle);

    virtual float GetWorldYaw() { return -yaw; };
    virtual float GetWorldPitch() { return pitch; };

	void save(NET_Packet& output_packet) override;
	void load(IReader& input_packet) override;

protected:
    void UpdateDistance(Fvector& point);
};

class CCameraLook2 : public CCameraLook
{
public:
    static Fvector m_cam_offset;

	CCameraLook2(IGameObject* p, u32 flags = 0) : CCameraLook(p, flags){};
    virtual ~CCameraLook2() {}
    virtual void OnActivate(CCameraBase* old_cam);
    virtual void Update(Fvector& point, Fvector& noise_dangle);
    virtual void Load(LPCSTR section);
};

class CCameraFixedLook : public CCameraLook
{
    typedef CCameraLook inherited;

public:
    CCameraFixedLook(IGameObject* p, u32 flags = 0) : CCameraLook(p, flags){};
    virtual ~CCameraFixedLook(){};
    virtual void Load(LPCSTR section);
    virtual void Move(int cmd, float val = 0, float factor = 1.0f);
    virtual void OnActivate(CCameraBase* old_cam);
    virtual void Update(Fvector& point, Fvector& noise_dangle);
    virtual void Set(float Y, float P, float R);

private:
    Fquaternion m_final_dir;
    Fquaternion m_current_dir;
};
