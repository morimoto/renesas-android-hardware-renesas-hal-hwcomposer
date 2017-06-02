/*
 * Copyright (C) 2016 GlobalLogic
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

#ifndef VEHICLELISTENERTHREAD_H
#define VEHICLELISTENERTHREAD_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include "hwcomposer.h"

namespace android {

class VehicleListenerThread {

    enum state {
        NOT_INITIALIZED = -1,
        SETTING_STATE = 0,
        WAITING_NEW_STATE,
    };

    state mState;
    bool mRunning;
    std::mutex mLock;
    std::condition_variable mTaskCondition;
    std::thread mThread;
    static void threadLoop(void * data);

    hwc_context * mHWCContext = nullptr;
    bool mCameraStatus;
    void InternalWork();
public:
    VehicleListenerThread();
    virtual ~VehicleListenerThread();

    void run();
    void stop();
    void setHWCContext(hwc_context * context);
    void setCameraStatus(bool status);
};

}

#endif //VEHICLELISTENERTHREAD_H
