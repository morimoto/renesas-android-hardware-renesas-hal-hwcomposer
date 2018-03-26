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

#include <cutils/log.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

namespace android {

static const int64_t kBillion = 1000000000LL;

Worker::Worker(const char* name, int priority)
    : mName(name), mPriority(priority), mExit(false), mInitialized(false) {
}

Worker::~Worker() {
    if (!mInitialized)
        return;

    pthread_kill(mThread, SIGTERM);
    pthread_cond_destroy(&mCond);
    pthread_mutex_destroy(&mLock);
}

int Worker::initWorker() {
    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
    int ret = pthread_cond_init(&mCond, &cond_attr);

    if (ret) {
        ALOGE("Failed to int thread %s condition %d", mName.c_str(), ret);
        return ret;
    }

    ret = pthread_mutex_init(&mLock, NULL);

    if (ret) {
        ALOGE("Failed to init thread %s lock %d", mName.c_str(), ret);
        pthread_cond_destroy(&mCond);
        return ret;
    }

    ret = pthread_create(&mThread, NULL, internalRoutine, this);

    if (ret) {
        ALOGE("Could not create thread %s %d", mName.c_str(), ret);
        pthread_mutex_destroy(&mLock);
        pthread_cond_destroy(&mCond);
        return ret;
    }

    mInitialized = true;
    return 0;
}

bool Worker::initialized() const {
    return mInitialized;
}

int Worker::lock() {
    return pthread_mutex_lock(&mLock);
}

int Worker::unlock() {
    return pthread_mutex_unlock(&mLock);
}

int Worker::signalLocked() {
    return signalThreadLocked(false);
}

int Worker::exitLocked() {
    int signal_ret = signalThreadLocked(true);

    if (signal_ret)
        ALOGE("Failed to signal thread %s with exit %d", mName.c_str(), signal_ret);

    int join_ret = pthread_join(mThread, NULL);

    if (join_ret && join_ret != ESRCH)
        ALOGE("Failed to join thread %s in exit %d", mName.c_str(), join_ret);

    return signal_ret | join_ret;
}

int Worker::signal() {
    int ret = lock();

    if (ret) {
        ALOGE("Failed to acquire lock in Signal() %d\n", ret);
        return ret;
    }

    int signal_ret = signalLocked();
    ret = unlock();

    if (ret) {
        ALOGE("Failed to release lock in Signal() %d\n", ret);
        return ret;
    }

    return signal_ret;
}

int Worker::exit() {
    int ret = lock();

    if (ret) {
        ALOGE("Failed to acquire lock in Exit() %d\n", ret);
        return ret;
    }

    int exit_ret = exitLocked();
    ret = unlock();

    if (ret) {
        ALOGE("Failed to release lock in Exit() %d\n", ret);
        return ret;
    }

    return exit_ret;
}

int Worker::waitForSignalOrExitLocked(int64_t max_nanoseconds) {
    if (mExit)
        return -EINTR;

    int ret = 0;

    if (max_nanoseconds < 0) {
        ret = pthread_cond_wait(&mCond, &mLock);
    } else {
        struct timespec abs_deadline;
        ret = clock_gettime(CLOCK_MONOTONIC, &abs_deadline);

        if (ret)
            return ret;

        int64_t nanos = (int64_t)abs_deadline.tv_nsec + max_nanoseconds;
        abs_deadline.tv_sec += nanos / kBillion;
        abs_deadline.tv_nsec = nanos % kBillion;
        ret = pthread_cond_timedwait(&mCond, &mLock, &abs_deadline);

        if (ret == ETIMEDOUT)
            ret = -ETIMEDOUT;
    }

    if (mExit)
        return -EINTR;

    return ret;
}

// static
void* Worker::internalRoutine(void* arg) {
    Worker* worker = (Worker*)arg;
    setpriority(PRIO_PROCESS, 0, worker->mPriority);

    while (true) {
        int ret = worker->lock();

        if (ret) {
            ALOGE("Failed to lock %s thread %d", worker->mName.c_str(), ret);
            continue;
        }

        bool exit = worker->mExit;
        ret = worker->unlock();

        if (ret) {
            ALOGE("Failed to unlock %s thread %d", worker->mName.c_str(), ret);
            break;
        }

        if (exit)
            break;

        worker->routine();
    }

    return NULL;
}

int Worker::signalThreadLocked(bool exit) {
    if (exit)
        mExit = exit;

    int ret = pthread_cond_signal(&mCond);

    if (ret) {
        ALOGE("Failed to signal condition on %s thread %d", mName.c_str(), ret);
        return ret;
    }

    return 0;
}

} // namespace android
