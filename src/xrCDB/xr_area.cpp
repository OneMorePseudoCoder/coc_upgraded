#include "stdafx.h"

#include "xr_area.h"
#include "xrEngine/xr_object.h"
#include "Common/LevelStructure.hpp"
#include "xrEngine/xr_collide_form.h"

using namespace collide;

//----------------------------------------------------------------------
// Class	: CObjectSpace
// Purpose	: stores space slots
//----------------------------------------------------------------------
CObjectSpace::CObjectSpace()
    : xrc("object space")
#ifdef CONFIG_PROFILE_LOCKS
      , lock(new Lock(MUTEX_PROFILE_ID(CObjectSpace::Lock)))
#else
      , lock(new Lock)
#endif // CONFIG_PROFILE_LOCKS
#ifdef DEBUG
      , m_pRender(0)
#endif
{
#ifdef DEBUG
    if (GEnv.RenderFactory)
        m_pRender = new FactoryPtr<IObjectSpaceRender>();

#endif
    m_BoundingVolume.invalidate();
}
//----------------------------------------------------------------------
CObjectSpace::~CObjectSpace()
{
#ifdef DEBUG
    xr_delete(m_pRender);
#endif
	delete lock;
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
int CObjectSpace::GetNearest(xr_vector<ISpatial*>& q_spatial, xr_vector<IGameObject*>& q_nearest, const Fvector& point, float range, IGameObject* ignore_object)
{
    q_spatial.clear();
    // Query objects
    q_nearest.clear();
    Fsphere Q;
    Q.set(point, range);
    Fvector B;
    B.set(range, range, range);
    g_SpatialSpace->q_box(q_spatial, 0, STYPE_COLLIDEABLE, point, B);

    // Iterate
    for (auto& it : q_spatial)
    {
        IGameObject* O = it->dcast_GameObject();
        if (0 == O)
            continue;
        if (O == ignore_object)
            continue;
        Fsphere mS = {O->GetSpatialData().sphere.P, O->GetSpatialData().sphere.R};
        if (Q.intersect(mS))
            q_nearest.push_back(O);
    }

    return q_nearest.size();
}

//----------------------------------------------------------------------
IC int CObjectSpace::GetNearest(xr_vector<IGameObject*>& q_nearest, const Fvector& point, float range, IGameObject* ignore_object)
{
    return (GetNearest(r_spatial, q_nearest, point, range, ignore_object));
}

//----------------------------------------------------------------------
IC int CObjectSpace::GetNearest(xr_vector<IGameObject*>& q_nearest, ICollisionForm* obj, float range)
{
    IGameObject* O = obj->Owner();
    return GetNearest(q_nearest, O->GetSpatialData().sphere.P, range + O->GetSpatialData().sphere.R, O);
}

//----------------------------------------------------------------------

void CObjectSpace::Load(CDB::build_callback build_callback, CDB::serialize_callback serialize_callback, CDB::deserialize_callback deserialize_callback)
{ 
	Load("$level$", "level.cform", build_callback, serialize_callback, deserialize_callback);
}

void CObjectSpace::Load(LPCSTR path, LPCSTR fname, CDB::build_callback build_callback, CDB::serialize_callback serialize_callback, CDB::deserialize_callback deserialize_callback)
{
    IReader* F = FS.r_open(path, fname);
    R_ASSERT(F);
    Load(F, build_callback, serialize_callback, deserialize_callback);
}

void CObjectSpace::Load(IReader* F, CDB::build_callback build_callback, CDB::serialize_callback serialize_callback, CDB::deserialize_callback deserialize_callback)
{
    hdrCFORM H;
    F->r(&H, sizeof(hdrCFORM));
    Fvector* verts = (Fvector*)F->pointer();
    CDB::TRI* tris = (CDB::TRI*)(verts + H.vertcount);
	Static.set_version(F->get_age());
	Create(verts, tris, H, build_callback, serialize_callback, deserialize_callback);
    FS.r_close(F);
}

void CObjectSpace::Create(Fvector* verts, CDB::TRI* tris, const hdrCFORM& H, CDB::build_callback build_callback, CDB::serialize_callback serialize_callback, CDB::deserialize_callback deserialize_callback)
{
    R_ASSERT(CFORM_CURRENT_VERSION == H.version);
	string_path fName;

	const bool bUseCache = !strstr(Core.Params, "-no_cdb_cache");
	const bool checkCrc32 = !strstr(Core.Params, "-skip_cdb_cache_crc32_check");

	strconcat(fName, "cdb_cache" "\\", FS.get_path("$level$")->m_Add, "objspace.bin");
	FS.update_path(fName, "$app_data_root$", fName);

	if (bUseCache && FS.exist(fName) && Static.deserialize(fName, checkCrc32, deserialize_callback))
		Msg("* Loaded ObjectSpace cache (%s)...", fName);
	else
	{
		Msg("* ObjectSpace cache for '%s' was not loaded. Building the model from scratch..", fName);
		Static.build(verts, H.vertcount, tris, H.facecount, build_callback);

		if (bUseCache)
			Static.serialize(fName, serialize_callback);
	}

    m_BoundingVolume.set(H.aabb);
    g_SpatialSpace->initialize(m_BoundingVolume);
    g_SpatialSpacePhysic->initialize(m_BoundingVolume);
}

//----------------------------------------------------------------------
#ifdef DEBUG
void CObjectSpace::dbgRender() { (*m_pRender)->dbgRender(); }
#endif

// XXX stats: add to statistics
void CObjectSpace::DumpStatistics(IGameFont& font, IPerformanceAlert* alert) { xrc.DumpStatistics(font, alert); }
