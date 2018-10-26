/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "pixelstats"

#include <android-base/logging.h>
#include <utils/StrongPointer.h>

#include "DropDetect.h"
#include "SysfsCollector.h"
#include "UeventListener.h"

using android::sp;
using device::google::crosshatch::DropDetect;
using device::google::crosshatch::SysfsCollector;
using device::google::crosshatch::UeventListener;

#define UFSHC_PATH(filename) "/sys/devices/platform/soc/1d84000.ufshc/" #filename
const struct SysfsCollector::SysfsPaths sysfs_paths = {
    .SlowioReadCntPath = UFSHC_PATH(slowio_read_cnt),
    .SlowioWriteCntPath = UFSHC_PATH(slowio_write_cnt),
    .SlowioUnmapCntPath = UFSHC_PATH(slowio_unmap_cnt),
    .SlowioSyncCntPath = UFSHC_PATH(slowio_sync_cnt),
    .CycleCountBinsPath = "/sys/class/power_supply/maxfg/cycle_counts_bins",
    .ImpedancePath = "/sys/class/misc/msm_cirrus_playback/resistance_left_right",
    .CodecPath = "/sys/devices/platform/soc/171c0000.slim/tavil-slim-pgd/tavil_codec/codec_state",
};

const char *const kAudioUevent = "/kernel/q6audio/q6voice_uevent";

int main() {
    LOG(INFO) << "starting PixelStats";

    sp<DropDetect> dropDetector = DropDetect::start();
    if (!dropDetector) {
        LOG(ERROR) << "Unable to launch drop detection";
        return 1;
    }

    UeventListener ueventListener(kAudioUevent);
    std::thread listenThread(&UeventListener::ListenForever, &ueventListener);
    listenThread.detach();

    SysfsCollector collector(sysfs_paths);
    collector.collect();  // This blocks forever.

    return 0;
}
