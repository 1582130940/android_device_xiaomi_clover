#
# Copyright (C) 2018-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from sdm660-common
include device/xiaomi/sdm660-common/BoardConfigCommon.mk

DEVICE_PATH := device/xiaomi/clover

# Assert
TARGET_OTA_ASSERT_DEVICE := clover

# Boot animation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1200

# Build
BUILD_BROKEN_DUP_RULES := true

# Camera
BOARD_QTI_CAMERA_32BIT_ONLY := true
TARGET_SUPPORT_HAL1 := false
TARGET_TS_MAKEUP := true
USE_DEVICE_SPECIFIC_CAMERA := true

# Display
TARGET_SCREEN_DENSITY := 320

# Kernel
TARGET_KERNEL_CONFIG := clover_defconfig

# Light
SOONG_CONFIG_NAMESPACES += CLOVER_BACKLIGHT
SOONG_CONFIG_CLOVER_BACKLIGHT := USE_LCD_BACKLIGHT_INTERFACE_ONLY
SOONG_CONFIG_CLOVER_BACKLIGHT_USE_LCD_BACKLIGHT_INTERFACE_ONLY := true

# Partitions
BOARD_USES_METADATA_PARTITION := true

# Properties
TARGET_SYSTEM_PROP += $(COMMON_PATH)/system.prop
TARGET_VENDOR_PROP += $(DEVICE_PATH)/vendor.prop

# Recovery
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/rootdir/etc/fstab.qcom

# Security patch level
VENDOR_SECURITY_PATCH := $(PLATFORM_SECURITY_PATCH)

# SELinux
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor

# VINTF
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/configs/manifest.xml

# Inherit the proprietary files
include vendor/xiaomi/clover/BoardConfigVendor.mk
