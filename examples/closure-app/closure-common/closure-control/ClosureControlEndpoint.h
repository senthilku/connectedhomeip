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
 #include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
 #include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
  #include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
 #include <app/clusters/closure-control-server/closure-control-cluster-matter-context.h>
 #include <app/clusters/closure-control-server/closure-control-server.h>
 
 #include <lib/core/CHIPError.h>
 #include <protocols/interaction_model/StatusCode.h>
 
 namespace chip {
 namespace app {
 namespace Clusters {
 namespace ClosureControl {
 
 using Protocols::InteractionModel::Status;
 
 // This is an application level delegate to handle Closure Control commands according to the specific business logic.
 class ClosureControlDelegate : public DelegateBase
 {
 public:
     enum Action_t
     {
         MOVE_ACTION = 0,
         MOVE_AND_LATCH_ACTION,
         STEP_ACTION,
         TARGET_CHANGE_ACTION,
 
         INVALID_ACTION
     } Action;
 
     ClosureControlDelegate(EndpointId endpoint) : mEndpoint(endpoint), mLogic(nullptr) {}
 
     Protocols::InteractionModel::Status HandleStopCommand() override;
     Protocols::InteractionModel::Status HandleMoveToCommand() override;
     Protocols::InteractionModel::Status HandleCalibrateCommand() override;
     
     
     // ClosureControlDelegate Specific Functions
 
     CHIP_ERROR Init();
     
     typedef void (*Callback_fn_initiated)(Action_t);
     typedef void (*Callback_fn_completed)(Action_t);
     void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);
 
     Callback_fn_initiated mActionInitiated_CB;
     Callback_fn_completed mActionCompleted_CB;
 
     void SetLogic(const ClusterLogic * logic);
 
     ClusterLogic * GetLogic() const;
 
     bool IsDeviceMoving() const;
 
     void SetDeviceMoving(const bool moving);
 
     Action_t GetAction();
 
     void SetAction(const Action_t action);
 
 private:
     bool isMoving         = false;
     bool isManualLatch    = false;
     Action_t mAction      = INVALID_ACTION;
     EndpointId mEndpoint  = kInvalidEndpointId;
     ClusterLogic * mLogic = nullptr;
 };
 
 class ClosureControlEndpoint
 {
 public:
     ClosureControlEndpoint(EndpointId endpoint) :
         mEndpoint(endpoint), mContext(mEndpoint), mDelegate(mEndpoint), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
     {
         mDelegate.SetLogic(&mLogic);
     }
 
     CHIP_ERROR Init()
     {
         ChipLogProgress(AppServer, "ClosureControlEndpoint::Init start");
         ReturnErrorOnFailure(mLogic.Init(kConformance));
         ReturnErrorOnFailure(mInterface.Init());
         ReturnErrorOnFailure(mDelegate.Init());
         ChipLogProgress(AppServer, "ClosureControlEndpoint::Init end");
         return CHIP_NO_ERROR;
     }
 
     ClosureControlDelegate & getDelegate() { return mDelegate; }
 
 private:
     const ClusterConformance kConformance = { .featureMap = 255, .supportsOverflow = true };
 
     EndpointId mEndpoint;
     MatterContext mContext;
     ClosureControlDelegate mDelegate;
     ClusterLogic mLogic;
     Interface mInterface;
 };
 } // namespace ClosureControl
 } // namespace Clusters
 } // namespace app
 } // namespace chip