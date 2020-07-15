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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_AUTO_FD_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_AUTO_FD_H

#include <unistd.h>

namespace android {

class UniqueFd {
public:
    UniqueFd() = default;
    UniqueFd(int fd) : mFd(fd) {
    }
    UniqueFd(UniqueFd&& rhs) {
        mFd = rhs.mFd;
        rhs.mFd = -1;
    }

    UniqueFd& operator=(UniqueFd&& rhs) {
        set(rhs.release());
        return *this;
    }

    ~UniqueFd() {
        if (mFd >= 0)
            close(mFd);
    }

    int release() {
        int old_fd = mFd;
        mFd = -1;
        return old_fd;
    }

    int set(int fd) {
        if (mFd >= 0)
            close(mFd);

        mFd = fd;
        return mFd;
    }

    int get() const {
        return mFd;
    }

private:
    int mFd = -1;
};

struct OutputFd {
    OutputFd() = default;
    OutputFd(int* fd) : mFd(fd) {
    }
    OutputFd(OutputFd&& rhs) {
        mFd = rhs.mFd;
        rhs.mFd = NULL;
    }

    OutputFd& operator=(OutputFd&& rhs) {
        mFd = rhs.mFd;
        rhs.mFd = NULL;
        return *this;
    }

    int set(int fd) {
        if (*mFd >= 0)
            close(*mFd);

        *mFd = fd;
        return fd;
    }

    int get() {
        return *mFd;
    }

    operator bool() const {
        return mFd != NULL;
    }

private:
    int* mFd = NULL;
};

} // namespace android

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_AUTO_FD_H
