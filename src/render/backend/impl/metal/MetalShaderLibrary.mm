//
//  MetalShaderLibrary.mm
//  planet
//
//  Created by Eugene Sturm on 8/8/16.
//

#include "MetalShaderLibrary.h"
#import <Foundation/Foundation.h>
#include <algorithm>
#include "Helpers.h"
#include "MetalEnumAdapter.h"

namespace gfx {

void MetalShaderLibrary::AddLibrary(id<MTLLibrary> mtlLibrary) {
    for (NSString* functionName : [mtlLibrary functionNames]) {
        std::string lowercaseFname = ToLowercase([functionName UTF8String]);
        auto comp                  = [&](const MetalLibraryFunction* f) { return f->functionName == lowercaseFname; };
        if (std::find_if(begin(_functions), end(_functions), comp) != end(_functions)) {
            // dont handle duplicates now
            continue;
        }

        MetalLibraryFunction* f = new MetalLibraryFunction();
        f->mtlLibrary           = mtlLibrary;
        f->mtlFunction          = [mtlLibrary newFunctionWithName:functionName];
        f->functionName         = lowercaseFname.substr(0, lowercaseFname.find_first_of("_"));
        f->type                 = MetalEnumAdapter::fromMTL([f->mtlFunction functionType]);

        _resourceManager->AddResource(f);
        _functions.push_back(f);
    }
    _mtlLibraries.push_back(mtlLibrary);
}

ShaderId MetalShaderLibrary::GetShader(ShaderType type, const std::string& functionName) {
    std::string lowercase = ToLowercase(functionName);
    auto result           = std::find_if(begin(_functions), end(_functions), [&](const MetalLibraryFunction* f) { return f->functionName == lowercase && f->type == type; });

    return result != _functions.end() ? (*result)->resourceId : 0;
}
}
