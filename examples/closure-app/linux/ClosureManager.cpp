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

#include "ClosureManager.h"
#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {

// Define a constant for the countdown time
constexpr uint32_t kCountdownTimeSeconds = 10;

// Define the Namespace and Tag for the endpoint
// Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure.adoc
constexpr uint8_t kNamespaceClosure   = 0x44;
constexpr uint8_t kTagClosureCovering = 0x00;
// Derived from
// https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure-Covering.adoc
constexpr uint8_t kNamespaceCovering   = 0x46;
constexpr uint8_t kTagCoveringVenetian = 0x03;
// Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-ClosurePanel.adoc
constexpr uint8_t kNamespaceClosurePanel = 0x45;
constexpr uint8_t kTagClosurePanelLift   = 0x00;
constexpr uint8_t kTagClosurePanelTilt   = 0x01;

// Define the list of semantic tags for the endpoint
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp1TagList[] = {
    { .namespaceID = kNamespaceClosure,
      .tag         = kTagClosureCovering,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) },
    { .namespaceID = kNamespaceCovering,
      .tag         = kTagCoveringVenetian,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp2TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelLift,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp3TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelTilt,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Tilt"_span)) },
};

} // namespace

ClosureManager ClosureManager::sClosureMgr;

void ClosureManager::Init()
{
  ChipLogError(AppServer, "############In ClosureManager::Init ###############");
  
    // Closure endpoints initialization
    ep1.Init();
    ep2.Init();
    ep3.Init();

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp1TagList));
    SetTagList(/* endpoint= */ 2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp2TagList));
    SetTagList(/* endpoint= */ 3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp3TagList));

    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();
  
    if (pTestEventDelegate != nullptr)
    {
        CHIP_ERROR err = pTestEventDelegate->AddHandler(&ep1.GetDelegate());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to add handler for delegate: %s", chip::ErrorStr(err));
        }
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot add handler for delegate");
    }

    ChipLogError(AppServer, "############ClosureManager::Init Done###############");
}

void UpdateCurrentStateToNextPosition(
                                chip::app::Clusters::ClosureDimension::ClusterState & epState,
                                DataModel::Nullable<GenericCurrentStateStruct> & currentState) 

{
  ChipLogError(AppServer, "############IN UpdateCurrentStateToNextPosition ###############");

    if (epState.target.IsNull())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Target State is null");
      return;
  }

  if (!epState.target.Value().position.HasValue())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to  Target position is not set");
      return;
  }

  if (epState.currentState.IsNull())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current State is null");
      return;
  }

  if (!epState.currentState.Value().position.HasValue())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current position is not set");
      return;
  }

  VerifyOrDie(epState.currentState.Value().position.HasValue());
  VerifyOrDie(epState.target.Value().position.HasValue());

  chip::Percent100ths presentCurrentPosition = epState.currentState.Value().position.Value();
  chip::Percent100ths targetPosition = epState.target.Value().position.Value();
  chip::Percent100ths nextCurrentPosition;

  if (presentCurrentPosition < targetPosition)
  {
      // Increment position by 1000 units, capped at target.
      nextCurrentPosition = std::min(static_cast<chip::Percent100ths>(presentCurrentPosition + 1000), targetPosition);
  }
  else if (presentCurrentPosition > targetPosition)
  {
      // Moving down: Decreasing the current position by a step of 1000 units, 
      // ensuring it does not go below the target position.
      nextCurrentPosition = std::max(static_cast<chip::Percent100ths>(presentCurrentPosition - 1000), targetPosition);
  }
  else
  {
      // Already at target: No further action is needed as the current position matches the target position.
      nextCurrentPosition = presentCurrentPosition;
  }
  
  currentState.SetNonNull().Set(
            MakeOptional(nextCurrentPosition),
            epState.currentState.Value().latch.HasValue() ? MakeOptional(epState.currentState.Value().latch.Value()) : NullOptional,
            epState.currentState.Value().speed.HasValue() ? MakeOptional(epState.currentState.Value().speed.Value()) : NullOptional
  );
}

void ClosureManager::HandleCalibrateActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    ChipLogError(AppServer, "############HandleCalibrateActionTimer###############");
    if (manager != nullptr)
    {
        manager->HandleClosureAction(ClosureManager::Action_t::CALIBRATE_ACTION);
    }
    else
    {
        ChipLogError(AppServer, "HandleCalibrateActionTimer called with null manager");
    }
}

