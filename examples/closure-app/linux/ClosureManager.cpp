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
#include <memory>
#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
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

ClosureManager::Action_t mCurrentAction = ClosureManager::Action_t::INVALID_ACTION;

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
  // ChipLogError(AppServer, "############In ClosureManager::Init ###############");
  ChipLogProgress(AppServer, "ClsureManager::Init()");
  
    // Closure endpoints initialization
    VerifyOrDie(ep1.Init() == CHIP_NO_ERROR);
    // ChipLogError(AppServer, "Closure Control Endpoint Ep1 initialized successfully");

    VerifyOrDie(ep2.Init() == CHIP_NO_ERROR);
    // ChipLogError(AppServer, "Closure Dimension Endpoint Ep2 initialized successfully");

    VerifyOrDie(ep3.Init() == CHIP_NO_ERROR);
    // ChipLogError(AppServer, "Closure Dimension Endpoint Ep3 initialized successfully");

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp1TagList));
    SetTagList(/* endpoint= */ 2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp2TagList));
    SetTagList(/* endpoint= */ 3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp3TagList));

    VerifyOrDie(ep1.SetInitialState() == CHIP_NO_ERROR);
    // ChipLogError(AppServer, "Closure Control Endpoint Ep1 initial state set successfully");

    VerifyOrDie(ep2.SetInitialState() == CHIP_NO_ERROR);
    // ChipLogError(AppServer, "Closure Dimension Endpoint Ep2 initial state set successfully");

    VerifyOrDie(ep3.SetInitialState() == CHIP_NO_ERROR);
    // ChipLogError(AppServer, "Closure Dimension Endpoint Ep3 initial state set successfully");

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

    // ChipLogError(AppServer, "############ClosureManager::Init Done###############");
}

bool UpdateCurrentStateToNextPosition(
                                chip::app::Clusters::ClosureDimension::ClusterState & epState,
                                DataModel::Nullable<GenericCurrentStateStruct> & currentState) 

{
  // ChipLogError(AppServer, "############IN UpdateCurrentStateToNextPosition ###############");

  if (epState.target.IsNull())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Target State is null");
      return false;
  }

  if (!epState.target.Value().position.HasValue())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to  Target position is not set");
      return false;
  }

  if (epState.currentState.IsNull())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current State is null");
      return false;
  }

  if (!epState.currentState.Value().position.HasValue())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current position is not set");
      return false;
  }

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
  return true;
}

void ClosureManager::HandleCalibrateActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    // ChipLogError(AppServer, "############HandleCalibrateActionTimer###############");
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
    // ChipLogError(AppServer, "############HandleStopActionTimer###############");
    
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
      else if( manager->isSetTargetInProgress )
      {
        ChipLogError(AppServer, "Stopping SetTarget action");
        manager->isSetTargetInProgress = false;
        action = ClosureManager::Action_t::STOP_SET_TARGET_ACTION;
      }
      else if (manager->isStepActionInProgress)
      {
        ChipLogError(AppServer, "Stopping Step action");
        manager->isStepActionInProgress = false;
        action = ClosureManager::Action_t::STOP_STEP_ACTION;
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

void ClosureManager::HandleMoveToActionTimer(System::Layer * layer, void * data)
{
  ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
  // ChipLogError(AppServer, "############HandleMoveToActionTimer###############");
  if (manager != nullptr)
  {
     manager->HandleMotionAction();
  }
  else
  {
      ChipLogError(AppServer, "HandleMoveToActionTimer called with null manager");
  }
}

void ClosureManager::HandleLatchActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    // ChipLogError(AppServer, "############HandleLatchActionTimer###############");
    if (manager != nullptr)
    {
      manager->HandleClosureAction(mCurrentAction);
    }
    else
    {
      ChipLogError(AppServer, "HandleLatchActionTimer called with null manager");
    }
}

bool IsSpeedUpdateNeeded(const chip::app::Clusters::ClosureControl::ClusterState & ep1State)
{
  // ChipLogError(AppServer, "############IN IsSpeedUpdateNeeded ###############");

  // Check if the speed is set in the target state
  if (ep1State.mOverallTarget.IsNull() || !ep1State.mOverallTarget.Value().speed.HasValue())
  {
    ChipLogError(AppServer, "Speed update is not needed as OverallTarget is null or speed is not set");
    return false;
  }

  // Check if the overall state is valid and has a speed value
  if (ep1State.mOverallState.IsNull() || !ep1State.mOverallState.Value().speed.HasValue() || 
      ep1State.mOverallState.Value().speed.Value().IsNull())
  {
    ChipLogError(AppServer, "Speed update is needed as OverallState is null or speed is not set, while OverallTarget has speed set");
    return true;
  }

  // Only return true if the speed value is different between target and state
  Globals::ThreeLevelAutoEnum targetSpeed = ep1State.mOverallTarget.Value().speed.Value();
  Globals::ThreeLevelAutoEnum stateSpeed = ep1State.mOverallState.Value().speed.Value().Value();
  ChipLogError(AppServer, "Target Speed: %d, State Speed: %d", 
               static_cast<int>(targetSpeed), static_cast<int>(stateSpeed));
  return targetSpeed != stateSpeed;
}


bool IsLatchActionNeeded(const chip::app::Clusters::ClosureControl::ClusterState & ep1State)
{
  // ChipLogError(AppServer, "############IN IsLatchActionNeeded ###############");

  // Check if the latch is set in the target state
  if (ep1State.mOverallTarget.IsNull() || !ep1State.mOverallTarget.Value().latch.HasValue())
  {
    ChipLogError(AppServer, "Latch action is not needed as OverallTarget is null or latch is not set");
    return false;
  }

  // Check if the overall state is valid and has a latch value
  if (ep1State.mOverallState.IsNull() || !ep1State.mOverallState.Value().latch.HasValue() || 
      ep1State.mOverallState.Value().latch.Value().IsNull())
  {
    ChipLogError(AppServer, "Latch action is needed as OverallState is null or latch is not set, while OverallTarget has latch set");
    return true;
  }

  // Only return true if the latch value is different between target and state
  bool targetLatch = ep1State.mOverallTarget.Value().latch.Value();
  bool stateLatch = ep1State.mOverallState.Value().latch.Value().Value();
  ChipLogError(AppServer, "Target Latch: %s, State Latch: %s", 
               targetLatch ? "true" : "false", stateLatch ? "true" : "false");
  return targetLatch != stateLatch;
}

void ClosureManager::HandleMotionAction()
{
  // ChipLogError(AppServer, "############IN HandleMotionAction ###############");
  ClosureManager & instance = ClosureManager::GetInstance();

  chip::app::Clusters::ClosureControl::ClusterState ep1State = instance.ep1.GetLogic().GetState();
  chip::app::Clusters::ClosureDimension::ClusterState ep2State = instance.ep2.GetLogic().GetState();
  chip::app::Clusters::ClosureDimension::ClusterState ep3State = instance.ep3.GetLogic().GetState();

  DataModel::Nullable<GenericCurrentStateStruct> currentState = DataModel::NullNullable;
  bool isEndPoint2ProgressPossible = false;
  bool isEndPoint3ProgressPossible = false;

  //Update the Speed field in Current State before motion if Speed field is changed
   if (IsSpeedUpdateNeeded(ep1State))
   {
       currentState = ep2State.currentState.Value().UpdateSpeed(ep1State.mOverallTarget.Value().speed);
       instance.ep2.GetLogic().SetCurrentState(currentState);
       currentState = ep3State.currentState.Value().UpdateSpeed(ep1State.mOverallTarget.Value().speed);
       instance.ep3.GetLogic().SetCurrentState(currentState);
    }


  if (ep2State.target.Value().position.HasValue() )
  {
    // Update the current state for Endpoint 2
    UpdateCurrentStateToNextPosition(ep2State, currentState);
    VerifyOrReturn(!currentState.IsNull(), ChipLogError(AppServer, "Updating Current state to next position failed for EndPoint 2"));
    instance.ep2.GetLogic().SetCurrentState(currentState);
    isEndPoint2ProgressPossible = (currentState.Value().position.Value() != ep2State.target.Value().position.Value());
    ChipLogError(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", currentState.Value().position.Value(),
                                                                                          ep2State.target.Value().position.Value());
  }

  if (ep3State.target.Value().position.HasValue() )
  {
    // Update the current state for Endpoint 3
    UpdateCurrentStateToNextPosition(ep3State, currentState);
    VerifyOrReturn(!currentState.IsNull(), ChipLogError(AppServer, "Updating Current state to next position failed for EndPoint 3"));
    instance.ep3.GetLogic().SetCurrentState(currentState);
    isEndPoint3ProgressPossible = (currentState.Value().position.Value() != ep3State.target.Value().position.Value());
    ChipLogError(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d", currentState.Value().position.Value(),
                                                                                          ep3State.target.Value().position.Value());
  }

  if (UpdateCurrentStateToNextPosition(ep3State, currentState))
  {
    instance.ep3.GetLogic().SetCurrentState(currentState);
    isEndPoint3ProgressPossible = (currentState.Value().position.Value() != ep3State.target.Value().position.Value());
    ChipLogError(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d", currentState.Value().position.Value(), 
                                                                                            ep3State.target.Value().position.Value());
  }

  bool progressPossible = isEndpoint2ProgressPossible || isEndpoint3ProgressPossible;

  ChipLogError(AppServer, "Progress Possible: %s", progressPossible ? "true" : "false");

    if (progressPossible)
    {
      DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleMoveToActionTimer, this);
      return;
    }

    if (IsLatchActionNeeded(ep1State)){
        ChipLogError(AppServer, "Starting latch action timer");
        mCurrentAction = ClosureManager::Action_t::MOVE_TO_ACTION;
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(2), HandleLatchActionTimer, this);
    } else {
      // Target reached and no latch action needed, call HandleClosureAction
      instance.HandleClosureAction(MOVE_TO_ACTION);
    }
}

