#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2021 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE=clover
VENDOR=xiaomi

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        product/etc/permissions/vendor.qti.hardware.data.connection-V1.0-java.xml | product/etc/permissions/vendor.qti.hardware.data.connection-V1.1-java.xml)
            sed -i 's/version="2.0"/version="1.0"/g' "${2}"
            ;;
        system_ext/etc/init/dpmd.rc)
            sed -i "s|/system/product/bin/|/system/system_ext/bin/|g" "${2}"
            ;;
        system_ext/etc/permissions/com.qti.dpmframework.xml | system_ext/etc/permissions/dpmapi.xml | system_ext/etc/permissions/telephonyservice.xml | system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.0-java.xml | system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.1-java.xml | system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.2-java.xml)
            sed -i "s|/system/product/framework/|/system/system_ext/framework/|g" "${2}"
            ;;
        system_ext/etc/permissions/qcrilhook.xml)
            sed -i 's|/product/framework/qcrilhook.jar|/system_ext/framework/qcrilhook.jar|g' "${2}"
            ;;
        system_ext/etc/permissions/audiosphere.xml)
            sed -i 's|/system/framework/|/system_ext/framework/|g' "${2}"
            ;;
        system_ext/lib64/libdpmframework.so)
            for LIBSHIM_DPMFRAMEWORK in $(grep -L "libshim_dpmframework.so" "${2}"); do
                "${PATCHELF}" --add-needed "libshim_dpmframework.so" "$LIBSHIM_DPMFRAMEWORK"
            done
            ;;
        system_ext/etc/init/perfservice.rc)
            sed -i 's|/system/bin/|/system_ext/bin/|g' "${2}"
            ;;
        vendor/lib/hw/camera.sdm660.so)
            for LIBCAMERA_SDM660_SHIM in $(grep -L "libcamera_sdm660_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libcamera_sdm660_shim.so" "$LIBCAMERA_SDM660_SHIM"
            done
            ;;
        vendor/lib/libFaceGrade.so)
           "${PATCHELF}" --remove-needed libandroid.so "${2}"
            ;;
        vendor/lib/libmmcamera_bokeh.so | vendor/lib/libmmcamera_ppeiscore.so)
           "${PATCHELF}" --replace-needed "libgui.so" "libgui_vendor.so" "${2}"
            ;;
    esac
}

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"

"${MY_DIR}/setup-makefiles.sh"
