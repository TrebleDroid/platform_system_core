/*
 * Copyright (C) 2017 The Android Open Source Project
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

#pragma once

#include <memory>
#include <string>

#include "result.h"
#include "fs_mgr.h"
#include "block_dev_initializer.h"

#include <libsnapshot/snapshot.h>

namespace android {
namespace init {

class FirstStageMount {
  public:
    virtual ~FirstStageMount() = default;

    // The factory method to create a FirstStageMount instance.
    static Result<std::unique_ptr<FirstStageMount>> Create(const std::string& cmdline);
    // Creates devices and logical partitions from storage devices
    bool DoCreateDevices();
    // Mounts fstab entries read from device tree.
    bool DoFirstStageMount();

  protected:
    FirstStageMount(fs_mgr::Fstab fstab);

    bool InitDevices();
    bool InitRequiredDevices(std::set<std::string> devices);
    bool CreateLogicalPartitions();
    bool CreateSnapshotPartitions(android::snapshot::SnapshotManager* sm);
    bool MountPartition(const fs_mgr::Fstab::iterator& begin, bool erase_same_mounts,
                        fs_mgr::Fstab::iterator* end = nullptr);

    bool MountPartitions();
    bool TrySwitchSystemAsRoot();
    bool IsDmLinearEnabled();
    void GetSuperDeviceName(std::set<std::string>* devices);
    bool InitDmLinearBackingDevices(const android::fs_mgr::LpMetadata& metadata);
    void UseDsuIfPresent();
    // Reads all fstab.avb_keys from the ramdisk for first-stage mount.
    void PreloadAvbKeys();
    // Copies /avb/*.avbpubkey used for DSU from the ramdisk to /metadata for key
    // revocation check by DSU installation service.
    void CopyDsuAvbKeys();

    virtual bool GetDmVerityDevices(std::set<std::string>* devices) = 0;
    virtual bool SetUpDmVerity(fs_mgr::FstabEntry* fstab_entry) = 0;

    bool need_dm_verity_;
    bool dsu_not_on_userdata_ = false;
    bool use_snapuserd_ = false;

    fs_mgr::Fstab fstab_;
    // The super path is only set after InitDevices, and is invalid before.
    std::string super_path_;
    std::string super_partition_name_;
    BlockDevInitializer block_dev_init_;
    // Reads all AVB keys before chroot into /system, as they might be used
    // later when mounting other partitions, e.g., /vendor and /product.
    std::map<std::string, std::vector<std::string>> preload_avb_key_blobs_;

};

void SetInitAvbVersionInRecovery();

}  // namespace init
}  // namespace android
