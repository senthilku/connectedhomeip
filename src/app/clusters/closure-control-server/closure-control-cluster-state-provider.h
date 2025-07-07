#pragma once

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

class ClosureControlClusterStateProvider
{
public:
    virtual ~ClosureControlClusterStateProvider() = default;
    virtual bool IsMainStateStopped() const = 0;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip