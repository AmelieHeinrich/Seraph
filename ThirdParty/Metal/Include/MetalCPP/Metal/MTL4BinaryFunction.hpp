//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Metal/MTL4BinaryFunction.hpp
//
// Copyright 2020-2025 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLLibrary.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class BinaryFunctionReflection;

class BinaryFunctionReflection : public NS::Referencing<BinaryFunctionReflection>
{
public:
    static BinaryFunctionReflection* alloc();

    BinaryFunctionReflection*        init();
};
class BinaryFunction : public NS::Referencing<BinaryFunction>
{
public:
    MTL::FunctionType         functionType() const;

    NS::String*               name() const;

    BinaryFunctionReflection* reflection() const;
};

}
_MTL_INLINE MTL4::BinaryFunctionReflection* MTL4::BinaryFunctionReflection::alloc()
{
    return NS::Object::alloc<MTL4::BinaryFunctionReflection>(_MTL_PRIVATE_CLS(MTL4BinaryFunctionReflection));
}

_MTL_INLINE MTL4::BinaryFunctionReflection* MTL4::BinaryFunctionReflection::init()
{
    return NS::Object::init<MTL4::BinaryFunctionReflection>();
}

_MTL_INLINE MTL::FunctionType MTL4::BinaryFunction::functionType() const
{
    return Object::sendMessage<MTL::FunctionType>(this, _MTL_PRIVATE_SEL(functionType));
}

_MTL_INLINE NS::String* MTL4::BinaryFunction::name() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(name));
}

_MTL_INLINE MTL4::BinaryFunctionReflection* MTL4::BinaryFunction::reflection() const
{
    return Object::sendMessage<MTL4::BinaryFunctionReflection*>(this, _MTL_PRIVATE_SEL(reflection));
}
