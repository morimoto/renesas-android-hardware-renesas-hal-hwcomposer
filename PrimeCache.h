#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_PRIME_CACHE_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_PRIME_CACHE_H

#include <map>

namespace android {

class PrimeCache {
    struct CacheInfo {
        int layerIndex = 0;
        uint32_t handle = 0;
    };
public:
    static PrimeCache& getInstance();
    void optimizeCache(int layerIndex);
    void optimizeCache();
    void eraseLayerCache(int layerIndex);
    void addEntry(uint64_t stamp, CacheInfo info);
    void clear();
    void setDrmFd(int drmFd);
    void setCachePerLayerLimit(int cpll);

    int destroyHandle(uint32_t handle) const;
    int getCachePerLayerLimit() const;
    uint32_t findEntry(uint64_t stamp, int bufferFd, int index);

    bool& isCacheEnabled();
private:
    PrimeCache();
    ~PrimeCache();
    PrimeCache(const PrimeCache&) = delete;
    PrimeCache(const PrimeCache&&) = delete;
    PrimeCache& operator=(const PrimeCache&) = delete;
    PrimeCache& operator=(PrimeCache&&) = delete;

    int mDrmFd = -1;
    int mCachePerLayerLimit = 0;
    bool mIsPrimeCacheEnabled = true;
    std::map<uint64_t, CacheInfo> mCache;
    std::map<int, int> mStat;
};

}

#endif // ANDROID_HARDWARE_GRAPHICS_COMPOSER_V2_3_PRIME_CACHE_H