void ClosureManager::HandleClosureAction(ClosureManager::Action_t action)
{
  
  // ChipLogError(AppServer, "############IN HandleClosureAction ###############");
  ClosureManager & instance = ClosureManager::GetInstance();

  switch (action)
  {
    case ClosureManager::Action_t::CALIBRATE_ACTION:
    {
      // ChipLogError(AppServer, "############IN CALIBRATE_ACTION###############");
      isCalibrationInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      // ChipLogError(AppServer, "############IN CALIBRATE_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::STOP_MOTION_ACTION:
    {
      // ChipLogError(AppServer, "############IN STOP_MOTION_ACTION ###############");
      isMoveToInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_MOTION_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::STOP_MOTION_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::STOP_MOTION_ACTION);
      // ChipLogError(AppServer, "############IN STOP_MOTION_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::STOP_CALIBRATE_ACTION:
    {
      // ChipLogError(AppServer, "############IN STOP_CALIBRATE_ACTION###############");
      isCalibrationInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_CALIBRATE_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::STOP_CALIBRATE_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::STOP_CALIBRATE_ACTION);
      // ChipLogError(AppServer, "############IN STOP_CALIBRATE_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::MOVE_TO_ACTION:
    {
      // ChipLogError(AppServer, "############IN MOVE_TO_ACTION ###############");
      isMoveToInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
      // ChipLogError(AppServer, "############IN MOVE_TO_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::SET_TARGET_ACTION:
    {
      // ChipLogError(AppServer, "############IN SET_TARGET_ACTION ###############");
      instance.ep1.OnActionComplete(ClosureManager::Action_t::SET_TARGET_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::SET_TARGET_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::SET_TARGET_ACTION);
      // ChipLogError(AppServer, "############IN SET_TARGET_ACTION done ###############");
      break;
    }

    case ClosureManager::Action_t::STEP_ACTION:
    {
      // ChipLogError(AppServer, "############IN STEP_ACTION ###############");
      isStepActionInProgress = false;
      instance.ep1.OnActionComplete(ClosureManager::Action_t::STEP_ACTION);
      instance.ep2.OnActionComplete(ClosureManager::Action_t::STEP_ACTION);
      instance.ep3.OnActionComplete(ClosureManager::Action_t::STEP_ACTION);
      // ChipLogError(AppServer, "############IN STEP_ACTION done ###############");
      break;
    }
    case ClosureManager::Action_t::STOP_SET_TARGET_ACTION:
        // ChipLogError(AppServer, "############IN STOP_SET_TARGET_ACTION ###############");
        isSetTargetInProgress = false;
        instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_SET_TARGET_ACTION);
        instance.ep2.OnActionComplete(ClosureManager::Action_t::STOP_SET_TARGET_ACTION);
        instance.ep3.OnActionComplete(ClosureManager::Action_t::STOP_SET_TARGET_ACTION);
        // ChipLogError(AppServer, "############IN STOP_SET_TARGET_ACTION done ###############");
        break;
    case ClosureManager::Action_t::STOP_STEP_ACTION:
        // ChipLogError(AppServer, "############IN STOP_STEP_ACTION ###############");
        isStepActionInProgress = false;
        instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_STEP_ACTION);
        instance.ep2.OnActionComplete(ClosureManager::Action_t::STOP_STEP_ACTION);
        instance.ep3.OnActionComplete(ClosureManager::Action_t::STOP_STEP_ACTION);
        // ChipLogError(AppServer, "############IN STOP_STEP_ACTION done ###############");
        break;

    case ClosureManager::Action_t::INVALID_ACTION:
            ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
            break;
        default:
            break;
  }
}


chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand(DataModel::Nullable<ElapsedS> & countdownTime)
{
  // ChipLogError(AppServer, "########### OnCalibrateCommand ###################");
  
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
  // ChipLogError(AppServer, "########### OnStopCommand ###################");
  DeviceLayer::SystemLayer().CancelTimer(HandleStopActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleCalibrateActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleMoveToActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleLatchActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleSetTargetActionTimer, this);
  DeviceLayer::SystemLayer().CancelTimer(HandleStepCommandTimer, this);
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
                        ChipLogError(AppServer, "MoveToCommand failed - Current state is Null on Endpoint 2"));

    VerifyOrReturnValue(!ep3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed - Current state is Null on Endpoint 3"));

    GenericTargetStruct ep2Target;
    GenericTargetStruct ep3Target;

    if (!ep2State.target.IsNull())
    {
        ep2Target = ep2State.target.Value();
    }
    else
    {
        ep2Target = GenericTargetStruct();
    }

    if (!ep3State.target.IsNull())
    {
        ep3Target = ep3State.target.Value();
    }
    else
    {
        ep3Target = GenericTargetStruct();
    }

    if (position.HasValue()){

      // ChipLogError(AppServer, "Updating target position for move to command");
        // Set the Closure panel target position for the panels based on the Closure position
        Optional<chip::Percent100ths> ep2Position = NullOptional;
        Optional<chip::Percent100ths> ep3Position = NullOptional;

      if ( position.Value() == TargetPositionEnum::kCloseInFull ) 
      {
          ep2Position = MakeOptional(static_cast<chip::Percent100ths>(10000));
          ep3Position = MakeOptional(static_cast<chip::Percent100ths>(10000));
      }
      else if ( position.Value() == TargetPositionEnum::kOpenInFull ) 
      {
          ep2Position = MakeOptional(static_cast<chip::Percent100ths>(0));
          ep3Position = MakeOptional(static_cast<chip::Percent100ths>(0));
      } 
      else 
      {
          ep2Position = MakeOptional(static_cast<chip::Percent100ths>(5000));
          ep3Position = MakeOptional(static_cast<chip::Percent100ths>(5000));
      }

        ep2Target = ep2Target.UpdatePosition(ep2Position);
        ep3Target = ep3Target.UpdatePosition(ep3Position);
    }

    if (latch.HasValue())
    {
      // ChipLogError(AppServer, "Updating target latch for move to command");
        ep2Target = ep2Target.UpdateLatch(MakeOptional(latch.Value()));
        ep3Target = ep3Target.UpdateLatch(MakeOptional(latch.Value()));
        // ChipLogError(AppServer, "Latch value set to %s for panel 2", ep2Target.latch.Value() ? "true" : "false");
        // ChipLogError(AppServer, "Latch value set to %s for panel 3", ep3Target.latch.Value() ? "true" : "false");
    }

    if (speed.HasValue())
    {
      // ChipLogError(AppServer, "Updating target speed for move to command");
        ep2Target = ep2Target.UpdateSpeed(MakeOptional(speed.Value()));
        ep3Target = ep3Target.UpdateSpeed(MakeOptional(speed.Value()));
        // ChipLogError(AppServer, "Speed value set to %d for panel 2", static_cast<int>(ep2Target.speed.Value()));
        // ChipLogError(AppServer, "Speed value set to %d for panel 3", static_cast<int>(ep3Target.speed.Value()));
    }

    VerifyOrReturnError(ep2.GetLogic().SetTarget(MakeNullable(ep2Target)) == CHIP_NO_ERROR,  Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Panel 2"));
    VerifyOrReturnError(ep3.GetLogic().SetTarget(MakeNullable(ep3Target)) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Panel 3"));
    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Panel 1"));

    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleMoveToActionTimer, this);
    isMoveToInProgress = true;

    return Status::Success;
}

void ClosureManager::HandleSetTargetActionTimer(chip::System::Layer * systemLayer, void * data)
{
  ClosureManager & manager = ClosureManager::GetInstance();
  // ChipLogError(AppServer, "############HandleSetTargetActionTimer###############");
  EndpointId endpointId = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(data));
  manager.HandleSetTargetAction(endpointId);
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & pos, 
                                                                             const Optional<bool> & latch, 
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             chip::EndpointId endpointId)
{
  // ChipLogError(AppServer, "########### OnSetTargetCommand ###################");

  // Set OverallTarget to null
  // TODO: How to update the OverallTarget for the closure?
  DataModel::Nullable<GenericOverallTarget> target;
  VerifyOrReturnValue(ep1.GetLogic().GetOverallTarget(target) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to get overall target for SetTarget command"));
  if (target.IsNull())
  {
    target.SetNonNull(GenericOverallTarget{});
  }

  target.Value().position = NullOptional; // Reset position to null

  if (latch.HasValue())
  {
    // ChipLogError(AppServer, "Updating target latch for SetTarget command");
    target.Value().latch = MakeOptional(latch.Value());
  }

  if (speed.HasValue())
  {
    // ChipLogError(AppServer, "Updating target speed for SetTarget command");
    target.Value().speed = MakeOptional(speed.Value());
  }

  VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(target) == CHIP_NO_ERROR, Status::Failure, 
                      ChipLogError(AppServer, "Failed to set overall target for SetTarget command"));

  (void) DeviceLayer::SystemLayer().CancelTimer(HandleSetTargetActionTimer, reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));

  (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleSetTargetActionTimer, reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));
  isSetTargetInProgress = true;
  // Trigger Motion Action

  return Status::Success;
}

