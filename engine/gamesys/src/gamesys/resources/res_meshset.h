// Copyright 2020 The Defold Foundation
// Licensed under the Defold License version 1.0 (the "License"); you may not use
// this file except in compliance with the License.
// 
// You may obtain a copy of the License, together with FAQs at
// https://www.defold.com/license
// 
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#ifndef DM_GAMESYS_RES_MESHSET_H
#define DM_GAMESYS_RES_MESHSET_H

#include <stdint.h>

#include <resource/resource.h>
#include <rig/rig_ddf.h>

namespace dmGameSystem
{
    struct MeshSetResource
    {
        dmRigDDF::MeshSet* m_MeshSet;
    };

    dmResource::Result ResMeshSetPreload(const dmResource::ResourcePreloadParams& params);

    dmResource::Result ResMeshSetCreate(const dmResource::ResourceCreateParams& params);

    dmResource::Result ResMeshSetDestroy(const dmResource::ResourceDestroyParams& params);

    dmResource::Result ResMeshSetRecreate(const dmResource::ResourceRecreateParams& params);
}

#endif // DM_GAMESYS_RES_MESHSET_H
