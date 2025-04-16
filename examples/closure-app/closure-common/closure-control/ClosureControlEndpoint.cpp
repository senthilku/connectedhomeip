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
#include <ClosureControlEndpoint.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

namespace {
    constexpr chip::Percent100ths LIMIT_RANGE_MIN = 0;
    constexpr chip::Percent100ths LIMIT_RANGE_MAX = 10000;
    constexpr chip::Percent100ths STEP            = 1000;
} // namespace

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

CHIP_ERROR ClosureControlDelegate::Init()
{
    ChipLogProgress(AppServer, "ClosureControlDelegate::Init start");
    GenericCurrentStateStruct current;
    current.position.SetValue(0);
    current.latching.SetValue(LatchingEnum::kNotLatched);
    current.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    CHIP_ERROR err;
    err = GetLogic()->SetCurrentState(current);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetCurrentState failed.");
        return err;
    }
    GenericTargetStruct target;
    target.position.SetValue(0);
    target.latch.SetValue(TargetLatchEnum::kUnlatch);
    target.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    err = GetLogic()->SetTarget(target);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetTarget failed.");
        return err;
    }

    Structs::RangePercent100thsStruct::Type limitRange;
    limitRange.min = LIMIT_RANGE_MIN;
    limitRange.max = LIMIT_RANGE_MAX;

    err = GetLogic()->SetLimitRange(limitRange);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetLimitRange failed.");
        return err;
    }

    err = GetLogic()->SetStepValue(STEP);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetStepValue failed.");
        return err;
    }
    ChipLogProgress(AppServer, "ClosureControlDelegate::Init done");
    return CHIP_NO_ERROR;
}

void ClosureControlDelegate::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

Status ClosureControlDelegate::HandleCalibrateCommand()
{
    return Status::Success;
}

Status ClosureControlDelegate::HandleMoveToCommand()
{
    return Status::Success;
}

Status ClosureControlDelegate::HandleStopCommand()
{
    return Status::Success;
}

void ClosureControlDelegate::SetLogic(const ClusterLogic * logic) 
{ 
    mLogic = const_cast<ClusterLogic *>(logic); 
}

ClusterLogic * ClosureControlDelegate::GetLogic() const 
{
    return mLogic; 
}

bool ClosureControlDelegate::IsDeviceMoving() const 
{ 
    return isMoving; 
}

void ClosureControlDelegate::SetDeviceMoving(const bool moving) 
{ 
    isMoving = moving; 
}

Action_t ClosureControlDelegate::GetAction() const 
{ 
    return mAction; 
}

void ClosureControlDelegate::SetAction(const Action_t action) 
{ 
    mAction = action; 
}