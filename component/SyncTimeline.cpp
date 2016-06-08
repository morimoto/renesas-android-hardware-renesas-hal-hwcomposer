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

#include "SyncTimeline.h"
#include <string>

SyncTimeline::SyncTimeline():
    m_fenceCount(0),
    m_incCount(0)
{
    int fd = sw_sync_timeline_create();
    if (fd == -1)
        return;
    m_fdInitialized = true;
    m_fd = fd;
}
void SyncTimeline::destroy()
{
    if (m_fdInitialized) {
        close(m_fd);
        m_fd = -1;
        m_fdInitialized = false;
    }
}
SyncTimeline::~SyncTimeline()
{
    destroy();
}
bool SyncTimeline::isValid() const
{
    if (!m_fdInitialized)
        return false;

    return (fcntl(m_fd, F_GETFD, 0) == 0);
}

int SyncTimeline::getFenceFd()
{
    Mutex::Autolock _l(m_mutex);

    std::string autoName = "hwcomposer_fence";
    m_fenceCount++;
    autoName += m_fenceCount;
    return sw_sync_fence_create(m_fd, autoName.c_str(), m_fenceCount);
}

int SyncTimeline::inc(int val)
{
    Mutex::Autolock _l(m_mutex);

    int result = sw_sync_timeline_inc(m_fd, val);
    if (result == 0)
        m_incCount++;
    return result;
}

