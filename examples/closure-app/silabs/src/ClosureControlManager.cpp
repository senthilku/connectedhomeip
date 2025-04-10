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
#include <ClosureControlManager.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

// Mock Error List generated for sample application usage.
const ClosureErrorEnum kCurrentErrorList[] = {
    {
        ClosureErrorEnum::kPhysicallyBlocked,
    },
    {
        ClosureErrorEnum::kBlockedBySensor,
    },
    {
        ClosureErrorEnum::kInternalInterference,
    },
    {
        ClosureErrorEnum::kMaintenanceRequired,
    },
    {
        ClosureErrorEnum::kTemperatureLimited,
    },
};

void ClosureControlManager::SetClosureControlInstance(ClosureControl::Instance & instance)
{
    mpClosureControlInstance = &instance;
}

Instance * ClosureControlManager::GetClosureControlInstance()
{
    return mpClosureControlInstance;
}

CHIP_ERROR ClosureControlManager::Init()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    features = mpClosureControlInstance->GetFeatures();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return CHIP_NO_ERROR;
}

void ClosureControlManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

DataModel::Nullable<uint32_t> ClosureControlManager::GetCountdownTime()
{
    if (mCountDownTime.IsNull())
        return DataModel::NullNullable;

    return DataModel::MakeNullable((uint32_t) (mCountDownTime.Value() - (mMovingTime + mCalibratingTime)));
}

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data)
{
    ClosureControlManager * delegate = reinterpret_cast<ClosureControlManager *>(data);

    Instance * instance = delegate->GetClosureControlInstance();

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    MainStateEnum state = instance->GetMainState();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    auto countdown_time = delegate->GetCountdownTime();

    if (countdown_time.IsNull() || (!countdown_time.IsNull() && countdown_time.Value() > 0))
    {
        if (state == MainStateEnum::kMoving)
        {
            delegate->mMovingTime++;
        }
        if (state == MainStateEnum::kCalibrating)
        {
            delegate->mCalibratingTime++;
        }
        if (state == MainStateEnum::kWaitingForMotion)
        {
            delegate->mMovingTime++;
            if (delegate->IsDeviceReadytoMove())
            {
                delegate->HandleMotion(false, true);
            }
        }
    }
    else if (!countdown_time.IsNull() && countdown_time.Value() <= 0)
    {
        delegate->HandleCountdownTimeExpired();
    }

    if (state == MainStateEnum::kMoving || state == MainStateEnum::kCalibrating || state == MainStateEnum::kWaitingForMotion)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, delegate);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    else
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, delegate);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
}

void ClosureControlManager::HandleCountdownTimeExpired()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
    MainStateEnum state = mpClosureControlInstance->GetMainState();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (state == MainStateEnum::kCalibrating)
    {
        mCalibratingTime = 0;
    }

    if (state == MainStateEnum::kMoving)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        mpClosureControlInstance->PostMovementCompletedEvent();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        
        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(MOVE_ACTION);
        }
    }

    mCountDownTime.SetNull();

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    mpClosureControlInstance->SetMainState(MainStateEnum::kStopped);
    mpClosureControlInstance->UpdateCountdownTimeFromDelegate();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

// Errorlist is being read and data is locked, no update should happend to list
CHIP_ERROR ClosureControlManager::StartCurrentErrorListRead()
{
    return CHIP_NO_ERROR;
}

// TODO: Return error list instead of emulated list
CHIP_ERROR ClosureControlManager::GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError)
{
    if (Index >= MATTER_ARRAY_SIZE(kCurrentErrorList))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    closureError = kCurrentErrorList[Index];

    return CHIP_NO_ERROR;
}

// Errorlist read is completed and lock on data is removed
CHIP_ERROR ClosureControlManager::EndCurrentErrorListRead()
{
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ClosureControlManager::Stop()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
    MainStateEnum state = mpClosureControlInstance->GetMainState();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (state == MainStateEnum::kCalibrating)
    {
        mCalibratingTime = 0;
    }

    if (state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion)
    {
        mMovingTime = 0;
    }

    if (mActionCompleted_CB)
    {
        mActionCompleted_CB(STOP_ACTION);
    }

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    err = mpClosureControlInstance->PostMovementCompletedEvent();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);

    mCountDownTime.SetNull();
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    mpClosureControlInstance->UpdateCountdownTimeFromDelegate();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return Status::Success;

}

