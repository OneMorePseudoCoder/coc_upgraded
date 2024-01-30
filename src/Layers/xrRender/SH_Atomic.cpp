#include "stdafx.h"
#pragma hdrstop

#include "sh_atomic.h"
#include "ResourceManager.h"

///////////////////////////////////////////////////////////////////////
//  SVS
SVS::SVS() : sh(0)
{}

SVS::~SVS()
{
    RImplementation.Resources->_DeleteVS(this);

    _RELEASE(sh);
}

///////////////////////////////////////////////////////////////////////
// SPS
SPS::~SPS()
{
    _RELEASE(sh);

    RImplementation.Resources->_DeletePS(this);
}

#if defined(USE_DX10) || defined(USE_DX11)
///////////////////////////////////////////////////////////////////////
// SGS
SGS::~SGS()
{
    _RELEASE(sh);

    RImplementation.Resources->_DeleteGS(this);
}

#if defined(USE_DX11)
SHS::~SHS()
{
    _RELEASE(sh);

    RImplementation.Resources->_DeleteHS(this);
}

SDS::~SDS()
{
    _RELEASE(sh);

    RImplementation.Resources->_DeleteDS(this);
}

SCS::~SCS()
{
    _RELEASE(sh);

    RImplementation.Resources->_DeleteCS(this);
}
#endif
#endif // USE_DX10

#if defined(USE_DX10) || defined(USE_DX11)
///////////////////////////////////////////////////////////////////////
//	SInputSignature
SInputSignature::SInputSignature(ID3DBlob* pBlob)
{
    VERIFY(pBlob);
    signature = pBlob;
    signature->AddRef();
};

SInputSignature::~SInputSignature()
{
    _RELEASE(signature);
    RImplementation.Resources->_DeleteInputSignature(this);
}
#endif	//	USE_DX10

///////////////////////////////////////////////////////////////////////
//	SState
SState::~SState()
{
    _RELEASE(state);
    RImplementation.Resources->_DeleteState(this);
}

///////////////////////////////////////////////////////////////////////
//	SDeclaration
SDeclaration::~SDeclaration()
{
    RImplementation.Resources->_DeleteDecl(this);
#if defined(USE_DX10) || defined(USE_DX11)
    xr_map<ID3DBlob*, ID3DInputLayout*>::iterator iLayout;
    iLayout = vs_to_layout.begin();
    for (; iLayout != vs_to_layout.end(); ++iLayout)
    {
        //	Release vertex layout
        _RELEASE(iLayout->second);
    }
#else // USE_DX10
    //	Release vertex layout
    _RELEASE(dcl);
#endif // USE_DX10
}
