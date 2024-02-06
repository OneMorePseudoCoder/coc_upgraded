// HUDCrosshair.cpp:  крестик прицела, отображающий текущую дисперсию
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "HUDCrosshair.h"
#include "xrUICore/ui_base.h"

CHUDCrosshair::CHUDCrosshair()
{
    hShader->create("hud\\crosshair");
    radius = 0;
}

CHUDCrosshair::~CHUDCrosshair() {}
void CHUDCrosshair::Load()
{
    //все размеры в процентах от длины экрана
    //длина крестика
    cross_length_perc = pSettings->r_float(HUD_CURSOR_SECTION, "cross_length");
    min_radius_perc = pSettings->r_float(HUD_CURSOR_SECTION, "min_radius");
    max_radius_perc = pSettings->r_float(HUD_CURSOR_SECTION, "max_radius");
    cross_color = pSettings->r_fcolor(HUD_CURSOR_SECTION, "cross_color").get();
}

//выставляет radius от min_radius до max_radius
void CHUDCrosshair::SetDispersion(float disp)
{
    Fvector4 r;
    Fvector R = {VIEWPORT_NEAR * _sin(disp), 0.f, VIEWPORT_NEAR};
    Device.mProject.transform(r, R);

    Fvector2 scr_size;
    scr_size.set(float(GEnv.Render->getTarget()->get_width()), float(GEnv.Render->getTarget()->get_height()));
    float radius_pixels = _abs(r.x) * scr_size.x / 2.0f;
    target_radius = radius_pixels;
}

#ifdef DEBUG
void CHUDCrosshair::SetFirstBulletDispertion(float fbdisp)
{
    Fvector4 r;
    Fvector R = {VIEWPORT_NEAR * _sin(fbdisp), 0.f, VIEWPORT_NEAR};
    Device.mProject.transform(r, R);

    Fvector2 scr_size;
    scr_size.set(float(GEnv.Render->getTarget()->get_width()), float(GEnv.Render->getTarget()->get_height()));
    fb_radius = _abs(r.x) * scr_size.x / 2.0f;
}

BOOL g_bDrawFirstBulletCrosshair = FALSE;

void CHUDCrosshair::OnRenderFirstBulletDispertion()
{
    VERIFY(g_bRendering);
    Fvector2 center;
    Fvector2 scr_size;
    scr_size.set(float(GEnv.Render->getTarget()->get_width()), float(GEnv.Render->getTarget()->get_height()));
    center.set(scr_size.x / 2.0f, scr_size.y / 2.0f);

    GEnv.UIRender->StartPrimitive(10, IUIRender::ptLineList, UI().m_currentPointType);

    u32 fb_cross_color = color_rgba(255, 0, 0, 255); // red

    float cross_length = /*cross_length_perc*/ 0.008f * scr_size.x;
    float min_radius = min_radius_perc * scr_size.x;
    float max_radius = max_radius_perc * scr_size.x;

    clamp(target_radius, min_radius, max_radius);

    float x_min = min_radius + fb_radius;
    float x_max = x_min + cross_length;

    float y_min = x_min;
    float y_max = x_max;

    // 0
    GEnv.UIRender->PushPoint(center.x, center.y + y_min, 0, fb_cross_color, 0, 0);
    GEnv.UIRender->PushPoint(center.x, center.y + y_max, 0, fb_cross_color, 0, 0);
    // 1
    GEnv.UIRender->PushPoint(center.x, center.y - y_min, 0, fb_cross_color, 0, 0);
    GEnv.UIRender->PushPoint(center.x, center.y - y_max, 0, fb_cross_color, 0, 0);
    // 2
    GEnv.UIRender->PushPoint(center.x + x_min, center.y, 0, fb_cross_color, 0, 0);
    GEnv.UIRender->PushPoint(center.x + x_max, center.y, 0, fb_cross_color, 0, 0);
    // 3
    GEnv.UIRender->PushPoint(center.x - x_min, center.y, 0, fb_cross_color, 0, 0);
    GEnv.UIRender->PushPoint(center.x - x_max, center.y, 0, fb_cross_color, 0, 0);

    // point
    GEnv.UIRender->PushPoint(center.x - 0.5f, center.y, 0, fb_cross_color, 0, 0);
    GEnv.UIRender->PushPoint(center.x + 0.5f, center.y, 0, fb_cross_color, 0, 0);

    // render
    GEnv.UIRender->SetShader(*hShader);
    GEnv.UIRender->FlushPrimitive();
}
#endif

extern ENGINE_API BOOL g_bRendering;
void CHUDCrosshair::OnRender(const Fvector2& center, const Fvector2& scr_size)
{
    VERIFY(g_bRendering);

	Fvector2 c;
	Fvector2 ss;

	if (Actor() && Actor()->active_cam() == EActorCameras::eacFirstEye)
	{
		ss.set(float(GEnv.Render->getTarget()->get_width()), float(GEnv.Render->getTarget()->get_height()));
		c.set(scr_size.x / 2.0f, scr_size.y / 2.0f);
	}
	else
	{
		ss = scr_size;
        c = center;
	}

    GEnv.UIRender->StartPrimitive(10, IUIRender::ptLineList, UI().m_currentPointType);

    float cross_length = cross_length_perc * ss.x;
    float min_radius = min_radius_perc * ss.x;
    float max_radius = max_radius_perc * ss.x;

    clamp(target_radius, min_radius, max_radius);

    float x_min = min_radius + radius;
    float x_max = x_min + cross_length;

    float y_min = x_min;
    float y_max = x_max;

    // 0
    GEnv.UIRender->PushPoint(c.x, c.y + y_min, 0, cross_color, 0, 0);
    GEnv.UIRender->PushPoint(c.x, c.y + y_max, 0, cross_color, 0, 0);
    // 1
    GEnv.UIRender->PushPoint(c.x, c.y - y_min, 0, cross_color, 0, 0);
    GEnv.UIRender->PushPoint(c.x, c.y - y_max, 0, cross_color, 0, 0);
    // 2
    GEnv.UIRender->PushPoint(c.x + x_min, c.y, 0, cross_color, 0, 0);
    GEnv.UIRender->PushPoint(c.x + x_max, c.y, 0, cross_color, 0, 0);
    // 3
    GEnv.UIRender->PushPoint(c.x - x_min, c.y, 0, cross_color, 0, 0);
    GEnv.UIRender->PushPoint(c.x - x_max, c.y, 0, cross_color, 0, 0);

    // point
    GEnv.UIRender->PushPoint(c.x - 0.5f, c.y, 0, cross_color, 0, 0);
    GEnv.UIRender->PushPoint(c.x + 0.5f, c.y, 0, cross_color, 0, 0);

    // render
    GEnv.UIRender->SetShader(*hShader);
    GEnv.UIRender->FlushPrimitive();

    if (!fsimilar(target_radius, radius))
    {
        // here was crosshair innertion emulation
        radius = target_radius;
    };
#ifdef DEBUG
    if (g_bDrawFirstBulletCrosshair)
        OnRenderFirstBulletDispertion();
#endif
}
