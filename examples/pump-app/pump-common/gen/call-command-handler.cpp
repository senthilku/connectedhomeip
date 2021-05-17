/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

// THIS FILE IS GENERATED BY ZAP

#include <stdint.h>

#include "af-structs.h"
#include "app/util/util.h"
#include "call-command-handler.h"
#include "callback.h"
#include "cluster-id.h"
#include "command-id.h"

using namespace chip;

EmberAfStatus emberAfBasicClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGeneralCommissioningClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGeneralDiagnosticsClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfNetworkCommissioningClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOperationalCredentialsClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfPumpConfigurationAndControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfThreadNetworkDiagnosticsClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfTrustedRootCertificatesClusterServerCommandParse(EmberAfClusterCommand * cmd);

static EmberAfStatus status(bool wasHandled, bool clusterExists, bool mfgSpecific)
{
    if (wasHandled)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else if (mfgSpecific)
    {
        return EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND;
    }
    else if (clusterExists)
    {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    }
    else
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    }
}

// Main command parsing controller.
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand * cmd)
{
    EmberAfStatus result = status(false, false, cmd->mfgSpecific);
    if (cmd->direction == (uint8_t) ZCL_DIRECTION_SERVER_TO_CLIENT &&
        emberAfContainsClientWithMfgCode(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, cmd->mfgCode))
    {
        switch (cmd->apsFrame->clusterId)
        {
        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    else if (cmd->direction == (uint8_t) ZCL_DIRECTION_CLIENT_TO_SERVER &&
             emberAfContainsServerWithMfgCode(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, cmd->mfgCode))
    {
        switch (cmd->apsFrame->clusterId)
        {
        case ZCL_BASIC_CLUSTER_ID:
            // No commands are enabled for cluster Basic
            result = status(false, true, cmd->mfgSpecific);
            break;
        case ZCL_GENERAL_COMMISSIONING_CLUSTER_ID:
            result = emberAfGeneralCommissioningClusterServerCommandParse(cmd);
            break;
        case ZCL_GENERAL_DIAGNOSTICS_CLUSTER_ID:
            // No commands are enabled for cluster General Diagnostics
            result = status(false, true, cmd->mfgSpecific);
            break;
        case ZCL_NETWORK_COMMISSIONING_CLUSTER_ID:
            result = emberAfNetworkCommissioningClusterServerCommandParse(cmd);
            break;
        case ZCL_ON_OFF_CLUSTER_ID:
            result = emberAfOnOffClusterServerCommandParse(cmd);
            break;
        case ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID:
            result = emberAfOperationalCredentialsClusterServerCommandParse(cmd);
            break;
        case ZCL_PUMP_CONFIG_CONTROL_CLUSTER_ID:
            // No commands are enabled for cluster Pump Configuration and Control
            result = status(false, true, cmd->mfgSpecific);
            break;
        case ZCL_THREAD_NETWORK_DIAGNOSTICS_CLUSTER_ID:
            // No commands are enabled for cluster Thread Network Diagnostics
            result = status(false, true, cmd->mfgSpecific);
            break;
        case ZCL_TRUSTED_ROOT_CERTIFICATES_CLUSTER_ID:
            result = emberAfTrustedRootCertificatesClusterServerCommandParse(cmd);
            break;
        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    return result;
}

// Cluster specific command parsing

EmberAfStatus emberAfGeneralCommissioningClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ARM_FAIL_SAFE_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            uint16_t expiryLengthSeconds;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 2)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            expiryLengthSeconds = emberAfGetInt16u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset       = static_cast<uint16_t>(payloadOffset + 2);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfGeneralCommissioningClusterArmFailSafeCallback(nullptr, expiryLengthSeconds, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_COMMISSIONING_COMPLETE_COMMAND_ID: {
            wasHandled = emberAfGeneralCommissioningClusterCommissioningCompleteCallback(nullptr);
            break;
        }
        case ZCL_SET_REGULATORY_CONFIG_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            uint8_t location;
            uint8_t * countryCode;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            location      = emberAfGetInt8u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 1);
            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            countryCode   = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + emberAfStringLength(countryCode) + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfGeneralCommissioningClusterSetRegulatoryConfigCallback(nullptr, location, countryCode, breadcrumb,
                                                                                       timeoutMs);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfNetworkCommissioningClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_THREAD_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan operationalDataset;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData  = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                operationalDataset = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + operationalDataset.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled =
                emberAfNetworkCommissioningClusterAddThreadNetworkCallback(nullptr, operationalDataset, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_ADD_WI_FI_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan ssid;
            chip::ByteSpan credentials;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                ssid              = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + ssid.size() + 1u);
            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                credentials       = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + credentials.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled =
                emberAfNetworkCommissioningClusterAddWiFiNetworkCallback(nullptr, ssid, credentials, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_DISABLE_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan networkID;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                networkID         = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + networkID.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfNetworkCommissioningClusterDisableNetworkCallback(nullptr, networkID, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_ENABLE_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan networkID;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                networkID         = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + networkID.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfNetworkCommissioningClusterEnableNetworkCallback(nullptr, networkID, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_GET_LAST_NETWORK_COMMISSIONING_RESULT_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfNetworkCommissioningClusterGetLastNetworkCommissioningResultCallback(nullptr, timeoutMs);
            break;
        }
        case ZCL_REMOVE_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan NetworkID;
            uint64_t Breadcrumb;
            uint32_t TimeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                NetworkID         = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + NetworkID.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            Breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            TimeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfNetworkCommissioningClusterRemoveNetworkCallback(nullptr, NetworkID, Breadcrumb, TimeoutMs);
            break;
        }
        case ZCL_SCAN_NETWORKS_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan ssid;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                ssid              = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + ssid.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfNetworkCommissioningClusterScanNetworksCallback(nullptr, ssid, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_UPDATE_THREAD_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan operationalDataset;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData  = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                operationalDataset = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + operationalDataset.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled =
                emberAfNetworkCommissioningClusterUpdateThreadNetworkCallback(nullptr, operationalDataset, breadcrumb, timeoutMs);
            break;
        }
        case ZCL_UPDATE_WI_FI_NETWORK_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan ssid;
            chip::ByteSpan credentials;
            uint64_t breadcrumb;
            uint32_t timeoutMs;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                ssid              = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + ssid.size() + 1u);
            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                credentials       = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + credentials.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            breadcrumb    = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 4)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            timeoutMs = emberAfGetInt32u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled =
                emberAfNetworkCommissioningClusterUpdateWiFiNetworkCallback(nullptr, ssid, credentials, breadcrumb, timeoutMs);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_OFF_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOffCallback(nullptr);
            break;
        }
        case ZCL_ON_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOnCallback(nullptr);
            break;
        }
        case ZCL_TOGGLE_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterToggleCallback(nullptr);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOperationalCredentialsClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_OP_CERT_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan NOC;
            chip::ByteSpan ICACertificate;
            chip::ByteSpan IPKValue;
            chip::NodeId CaseAdminNode;
            uint16_t AdminVendorId;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                NOC               = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + NOC.size() + 1u);
            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                ICACertificate    = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + ICACertificate.size() + 1u);
            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                IPKValue          = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }
            payloadOffset = static_cast<uint16_t>(payloadOffset + IPKValue.size() + 1u);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            CaseAdminNode = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 2)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            AdminVendorId = emberAfGetInt16u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfOperationalCredentialsClusterAddOpCertCallback(nullptr, NOC, ICACertificate, IPKValue,
                                                                               CaseAdminNode, AdminVendorId);
            break;
        }
        case ZCL_OP_CSR_REQUEST_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan CSRNonce;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                CSRNonce          = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }

            wasHandled = emberAfOperationalCredentialsClusterOpCSRRequestCallback(nullptr, CSRNonce);
            break;
        }
        case ZCL_REMOVE_ALL_FABRICS_COMMAND_ID: {
            wasHandled = emberAfOperationalCredentialsClusterRemoveAllFabricsCallback(nullptr);
            break;
        }
        case ZCL_REMOVE_FABRIC_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::FabricId FabricId;
            chip::NodeId NodeId;
            uint16_t VendorId;

            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            FabricId      = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 8)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            NodeId        = emberAfGetInt64u(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset = static_cast<uint16_t>(payloadOffset + 8);
            if (cmd->bufLen < payloadOffset + 2)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            VendorId = emberAfGetInt16u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfOperationalCredentialsClusterRemoveFabricCallback(nullptr, FabricId, NodeId, VendorId);
            break;
        }
        case ZCL_SET_FABRIC_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            uint16_t VendorId;

            if (cmd->bufLen < payloadOffset + 2)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            VendorId = emberAfGetInt16u(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfOperationalCredentialsClusterSetFabricCallback(nullptr, VendorId);
            break;
        }
        case ZCL_UPDATE_FABRIC_LABEL_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            uint8_t * Label;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            Label = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(nullptr, Label);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfTrustedRootCertificatesClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_TRUSTED_ROOT_CERTIFICATE_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan RootCertificate;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                RootCertificate   = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }

            wasHandled = emberAfTrustedRootCertificatesClusterAddTrustedRootCertificateCallback(nullptr, RootCertificate);
            break;
        }
        case ZCL_REMOVE_TRUSTED_ROOT_CERTIFICATE_COMMAND_ID: {
            uint16_t payloadOffset = cmd->payloadStartIndex;
            chip::ByteSpan TrustedRootIdentifier;

            if (cmd->bufLen < payloadOffset + 1u)
            {
                return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            {
                uint8_t * rawData     = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
                TrustedRootIdentifier = chip::ByteSpan(rawData + 1u, emberAfStringLength(rawData));
            }

            wasHandled = emberAfTrustedRootCertificatesClusterRemoveTrustedRootCertificateCallback(nullptr, TrustedRootIdentifier);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