void ClosureManager::HandleSetTargetAction(EndpointId endpointId)
{
  // ChipLogError(AppServer, "############IN HandleSetTargetAction ###############");
  ClosureManager & instance = ClosureManager::GetInstance();
  chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = nullptr;

  if (endpointId == instance.ep2.GetEndpoint())
  {
    ep = &instance.ep2;
  }
  else if (endpointId == instance.ep3.GetEndpoint())
  {
    ep = &instance.ep3;
  }

  if (ep == nullptr)
  {
    ChipLogError(AppServer, "Invalid endpoint ID: %d", endpointId);
    return;
  }

  chip::app::Clusters::ClosureDimension::ClusterState epState = ep->GetLogic().GetState();
  DataModel::Nullable<GenericCurrentStateStruct> currentState = DataModel::NullNullable;
  
  if (epState.target.IsNull() || !epState.target.Value().position.HasValue())
  {
    ChipLogError(AppServer, "Target position is not set for Endpoint %d", endpointId);
    return;
  }
  
  bool isProgressPossible = false;
  
  // Update currentState speed with target speed if needed
  Globals::ThreeLevelAutoEnum targetSpeed = epState.target.Value().speed.HasValue() ? epState.target.Value().speed.Value() : Globals::ThreeLevelAutoEnum::kAuto;
  Globals::ThreeLevelAutoEnum currentSpeed = epState.currentState.Value().speed.HasValue() ? epState.currentState.Value().speed.Value() : Globals::ThreeLevelAutoEnum::kAuto;
  ChipLogError(AppServer, "Target Speed: %d, Current Speed: %d", static_cast<int>(targetSpeed), static_cast<int>(currentSpeed));
  // If the target speed is different from the current speed, update the current state with the target speed
  if (targetSpeed != currentSpeed)
  {
    currentState.SetNonNull().Set(
        epState.currentState.Value().position.HasValue() ? MakeOptional(epState.currentState.Value().position.Value()) : NullOptional,
        epState.currentState.Value().latch.HasValue() ? MakeOptional(epState.currentState.Value().latch.Value()) : NullOptional,
        MakeOptional(targetSpeed)
    );
    ep->GetLogic().SetCurrentState(currentState);
    ChipLogError(AppServer, "Updated Current Speed to Target Speed: %d", static_cast<int>(targetSpeed));
    epState = ep->GetLogic().GetState(); // Refresh the state after updating current speed
  }

  if(UpdateCurrentStateToNextPosition(epState, currentState))
  {
    ep->GetLogic().SetCurrentState(currentState);
    isProgressPossible = (currentState.Value().position.Value() != epState.target.Value().position.Value());
    ChipLogError(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", ep->GetDelegate().GetEndpoint(), currentState.Value().position.Value(),
                                                                                            epState.target.Value().position.Value());
  }

  // ChipLogError(AppServer, "Progress Possible: %s", isProgressPossible ? "true" : "false");

  if (isProgressPossible)
  {
    // If progress is possible, start the timer to continue the action
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleSetTargetActionTimer, reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));
  }
  else
  {
    bool isLatchActionNeeded = false;
    if (epState.target.Value().latch.HasValue())
    {
      // ChipLogError(AppServer, "Target latch value : %s", epState.target.Value().latch.Value() ? "true" : "false");
      if (currentState.Value().latch.HasValue())
      {
        // ChipLogError(AppServer, "Current latch value : %s", currentState.Value().latch.Value() ? "true" : "false");
      }
      if (!currentState.Value().latch.HasValue() || epState.target.Value().latch.Value() != currentState.Value().latch.Value())
      {
        isLatchActionNeeded = true;
      }
    }
    if (isLatchActionNeeded)
    {
      ChipLogError(AppServer, "Starting latch action timer for Endpoint %d", endpointId);
      mCurrentAction = ClosureManager::Action_t::SET_TARGET_ACTION;
      DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(2), HandleLatchActionTimer, (void *) this);
    }
    else
    {
      // If no latch action is needed, call HandleClosureAction to complete the action
      instance.HandleClosureAction(ClosureManager::Action_t::SET_TARGET_ACTION);
    }
  }
}

