/* Copyright (c) 2015-2016, 2020, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#define ATRACE_TAG ATRACE_TAG_CAMERA
#define LOG_TAG "QCamera3CropRegionMapper"

// Camera dependencies
#include "QCamera3CropRegionMapper.h"
#include "QCamera3HWI.h"
#include "math.h"

extern "C" {
#include "mm_camera_dbg.h"
}

using namespace android;

namespace qcamera {

/*===========================================================================
 * FUNCTION   : QCamera3CropRegionMapper
 *
 * DESCRIPTION: Constructor
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCamera3CropRegionMapper::QCamera3CropRegionMapper()
        : mSensorW(0),
          mSensorH(0),
          mActiveArrayW(0),
          mActiveArrayH(0)
{
}

/*===========================================================================
 * FUNCTION   : ~QCamera3CropRegionMapper
 *
 * DESCRIPTION: destructor
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/

QCamera3CropRegionMapper::~QCamera3CropRegionMapper()
{
}

/*===========================================================================
 * FUNCTION   : update
 *
 * DESCRIPTION: update sensor active array size and sensor output size
 *
 * PARAMETERS :
 *   @active_array_w : active array width
 *   @active_array_h : active array height
 *   @sensor_w       : sensor output width
 *   @sensor_h       : sensor output height
 *
 * RETURN     : none
 *==========================================================================*/
void QCamera3CropRegionMapper::update(uint32_t active_array_w,
        uint32_t active_array_h, uint32_t sensor_w,
        uint32_t sensor_h)
{
    // Sanity check
    if (active_array_w == 0 || active_array_h == 0 ||
            sensor_w == 0 || sensor_h == 0) {
        LOGE("active_array size and sensor output size must be non zero");
        return;
    }
    if (active_array_w < sensor_w || active_array_h < sensor_h) {
        LOGE("invalid input: active_array [%d, %d], sensor size [%d, %d]",
                 active_array_w, active_array_h, sensor_w, sensor_h);
        return;
    }
    mSensorW = sensor_w;
    mSensorH = sensor_h;
    mActiveArrayW = active_array_w;
    mActiveArrayH = active_array_h;

    LOGH("active_array: %d x %d, sensor size %d x %d",
            mActiveArrayW, mActiveArrayH, mSensorW, mSensorH);
}

/*===========================================================================
 * FUNCTION   : toActiveArray
 *
 * DESCRIPTION: Map crop rectangle from sensor output space to active array space
 *
 * PARAMETERS :
 *   @crop_left   : x coordinate of top left corner of rectangle
 *   @crop_top    : y coordinate of top left corner of rectangle
 *   @crop_width  : width of rectangle
 *   @crop_height : height of rectangle
 *
 * RETURN     : none
 *==========================================================================*/
void QCamera3CropRegionMapper::toActiveArray(int32_t& crop_left, int32_t& crop_top,
        int32_t& crop_width, int32_t& crop_height)
{
    if (mSensorW == 0 || mSensorH == 0 ||
            mActiveArrayW == 0 || mActiveArrayH == 0) {
        LOGE("sensor/active array sizes are not initialized!");
        return;
    }

    crop_left = crop_left * mActiveArrayW / (mSensorW * 1.0) + 0.5;
    crop_top = crop_top * mActiveArrayH / (mSensorH * 1.0) + 0.5;
    crop_width = crop_width * mActiveArrayW / (mSensorW * 1.0) + 0.5;
    crop_height = crop_height * mActiveArrayH / (mSensorH * 1.0) + 0.5;

    boundToSize(crop_left, crop_top, crop_width, crop_height,
            mActiveArrayW, mActiveArrayH);
}

/*===========================================================================
 * FUNCTION   : toSensor
 *
 * DESCRIPTION: Map crop rectangle from active array space to sensor output space
 *
 * PARAMETERS :
 *   @crop_left   : x coordinate of top left corner of rectangle
 *   @crop_top    : y coordinate of top left corner of rectangle
 *   @crop_width  : width of rectangle
 *   @crop_height : height of rectangle
 *
 * RETURN     : none
 *==========================================================================*/

void QCamera3CropRegionMapper::toSensor(int32_t& crop_left, int32_t& crop_top,
        int32_t& crop_width, int32_t& crop_height)
{
   float tmpLeft, tmpTop, tmpWidth, tmpHeight;

    if (mSensorW == 0 || mSensorH == 0 ||
            mActiveArrayW == 0 || mActiveArrayH == 0) {
        LOGE("sensor/active array sizes are not initialized!");
        return;
    }

    tmpLeft = (float)crop_left * mSensorW / (mActiveArrayW * 1.0) + 0.5;
    tmpTop = (float)crop_top * mSensorH / (mActiveArrayH * 1.0) + 0.5;
    tmpWidth = (float)crop_width * mSensorW / (mActiveArrayW * 1.0) + 0.5;
    tmpHeight = (float)crop_height * mSensorH / (mActiveArrayH * 1.0) + 0.5;


    crop_left = ceil(tmpLeft);
    crop_top  = ceil(tmpTop);
    crop_width = ceil(tmpWidth);
    crop_height = ceil(tmpHeight);

    LOGD("before bounding left %d, top %d, width %d, height %d",
         crop_left, crop_top, crop_width, crop_height);
    boundToSize(crop_left, crop_top, crop_width, crop_height,
            mSensorW, mSensorH);
    LOGD("after bounding left %d, top %d, width %d, height %d",
         crop_left, crop_top, crop_width, crop_height);
}

