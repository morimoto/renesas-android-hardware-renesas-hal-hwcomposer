#include "PrimeCache.h"
#include "HwcDump.h"

#include <vector>

#include <log/log.h>
#include <xf86drm.h>

namespace android {

PrimeCache::PrimeCache() {
    srand(time(nullptr));
}

PrimeCache::~PrimeCache() {
    clear();
}

void PrimeCache::optimizeCache(int layerIndex) {
    if (mStat[layerIndex] >= mCachePerLayerLimit) {
        std::vector<uint64_t> selection;
        for (auto &cur : mCache) {
            if (cur.second.layerIndex == layerIndex) {
                selection.push_back(cur.first);
            }
        }
        while (mStat[layerIndex] >= mCachePerLayerLimit) {
            int idx = rand() % selection.size();
            auto toDel = selection[idx];
            auto &tmp = mCache[toDel];
            destroyHandle(tmp.handle);
            mCache.erase(toDel);
            --mStat[layerIndex];
            selection.erase(selection.begin() + idx);
        }
    }
}

void PrimeCache::optimizeCache() {
    for (auto &cur : mStat) {
        if (cur.second > mCachePerLayerLimit) {
            optimizeCache(cur.first);
        }
    }
}

int PrimeCache::destroyHandle(uint32_t handle) const {
    drm_gem_close arg = { handle, 0, };
    int ret = -1;
    if ((ret = drmIoctl(mDrmFd, DRM_IOCTL_GEM_CLOSE, &arg))) {
        ALOGE("mIonBuffers: free handle=%d "
                "from drmPrimeFDToHandle", handle);
    }
    return ret;
}

void PrimeCache::eraseLayerCache(int layerIndex) {
    for (auto mit = mCache.begin(); mit != mCache.end(); ) {
        if (mit->second.layerIndex == layerIndex) {
            destroyHandle(mit->second.handle);
            mCache.erase(mit++);
        } else {
            ++mit;
        }
    }
    mStat.erase(layerIndex);
}

void PrimeCache::addEntry(uint64_t stamp, CacheInfo info) {
    optimizeCache(info.layerIndex);
    mCache[stamp] = info;
    ++mStat[info.layerIndex];
}

uint32_t PrimeCache::findEntry(uint64_t stamp, int bufferFd, int index) {
    int ret = -1;
    uint32_t handle = 0;
    auto cc = mCache.find(stamp);
    if (cc != mCache.end()) {
        handle = cc->second.handle;
    } else {
        if ((ret = drmPrimeFDToHandle(mDrmFd, bufferFd, &handle))) {
            ALOGE("mIonBuffers: drmPrimeFDToHandle");
        } else {
            addEntry(stamp, {index, handle});
        }
    }
    return (handle);
}

void PrimeCache::clear() {
    for (auto &cur : mCache) {
        destroyHandle(cur.second.handle);
    }
    mCache.clear();
    mStat.clear();
}

void PrimeCache::setDrmFd(int drmFd) {
    mDrmFd = drmFd;
}

void PrimeCache::setCachePerLayerLimit(int cpll) {
    mCachePerLayerLimit = cpll;
}

void PrimeCache::setIsPrimeCacheEnabled(bool isPrimeCacheEnabled) {
    mIsPrimeCacheEnabled = isPrimeCacheEnabled;
}

int PrimeCache::getCachePerLayerLimit() const {
    return mCachePerLayerLimit;
}

bool PrimeCache::getIsPrimeCacheEnabled() const {
    return mIsPrimeCacheEnabled;
}

}
