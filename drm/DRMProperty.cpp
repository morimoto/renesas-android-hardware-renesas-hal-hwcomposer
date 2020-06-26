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

#include "DRMProperty.h"

#include <log/log.h>

namespace android {

DRMProperty::DRMPropertyEnum::DRMPropertyEnum(drm_mode_property_enum* e) :
    mValue(e->value), mName(e->name) {
}

DRMProperty::DRMProperty(drmModePropertyPtr p, uint64_t value) :
    mId(0), mType(DRM_PROPERTY_TYPE_INVALID), mFlags(0), mName("") {
    init(p, value);
}

void DRMProperty::init(drmModePropertyPtr p, uint64_t value) {
    mId = p->prop_id;
    mFlags = p->flags;
    mName = p->name;
    mValue = value;

    for (int i = 0; i < p->count_values; ++i)
        mValues.push_back(p->values[i]);

    for (int i = 0; i < p->count_enums; ++i)
        mEnums.push_back(DRMPropertyEnum(&p->enums[i]));

    for (int i = 0; i < p->count_blobs; ++i)
        mBlobIds.push_back(p->blob_ids[i]);

    if (mFlags & DRM_MODE_PROP_RANGE)
        mType = DRM_PROPERTY_TYPE_INT;
    else if (mFlags & DRM_MODE_PROP_ENUM)
        mType = DRM_PROPERTY_TYPE_ENUM;
    else if (mFlags & DRM_MODE_PROP_OBJECT)
        mType = DRM_PROPERTY_TYPE_OBJECT;
    else if (mFlags & DRM_MODE_PROP_BLOB)
        mType = DRM_PROPERTY_TYPE_BLOB;
}

uint32_t DRMProperty::getId() const {
    return mId;
}

std::string DRMProperty::getName() const {
    return mName;
}

int DRMProperty::getValue(uint64_t* value) const {
    if (mType == DRM_PROPERTY_TYPE_BLOB) {
        *value = mValue;
        return 0;
    }

    if (mValues.size() == 0)
        return -ENOENT;

    switch (mType) {
    case DRM_PROPERTY_TYPE_INT:
        *value = mValue;
        return 0;

    case DRM_PROPERTY_TYPE_ENUM:
        if (mValue >= mEnums.size())
            return -ENOENT;

        *value = mEnums[mValue].mValue;
        return 0;

    case DRM_PROPERTY_TYPE_OBJECT:
        *value = mValue;
        return 0;

    default:
        return -EINVAL;
    }
}

std::tuple<uint64_t, int> DRMProperty::getEnumValueWithName(
        std::string name) const {
    for (auto it : mEnums) {
        if (it.mName.compare(name) == 0) {
            return std::make_tuple(it.mValue, 0);
        }
    }

    ALOGE("Failed to get value with name %s for propperty %s",
          name.c_str(), mName.c_str());
    return std::make_tuple(UINT64_MAX, -EINVAL);
}

int DRMProperty::getProperty(uint32_t fd, uint32_t obj_id, uint32_t obj_type,
                             const char* prop_name, DRMProperty* property) {
    drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, obj_id,
                                       obj_type);

    if (!props) {
        ALOGE("DRMProperty::GetProperty| Failed to get properties for %d/%x", obj_id,
              obj_type);
        return -ENODEV;
    }

    bool found = false;

    for (int i = 0; !found && (size_t)i < props->count_props; ++i) {
        drmModePropertyPtr p = drmModeGetProperty(fd, props->props[i]);

        if (!strcmp(p->name, prop_name)) {
            property->init(p, props->prop_values[i]);
            found = true;
        }

        drmModeFreeProperty(p);
    }

    drmModeFreeObjectProperties(props);
    return found ? 0 : -ENOENT;
}

int DRMProperty::getPlaneProperty(uint32_t fd, uint32_t plane_id,
                                  const char* prop_name, DRMProperty* property) {
    return getProperty(fd, plane_id, DRM_MODE_OBJECT_PLANE, prop_name, property);
}

int DRMProperty::getCrtcProperty(uint32_t fd, uint32_t crtc_id,
                                 const char* prop_name, DRMProperty* property) {
    return getProperty(fd, crtc_id, DRM_MODE_OBJECT_CRTC, prop_name, property);
}

int DRMProperty::getConnectorProperty(uint32_t fd, uint32_t connector_id,
                                      const char* prop_name, DRMProperty* property) {
    return getProperty(fd, connector_id, DRM_MODE_OBJECT_CONNECTOR, prop_name,
                       property);
}

} // namespace android
