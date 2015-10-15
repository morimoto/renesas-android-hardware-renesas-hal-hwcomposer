/* Copyright (c) Renesas Electronics Corporation
 *
 * The contents of this file are subject to the MIT license as set out below.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GRALLOC_CUSTOM_H
#define GRALLOC_CUSTOM_H

#include <hardware/gralloc.h>

/* Custom Pixel Formats */
#define HAL_PIXEL_FORMAT_BGRX_8888     (0x101)
#define HAL_PIXEL_FORMAT_sBGR_A_8888     (0x102)
#define HAL_PIXEL_FORMAT_sBGR_X_8888     (0x103)
#define HAL_PIXEL_FORMAT_NV12          (0x106)
#define HAL_PIXEL_FORMAT_NV12_CUSTOM   (0x107)
#define HAL_PIXEL_FORMAT_NV21          (HAL_PIXEL_FORMAT_YCrCb_420_SP)
#define HAL_PIXEL_FORMAT_NV21_CUSTOM   (0x109)
#define HAL_PIXEL_FORMAT_UYVY          (0x10A)

 /* We must align YV12 to a multiple of 32bytes as NEON optimizations
 * in stagefright require the YV12 planes to be 128bit aligned.
 */
#define YV12_ALIGN  32

#endif /* GRALLOC_CUSTOM_H */
