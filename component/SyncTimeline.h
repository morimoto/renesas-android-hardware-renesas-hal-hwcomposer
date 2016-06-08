/*
 * Copyright 2012 Google, Inc
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

#ifndef SYNCTIMELINE_H_
#define SYNCTIMELINE_H_

//#include <sync/sync.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sw_sync.h>
#include <utils/Mutex.h>

using namespace android;

class SyncTimeline {
    int m_fd = -1;
    bool m_fdInitialized = false;
    unsigned int m_fenceCount;
    unsigned int m_incCount;
    Mutex m_mutex;

public:
    SyncTimeline(const SyncTimeline &) = delete;
    SyncTimeline& operator=(SyncTimeline&) = delete;
    SyncTimeline();
    virtual ~SyncTimeline();

    void destroy();
    bool isValid() const;
    int inc(int val = 1);

    int getFenceFd();
};

#endif /* SYNCTIMELINE_H_ */
