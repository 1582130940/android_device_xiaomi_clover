#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

function blob_fixup() {
    case "${1}" in
        vendor/lib/libmmcamera_bokeh.so | vendor/lib/libmmcamera_ppeiscore.so)
           "${PATCHELF}" --replace-needed "libgui.so" "libgui_vendor.so" "${2}"
            ;;
        vendor/lib/lib_lowlight.so | vendor/lib/libarcsoft_beautyshot.so | vendor/lib/libchromaflash.so | vendor/lib/libdualcameraddm.so | vendor/lib/libmmcamera_hdr_gb_lib.so | vendor/lib/liboptizoom.so | vendor/lib/libseemore.so | vendor/lib/libtrueportrait.so | vendor/lib/libts_detected_face_hal.so | vendor/lib/libts_face_beautify_hal.so | vendor/lib/libubifocus.so | vendor/lib/libvideobokeh.so)
           "${PATCHELF}" --replace-needed "libstdc++.so" "libstdc++_vendor.so" "${2}"
            ;;
        vendor/lib/vendor.qti.hardware.fingerprint@1.0.so | vendor/lib64/vendor.qti.hardware.fingerprint@1.0.so)
            "${PATCHELF}" --remove-needed "android.hidl.base@1.0.so" "${2}"
            ;;
    esac
}

# If we're being sourced by the common script that we called,
# stop right here. No need to go down the rabbit hole.
if [ "${BASH_SOURCE[0]}" != "${0}" ]; then
    return
fi

set -e

export DEVICE=clover
export DEVICE_COMMON=sdm660-common
export VENDOR=xiaomi
export VENDOR_COMMON=${VENDOR}

"./../../${VENDOR_COMMON}/${DEVICE_COMMON}/extract-files.sh" "$@"
