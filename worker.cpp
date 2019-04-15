/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "worker.h"

#include <sys/resource.h>

namespace android {

Worker::Worker(const char* name, int priority)
    : mName(name), mPriority(priority), mExit(false), mInitialized(false) {
}

Worker::~Worker() {
    exitWorker();
}

int Worker::initWorker() {
    std::lock_guard<std::mutex> lk(mMutex);
    if (initialized()) {
        return -EALREADY;
    }

    mThread = std::unique_ptr<std::thread>(
                new std::thread(&Worker::internalRoutine, this));
    mInitialized = true;
    mExit = false;

    return 0;
}

void Worker::exitWorker() {
    std::unique_lock<std::mutex> lk(mMutex);
    mExit = true;
    if (initialized()) {
        lk.unlock();
        mCond.notify_all();
        mThread->join();
        mInitialized = false;
    }
}

int Worker::waitForSignalOrExitLocked(int64_t max_nanoseconds) {
    int ret = 0;
    if (shouldExit()) {
        return -EINTR;
    }

    std::unique_lock<std::mutex> lk(mMutex, std::adopt_lock);
    if (max_nanoseconds < 0) {
        mCond.wait(lk);
    } else if (std::cv_status::timeout ==
               mCond.wait_for(lk, std::chrono::nanoseconds(max_nanoseconds))) {
        ret = -ETIMEDOUT;
    }

    // exit takes precedence on timeout
    if (shouldExit()) {
        ret = -EINTR;
    }

    // release leaves mutex locked when going out of scope
    lk.release();

    return ret;
}

void Worker::internalRoutine() {
    setpriority(PRIO_PROCESS, 0, mPriority);

    std::unique_lock<std::mutex> lk(mMutex, std::defer_lock);

    while (true) {
        lk.lock();
        if (shouldExit()) {
            return;
        }
        lk.unlock();

        routine();
    }
}

} // namespace android