void ClosureManager::HandleStopActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    ChipLogError(AppServer, "############HandleStopActionTimer###############");
    
    if (manager != nullptr)
    {
      ClosureManager::Action_t action;
      if (manager->isCalibrationInProgress ) 
      {
        ChipLogError(AppServer, "Stopping calibration action"); 
        manager->isCalibrationInProgress = false;
        action = ClosureManager::Action_t::STOP_CALIBRATE_ACTION; 
      }
      else if (manager->isMoveToInProgress)
      {
        ChipLogError(AppServer, "Stopping Motion action");
        manager->isMoveToInProgress = false;
        action = ClosureManager::Action_t::STOP_MOTION_ACTION;
      } 
      else
      {
        action = ClosureManager::Action_t::INVALID_ACTION;
      }

        manager->HandleClosureAction(action);

    }
    else
    {
        ChipLogError(AppServer, "HandleStopActionTimer called with null manager. aAppState: %p, System state might be uninitialized or improperly configured.", aAppState);
    }
}

void ClosureManager::HandleMoveToActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
  ChipLogError(AppServer, "############HandleMoveToActionTimer###############");
    if (manager != nullptr)
    {
        manager->HandleMotionAction();
    }
    else
    {
        ChipLogError(AppServer, "HandleMoveToActionTimer called with null manager");
    }

}

void ClosureManager::HandleMotionAction()
{
  ChipLogError(AppServer, "############IN HandleMotionAction ###############");
  ClosureManager & instance = ClosureManager::GetInstance();

  chip::app::Clusters::ClosureDimension::ClusterState ep2State = instance.ep2.GetLogic().GetState();
  chip::app::Clusters::ClosureDimension::ClusterState ep3State = instance.ep3.GetLogic().GetState();

  DataModel::Nullable<GenericCurrentStateStruct> currentState = DataModel::NullNullable;
  UpdateCurrentStateToNextPosition(ep2State, currentState);
  VerifyOrReturn(!currentState.IsNull(), ChipLogError(AppServer, "Updating Current state to next position failed for EndPoint 2"));
  instance.ep2.GetLogic().SetCurrentState(currentState);
  bool isEndPoint2ProgressPossible = (currentState.Value().position.Value() != ep2State.target.Value().position.Value());
  ChipLogError(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", currentState.Value().position.Value(), 
                                                                                          ep2State.target.Value().position.Value());

  UpdateCurrentStateToNextPosition(ep3State, currentState);
  VerifyOrReturn(!currentState.IsNull(), ChipLogError(AppServer, "Updating Current state to next position failed for EndPoint 3"));
  instance.ep3.GetLogic().SetCurrentState(currentState);
  bool isEndPoint3ProgressPossible = (currentState.Value().position.Value() != ep3State.target.Value().position.Value());
  ChipLogError(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d", currentState.Value().position.Value(), 
                                                                                          ep3State.target.Value().position.Value());

  bool progressPossible = isEndPoint2ProgressPossible || isEndPoint3ProgressPossible;

  ChipLogError(AppServer, "Progress Possible: %s", progressPossible ? "true" : "false");

    if (progressPossible)
    {
      DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleMoveToActionTimer, this);
    }
    else
    {
      // Target reached or progress not possible, call HandleClosureAction
      instance.HandleClosureAction(ClosureManager::Action_t::MOVE_TO_ACTION);
    }
}

void ClosureManager::HandleClosureAction(ClosureManager::Action_t action)
{
  
  ChipLogError(AppServer, "############IN HandleClosureAction ###############");
  ClosureManager & instance = ClosureManager::GetInstance();

  switch (action)
  {
    case ClosureManager::Action_t::CALIBRATE_ACTION:
    {
      // Perform hardware calibration
      ChipLogError(AppServer, "############IN CALIBRATE_ACTION###############");
      isCalibrationInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      ChipLogError(AppServer, "############IN CALIBRATE_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::STOP_MOTION_ACTION:
    {
      // Perform hardware stop motion action
      ChipLogError(AppServer, "############IN STOP_MOTION_ACTION ###############");
      isMoveToInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_MOTION_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::STOP_MOTION_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::STOP_MOTION_ACTION);
      ChipLogError(AppServer, "############IN STOP_MOTION_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::STOP_CALIBRATE_ACTION:
    {
      // Perform hardware stop action
      ChipLogError(AppServer, "############IN STOP_CALIBRATE_ACTION###############");
      instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_CALIBRATE_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::STOP_CALIBRATE_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::STOP_CALIBRATE_ACTION);
      ChipLogError(AppServer, "############IN STOP_CALIBRATE_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::MOVE_TO_ACTION:
    {
      // Perform hardware move to action
      ChipLogError(AppServer, "############IN MOVE_TO_ACTION ###############");
      isMoveToInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
      ChipLogError(AppServer, "############IN MOVE_TO_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::INVALID_ACTION:
            ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
            break;
        default:
            break;
  }
}


chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand(DataModel::Nullable<ElapsedS> & countdownTime)
{
  ChipLogError(AppServer, "########### OnCalibrateCommand ###################");
  
  DeviceLayer::SystemLayer().CancelTimer(HandleCalibrateActionTimer, this);

  VerifyOrReturnValue(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set countdown time for calibration"));
  VerifyOrReturnValue(ep1.GetLogic().SetOverallState(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set overall state for calibration"));
  VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set overall target for calibration"));
  VerifyOrReturnValue(ep2.GetLogic().SetCurrentState(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set current state for calibration on Endpoint 2"));
  VerifyOrReturnValue(ep2.GetLogic().SetTarget(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set target for calibration on Endpoint 2"));
  VerifyOrReturnValue(ep3.GetLogic().SetCurrentState(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set current state for calibration on Endpoint 3"));
  VerifyOrReturnValue(ep3.GetLogic().SetTarget(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set target for calibration on Endpoint 3"));

  

  DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kCountdownTimeSeconds), HandleCalibrateActionTimer, this);
  isCalibrationInProgress = true;

  return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
  ChipLogError(AppServer, "########### OnStopCommand ###################");
  DeviceLayer::SystemLayer().CancelTimer(HandleStopActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleCalibrateActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleMoveToActionTimer, this);
  DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kCountdownTimeSeconds), HandleStopActionTimer, this);
  return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> & position, 
                                                                          const Optional<bool> & latch,
                                                                          const Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                                                                          chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime)
{
  
    DeviceLayer::SystemLayer().CancelTimer(HandleMoveToActionTimer, this);

    chip::app::Clusters::ClosureDimension::ClusterState ep2State = ep2.GetLogic().GetState();

    chip::app::Clusters::ClosureDimension::ClusterState ep3State = ep3.GetLogic().GetState();

    DataModel::Nullable<GenericCurrentStateStruct> ep2CurrentState = ep2State.currentState;

    DataModel::Nullable<GenericCurrentStateStruct> ep3CurrentState = ep3State.currentState;

    VerifyOrReturnValue(!ep2CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));

    VerifyOrReturnValue(!ep3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "HandleMoveToActionTimer called with null Current state on Panel 3"));

    VerifyOrReturnValue(ep2CurrentState.Value().position.HasValue(), Status::Failure,
                        ChipLogError(AppServer, "HandleMoveToActionTimer called with null position value on Current state of Panel 2"));

    VerifyOrReturnValue(ep3CurrentState.Value().position.HasValue(), Status::Failure,
                        ChipLogError(AppServer, "HandleMoveToActionTimer called with null position value on Current state of Panel 3"));

    auto CreateTargetStruct = [](ElapsedS panelPosition, const Optional<bool> & panelLatch, 
            const Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & panelSpeed) -> GenericTargetStruct {
        return GenericTargetStruct{
            MakeOptional(panelPosition),
            panelLatch.HasValue() ? MakeOptional(panelLatch.Value()) : NullOptional,
            panelSpeed.HasValue() ? MakeOptional(panelSpeed.Value()) : NullOptional
        };
    };

    ElapsedS ep2Position;
    ElapsedS ep3Position;

    if ( position.HasValue() && position.Value() == TargetPositionEnum::kCloseInFull ) 
    {
        ep2Position = ElapsedS{ 10000 };
        ep3Position = ElapsedS{ 10000 };
    }
    else if (position.HasValue() && position.Value() == TargetPositionEnum::kOpenInFull ) 
    {
        ep2Position = ElapsedS{ 0 }; 
        ep3Position = ElapsedS{ 0 }; 
    } 
    else 
    {
        isMoveToInProgress = false;
        ChipLogError(AppServer, "Invalid target position for move to command");
        return Status::Failure;
    } 
    ChipLogError(AppServer, "Target Position for Panel 2: %d, Target Position for Panel 3: %d", ep2Position, ep3Position);
    VerifyOrReturnError(ep2.GetLogic().SetTarget(MakeNullable(CreateTargetStruct(ep2Position, latch, speed))) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Panel 2"));
    VerifyOrReturnError(ep3.GetLogic().SetTarget(MakeNullable(CreateTargetStruct(ep3Position, latch, speed))) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Panel 3"));
    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Panel 1"));

    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleMoveToActionTimer, this);
    isMoveToInProgress = true;

    return Status::Success;
}