Protocols::InteractionModel::Status ClosureControlManager::MoveTo(const Optional<TargetPositionEnum> & tag,
                                                                  const Optional<bool> & latch,
                                                                  const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    bool motionNeeded = false;
    bool latchNeeded  = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    MainStateEnum state                = mpClosureControlInstance->GetMainState();
    GenericOverallTarget overallTarget = mpClosureControlInstance->GetOverallTarget();
    GenericOverallState overallState   = mpClosureControlInstance->GetOverallState();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    VerifyOrReturnValue(tag.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    if (tag.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(tag.Value()) != TargetPositionEnum::kUnknownEnumValue,
                            Status::ConstraintError);

        if (features.Has(Feature::kPositioning))
        {
            VerifyOrReturnValue(CheckErrorondevice(), Status::Failure);

            if (overallState.positioning.Value() == getStatePositionFromTarget(tag.Value()))
            {
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                mpClosureControlInstance->SetMainState(MainStateEnum::kStopped);
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            }
            else
            {
                motionNeeded           = true;
                overallTarget.position = tag;
            }
        }
    }

    if (latch.HasValue())
    {
        VerifyOrReturnValue(latch.Value() == true || latch.Value() == false, Status::ConstraintError);

        if (features.Has(Feature::kMotionLatching))
        {
            VerifyOrReturnValue(isManualLatch, Status::InvalidAction);
            if (overallState.latch.Value() != latch.Value())
            {
                latchNeeded         = true;
                overallTarget.latch = latch;
            }
        }
    }

    if (speed.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                            Status::ConstraintError);
        if (features.Has(Feature::kSpeed))
        {
            overallTarget.speed = speed;
            motionNeeded        = true;
        }
    }

    // If device is already at TargetState ,no Action is required will give Status::Success
    VerifyOrReturnValue(motionNeeded || latchNeeded, Status::Success);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    mpClosureControlInstance->SetOverallTarget(overallTarget);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (IsDeviceReadytoMove())
    {
        if (state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion)
        {
            return HandleMotion(latchNeeded, true);
        }
        else
        {
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            err = mpClosureControlInstance->SetMainState(MainStateEnum::kMoving);
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);
            
            return HandleMotion(latchNeeded, false);
            
        }
    }
    else
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        err = mpClosureControlInstance->SetMainState(MainStateEnum::kWaitingForMotion);
        mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleWaitforMotionCountDown));
        mpClosureControlInstance->UpdateCountdownTimeFromDelegate();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);

        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    }
    
    return Status::Success;
}

Protocols::InteractionModel::Status ClosureControlManager::Calibrate()
{
    mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleCalibrateCountDown));

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    mpClosureControlInstance->UpdateCountdownTimeFromDelegate();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (mActionInitiated_CB)
    {
        mActionInitiated_CB(CALIBRATE_ACTION);
    }

    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    return Status::Success;
}

Protocols::InteractionModel::Status ClosureControlManager::HandleMotion(bool latchNeeded, bool NewTarget)
{
    Action_t action = INVALID_ACTION;

    // Target changes when target is in motion
    if (NewTarget)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        action = TARGET_CHANGE_ACTION;
    }
    else
    {
        if (latchNeeded)
        {
            action = MOVE_AND_LATCH_ACTION;
        }
        else
        {
            action = MOVE_ACTION;
        }
    }

    mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleMotionCountDown));

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    mpClosureControlInstance->UpdateCountdownTimeFromDelegate();
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    

    if (mActionInitiated_CB)
    {
        mActionInitiated_CB(action);
    }
    
    return Status::Success;
}

void ClosureControlManager::ClosureControlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId)
{
    //TODO: UI handling of attribute change
    switch (attributeId)
    {
    case Attributes::CountdownTime::Id:
        //Display CountdownTime in UI
        break;
    case Attributes::MainState::Id:
        //Display Mainstate in UI
        break;
    case Attributes::CurrentErrorList::Id:
        //Display ErrorList in UI
        break;
    case Attributes::OverallState::Id:
        //Display Overallstate in UI
        break;
    case Attributes::OverallTarget::Id: 
        //Display TargetState in UI
        break;
    default:
        return;
    }
}

bool ClosureControlManager::CheckErrorondevice()
{
    // TODO: derive error list for currenterrorlist
    bool errorList = false;
    if (errorList)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        mpClosureControlInstance->SetMainState(MainStateEnum::kError);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        return false;
    }

    return true;
}

bool ClosureControlManager::IsDeviceReadytoMove()
{
    // Check if device needs some action before movement.(manufacture specific)
    return true;
}