void ClosureManager::HandleStepAction(EndpointId endpointId)
{
  // ChipLogError(AppServer, "############IN HandleStepAction ###############");
  ClosureManager & instance = ClosureManager::GetInstance();
  Percent100ths stepValue;

  chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = nullptr;
  if (endpointId == instance.ep2.GetDelegate().GetEndpoint())
  {
    ep = &instance.ep2;
  }
  else if (endpointId == instance.ep3.GetDelegate().GetEndpoint())
  {
    ep = &instance.ep3;
  }

  if (ep->GetLogic().GetStepValue(stepValue) != CHIP_NO_ERROR ) {
    ChipLogError(AppServer, "Failed to get step value for Endpoint %d", endpointId);
  }

  StepDirectionEnum stepDirection = ep->GetDelegate().GetTargetDirection();
  chip::app::Clusters::ClosureDimension::ClusterState epState = ep->GetLogic().GetState();

  DataModel::Nullable<GenericCurrentStateStruct> currentState = DataModel::NullNullable;
  bool isProgressPossible = false;

  chip::Percent100ths presentCurrentPosition = epState.currentState.Value().position.Value();
  chip::Percent100ths targetPosition = epState.target.Value().position.Value();
  
  if (presentCurrentPosition == targetPosition)
  {
    ChipLogProgress(AppServer, "Target position reached");
    return ;
  }

  // Update currentState speed with target speed if needed
  Globals::ThreeLevelAutoEnum targetSpeed = epState.target.Value().speed.HasValue() ? epState.target.Value().speed.Value() : Globals::ThreeLevelAutoEnum::kAuto;
  Globals::ThreeLevelAutoEnum currentSpeed = epState.currentState.Value().speed.HasValue() ? epState.currentState.Value().speed.Value() : Globals::ThreeLevelAutoEnum::kAuto;
  ChipLogError(AppServer, "Target Speed: %d, Current Speed: %d", static_cast<int>(targetSpeed), static_cast<int>(currentSpeed));
  // If the target speed is different from the current speed, update the current state with the target speed
  if (targetSpeed != currentSpeed)
  {
    currentState.SetNonNull().Set(
        epState.currentState.Value().position.HasValue() ? MakeOptional(epState.currentState.Value().position.Value()) : NullOptional,
        epState.currentState.Value().latch.HasValue() ? MakeOptional(epState.currentState.Value().latch.Value()) : NullOptional,
        MakeOptional(targetSpeed)
    );
    ep->GetLogic().SetCurrentState(currentState);
    ChipLogError(AppServer, "Updated Current Speed to Target Speed: %d", static_cast<int>(targetSpeed));
    epState = ep->GetLogic().GetState(); // Refresh the state after updating current speed
  }
  
  chip::Percent100ths nextCurrentPosition;
  // Increment or decrement position by stepValue, capped at target.
  if (stepDirection == StepDirectionEnum::kIncrease)
  {
      // Moving up: Increment the current position by stepValue, ensuring it does not exceed the target position.
      nextCurrentPosition = std::min(static_cast<chip::Percent100ths>(presentCurrentPosition + stepValue), targetPosition);
  }
  else
  {
      // Moving down: Decrement the current position by stepValue, ensuring it does not go below the target position.
      nextCurrentPosition = std::max(static_cast<chip::Percent100ths>(presentCurrentPosition - stepValue), targetPosition);
  }

  currentState.SetNonNull().Set(
            MakeOptional(nextCurrentPosition),
            epState.currentState.Value().latch.HasValue() ? MakeOptional(epState.currentState.Value().latch.Value()) : NullOptional,
            epState.currentState.Value().speed.HasValue() ? MakeOptional(epState.currentState.Value().speed.Value()) : NullOptional
            );
  ep->GetLogic().SetCurrentState(currentState);
  isProgressPossible = (currentState.Value().position.Value() != epState.target.Value().position.Value());
  ChipLogError(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", ep->GetDelegate().GetEndpoint(), currentState.Value().position.Value(),
                                                                                          epState.target.Value().position.Value());
  // ChipLogError(AppServer, "Progress Possible: %s", isProgressPossible ? "true" : "false");

  if (isProgressPossible)
  {
    // If progress is possible, start the timer to continue the action
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleStepCommandTimer, reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));
  }
  else
  {
    instance.HandleClosureAction(ClosureManager::Action_t::STEP_ACTION);
  }

}

