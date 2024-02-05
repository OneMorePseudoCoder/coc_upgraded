#include "stdafx.h"
#include "xr_effgamma.h"
#include "xrCore/Media/Image.hpp"
#include "xrEngine/xrImage_Resampler.h"
#if defined(USE_DX10) || defined(USE_DX11)
#include "d3dx10tex.h"
#endif // USE_DX10

using namespace XRay::Media;

#define GAMESAVE_SIZE 128

IC u32 convert(float c)
{
    u32 C = iFloor(c);
    if (C > 255)
        C = 255;
    return C;
}

IC void MouseRayFromPoint(Fvector& direction, int x, int y, Fmatrix& m_CamMat)
{
    int halfwidth = Device.dwWidth / 2;
    int halfheight = Device.dwHeight / 2;

    Ivector2 point2;
    point2.set(x - halfwidth, halfheight - y);

    float size_y = VIEWPORT_NEAR * tanf(deg2rad(60.f) * 0.5f);
    float size_x = size_y / (Device.fHeight_2 / Device.fWidth_2);

    float r_pt = float(point2.x) * size_x / (float)halfwidth;
    float u_pt = float(point2.y) * size_y / (float)halfheight;

    direction.mul(m_CamMat.k, VIEWPORT_NEAR);
    direction.mad(direction, m_CamMat.j, u_pt);
    direction.mad(direction, m_CamMat.i, r_pt);
    direction.normalize();
}

#define SM_FOR_SEND_WIDTH 640
#define SM_FOR_SEND_HEIGHT 480

