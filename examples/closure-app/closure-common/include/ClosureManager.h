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

/*
 * @class ClosureManager
 * @brief Manages the initialization and operations related to closure and
 *        closure panel endpoints in the application.
 *
 * @note This class is part of the closure application example
 */

#pragma once

#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"
#include <lib/core/DataModelTypes.h>

class ClosureManager
{
public:
    enum Action_t
    {
        CALIBRATE_ACTION   = 0,
        MOVE_TO_ACTION     = 1,
        STOP_MOTION_ACTION = 2,
        STOP_CALIBRATE_ACTION = 3,
        SET_TARGET_ACTION  = 4,
        STEP_ACTION = 5,
        STOP_SET_TARGET_ACTION = 6,
        STOP_STEP_ACTION = 7,

        INVALID_ACTION
    };
    /**
     * @brief Initializes the ClosureManager.
     *
     * This method sets up the necessary resources and configurations required
     * for the Closure Application to function properly.
     */
    void Init();

    static ClosureManager & GetInstance() { return sClosureMgr; }

    /**
     * Handles the "Calibrate" command for the closure manager.
     *
     * This method initiates the calibration process for the closure system. It resets the states
     * and targets of all endpoints to null, sets a countdown time for the calibration process,
     * and starts a timer to handle the calibration action after a specified duration.
     *
     * @param countdownTime A nullable parameter representing the countdown time for calibration.
     *                      This value is currently unused in the implementation.
     * @return Status::Success if the calibration process is successfully initiated.
     */
    chip::Protocols::InteractionModel::Status OnCalibrateCommand(chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime);

    chip::Protocols::InteractionModel::Status OnMoveToCommand(
        const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> & position,
        const chip::Optional<bool> & latch,
        const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
        chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime);

    chip::Protocols::InteractionModel::Status OnStopCommand();

    chip::Protocols::InteractionModel::Status OnSetTargetCommand(
        const chip::Optional<chip::Percent100ths> & pos, 
        const chip::Optional<bool> & latch, 
        const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
        chip::EndpointId endpointId);

    chip::Protocols::InteractionModel::Status OnStepCommand(
        const chip::app::Clusters::ClosureDimension::StepDirectionEnum & direction, 
        const uint16_t & numberOfSteps, 
        const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
        chip::EndpointId endpointId);
    
    chip::app::Clusters::ClosureControl::ClosureControlEndpoint ep1{ kClosureEndpoint };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint ep2{ kClosurePanel1Endpoint };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint ep3{ kClosurePanel2Endpoint };

private:
    static ClosureManager sClosureMgr;
    // Define the endpoint ID for the Closure
    static constexpr chip::EndpointId kClosureEndpoint       = 1;
    static constexpr chip::EndpointId kClosurePanel1Endpoint = 2;
    static constexpr chip::EndpointId kClosurePanel2Endpoint = 3;
    
    /**
     * @brief
     *  The callback function to be called when "calibration timer" expires.
     */
    static void HandleCalibrateActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleStopActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleMoveToActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleLatchActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleSetTargetActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleStepCommandTimer(chip::System::Layer * systemLayer, void * aAppState);

    void HandleClosureAction(ClosureManager::Action_t action);
    void HandleMotionAction();
    void HandleSetTargetAction(chip::EndpointId endpointId);
    void HandleStepAction(chip::EndpointId endpointId);
    bool isCalibrationInProgress = false;
    bool isMoveToInProgress = false;
    bool isSetTargetInProgress = false;
    bool isStepActionInProgress = false;
    chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
};
