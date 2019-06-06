#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_PRIME_CACHE_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_1_PRIME_CACHE_H

#include <map>

namespace android {

class PrimeCache {
    struct CacheInfo {
        int layerIndex = 0;
        uint32_t handle = 0;
    };
public:
    PrimeCache();
    ~PrimeCache();
    void optimizeCache(int layerIndex);
    void optimizeCache();
    int destroyHandle(uint32_t handle) const;
    void eraseLayerCache(int layerIndex);
    void addEntry(uint64_t stamp, CacheInfo info);
    uint32_t findEntry(uint64_t stamp, int bufferFd, int index);
    void clear();

    void setDrmFd(int drmFd);
    void setCachePerLayerLimit(int cpll);
    void setIsPrimeCacheEnabled(bool isPrimeCacheEnabled);

    int getCachePerLayerLimit() const;
    bool getIsPrimeCacheEnabled() const;

private:
    int mDrmFd = -1;
    int mCachePerLayerLimit = 0;
    bool mIsPrimeCacheEnabled = true;
    std::map<uint64_t, CacheInfo> mCache;
    std::map<int, int> mStat;
};

}

#endif
