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

#include <ClosureDimensionEndpoint.h>
#include <ClosureManager.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters::ClosureDimension;

using Protocols::InteractionModel::Status;

Status PrintOnlyDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
                                          const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleSetTarget");
    // Add the SetTarget handling logic here
    return Status::Success;
}

Status PrintOnlyDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                     const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleStep");
    // Add the Step handling logic here
    return Status::Success;
}

CHIP_ERROR ClosureDimensionEndpoint::Init()
{
    ClusterConformance conformance;
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kUnit)
        .Set(Feature::kLimitation)
        .Set(Feature::kSpeed);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kOverflow);

    ClusterInitParameters clusterInitParameters;

    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ReturnErrorOnFailure(mInterface.Init());
    return CHIP_NO_ERROR;
}

void ClosureDimensionEndpoint::OnActionComplete(uint8_t action) 
{
    ChipLogError(AppServer, "####### CLDM IN ActionComplete 0############");
    ClosureManager::Action_t closureAction = static_cast<ClosureManager::Action_t>(action);

    if (closureAction == ClosureManager::Action_t::INVALID_ACTION)
    {
        ChipLogError(AppServer, "Invalid action received in OnActionComplete");
        return;
    }

    // Call the logic to handle the action completion
    switch (closureAction)
    {
    case ClosureManager::Action_t::STOP_ACTION:
    {
        ChipLogError(AppServer, "####### CLDM IN STOP_ACTION ############");
        ClusterState state = mLogic.GetState();

        if (!state.currentState.IsNull())
        {
            const auto & presentState = state.currentState.Value();
            state.currentState.Value().Set(
                MakeOptional(5000),
                presentState.latch.HasValue()
                    ? MakeOptional(presentState.latch.Value())
                    : NullOptional,
                presentState.speed.HasValue()
                    ? MakeOptional(presentState.speed.Value())
                    : NullOptional
            );
        } else {
            state.currentState.SetNonNull().Set(
                MakeOptional(5000), NullOptional, NullOptional);
        }


        mLogic.SetCurrentState(state.currentState);

        GenericTargetStruct target(state.currentState.Value().position.HasValue()
                    ? MakeOptional(state.currentState.Value().position.Value())
                    : NullOptional,
                    state.currentState.Value().latch.HasValue()
                    ? MakeOptional(state.currentState.Value().latch.Value())
                    : NullOptional,
                    state.currentState.Value().speed.HasValue()
                    ? MakeOptional(state.currentState.Value().speed.Value())
                    : NullOptional
                    );

        mLogic.SetTarget(DataModel::MakeNullable(target));

        ChipLogError(AppServer, "####### CLDM STOP_ACTION done ############");
        break;
    }
    case ClosureManager::Action_t::CALIBRATE_ACTION:
    {
        ChipLogError(AppServer, "####### CLDM IN CALIBRATE_ACTION ############");
        DataModel::Nullable<GenericCurrentStateStruct> currentState(
            GenericCurrentStateStruct(MakeOptional(10000),
                                      MakeOptional(true),
                                      MakeOptional(Globals::ThreeLevelAutoEnum::kAuto)));
        DataModel::Nullable<GenericTargetStruct> target{ DataModel::NullNullable };

        mLogic.SetCurrentState(currentState);
        mLogic.SetTarget(target);
        mLogic.SetTarget(target);
        ChipLogError(AppServer, "####### CLDM CALIBRATE_ACTION done ############");
        break;
    }
    case ClosureManager::Action_t::MOVE_TO_ACTION:
    {
        //TODO
        break;
    }
    default:
        ChipLogError(AppServer, "Invalid action received in OnActionComplete");
        return;
    }
}