#if defined(USE_DX10) || defined(USE_DX11)
void CRender::ScreenshotImpl(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer)
{
    ID3DResource* pSrcTexture;
    HW.pBaseRT->GetResource(&pSrcTexture);

    VERIFY(pSrcTexture);

    // Save
    switch (mode)
    {
    case IRender::SM_FOR_GAMESAVE:
    {
        ID3DTexture2D* pSrcSmallTexture;

        D3D_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = GAMESAVE_SIZE;
        desc.Height = GAMESAVE_SIZE;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_BC1_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D_USAGE_DEFAULT;
        desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
        CHK_DX(HW.pDevice->CreateTexture2D(&desc, NULL, &pSrcSmallTexture));

#ifdef USE_DX11
        CHK_DX(D3DX11LoadTextureFromTexture(HW.pContext, pSrcTexture, NULL, pSrcSmallTexture));
#else
        CHK_DX(D3DX10LoadTextureFromTexture(pSrcTexture, NULL, pSrcSmallTexture));
#endif

        // save (logical & physical)
        ID3DBlob* saved = nullptr;
#ifdef USE_DX11
        HRESULT hr = D3DX11SaveTextureToMemory(HW.pContext, pSrcSmallTexture, D3DX11_IFF_DDS, &saved, 0);
#else
        HRESULT hr = D3DX10SaveTextureToMemory(pSrcSmallTexture, D3DX10_IFF_DDS, &saved, 0);
#endif
        if (hr == D3D_OK)
        {
            IWriter* fs = FS.w_open(name);
            if (fs)
            {
                fs->w(saved->GetBufferPointer(), (u32)saved->GetBufferSize());
                FS.w_close(fs);
            }
        }
        _RELEASE(saved);

        // cleanup
        _RELEASE(pSrcSmallTexture);
    }
    break;
    case IRender::SM_FOR_MPSENDING:
    {
        ID3DTexture2D* pSrcSmallTexture;

        D3D_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = SM_FOR_SEND_WIDTH;
        desc.Height = SM_FOR_SEND_HEIGHT;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_BC1_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D_USAGE_DEFAULT;
        desc.BindFlags = D3D_BIND_SHADER_RESOURCE;
        CHK_DX(HW.pDevice->CreateTexture2D(&desc, NULL, &pSrcSmallTexture));

#ifdef USE_DX11
        CHK_DX(D3DX11LoadTextureFromTexture(HW.pContext, pSrcTexture, NULL, pSrcSmallTexture));
#else
        CHK_DX(D3DX10LoadTextureFromTexture(pSrcTexture, NULL, pSrcSmallTexture));
#endif
        // save (logical & physical)
        ID3DBlob* saved = nullptr;
#ifdef USE_DX11
        HRESULT hr = D3DX11SaveTextureToMemory(HW.pContext, pSrcSmallTexture, D3DX11_IFF_DDS, &saved, 0);
#else
        HRESULT hr = D3DX10SaveTextureToMemory(pSrcSmallTexture, D3DX10_IFF_DDS, &saved, 0);
#endif
        if (hr == D3D_OK)
        {
            if (!memory_writer)
            {
                IWriter* fs = FS.w_open(name);
                if (fs)
                {
                    fs->w(saved->GetBufferPointer(), (u32)saved->GetBufferSize());
                    FS.w_close(fs);
                }
            }
            else
            {
                memory_writer->w(saved->GetBufferPointer(), (u32)saved->GetBufferSize());
            }
        }
        _RELEASE(saved);

        // cleanup
        _RELEASE(pSrcSmallTexture);
    }
        break;
    case IRender::SM_NORMAL:
    {
        string64 t_stemp;
        string_path buf;
        xr_sprintf(buf, sizeof buf, "ss_%s_%s_(%s).jpg", Core.UserName, timestamp(t_stemp), g_pGameLevel ? g_pGameLevel->name().c_str() : "mainmenu");
        ID3DBlob* saved = nullptr;
#ifdef USE_DX11
        CHK_DX(D3DX11SaveTextureToMemory(HW.pContext, pSrcTexture, D3DX11_IFF_JPG, &saved, 0));
#else
        CHK_DX(D3DX10SaveTextureToMemory(pSrcTexture, D3DX10_IFF_JPG, &saved, 0));
#endif
        IWriter* fs = FS.w_open("$screenshots$", buf);
        R_ASSERT(fs);
        fs->w(saved->GetBufferPointer(), (u32)saved->GetBufferSize());
        FS.w_close(fs);
        _RELEASE(saved);

        if (strstr(Core.Params, "-ss_tga"))
        { // hq
            xr_sprintf(buf, sizeof buf, "ssq_%s_%s_(%s).tga", Core.UserName, timestamp(t_stemp), g_pGameLevel ? g_pGameLevel->name().c_str() : "mainmenu");
            ID3DBlob* saved2 = nullptr;
#ifdef USE_DX11
            CHK_DX(D3DX11SaveTextureToMemory(HW.pContext, pSrcTexture, D3DX11_IFF_BMP, &saved2, 0));
#else
            CHK_DX(D3DX10SaveTextureToMemory(pSrcTexture, D3DX10_IFF_BMP, &saved2, 0));
#endif
            IWriter* fs2 = FS.w_open("$screenshots$", buf);
            R_ASSERT(fs2);
            fs2->w(saved2->GetBufferPointer(), (u32)saved2->GetBufferSize());
            FS.w_close(fs2);
            _RELEASE(saved2);
        }
    }
        break;
    case IRender::SM_FOR_LEVELMAP:
    case IRender::SM_FOR_CUBEMAP:
    {
        VERIFY(!"CRender::Screenshot. This screenshot type is not supported for DX10.");
    }
        break;
    }

    _RELEASE(pSrcTexture);
}

#else // USE_DX10

