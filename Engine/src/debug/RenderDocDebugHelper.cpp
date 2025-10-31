#include "RenderDocDebugHelper.h"

#include "core/Logger.h"

#define USE_RENDER_DOC

#ifdef USE_RENDER_DOC
#include "C:\Program Files\RenderDoc\renderdoc_app.h"
static RENDERDOC_API_1_6_0* rdoc_api = nullptr;
#endif

void RenderDocDebugHelper::startFrameCapture()
{
#ifdef USE_RENDER_DOC
	
    if (!rdoc_api)
    {
        //RenderDoc injects renderdoc.dll into your process automatically
        HMODULE mod = GetModuleHandleA("renderdoc.dll");
        if (mod)
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI =
                (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");

            if (RENDERDOC_GetAPI)
            {
                int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0,
                    (void**)&rdoc_api);

                if (ret != 1)
                    rdoc_api = nullptr;
            }
        }

        if (!rdoc_api)
        {
            logError("Could not locate renderdoc dll, no renderdoc frames will be captured.");
        }
    }

    if (rdoc_api)
    {
        rdoc_api->StartFrameCapture(nullptr, nullptr);
    }
#endif
}

void RenderDocDebugHelper::stopFrameCapture()
{
#ifdef USE_RENDER_DOC
    if (rdoc_api)
    {
        rdoc_api->EndFrameCapture(nullptr, nullptr);
    }
#endif
}