/*===========================================================================
 * FUNCTION   : boundToSize
 *
 * DESCRIPTION: Bound a particular rectangle inside a bounding box
 *
 * PARAMETERS :
 *   @left    : x coordinate of top left corner of rectangle
 *   @top     : y coordinate of top left corner of rectangle
 *   @width   : width of rectangle
 *   @height  : height of rectangle
 *   @bound_w : width of bounding box
 *   @bound_y : height of bounding box
 *
 * RETURN     : none
 *==========================================================================*/
void QCamera3CropRegionMapper::boundToSize(int32_t& left, int32_t& top,
            int32_t& width, int32_t& height, int32_t bound_w, int32_t bound_h)
{
    if (left < 0) {
        left = 0;
    }
    if (top < 0) {
        top = 0;
    }

    if ((left + width) > bound_w) {
        width = bound_w - left;
    }
    if ((top + height) > bound_h) {
        height = bound_h - top;
    }
}

/*===========================================================================
 * FUNCTION   : toActiveArray
 *
 * DESCRIPTION: Map co-ordinate from sensor output space to active array space
 *
 * PARAMETERS :
 *   @x   : x coordinate
 *   @y   : y coordinate
 *
 * RETURN     : none
 *==========================================================================*/
void QCamera3CropRegionMapper::toActiveArray(uint32_t& x, uint32_t& y)
{
    if (mSensorW == 0 || mSensorH == 0 ||
            mActiveArrayW == 0 || mActiveArrayH == 0) {
        LOGE("sensor/active array sizes are not initialized!");
        return;
    }
    if ((x > static_cast<uint32_t>(mSensorW)) ||
            (y > static_cast<uint32_t>(mSensorH))) {
        LOGE("invalid co-ordinate (%d, %d) in (0, 0, %d, %d) space",
                 x, y, mSensorW, mSensorH);
        return;
    }
    x = x * mActiveArrayW / mSensorW;
    y = y * mActiveArrayH / mSensorH;
}

/*===========================================================================
 * FUNCTION   : toSensor
 *
 * DESCRIPTION: Map co-ordinate from active array space to sensor output space
 *
 * PARAMETERS :
 *   @x   : x coordinate
 *   @y   : y coordinate
 *
 * RETURN     : none
 *==========================================================================*/

void QCamera3CropRegionMapper::toSensor(uint32_t& x, uint32_t& y)
{
    if (mSensorW == 0 || mSensorH == 0 ||
            mActiveArrayW == 0 || mActiveArrayH == 0) {
        LOGE("sensor/active array sizes are not initialized!");
        return;
    }

    if ((x > static_cast<uint32_t>(mActiveArrayW)) ||
            (y > static_cast<uint32_t>(mActiveArrayH))) {
        LOGE("invalid co-ordinate (%d, %d) in (0, 0, %d, %d) space",
                 x, y, mSensorW, mSensorH);
        return;
    }
    x = x * mSensorW / mActiveArrayW;
    y = y * mSensorH / mActiveArrayH;
}

void QCamera3CropRegionMapper::convertFDROI(int32_t& crop_left, int32_t& crop_top,
        int32_t& crop_width, int32_t& crop_height)
{
    float width_ratio, height_ratio, upscale_ratio;
    int new_width, new_height, new_left, new_top;

    if (mSensorW == 0 || mSensorH == 0 ||
            mActiveArrayW == 0 || mActiveArrayH == 0) {
        LOGE("sensor/active array sizes are not initialized!");
        return;
    }

    width_ratio = (float)mActiveArrayW / mSensorW;
    height_ratio = (float)mActiveArrayH / mSensorH;

    if(width_ratio > height_ratio)
        upscale_ratio = height_ratio;
    else
        upscale_ratio = width_ratio;

    new_width = mSensorW * upscale_ratio;
    new_height = mSensorH * upscale_ratio;
    new_left = (mActiveArrayW - new_width) / 2;
    new_top = (mActiveArrayH - new_height) / 2;


    width_ratio = (float)new_width / mSensorW;
    height_ratio = (float)new_height / mSensorH;

    crop_left = crop_left * width_ratio + new_left;
    crop_top = crop_top * height_ratio + new_top;
    crop_width = crop_width * width_ratio;
    crop_height = crop_height * height_ratio;

    boundToSize(crop_left, crop_top, crop_width, crop_height,
            mActiveArrayW, mActiveArrayH);
}

}; //end namespace android
