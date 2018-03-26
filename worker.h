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

#include <string>
#include <pthread.h>
#include <stdint.h>

namespace android {

class Worker {
public:
    int lock();
    int unlock();

    // Must be called with the lock acquired
    int signalLocked();
    int exitLocked();

    // Convenience versions of above, acquires the lock
    int signal();
    int exit();

protected:
    Worker(const char* name, int priority);
    virtual ~Worker();

    int initWorker();
    bool initialized() const;
    virtual void routine() = 0;

    /*
    * Must be called with the lock acquired. max_nanoseconds may be negative to
    * indicate infinite timeout, otherwise it indicates the maximum time span to
    * wait for a signal before returning.
    * Returns -EINTR if interrupted by exit request, or -ETIMEDOUT if timed out
    */
    int waitForSignalOrExitLocked(int64_t max_nanoseconds = -1);

private:
    static void* internalRoutine(void* worker);

    // Must be called with the lock acquired
    int signalThreadLocked(bool exit);

    std::string mName;
    int mPriority;

    pthread_t mThread;
    pthread_mutex_t mLock;
    pthread_cond_t mCond;

    bool mExit;
    bool mInitialized;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_WORKER_H
