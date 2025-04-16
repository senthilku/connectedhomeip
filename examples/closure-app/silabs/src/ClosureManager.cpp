#include "ClosureManager.h"
#include "ClosureControlEndpoint.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/util/attribute-storage.h>
#include <app-common/zap-generated/cluster-objects.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;

namespace {
    
    // Define the endpoint ID for the Closure Control cluster
    constexpr chip::EndpointId kClosureEndpoint = 1;
    
    // Define the endpoint for the Closure Control cluster
    ClosureControlEndpoint ep1(1);
    
    // Define the Namespace and Tag for the endpoint
    // Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure.adoc
    constexpr const uint8_t kNamespaceClosure     = 0x44;
    constexpr const uint8_t kTagClosureCovering   = 0x00;
    // Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure-Covering.adoc
    constexpr const uint8_t kNamespaceCovering    = 0x46;
    constexpr const uint8_t kTagCoveringVenetian  = 0x03;
    

    // Define the list of semantic tags for the endpoint
    const Clusters::Descriptor::Structs::SemanticTagStruct::Type gEp1TagList[] = {
        { 
            .namespaceID = kNamespaceClosure,
            .tag         = kTagClosureCovering,
            .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) 
        },
        {
            .namespaceID = kNamespaceCovering,
            .tag         = kTagCoveringVenetian,
            .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) 
        },
    };
    
} // namespace

ClosureManager ClosureManager::sClosureMgr;

void ClosureManager::Init()
{
    ChipLogProgress(AppServer,"Closure-app ClosureManager Init Start.");
    
    DeviceLayer::PlatformMgr().LockChipStack();
    // Closure Endpoint Initilization
    ep1.Init(); 
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(gEp1TagList));
    DeviceLayer::PlatformMgr().UnlockChipStack();
    
    ChipLogProgress(AppServer,"Closure-app ClosureManager Init Done.");
}
