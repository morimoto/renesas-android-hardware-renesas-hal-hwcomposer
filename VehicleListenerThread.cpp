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

#include "VehicleListenerThread.h"
#include <cutils/log.h>
#include <sys/resource.h>

namespace android {

VehicleListenerThread::VehicleListenerThread(): mState(NOT_INITIALIZED), mRunning(false) {

}

VehicleListenerThread::~VehicleListenerThread() {

}

void VehicleListenerThread::run() {
    {
        std::unique_lock<std::mutex> lock(mLock);
        mRunning = true;
    }
    mThread = std::thread(threadLoop, this);
    pthread_setname_np(mThread.native_handle(), "VehicleListenerThread");
}

void VehicleListenerThread::stop() {
    {
        std::unique_lock<std::mutex> lock(mLock);
        mRunning = false;
    }
    mThread.join();
}

void VehicleListenerThread::threadLoop(void * data) {

    VehicleListenerThread* thread = reinterpret_cast<VehicleListenerThread*>(data);

    setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY);

    while(1) {

        {
            std::unique_lock<std::mutex> lock(thread->mLock);
            if (!thread->mRunning) {
                break;
            }

        }

        thread->InternalWork();
    }
}

void VehicleListenerThread::InternalWork() {

    std::unique_lock<std::mutex> lock(mLock);
    if ((mState == NOT_INITIALIZED) || (mState == WAITING_NEW_STATE))
        mTaskCondition.wait(lock);

    if (!mCameraStatus) {
        if (mHWCContext->drm_disp->enableDRMMaster()) {
            mState = WAITING_NEW_STATE;
        } else {
            //Wait some time and try again
            usleep(30000);
        }
    } else {
        mHWCContext->drm_disp->disableDRMMaster();
        mState = WAITING_NEW_STATE;
    }

    if (!mCameraStatus && mState == WAITING_NEW_STATE && mHWCContext) {
        mHWCContext->notice.invalidate();
    }

}

void VehicleListenerThread::setCameraStatus(bool status) {
    std::unique_lock<std::mutex> lock(mLock);

    if ((mState == NOT_INITIALIZED) || (mCameraStatus != status)) {
        mCameraStatus = status;
        mState = SETTING_STATE;
        mTaskCondition.notify_one();
    }

}

void VehicleListenerThread::setHWCContext(hwc_context * context) {
    mHWCContext = context;
}

}
