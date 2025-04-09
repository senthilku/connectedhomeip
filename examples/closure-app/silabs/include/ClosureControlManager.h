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
#include <app/clusters/closure-control-server/closure-control-server.h>

#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

// This is an application level delegate to handle Closure Control commands according to the specific business logic.
class ClosureControlManager : public ClosureControl::Delegate
{
public:
    enum Action_t : uint8_t
    {
        MOVE_ACTION = 0,
        MOVE_AND_LATCH_ACTION,
        STOP_ACTION,
        CALIBRATE_ACTION,
        TARGET_CHANGE_ACTION,

        INVALID_ACTION
    };

    uint32_t mMovingTime                          = 0;
    uint32_t mCalibratingTime                     = 0;
    const uint32_t kExampleCalibrateCountDown     = 10;
    const uint32_t kExampleMotionCountDown        = 15;
    const uint32_t kExampleWaitforMotionCountDown = 15;

    app::DataModel::Nullable<uint32_t> mCountDownTime;

    typedef void (*Callback_fn_initiated)(Action_t action);
    typedef void (*Callback_fn_completed)(Action_t action);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    void SetClosureControlInstance(ClosureControl::Instance & instance);
    Instance * GetClosureControlInstance();

    /*********************************************************************************
     *
     * Methods implementing the ClosureControl::Delegate interface
     *
     *********************************************************************************/
    Protocols::InteractionModel::Status Stop() override;
    Protocols::InteractionModel::Status MoveTo(const Optional<TargetPositionEnum> & tag, const Optional<bool> & latch,
                                               const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    Protocols::InteractionModel::Status Calibrate() override;

    // ------------------------------------------------------------------
    // Get attribute methods

    DataModel::Nullable<uint32_t> GetCountdownTime() override;

    /***************************************************************************
     *
     * ClosureControlDelegate specific methods
     *
     ***************************************************************************/
    CHIP_ERROR StartCurrentErrorListRead() override;
    CHIP_ERROR GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError) override;
    CHIP_ERROR EndCurrentErrorListRead() override;
    
    void ClosureControlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId);

    void ClosureControlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId);
    /**
     * @brief Handles the countdown timer expiration event
     */
    void HandleCountdownTimeExpired();
    /**
     * @brief Checks if the device can move or need pre-motion stages to complete
     * @return true if device is ready to move
     *         false if device is not ready to move
     */
    bool IsDeviceReadytoMove();
    /**
     * @brief Handles the motion request of Closure
     * @param [in] latchNeeded - true if latch is needed
     * @param [in] NewTarget - true if target is changed
     * @return Protocols::InteractionModel::Status - success or failure
     */
    Protocols::InteractionModel::Status HandleMotion(bool latchNeeded, bool NewTarget);

private:
    /***************************************************************************
     *
     * ClosureControlManager specific variables
     *
     ***************************************************************************/

    // Need the following so can determine which features are supported
    ClosureControl::Instance * mpClosureControlInstance = nullptr;

    bool isManualLatch = false;
    /**
     * @brief Checks if device is error state or not and sets mainstate to error.
     * @return true if device is error state
     *         false if device is not in error state
     */
    bool CheckErrorondevice();

    static ClosureControlManager sClosureCtrlMgr;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;
};

ClosureControlManager * GetClosureControlManager();

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
