/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppConfig.h"
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>
#include <string.h>

#include <mbedtls/platform.h>

#if CHIP_ENABLE_OPENTHREAD
#include <openthread-core-config.h>
#include <openthread/cli.h>
#include <openthread/config.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/platform/openthread-system.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <utils/uart.h>

#include "platform-efr32.h"

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#include "openthread/heap.h"
#endif // OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#endif // CHIP_ENABLE_OPENTHREAD

#include "init_efrPlatform.h"
#include "sl_component_catalog.h"

#ifndef CCP_SI917_BRINGUP
#include "sl_mbedtls.h"
#endif

//#ifndef CCP_SI917_BRINGUP
#include "sl_system_init.h"
#include "efr32_utils.h"
//#endif

void initAntenna(void);

void init_efrPlatform(void)
{

    
    sl_system_init();
    EFR32_LOG("CCP, init_efrPlatform");


    sl_LED_Toggle();
    sl_LED_Toggle();
    sl_LED_Toggle();	
#ifndef CCP_SI917_BRINGUP
    sl_system_init();
    sl_mbedtls_init();
#endif /* CCP_SI917_BRINGUP */

#if CHIP_ENABLE_OPENTHREAD
    efr32RadioInit();
    efr32AlarmInit();
#endif // CHIP_ENABLE_OPENTHREAD

   EFR32_LOG(" END.....init_efrPlatform");
}

#ifdef __cplusplus
}
#endif
