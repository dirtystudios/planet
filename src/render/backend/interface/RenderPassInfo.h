//
//  RenderPassInfo.h
//  planet
//
//  Created by Eugene Sturm on 6/20/18.
//

#pragma once

#include "AttachmentDesc.h"

namespace gfx
{
    struct RenderPassInfo
    {
        AttachmentDesc attachments[10];
        uint32_t attachmentCount { 0 };
        
        AttachmentDesc stencilAttachment;
        bool hasStencil { false };
        
        AttachmentDesc depthAttachment;
        bool hasDepth { false };
    };
}
