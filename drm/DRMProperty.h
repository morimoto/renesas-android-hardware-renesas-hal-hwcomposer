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

#ifndef ANDROID_HARDWARE_GRAPHICS_COMPOSER_DRM_PROPERTY_H
#define ANDROID_HARDWARE_GRAPHICS_COMPOSER_DRM_PROPERTY_H

#include <string>
#include <vector>
#include <xf86drmMode.h>
#include <cerrno>

namespace android {

enum DrmPropertyType {
    DRM_PROPERTY_TYPE_INT,
    DRM_PROPERTY_TYPE_ENUM,
    DRM_PROPERTY_TYPE_OBJECT,
    DRM_PROPERTY_TYPE_BLOB,
    DRM_PROPERTY_TYPE_INVALID,
};

class DRMProperty {
public:
    DRMProperty() = default;
    DRMProperty(drmModePropertyPtr p, uint64_t value);
    DRMProperty(const DRMProperty&) = default;
    DRMProperty& operator=(const DRMProperty&) = default;

    void init(drmModePropertyPtr p, uint64_t value);
    uint32_t getId() const;
    std::string getName() const;
    int getValue(uint64_t* value) const;
    std::tuple<uint64_t, int> getEnumValueWithName(std::string name) const;

    static int getProperty(uint32_t fd, uint32_t obj_id, uint32_t obj_type,
                           const char* prop_name, DRMProperty* property);
    static int getPlaneProperty(uint32_t fd, uint32_t plane_id,
                                const char* prop_name, DRMProperty* property);
    static int getCrtcProperty(uint32_t fd, uint32_t crtc_id, const char* prop_name,
                               DRMProperty* property);
    static int getConnectorProperty(uint32_t fd, uint32_t connector_id,
                                    const char* prop_name, DRMProperty* property);

private:
    class DRMPropertyEnum {
    public:
        DRMPropertyEnum(drm_mode_property_enum* e);
        ~DRMPropertyEnum() = default;

        uint64_t mValue;
        std::string mName;
    };

    uint32_t mId = 0;

    DrmPropertyType mType = DRM_PROPERTY_TYPE_INVALID;
    uint32_t mFlags = 0;
    std::string mName;
    uint64_t mValue = 0;

    std::vector<uint64_t> mValues;
    std::vector<DRMPropertyEnum> mEnums;
    std::vector<uint32_t> mBlobIds;
};

} // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_COMPOSER_DRM_PROPERTY_H
