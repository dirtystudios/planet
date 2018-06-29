//
//  AttachmentDesc.h
//  planet
//
//  Created by Eugene Sturm on 6/20/18.
//

#pragma once

#include "PixelFormat.h"
#include "LoadAction.h"
#include "StoreAction.h"

namespace gfx
{
    struct AttachmentDesc
    {
        PixelFormat format { PixelFormat::Invalid };
        LoadAction loadAction { LoadAction::DontCare };
        StoreAction storeAction { StoreAction::DontCare };
    };
}