void CRender::ScreenshotImpl(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer)
{
    if (!Device.b_is_Ready)
        return;
    // Create temp-surface
    IDirect3DSurface9* pFB;
    D3DLOCKED_RECT D;
    HRESULT hr;
	int width = Device.dwWidth;
	int height = Device.dwHeight;
	RECT* srcRect = 0;

	//MSDN IDirect3DDevice9::GetFrontBufferData method
	//For windowed mode, the size of the destination surface should be the size of the desktop. 
	if ((psDeviceFlags.test(rsFullscreen)) == 0) 
	{
		RECT desktop;
		const HWND hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &desktop);
		width = desktop.right;
		height = desktop.bottom;
		RECT windowRect;
		const HWND hActive = GetActiveWindow();
		GetClientRect(hActive, &windowRect);
		srcRect = new RECT;
		POINT p; p.x = windowRect.left; p.y = windowRect.top;
		ClientToScreen(hActive, &p);
		srcRect->left = p.x;
		srcRect->top = p.y;
		p.x = windowRect.right; p.y = windowRect.bottom;
		ClientToScreen(hActive, &p);
		srcRect->right = p.x;
		srcRect->bottom = p.y;
	}

	hr = HW.pDevice->CreateOffscreenPlainSurface(width, height,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&pFB,NULL);
    if (FAILED(hr))
        return;
    hr = HW.pDevice->GetRenderTargetData(HW.pBaseRT, pFB);
    if (FAILED(hr))
        goto _end_;
    hr = pFB->LockRect(&D, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
        goto _end_;
    // Image processing (gamma-correct)
    u32* pPixel = (u32*)D.pBits;
    u32* pEnd = pPixel + (Device.dwWidth * Device.dwHeight);

    // Kill alpha
    for (; pPixel != pEnd; pPixel++)
    {
        u32 p = *pPixel;
        *pPixel = color_xrgb(color_get_R(p), color_get_G(p), color_get_B(p));
    }

    hr = pFB->UnlockRect();
    if (hr != D3D_OK)
        goto _end_;

    // Save
    switch (mode)
    {
    case IRender::SM_FOR_GAMESAVE:
    {
        // texture
        ID3DTexture2D* texture = nullptr;
        hr = D3DXCreateTexture(HW.pDevice, GAMESAVE_SIZE, GAMESAVE_SIZE, 1, 0, D3DFMT_DXT1, D3DPOOL_SCRATCH, &texture);
        if (hr != D3D_OK)
            goto _end_;
        if (nullptr == texture)
            goto _end_;

        // resize&convert to surface
        IDirect3DSurface9* surface = nullptr;
        hr = texture->GetSurfaceLevel(0, &surface);
        if (hr != D3D_OK)
            goto _end_;
        VERIFY(surface);
        hr = D3DXLoadSurfaceFromSurface(surface, nullptr, nullptr, pFB, nullptr, srcRect, D3DX_DEFAULT, 0);
        _RELEASE(surface);
        if (hr != D3D_OK)
            goto _end_;

        // save (logical & physical)
        ID3DXBuffer* saved = nullptr;
        hr = D3DXSaveTextureToFileInMemory(&saved, D3DXIFF_DDS, texture, nullptr);
        if (hr != D3D_OK)
            goto _end_;

        IWriter* fs = FS.w_open(name);
        if (fs)
        {
            fs->w(saved->GetBufferPointer(), saved->GetBufferSize());
            FS.w_close(fs);
        }
        _RELEASE(saved);

        // cleanup
        _RELEASE(texture);
    }
	break;
    case IRender::SM_FOR_MPSENDING:
    {
        // texture
        ID3DTexture2D* texture = nullptr;
        hr = D3DXCreateTexture(HW.pDevice, SM_FOR_SEND_WIDTH, SM_FOR_SEND_HEIGHT, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SCRATCH, &texture);
        if (hr != D3D_OK)
            goto _end_;
        if (nullptr == texture)
            goto _end_;

        // resize&convert to surface
        IDirect3DSurface9* surface = nullptr;
        hr = texture->GetSurfaceLevel(0, &surface);
        if (hr != D3D_OK)
            goto _end_;
        VERIFY(surface);
        hr = D3DXLoadSurfaceFromSurface(surface, nullptr, nullptr, pFB, nullptr, nullptr, D3DX_DEFAULT, 0);
        _RELEASE(surface);
        if (hr != D3D_OK)
            goto _end_;

        // save (logical & physical)
        ID3DXBuffer* saved = nullptr;
        hr = D3DXSaveTextureToFileInMemory(&saved, D3DXIFF_DDS, texture, nullptr);
        if (hr != D3D_OK)
            goto _end_;

        if (!memory_writer)
        {
            IWriter* fs = FS.w_open(name);
            if (fs)
            {
                fs->w(saved->GetBufferPointer(), saved->GetBufferSize());
                FS.w_close(fs);
            }
        }
        else
        {
            memory_writer->w(saved->GetBufferPointer(), saved->GetBufferSize());
        }

        _RELEASE(saved);

        // cleanup
        _RELEASE(texture);
    }
	break;
    case IRender::SM_NORMAL:
    {
        string64 t_stemp;
        string_path buf;
        xr_sprintf(buf, sizeof buf, "ss_%s_%s_(%s).jpg", Core.UserName, timestamp(t_stemp), g_pGameLevel ? g_pGameLevel->name().c_str() : "mainmenu");
        ID3DXBuffer* saved = nullptr;
        CHK_DX(D3DXSaveSurfaceToFileInMemory(&saved, D3DXIFF_JPG, pFB, nullptr, srcRect));
        IWriter* fs = FS.w_open("$screenshots$", buf);
        R_ASSERT(fs);
        fs->w(saved->GetBufferPointer(), saved->GetBufferSize());
        FS.w_close(fs);
        _RELEASE(saved);
        if (strstr(Core.Params, "-ss_tga"))
        { // hq
            xr_sprintf(buf, sizeof(buf), "ssq_%s_%s_(%s).tga", Core.UserName, timestamp(t_stemp), (g_pGameLevel) ? g_pGameLevel->name().c_str() : "mainmenu");
            ID3DXBuffer* saved = nullptr;
            CHK_DX(D3DXSaveSurfaceToFileInMemory(&saved, D3DXIFF_TGA, pFB, nullptr, srcRect));
            IWriter* fs = FS.w_open("$screenshots$", buf);
            R_ASSERT(fs);
            fs->w(saved->GetBufferPointer(), saved->GetBufferSize());
            FS.w_close(fs);
            _RELEASE(saved);
        }
		else if (strstr(Core.Params, "-ss_png")) // hq png
		{ 
			xr_sprintf(buf, sizeof(buf), "ssq_%s_%s_(%s).png", Core.UserName, timestamp(t_stemp), (g_pGameLevel) ? g_pGameLevel->name().c_str() : "mainmenu");
			ID3DXBuffer* saved = nullptr;
			CHK_DX(D3DXSaveSurfaceToFileInMemory(&saved, D3DXIFF_PNG, pFB, nullptr, srcRect));
			IWriter* fs	= FS.w_open("$screenshots$", buf); 
			R_ASSERT(fs);
			fs->w(saved->GetBufferPointer(), saved->GetBufferSize());
			FS.w_close(fs);
			_RELEASE(saved);
		}
    }
	break;
    case IRender::SM_FOR_LEVELMAP:
    case IRender::SM_FOR_CUBEMAP:
    {
        // string64 t_stemp;
        string_path buf;
        VERIFY(name);
        strconcat(sizeof(buf), buf, name, ".tga");
        IWriter* fs = FS.w_open("$screenshots$", buf);
        R_ASSERT(fs);
        // TODO: DX10: This is totally incorrect but mimics 
        // original behavior. Fix later.
        hr = pFB->LockRect(&D, nullptr, D3DLOCK_NOSYSLOCK);
        if (hr != D3D_OK)
            return;
        hr = pFB->UnlockRect();
        if (hr != D3D_OK)
            goto _end_;

        // save
        u32* data = (u32*)xr_malloc(Device.dwHeight * Device.dwHeight * 4);
        imf_Process(data, Device.dwHeight, Device.dwHeight, (u32*)D.pBits, Device.dwWidth, Device.dwHeight, imf_lanczos3);
        Image img;
        img.Create(u16(Device.dwHeight), u16(Device.dwHeight), data, ImageFormat::RGBA8);
        img.SaveTGA(*fs, true);
        xr_free(data);
        FS.w_close(fs);
    }
	break;
    }

_end_:
    _RELEASE(pFB);
}

