#include "OtaTlvEncryptionKey.h"
#include <lib/support/CodeUtils.h>
#include <platform/silabs/SilabsConfig.h>

#include <stdio.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace OtaTlvEncryptionKey {

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

CHIP_ERROR OtaTlvEncryptionKey::Import(const uint8_t * key, size_t key_len)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR OtaTlvEncryptionKey::Decrypt(MutableByteSpan & block, uint32_t & mIVOffset)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace OtaTlvEncryptionKey
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