void ClosureManager::HandleStepCommandTimer(System::Layer * systemLayer, void * data)
{
  ClosureManager & manager = ClosureManager::GetInstance();
  // ChipLogError(AppServer, "############HandleStepCommandTimer###############");
  EndpointId endpointId = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(data));
  manager.HandleStepAction(endpointId);
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(
    const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
    const Optional<Globals::ThreeLevelAutoEnum> & speed, chip::EndpointId endpointId)
{
  // ChipLogError(AppServer, "########### OnStepCommand ###################");

  DataModel::Nullable<GenericOverallTarget> target;
  VerifyOrReturnValue(ep1.GetLogic().GetOverallTarget(target) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to get overall target for SetTarget command"));
  if (target.IsNull())
  {
    target.SetNonNull(GenericOverallTarget{});
  }

  target.Value().position = NullOptional; // Reset position to null

  if (speed.HasValue())
  {
    ChipLogError(AppServer, "Updating target speed for SetTarget command");
    target.Value().speed = MakeOptional(speed.Value());
  }

  // Set OverallTarget to null
  VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(target) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set overall target for Step command"));

  (void) DeviceLayer::SystemLayer().CancelTimer(HandleStepCommandTimer, reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));

  (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(2), HandleStepCommandTimer, reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));

  // Trigger Motion Action
  isStepActionInProgress = true;
  return Status::Success;
}