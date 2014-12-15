
#include <qglobal.h>

#include "fsdk_mgr.h"
#include "javascript/IJavaScript.h"

CJS_RuntimeFactory::~CJS_RuntimeFactory()
{
}

IFXJS_Runtime* CJS_RuntimeFactory::NewJSRuntime(CPDFDoc_Environment* pApp)
{
    Q_UNUSED(pApp);
    return 0;
}

void CJS_RuntimeFactory::DeleteJSRuntime(IFXJS_Runtime* pRuntime)
{
    Q_UNUSED(pRuntime);
}

void CJS_RuntimeFactory::AddRef()
{
    m_nRef++;
}

void CJS_RuntimeFactory::Release()
{
    if (--m_nRef) {
        // ### Shutdown
    }
}

CJS_GlobalData* CJS_RuntimeFactory::NewGlobalData(CPDFDoc_Environment* pApp)
{
    Q_UNUSED(pApp);
    return 0;
}

void CJS_RuntimeFactory::ReleaseGlobalData()
{
}
