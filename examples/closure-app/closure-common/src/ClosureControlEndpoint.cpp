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
#include <ClosureManager.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

namespace {

constexpr ElapsedS kDefaultCountdownTime = 30;

enum class ClosureControlTestEventTrigger : uint64_t
{
    // MainState is SetupRequired(7) Test Event | Simulate that the device is in SetupRequired state
    kMainStateIsSetupReuired = 0x0104000000000000,

    // MainState is Protected(5) Test Event | Simulate that the device is in protected state
    kMainStateIsProtected = 0x0104000000000001,

    // MainState is Disengaged(6) Test Event | Simulate that the device is in disengaged state
    kMainStateIsDisengaged = 0x0104000000000002,

    // MainState Test clear Event | Returns the device to pre-test status for that test event.
    kClearEvent = 0x0104000000000003,

    // MainState is Error(3) Test Event | Simulate that the device is in error state, add at least one element to the
    // CurrentErrorList attribute
    kMainStateIsError = 0x0104000000000004,
};

} // namespace

Status PrintOnlyDelegate::HandleCalibrateCommand(DataModel::Nullable<ElapsedS> & countdownTime)
{
    ChipLogError(AppServer, "###########HandleCalibrateCommand###############");
    mLogic->SetCountdownTimeFromDelegate(30); // Set a default countdown time for calibration
    ClosureManager::GetInstance().OnCalibrateCommand(countdownTime);
    return Status::Success;
}

Status PrintOnlyDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                              const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                              DataModel::Nullable<ElapsedS> & countdownTime)
{
    ChipLogProgress(AppServer, "###########HandleMoveToCommand###############");
    ClosureManager::GetInstance().OnMoveToCommand(countdownTime);
    return Status::Success;
}

Status PrintOnlyDelegate::HandleStopCommand()
{
    ChipLogProgress(AppServer, "###########HandleStopCommand###############");
    ClosureManager::GetInstance().OnStopCommand();
    return Status::Success;
}

CHIP_ERROR PrintOnlyDelegate::GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError)
{
    // This function should return the current error at the specified index.
    // For now, we dont have a ErrorList implemented, so will return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED.
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

bool PrintOnlyDelegate::IsReadyToMove()
{
    // This function should return true if the closure is ready to move.
    // For now, we will return true.
    return true;
}

bool PrintOnlyDelegate::IsManualLatchingNeeded()
{
    // This function should return true if manual latching is needed.
    // For now, we will return false.
    return false;
}

ElapsedS PrintOnlyDelegate::GetCalibrationCountdownTime()
{
    // This function should return the calibration countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

ElapsedS PrintOnlyDelegate::GetMovingCountdownTime()
{
    // This function should return the moving countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

ElapsedS PrintOnlyDelegate::GetWaitingForMotionCountdownTime()
{
    // This function should return the waiting for motion countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

CHIP_ERROR PrintOnlyDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    eventTrigger                           = clearEndpointInEventTrigger(eventTrigger);
    ClosureControlTestEventTrigger trigger = static_cast<ClosureControlTestEventTrigger>(eventTrigger);
    ClusterLogic * logic                   = GetLogic();
    CHIP_ERROR err                         = CHIP_NO_ERROR;

    switch (trigger)
    {
    case ClosureControlTestEventTrigger::kMainStateIsSetupReuired:
        logic->SetMainState(MainStateEnum::kSetupRequired);
        break;
    case ClosureControlTestEventTrigger::kMainStateIsProtected:
        logic->SetMainState(MainStateEnum::kProtected);
        break;
    case ClosureControlTestEventTrigger::kMainStateIsError:
        logic->SetMainState(MainStateEnum::kError);
        break;
    case ClosureControlTestEventTrigger::kMainStateIsDisengaged:
        logic->SetMainState(MainStateEnum::kDisengaged);
        break;
    case ClosureControlTestEventTrigger::kClearEvent:
        // TODO
        break;
    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    return err;
}

CHIP_ERROR ClosureControlEndpoint::Init()
{
    ClusterConformance conformance;
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kVentilation)
        .Set(Feature::kPedestrian)
        .Set(Feature::kCalibration)
        .Set(Feature::kProtection)
        .Set(Feature::kManuallyOperable);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kCountdownTime);

    ClusterInitParameters initParams;

    ReturnErrorOnFailure(mLogic.Init(conformance, initParams));
    ReturnErrorOnFailure(mInterface.Init());

    return CHIP_NO_ERROR;
}

void ClosureControlEndpoint::OnActionComplete(uint8_t action) 
{
    ChipLogError(AppServer, "#######In OnActionComplete############");
    ClosureManager::Action_t closureAction = static_cast<ClosureManager::Action_t>(action);
    switch (closureAction)
    {
    case ClosureManager::Action_t::STOP_ACTION:
    {
        ChipLogError(AppServer, "#######In STOP_ACTION ############");
        mLogic.SetMainState(MainStateEnum::kStopped);
        ClusterState state = mLogic.GetState();

        if (!state.mOverallState.IsNull())
        {
            const auto & presentState = state.mOverallState.Value();
            state.mOverallState.Value().Set(
                MakeOptional(MakeNullable(PositioningEnum::kPartiallyOpened)),
                presentState.latch.HasValue() && !presentState.latch.Value().IsNull()
                    ? MakeOptional(MakeNullable(presentState.latch.Value().Value()))
                    : NullOptional,
                presentState.speed.HasValue() && !presentState.speed.Value().IsNull()
                    ? MakeOptional(MakeNullable(presentState.speed.Value().Value()))
                    : NullOptional,
                presentState.secureState.HasValue()
                    ? MakeOptional(MakeNullable(presentState.secureState.Value().Value()))
                    : NullOptional
            );
        } else {
            state.mOverallState.SetNonNull().Set(
                MakeOptional(MakeNullable(PositioningEnum::kPartiallyOpened)),
                NullOptional, NullOptional, NullOptional
            );
        }

        mLogic.SetOverallState(state.mOverallState);
        mLogic.SetCountdownTimeFromDelegate(0);
        mLogic.GenerateMovementCompletedEvent();

        ChipLogError(AppServer, "####### STOP_ACTION done ############");
        break;
    }
    case ClosureManager::Action_t::CALIBRATE_ACTION:
    {  
        ChipLogError(AppServer, "#######In CALIBRATE_ACTION ############");
        
        DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(MakeOptional(DataModel::MakeNullable(PositioningEnum::kFullyClosed)),
                            MakeOptional(DataModel::MakeNullable(true)),
                            MakeOptional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kAuto)),
                            MakeOptional(DataModel::MakeNullable(true))));
        DataModel::Nullable<GenericOverallTarget> overallTarget = DataModel::NullNullable;
        
        mLogic.SetMainState(MainStateEnum::kStopped);
        mLogic.SetOverallState(overallState);
        mLogic.SetOverallTarget(overallTarget);
        mLogic.SetCountdownTimeFromDelegate(0);
        mLogic.GenerateMovementCompletedEvent();

        ChipLogError(AppServer, "####### CALIBRATE_ACTION done ############");
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
