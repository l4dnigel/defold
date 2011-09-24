#include "res_render_prototype.h"

#include <render/render_ddf.h>

#include "../gamesys.h"

namespace dmGameSystem
{
    void ResourceReloadedCallback(void* user_data, dmResource::SResourceDescriptor* descriptor, const char* name)
    {
        RenderScriptPrototype* prototype = (RenderScriptPrototype*)user_data;
        if (descriptor->m_NameHash == prototype->m_NameHash)
        {
            dmRender::OnReloadRenderScriptInstance(prototype->m_Instance);
        }
    }

    bool AcquireResources(dmResource::HFactory factory, const void* buffer, uint32_t buffer_size,
        dmRender::HRenderContext render_context, RenderScriptPrototype* prototype, const char* filename)
    {
        dmRenderDDF::RenderPrototypeDesc* prototype_desc;
        dmDDF::Result e  = dmDDF::LoadMessage(buffer, buffer_size, &prototype_desc);
        if ( e != dmDDF::RESULT_OK )
        {
            return false;
        }
        bool result = true;
        if (dmResource::FACTORY_RESULT_OK == dmResource::Get(factory, prototype_desc->m_Script, (void**)&prototype->m_Script))
        {
            if (prototype->m_Instance == 0x0)
            {
                dmResource::SResourceDescriptor descriptor;
                if (dmResource::FACTORY_RESULT_OK == dmResource::GetDescriptor(factory, prototype_desc->m_Script, &descriptor))
                    prototype->m_NameHash = descriptor.m_NameHash;
                prototype->m_Instance = dmRender::NewRenderScriptInstance(render_context, prototype->m_Script);
            }
            else
            {
                dmRender::SetRenderScriptInstanceRenderScript(prototype->m_Instance, prototype->m_Script);
                dmRender::ClearRenderScriptInstanceMaterials(prototype->m_Instance);
            }
            prototype->m_Materials.SetCapacity(prototype_desc->m_Materials.m_Count);
            for (uint32_t i = 0; i < prototype_desc->m_Materials.m_Count; ++i)
            {
                dmRender::HMaterial material;
                if (dmResource::FACTORY_RESULT_OK == dmResource::Get(factory, prototype_desc->m_Materials[i].m_Material, (void**)&material))
                    prototype->m_Materials.Push(material);
                else
                    break;
            }
            if (!prototype->m_Materials.Full())
            {
                result = false;
            }
            else
            {
                for (uint32_t i = 0; i < prototype->m_Materials.Size(); ++i)
                {
                    dmRender::AddRenderScriptInstanceMaterial(prototype->m_Instance, prototype_desc->m_Materials[i].m_Name, prototype->m_Materials[i]);
                }
            }
        }
        else
        {
            result = false;
        }
        dmDDF::FreeMessage(prototype_desc);
        return result;
    }

    void ReleaseResources(dmResource::HFactory factory, RenderScriptPrototype* prototype)
    {
        if (prototype->m_Script)
            dmResource::Release(factory, prototype->m_Script);
        for (uint32_t i = 0; i < prototype->m_Materials.Size(); ++i)
            dmResource::Release(factory, prototype->m_Materials[i]);
    }

    dmResource::CreateResult ResRenderPrototypeCreate(dmResource::HFactory factory,
            void* context,
            const void* buffer, uint32_t buffer_size,
            dmResource::SResourceDescriptor* resource,
            const char* filename)
    {
        dmRender::HRenderContext render_context = (dmRender::HRenderContext)context;
        RenderScriptPrototype* prototype = new RenderScriptPrototype();
        memset(prototype, 0, sizeof(RenderScriptPrototype));
        if (AcquireResources(factory, buffer, buffer_size, render_context, prototype, filename))
        {
            resource->m_Resource = (void*) prototype;
            dmResource::RegisterResourceReloadedCallback(factory, ResourceReloadedCallback, prototype);
            return dmResource::CREATE_RESULT_OK;
        }
        else
        {
            ReleaseResources(factory, prototype);
            if (prototype->m_Instance)
                dmRender::DeleteRenderScriptInstance(prototype->m_Instance);
            delete prototype;
            return dmResource::CREATE_RESULT_UNKNOWN;
        }
    }

    dmResource::CreateResult ResRenderPrototypeDestroy(dmResource::HFactory factory,
            void* context,
            dmResource::SResourceDescriptor* resource)
    {
        RenderScriptPrototype* prototype = (RenderScriptPrototype*)resource->m_Resource;
        ReleaseResources(factory, prototype);
        if (prototype->m_Instance)
            dmRender::DeleteRenderScriptInstance(prototype->m_Instance);
        dmResource::UnregisterResourceReloadedCallback(factory, ResourceReloadedCallback, prototype);
        delete prototype;
        return dmResource::CREATE_RESULT_OK;
    }

    dmResource::CreateResult ResRenderPrototypeRecreate(dmResource::HFactory factory,
            void* context,
            const void* buffer, uint32_t buffer_size,
            dmResource::SResourceDescriptor* resource,
            const char* filename)
    {
        dmRender::HRenderContext render_context = (dmRender::HRenderContext)context;
        RenderScriptPrototype* prototype = (RenderScriptPrototype*)resource->m_Resource;
        RenderScriptPrototype tmp_prototype;
        memset(&tmp_prototype, 0, sizeof(RenderScriptPrototype));
        tmp_prototype.m_Instance = prototype->m_Instance;
        if (AcquireResources(factory, buffer, buffer_size, render_context, &tmp_prototype, filename))
        {
            ReleaseResources(factory, prototype);
            prototype->m_Script = tmp_prototype.m_Script;
            prototype->m_Materials.Swap(tmp_prototype.m_Materials);
            return dmResource::CREATE_RESULT_OK;
        }
        else
        {
            ReleaseResources(factory, &tmp_prototype);
            return dmResource::CREATE_RESULT_UNKNOWN;
        }
    }
}