#endif // USE_DX10

void CRender::Screenshot(ScreenshotMode mode, LPCSTR name) { ScreenshotImpl(mode, name, nullptr); }
void CRender::Screenshot(ScreenshotMode mode, CMemoryWriter& memory_writer)
{
    if (mode != SM_FOR_MPSENDING)
    {
        Log("~ Not implemented screenshot mode...");
        return;
    }
    ScreenshotImpl(mode, nullptr, &memory_writer);
}

void CRender::ScreenshotAsyncBegin()
{
    VERIFY(!m_bMakeAsyncSS);
    m_bMakeAsyncSS = true;
}

#if defined(USE_DX10) || defined(USE_DX11)

void CRender::ScreenshotAsyncEnd(CMemoryWriter& memory_writer)
{
    VERIFY(!m_bMakeAsyncSS);

    // Don't own. No need to release.
    ID3DTexture2D* pTex = Target->t_ss_async;

    D3D_MAPPED_TEXTURE2D MappedData;

#ifdef USE_DX11
    HW.pContext->Map(pTex, 0, D3D_MAP_READ, 0, &MappedData);
#else
    pTex->Map(0, D3D_MAP_READ, 0, &MappedData);
#endif

    {
        u32* pPixel = (u32*)MappedData.pData;
        u32* pEnd = pPixel + (Device.dwWidth * Device.dwHeight);

        // Kill alpha and swap r and b.
        for (; pPixel != pEnd; pPixel++)
        {
            u32 p = *pPixel;
            *pPixel = color_xrgb(color_get_B(p), color_get_G(p), color_get_R(p));
        }

        memory_writer.w(&Device.dwWidth, sizeof(Device.dwWidth));
        memory_writer.w(&Device.dwHeight, sizeof(Device.dwHeight));
        memory_writer.w(MappedData.pData, (Device.dwWidth * Device.dwHeight) * 4);
    }

#ifdef USE_DX11
    HW.pContext->Unmap(pTex, 0);
#else
    pTex->Unmap(0);
#endif
}

