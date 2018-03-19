/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2017 GlobalLogic
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

#ifndef VEHICLE_CALLBACKLISTENER_H
#define VEHICLE_CALLBACKLISTENER_H

#include <android/hardware/automotive/vehicle/2.0/IVehicle.h>
#include "hwcomposer.h"
#include "VehicleListenerThread.h"

using namespace ::android::hardware::automotive::vehicle::V2_0;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::sp;

template<typename ENUM>
inline constexpr typename std::underlying_type<ENUM>::type toInt(
        ENUM const value) {
    return static_cast<typename std::underlying_type<ENUM>::type>(value);
}

inline constexpr VehiclePropertyType getPropType(int32_t prop) {
    return static_cast<VehiclePropertyType>(
            prop & toInt(VehiclePropertyType::MASK));
}

class VehicleCallbackListener : public IVehicleCallback {
public:
    // Methods from ::android::hardware::automotive::vehicle::V2_0::IVehicleCallback follow.
    Return<void> onPropertyEvent(const hidl_vec <VehiclePropValue> & values) override {
        if (getPropType(values[0].prop) == VehiclePropertyType::INT32) {
            if (values[0].value.int32Values[0] == static_cast<int32_t>(VehicleGear::GEAR_REVERSE)) {
                mThread->setCameraStatus(true);
            } else {
                mThread->setCameraStatus(false);
            }
        }
        return Return<void>();
    }

    Return<void> onPropertySet(const VehiclePropValue & /*value*/) override {
        return Return<void>();
    }

    Return<void> onPropertySetError(StatusCode      /* errorCode */,
                                    int32_t         /* propId */,
                                    int32_t         /* areaId */) override {
        // We don't set values, so we don't listen for set errors
        return Return<void>();
    }
    VehicleCallbackListener() {
    mThread = std::make_shared<VehicleListenerThread>();
    mThread->run();
    }

    void setHWCContext(hwc_context * context){
        mHWCContext = context;
        mThread->setHWCContext(mHWCContext);
    }

    void setCameraStatus(bool status) {
        mThread->setCameraStatus(status);
    }
private:
    hwc_context * mHWCContext = nullptr;
    std::shared_ptr<VehicleListenerThread> mThread;

};

#endif //VEHICLE_CALLBACKLISTENER_H
