/*
 * Copyright (C) 2015 The Android Open Source Project
 * Copyright (C) 2018 GlobalLogic
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

#include "DRMMode.h"

namespace android {

DRMMode::DRMMode(drmModeModeInfoPtr m)
    : mId(0),
      mClock(m->clock),
      mHDisplay(m->hdisplay),
      mHSyncStart(m->hsync_start),
      mHSyncEnd(m->hsync_end),
      mHTotal(m->htotal),
      mHSkew(m->hskew),
      mVDisplay(m->vdisplay),
      mVSyncStart(m->vsync_start),
      mVSyncEnd(m->vsync_end),
      mVTotal(m->vtotal),
      mVScan(m->vscan),
      mVRefresh(m->vrefresh),
      mFlags(m->flags),
      mType(m->type),
      mName(m->name) {
}

bool DRMMode::operator==(const drmModeModeInfo& m) const {
    return mClock == m.clock && mHDisplay == m.hdisplay &&
           mHSyncStart == m.hsync_start && mHSyncEnd == m.hsync_end &&
           mHTotal == m.htotal && mHSkew == m.hskew &&
           mVDisplay == m.vdisplay && mVSyncStart == m.vsync_start &&
           mVSyncEnd == m.vsync_end && mVTotal == m.vtotal &&
           mVScan == m.vscan && mFlags == m.flags && mType == m.type;
}

void DRMMode::toDrmModeModeInfo(drmModeModeInfo* m) const {
    m->clock = mClock;
    m->hdisplay = mHDisplay;
    m->hsync_start = mHSyncStart;
    m->hsync_end = mHSyncEnd;
    m->htotal = mHTotal;
    m->hskew = mHSkew;
    m->vdisplay = mVDisplay;
    m->vsync_start = mVSyncStart;
    m->vsync_end = mVSyncEnd;
    m->vtotal = mVTotal;
    m->vscan = mVScan;
    m->vrefresh = mVRefresh;
    m->flags = mFlags;
    m->type = mType;
    strncpy(m->name, mName.c_str(), DRM_DISPLAY_MODE_LEN);
}

uint32_t DRMMode::getId() const {
    return mId;
}

void DRMMode::setId(uint32_t id) {
    mId = id;
}

uint32_t DRMMode::getClock() const {
    return mClock;
}

uint32_t DRMMode::getHDisplay() const {
    return mHDisplay;
}

uint32_t DRMMode::getHSyncStart() const {
    return mHSyncStart;
}

uint32_t DRMMode::getHSyncEnd() const {
    return mHSyncEnd;
}

uint32_t DRMMode::getHTotal() const {
    return mHTotal;
}

uint32_t DRMMode::getHSkew() const {
    return mHSkew;
}

uint32_t DRMMode::getVDisplay() const {
    return mVDisplay;
}

uint32_t DRMMode::getVSyncStart() const {
    return mVSyncStart;
}

uint32_t DRMMode::getVSyncEnd() const {
    return mVSyncEnd;
}

uint32_t DRMMode::getVTotal() const {
    return mVTotal;
}

uint32_t DRMMode::getVScan() const {
    return mVScan;
}

float DRMMode::getVRefresh() const {
    return mVRefresh ? mVRefresh * 1.0f :
           mClock / (float)(mVTotal * mHTotal) * 1000.0f;
}

uint32_t DRMMode::getFlags() const {
    return mFlags;
}

uint32_t DRMMode::getType() const {
    return mType;
}

std::string DRMMode::getName() const {
    return mName;
}

} // namespace android
