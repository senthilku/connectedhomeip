#include "OtaTlvEncryptionKey.h"
#include <lib/support/CodeUtils.h>
#include <platform/silabs/SilabsConfig.h>
#include "mbedtls/aes.h"
#include "mbedtls/cipher.h"
#include "mbedtls/platform.h"

#include <stdio.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace OtaTlvEncryptionKey {

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

CHIP_ERROR OtaTlvEncryptionKey::Import(const uint8_t * key, size_t key_len)
{
    if (key_len != 16) // Ensure the key length is 128 bits (16 bytes)
    {
        printf("Invalid key length: %zu\n", key_len);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store the key in a member variable for later use
    memcpy(mKey, key, key_len);
    mKeyLen = key_len;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaTlvEncryptionKey::Decrypt(MutableByteSpan & block, uint32_t & mIVOffset)
{
    constexpr uint8_t au8Iv[] = { 0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x00, 0x00, 0x00, 0x00 };
    uint8_t iv[16];
    mbedtls_aes_context aes_ctx;
    uint32_t u32IVCount;
    uint32_t Offset = 0;

    memcpy(iv, au8Iv, sizeof(au8Iv));

    u32IVCount = (((uint32_t) iv[12]) << 24) | (((uint32_t) iv[13]) << 16) | (((uint32_t) iv[14]) << 8) | (iv[15]);
    u32IVCount += (mIVOffset >> 4);

    iv[12] = (uint8_t) ((u32IVCount >> 24) & 0xff);
    iv[13] = (uint8_t) ((u32IVCount >> 16) & 0xff);
    iv[14] = (uint8_t) ((u32IVCount >> 8) & 0xff);
    iv[15] = (uint8_t) (u32IVCount & 0xff);

    mbedtls_aes_init(&aes_ctx);

    // Set the AES decryption key
    if (mbedtls_aes_setkey_dec(&aes_ctx, mKey, mKeyLen * 8) != 0)
    {
        printf("Failed to set AES decryption key\n");
        mbedtls_aes_free(&aes_ctx);
        return CHIP_ERROR_INTERNAL;
    }

    while (Offset + 16 <= block.size())
    {
        // Decrypt the block
        if (mbedtls_aes_crypt_ctr(&aes_ctx, 16, &u32IVCount, iv, iv, &block[Offset], &block[Offset]) != 0)
        {
            printf("Failed to decrypt block\n");
            mbedtls_aes_free(&aes_ctx);
            return CHIP_ERROR_INTERNAL;
        }

        /* Increment the IV for the next block */
        u32IVCount++;

        iv[12] = (uint8_t) ((u32IVCount >> 24) & 0xff);
        iv[13] = (uint8_t) ((u32IVCount >> 16) & 0xff);
        iv[14] = (uint8_t) ((u32IVCount >> 8) & 0xff);
        iv[15] = (uint8_t) (u32IVCount & 0xff);

        Offset += 16; /* Increment the buffer offset */
        mIVOffset += 16;
    }

    printf("Decrypted ciphertext\n");

    mbedtls_aes_free(&aes_ctx);

    return CHIP_NO_ERROR;
}

} // namespace OtaTlvEncryptionKey
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
