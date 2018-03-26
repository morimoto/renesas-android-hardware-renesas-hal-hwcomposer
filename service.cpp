/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sched.h>

#include <vendor/renesas/graphics/composer/1.0/IComposer.h>

#include <binder/ProcessState.h>
#include <hidl/LegacySupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

#include "Hwc.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

using vendor::renesas::graphics::composer::V1_0::IComposer;
using android::hardware::graphics::composer::V2_1::implementation::HwcHal;

int main() {
    android::sp<IComposer> vendor_service = new HwcHal();
    configureRpcThreadpool(4, true /* callerWillJoin */);
    android::status_t status = vendor_service->registerAsService();
    android::sp<android::hardware::graphics::composer::V2_1::IComposer> service =
        vendor_service;
    status = service->registerAsService();
    LOG_ALWAYS_FATAL_IF(status != android::OK,
                        "Error while registering hwcomposer: %d", status);
    // same as SF main thread
    struct sched_param param = {0};
    param.sched_priority = 2;

    if (sched_setscheduler(0, SCHED_FIFO | SCHED_RESET_ON_FORK,
                           &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO: %d", errno);
    }

    joinRpcThreadpool();
}
