/*
 * @Author: your name
 * @Date: 2020-08-06 09:23:21
 * @LastEditTime: 2020-08-06 09:27:28
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \git\VideoRenderPlugin\Nv2RGBRender_Gpu\ColorSpace.h
 */
#pragma once
#include <stdint.h>
#include <cuda_runtime.h>

typedef enum ColorSpaceStandard {
    ColorSpaceStandard_BT709 = 1,
    ColorSpaceStandard_Unspecified = 2,
    ColorSpaceStandard_Reserved = 3,
    ColorSpaceStandard_FCC = 4,
    ColorSpaceStandard_BT470 = 5,
    ColorSpaceStandard_BT601 = 6,
    ColorSpaceStandard_SMPTE240M = 7,
    ColorSpaceStandard_YCgCo = 8,
    ColorSpaceStandard_BT2020 = 9,
    ColorSpaceStandard_BT2020C = 10
} ColorSpaceStandard;

union BGRA32 {
    uint32_t d;
    uchar4 v;
    struct {
        uint8_t b, g, r, a;
    } c;
};

union RGBA32 {
    uint32_t d;
    uchar4 v;
    struct {
        uint8_t r, g, b, a;
    } c;
};

union BGRA64 {
    uint64_t d;
    ushort4 v;
    struct {
        uint16_t b, g, r, a;
    } c;
};

union RGBA64 {
    uint64_t d;
    ushort4 v;
    struct {
        uint16_t r, g, b, a;
    } c;
};

template <class COLOR32>
void Nv12ToColor32(uint8_t *dpNv12, int nNv12Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 0);
template <class COLOR64>
void Nv12ToColor64(uint8_t *dpNv12, int nNv12Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 0);

template <class COLOR32>
void P016ToColor32(uint8_t *dpP016, int nP016Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 4);
template <class COLOR64>
void P016ToColor64(uint8_t *dpP016, int nP016Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 4);

template <class COLOR32>
void YUV444ToColor32(uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 0);
template <class COLOR64>
void YUV444ToColor64(uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 0);

template <class COLOR32>
void YUV444P16ToColor32(uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 4);
template <class COLOR64>
void YUV444P16ToColor64(uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight, int iMatrix = 4);

template <class COLOR32>
void Nv12ToColorPlanar(uint8_t *dpNv12, int nNv12Pitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight, int iMatrix = 0);
template <class COLOR32>
void P016ToColorPlanar(uint8_t *dpP016, int nP016Pitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight, int iMatrix = 4);

template <class COLOR32>
void YUV444ToColorPlanar(uint8_t *dpYUV444, int nPitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight, int iMatrix = 0);
template <class COLOR32>
void YUV444P16ToColorPlanar(uint8_t *dpYUV444, int nPitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight, int iMatrix = 4);

void Bgra64ToP016(uint8_t *dpBgra, int nBgraPitch, uint8_t *dpP016, int nP016Pitch, int nWidth, int nHeight, int iMatrix = 4);