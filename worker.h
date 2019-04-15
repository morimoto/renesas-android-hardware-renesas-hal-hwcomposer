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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_WORKER_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_WORKER_H

#include <cstdint>
#include <cstdlib>
#include <string>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace android {

class Worker {
public:
    void lock() {
        mMutex.lock();
    }
    void unlock() {
        mMutex.unlock();
    }

    void signal() {
        mCond.notify_all();
    }
    void exitWorker();

    bool initialized() const {
        return mInitialized;
    }

protected:
    Worker(const char* name, int priority);
    virtual ~Worker();

    int initWorker();
    virtual void routine() = 0;

    /*
    * Must be called with the lock acquired. max_nanoseconds may be negative to
    * indicate infinite timeout, otherwise it indicates the maximum time span to
    * wait for a signal before returning.
    * Returns -EINTR if interrupted by exit request, or -ETIMEDOUT if timed out
    */
    int waitForSignalOrExitLocked(int64_t max_nanoseconds = -1);

    bool shouldExit() const {
        return mExit;
    }

    std::mutex mMutex;
    std::condition_variable mCond;

private:
    void internalRoutine();

    std::string mName;
    int mPriority;

    std::unique_ptr<std::thread> mThread;

    bool mExit;
    bool mInitialized;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_WORKER_H
