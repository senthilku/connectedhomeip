/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

inline PositioningEnum getStatePositionFromTarget(TargetPositionEnum tagPosition)
{
    switch (tagPosition)
    {
    case TargetPositionEnum::kCloseInFull:
        return PositioningEnum::kFullyClosed;
    case TargetPositionEnum::kOpenInFull:
        return PositioningEnum::kFullyOpened;
    case TargetPositionEnum::kPedestrian:
        return PositioningEnum::kOpenedForPedestrian;
    case TargetPositionEnum::kVentilation:
        return PositioningEnum::kOpenedForVentilation;
    case TargetPositionEnum::kSignature:
        return PositioningEnum::kOpenedAtSignature;
    default:
        break;
    }
    return PositioningEnum::kUnknownEnumValue;
}

/**
 * Structure represents the overall target state of a closure control cluster derivation instance.
 */
struct GenericOverallTarget : public Structs::OverallTargetStruct::Type
{
    GenericOverallTarget(Optional<TargetPositionEnum> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
                         Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        Set(positionValue, latchValue, speedValue);
    }

    GenericOverallTarget(const GenericOverallTarget & overallTarget) { *this = overallTarget; }

    GenericOverallTarget & operator=(const GenericOverallTarget & overallTarget)
    {
        Set(overallTarget.position, overallTarget.latch, overallTarget.speed);
        return *this;
    }

    void Set(Optional<TargetPositionEnum> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        position = positionValue;
        latch    = latchValue;
        speed    = speedValue;
    }

    bool operator==(const Structs::OverallTargetStruct::Type & rhs) const
    {
        return position == rhs.position && latch == rhs.latch && speed == rhs.speed;
    }
};

/**
 * Structure represents the overall state of a closure control cluster derivation instance.
 */
struct GenericOverallState : public Structs::OverallStateStruct::Type
{
    GenericOverallState(Optional<DataModel::Nullable<PositioningEnum>> positioningValue       = NullOptional,
                        Optional<DataModel::Nullable<bool>> latchValue                        = NullOptional,
                        Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>> speedValue = NullOptional,
                        Optional<DataModel::Nullable<bool>> secureStateValue                  = NullOptional)
    {
        Set(positioningValue, latchValue, speedValue, secureStateValue);
    }

    GenericOverallState(const GenericOverallState & overallState) { *this = overallState; }

    GenericOverallState & operator=(const GenericOverallState & overallState)
    {
        Set(overallState.positioning, overallState.latch, overallState.speed, overallState.secureState);
        return *this;
    }

    GenericOverallState & operator=(const GenericOverallTarget & overallTarget)
    {
        GenericOverallState overallState;

        if (overallTarget.position.HasValue())
        {
            overallState.positioning.Value() = getStatePositionFromTarget(overallTarget.position.Value());
        }
        else
        {
            overallState.positioning = NullOptional;
        }
        Set(overallState.positioning, overallTarget.latch, overallTarget.speed);
        return *this;
    }

    void Set(Optional<DataModel::Nullable<PositioningEnum>> positioningValue       = NullOptional,
             Optional<DataModel::Nullable<bool>> latchValue                        = NullOptional,
             Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>> speedValue = NullOptional,
             Optional<DataModel::Nullable<bool>> secureStateValue                  = NullOptional)
    {
        positioning = positioningValue;
        latch       = latchValue;
        speed       = speedValue;
        secureState = secureStateValue;
    }

    void Set(Optional<TargetPositionEnum> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        if (positionValue.HasValue())
        {
            positioning.Value() = getStatePositionFromTarget(positionValue.Value());
        }
        else
        {
            positioning = NullOptional;
        }
        latch = latchValue;
        speed = speedValue;
    }

    bool operator==(const Structs::OverallStateStruct::Type & rhs) const
    {
        return positioning == rhs.positioning && latch == rhs.latch && speed == rhs.speed && secureState == rhs.secureState;
    }
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