#else // USE_DX10

void CRender::ScreenshotAsyncEnd(CMemoryWriter& memory_writer)
{
    if (!Device.b_is_Ready)
        return;

    VERIFY(!m_bMakeAsyncSS);

    D3DLOCKED_RECT D;
    HRESULT hr;
    IDirect3DSurface9* pFB;

    pFB = Target->pFB;

    hr = pFB->LockRect(&D, nullptr, D3DLOCK_NOSYSLOCK);
    if (hr != D3D_OK)
        return;

#if RENDER == R_R1
    u32 rtWidth = Target->get_rtwidth();
    u32 rtHeight = Target->get_rtheight();
#else // RENDER != R_R1
    u32 rtWidth = Device.dwWidth;
    u32 rtHeight = Device.dwHeight;
#endif // RENDER != R_R1

    // Image processing (gamma-correct)
    u32* pPixel = (u32*)D.pBits;
    u32* pOrigin = pPixel;
    u32* pEnd = pPixel + (rtWidth * rtHeight);

    // Kill alpha
#if RENDER != R_R1
    if (Target->rt_Color->fmt == D3DFMT_A16B16G16R16F)
    {
        static const int iMaxPixelsInARow = 1024;
        D3DXFLOAT16* pPixelElement16 = (D3DXFLOAT16*)pPixel;

        FLOAT tmpArray[4 * iMaxPixelsInARow];
        while (pPixel != pEnd)
        {
            const int iProcessPixels = _min(iMaxPixelsInARow, (s32)(pEnd - pPixel));

            D3DXFloat16To32Array(tmpArray, pPixelElement16, iProcessPixels * 4);

            for (int i = 0; i < iProcessPixels; ++i)
            {
                *pPixel = color_argb_f(1.0f, tmpArray[i * 4], tmpArray[i * 4 + 1], tmpArray[i * 4 + 2]);

                ++pPixel;
            }

            pPixelElement16 += iProcessPixels * 4;
        }
    }
    else
#endif // RENDER != R_R1
    {
        for (; pPixel != pEnd; pPixel++)
        {
            u32 p = *pPixel;
            *pPixel = color_xrgb(color_get_R(p), color_get_G(p), color_get_B(p));
        }
    }

	memory_writer.w(&rtWidth, sizeof(rtWidth));
	memory_writer.w(&rtHeight, sizeof(rtHeight));
	memory_writer.w(pOrigin, (rtWidth * rtHeight) * 4);

    hr = pFB->UnlockRect();
}

#endif // USE_DX10

void DoAsyncScreenshot() { RImplementation.Target->DoAsyncScreenshot(); }
