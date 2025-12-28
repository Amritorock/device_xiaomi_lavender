/*
   Copyright (c) 2016, The CyanogenMod Project
   Copyright (c) 2019-2025, The LineageOS Project
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/sysinfo.h>
#include <sys/_system_properties.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include <android-base/properties.h>

#include "vendor_init.h"

using android::base::GetProperty;

void property_override(const char* prop, const char* value) {
    prop_info* pi = (prop_info*)__system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

static std::string to_upper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return str;
}

static void load_dalvikvm_props() {
    struct sysinfo sys;
    sysinfo(&sys);

    if (sys.totalram > 3072ull * 1024 * 1024) {
        property_override("dalvik.vm.heapstartsize", "8m");
        property_override("dalvik.vm.heapgrowthlimit", "192m");
        property_override("dalvik.vm.heaptargetutilization", "0.6");
        property_override("dalvik.vm.heapminfree", "8m");
        property_override("dalvik.vm.heapmaxfree", "16m");
    } else {
        property_override("dalvik.vm.heapstartsize", "8m");
        property_override("dalvik.vm.heapgrowthlimit", "192m");
        property_override("dalvik.vm.heaptargetutilization", "0.75");
        property_override("dalvik.vm.heapminfree", "2m");
        property_override("dalvik.vm.heapmaxfree", "8m");
    }

    property_override("dalvik.vm.heapsize", "512m");
}

struct lavender_props {
    std::string build_description;
    std::string build_fingerprint;
    std::string model;
    std::string device;
};

static const std::vector<std::string> ro_props_default_source_order = {
    "",
    "odm.",
    "product.",
    "system.",
    "vendor.",
    "system_ext.",
};

static void set_variant_props(const lavender_props& prop) {
    property_override("ro.build.description", prop.build_description.c_str());
    property_override("ro.build.product", prop.device.c_str());

    for (const auto& source : ro_props_default_source_order) {
        std::string fp = "ro." + source + "build.fingerprint";
        std::string dev = "ro.product." + source + "device";
        std::string mdl = "ro.product." + source + "model";

        property_override(fp.c_str(), prop.build_fingerprint.c_str());
        property_override(dev.c_str(), prop.device.c_str());
        property_override(mdl.c_str(), prop.model.c_str());
    }
}

void vendor_load_properties() {
    load_dalvikvm_props();

    std::string hwc = to_upper(GetProperty("ro.boot.hwc", ""));

    const std::string build_desc =
        "lavender-user 10 QKQ1.190910.002 V11.0.1.0.QFGMIXM release-keys";

    const lavender_props global_48 = {
        build_desc,
        "xiaomi/lavender/lavender:10/QKQ1.190910.002/V11.0.1.0.QFGMIXM/release-keys",
        "Redmi Note 7 (48MP)",
        "lavender"
    };

    const lavender_props india_12 = {
        build_desc,
        "xiaomi/lavender/lavender:10/QKQ1.190910.002/V11.0.1.0.QFGINXM/release-keys",
        "Redmi Note 7",
        "lavender"
    };

    const lavender_props india_48 = {
        build_desc,
        "xiaomi/lavender/lavender:10/QKQ1.190910.002/V11.0.1.0.QFGINXM/release-keys",
        "Redmi Note 7S",
        "lavender"
    };

    if (hwc == "INDIA_48_5") {
        set_variant_props(india_48);
    } else if (hwc == "INDIA") {
        set_variant_props(india_12);
    } else {
        set_variant_props(global_48);
    }
}

