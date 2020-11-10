/*
 * Copyright (C) 2020 GlobalLogic
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

package hwcomposer_defaults

import (
     "android/soong/android"
     "android/soong/cc"
)

func globalFlags(ctx android.BaseContext) []string {
    var cflags []string

    if ctx.Config().IsEnvTrue("TARGET_DISABLE_PRIME_CACHE") {
        cflags = append(cflags, "-DHWC_PRIME_CACHE=0")
    } else {
        cflags = append(cflags, "-DHWC_PRIME_CACHE=1")
    }

    if ctx.Config().IsEnvTrue("ENABLE_CMS") {
        cflags = append(cflags, "-DHWC_ENABLE_CMS=1")
    }

    if ctx.Config().IsEnvTrue("TARGET_ENABLE_HOTPLUG_SUPPORT") {
        cflags = append(cflags, "-DHWC_HOTPLUG_SUPPORT=1")
    }

    switch ctx.AConfig().Getenv("TARGET_PRODUCT") {
    case "salvator":
        cflags = append(cflags, "-DTARGET_BOARD_SALVATOR")
    case "kingfisher":
        cflags = append(cflags, "-DTARGET_BOARD_KINGFISHER")
    }

    switch ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM") {
    case "r8a77965":
        cflags = append(cflags, "-DTARGET_BOARD_PLATFORM_R8A77965")
    case "r8a7796":
        cflags = append(cflags, "-DTARGET_BOARD_PLATFORM_R8A7796")
    default: //r8a7795
        cflags = append(cflags, "-DTARGET_BOARD_PLATFORM_R8A7795")
    }

    return cflags
}

func hwcomposerDefaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
    }

    p := &props{}
    p.Cflags = globalFlags(ctx)

    ctx.AppendProperties(p)

}

func init() {
    android.RegisterModuleType("hwcomposer_defaults", HWComposerDefaultsFactory)
}

func HWComposerDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, hwcomposerDefaults)

    return module
}
