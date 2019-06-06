/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "HwcBuffer.h"
#include "HwcLayer.h"
#include "platform.h"

namespace android {

const DrmHwcBo* DrmHwcBuffer::operator->() const {
    if (mImporter == nullptr) {
        ALOGE("Access of non-existent BO");
        return nullptr;
    }

    return &mBo;
}

void DrmHwcBuffer::clear() {
    if (mImporter != nullptr) {
        mImporter->releaseBuffer(&mBo);
        mImporter = nullptr;
    }
}

int DrmHwcBuffer::importBuffer(buffer_handle_t handle, Importer* importer, int index) {
    DrmHwcBo tmp_bo;
    tmp_bo.index = index;

    CHECK_RES_FATAL(importer->importBuffer(handle, &tmp_bo));
    clear();

    mImporter = importer;
    mBo = std::move(tmp_bo);
    return 0;
}

int DrmHwcBuffer::createFrameBuffer() {
    if (mImporter == nullptr) {
        ALOGE("Access of non-existent BO");
        return -1;
    }

    return mImporter->createFrameBuffer(&mBo);
}

int DrmHwcLayer::importBuffer(Importer* importer) {
    CHECK_RES_FATAL(mBuffer.importBuffer(mBuffHandle, importer, index));
    return 0;
}

} // namespace android
