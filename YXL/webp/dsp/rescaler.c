#define PM_IS_LIBRARY
#line 1 "G:/faceunity/jc/bin/win32_release/../../wrapper/webp/dsp/rescaler.c"
// Copyright 2014 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Rescaling functions
//
// Author: Skal (pascal.massimino@gmail.com)

#include <assert.h>

#include "./dsp.h"
#include "../utils/rescaler.h"

//------------------------------------------------------------------------------
// Implementations of critical functions ImportRow / ExportRow

#define ROUNDER (WEBP_RESCALER_ONE >> 1)
#define MULT_FIX(x, y) (((uint64_t)(x) * (y) + ROUNDER) >> WEBP_RESCALER_RFIX)

//------------------------------------------------------------------------------
// Row import

void DEDUP_WEBP_RescalerImportRowExpandC(DEDUP_WEBP_Rescaler* const wrk, const uint8_t* src) {
  const int x_stride = wrk->num_channels;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  int channel;
  assert(!DEDUP_WEBP_RescalerInputDone(wrk));
  assert(wrk->x_expand);
  for (channel = 0; channel < x_stride; ++channel) {
    int x_in = channel;
    int x_out = channel;
    // simple bilinear interpolation
    int accum = wrk->x_add;
    int left = src[x_in];
    int right = (wrk->src_width > 1) ? src[x_in + x_stride] : left;
    x_in += x_stride;
    while (1) {
      wrk->frow[x_out] = right * wrk->x_add + (left - right) * accum;
      x_out += x_stride;
      if (x_out >= x_out_max) break;
      accum -= wrk->x_sub;
      if (accum < 0) {
        left = right;
        x_in += x_stride;
        assert(x_in < wrk->src_width * x_stride);
        right = src[x_in];
        accum += wrk->x_add;
      }
    }
    assert(wrk->x_sub == 0 /* <- special case for src_width=1 */ || accum == 0);
  }
}

void DEDUP_WEBP_RescalerImportRowShrinkC(DEDUP_WEBP_Rescaler* const wrk, const uint8_t* src) {
  const int x_stride = wrk->num_channels;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  int channel;
  assert(!DEDUP_WEBP_RescalerInputDone(wrk));
  assert(!wrk->x_expand);
  for (channel = 0; channel < x_stride; ++channel) {
    int x_in = channel;
    int x_out = channel;
    uint32_t sum = 0;
    int accum = 0;
    while (x_out < x_out_max) {
      uint32_t base = 0;
      accum += wrk->x_add;
      while (accum > 0) {
        accum -= wrk->x_sub;
        assert(x_in < wrk->src_width * x_stride);
        base = src[x_in];
        sum += base;
        x_in += x_stride;
      }
      {        // Emit next horizontal pixel.
        const rescaler_t frac = base * (-accum);
        wrk->frow[x_out] = sum * wrk->x_sub - frac;
        // fresh fractional start for next pixel
        sum = (int)MULT_FIX(frac, wrk->fx_scale);
      }
      x_out += x_stride;
    }
    assert(accum == 0);
  }
}

//------------------------------------------------------------------------------
// Row export

void DEDUP_WEBP_RescalerExportRowExpandC(DEDUP_WEBP_Rescaler* const wrk) {
  int x_out;
  uint8_t* const dst = wrk->dst;
  rescaler_t* const irow = wrk->irow;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  const rescaler_t* const frow = wrk->frow;
  assert(!DEDUP_WEBP_RescalerOutputDone(wrk));
  assert(wrk->y_accum <= 0);
  assert(wrk->y_expand);
  assert(wrk->y_sub != 0);
  if (wrk->y_accum == 0) {
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const uint32_t J = frow[x_out];
      const int v = (int)MULT_FIX(J, wrk->fy_scale);
      assert(v >= 0 && v <= 255);
      dst[x_out] = v;
    }
  } else {
    const uint32_t B = WEBP_RESCALER_FRAC(-wrk->y_accum, wrk->y_sub);
    const uint32_t A = (uint32_t)(WEBP_RESCALER_ONE - B);
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const uint64_t I = (uint64_t)A * frow[x_out]
                       + (uint64_t)B * irow[x_out];
      const uint32_t J = (uint32_t)((I + ROUNDER) >> WEBP_RESCALER_RFIX);
      const int v = (int)MULT_FIX(J, wrk->fy_scale);
      assert(v >= 0 && v <= 255);
      dst[x_out] = v;
    }
  }
}

void DEDUP_WEBP_RescalerExportRowShrinkC(DEDUP_WEBP_Rescaler* const wrk) {
  int x_out;
  uint8_t* const dst = wrk->dst;
  rescaler_t* const irow = wrk->irow;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  const rescaler_t* const frow = wrk->frow;
  const uint32_t yscale = wrk->fy_scale * (-wrk->y_accum);
  assert(!DEDUP_WEBP_RescalerOutputDone(wrk));
  assert(wrk->y_accum <= 0);
  assert(!wrk->y_expand);
  if (yscale) {
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const uint32_t frac = (uint32_t)MULT_FIX(frow[x_out], yscale);
      const int v = (int)MULT_FIX(irow[x_out] - frac, wrk->fxy_scale);
      assert(v >= 0 && v <= 255);
      dst[x_out] = v;
      irow[x_out] = frac;   // new fractional start
    }
  } else {
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const int v = (int)MULT_FIX(irow[x_out], wrk->fxy_scale);
      assert(v >= 0 && v <= 255);
      dst[x_out] = v;
      irow[x_out] = 0;
    }
  }
}

#undef MULT_FIX
#undef ROUNDER

//------------------------------------------------------------------------------
// Main entry calls

void DEDUP_WEBP_RescalerImportRow(DEDUP_WEBP_Rescaler* const wrk, const uint8_t* src) {
  assert(!DEDUP_WEBP_RescalerInputDone(wrk));
  if (!wrk->x_expand) {
    DEDUP_WEBP_RescalerImportRowShrink(wrk, src);
  } else {
    DEDUP_WEBP_RescalerImportRowExpand(wrk, src);
  }
}

void DEDUP_WEBP_RescalerExportRow(DEDUP_WEBP_Rescaler* const wrk) {
  if (wrk->y_accum <= 0) {
    assert(!DEDUP_WEBP_RescalerOutputDone(wrk));
    if (wrk->y_expand) {
      DEDUP_WEBP_RescalerExportRowExpand(wrk);
    } else if (wrk->fxy_scale) {
      DEDUP_WEBP_RescalerExportRowShrink(wrk);
    } else {  // special case
      int i;
      assert(wrk->src_height == wrk->dst_height && wrk->x_add == 1);
      assert(wrk->src_width == 1 && wrk->dst_width <= 2);
      for (i = 0; i < wrk->num_channels * wrk->dst_width; ++i) {
        wrk->dst[i] = wrk->irow[i];
        wrk->irow[i] = 0;
      }
    }
    wrk->y_accum += wrk->y_add;
    wrk->dst += wrk->dst_stride;
    ++wrk->dst_y;
  }
}

//------------------------------------------------------------------------------

DEDUP_WEBP_RescalerImportRowFunc DEDUP_WEBP_RescalerImportRowExpand;
DEDUP_WEBP_RescalerImportRowFunc DEDUP_WEBP_RescalerImportRowShrink;

DEDUP_WEBP_RescalerExportRowFunc DEDUP_WEBP_RescalerExportRowExpand;
DEDUP_WEBP_RescalerExportRowFunc DEDUP_WEBP_RescalerExportRowShrink;

extern void DEDUP_WEBP_RescalerDspInitSSE2(void);
extern void DEDUP_WEBP_RescalerDspInitMIPS32(void);
extern void DEDUP_WEBP_RescalerDspInitMIPSdspR2(void);
extern void DEDUP_WEBP_RescalerDspInitMSA(void);
extern void DEDUP_WEBP_RescalerDspInitNEON(void);

static volatile DEDUP_vP8_CPUInfo rescaler_last_cpuinfo_used =
    (DEDUP_vP8_CPUInfo)&rescaler_last_cpuinfo_used;

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_WEBP_RescalerDspInit(void) {
  if (rescaler_last_cpuinfo_used == DEDUP_vP8_GetCPUInfo) return;

  DEDUP_WEBP_RescalerImportRowExpand = DEDUP_WEBP_RescalerImportRowExpandC;
  DEDUP_WEBP_RescalerImportRowShrink = DEDUP_WEBP_RescalerImportRowShrinkC;
  DEDUP_WEBP_RescalerExportRowExpand = DEDUP_WEBP_RescalerExportRowExpandC;
  DEDUP_WEBP_RescalerExportRowShrink = DEDUP_WEBP_RescalerExportRowShrinkC;

  if (DEDUP_vP8_GetCPUInfo != NULL) {
#if defined(WEBP_USE_SSE2)
    if (DEDUP_vP8_GetCPUInfo(kSSE2)) {
      DEDUP_WEBP_RescalerDspInitSSE2();
    }
#endif
#if defined(WEBP_USE_NEON)
    if (DEDUP_vP8_GetCPUInfo(kNEON)) {
      DEDUP_WEBP_RescalerDspInitNEON();
    }
#endif
#if defined(WEBP_USE_MIPS32)
    if (DEDUP_vP8_GetCPUInfo(kMIPS32)) {
      DEDUP_WEBP_RescalerDspInitMIPS32();
    }
#endif
#if defined(WEBP_USE_MIPS_DSP_R2)
    if (DEDUP_vP8_GetCPUInfo(kMIPSdspR2)) {
      DEDUP_WEBP_RescalerDspInitMIPSdspR2();
    }
#endif
#if defined(WEBP_USE_MSA)
    if (DEDUP_vP8_GetCPUInfo(kMSA)) {
      DEDUP_WEBP_RescalerDspInitMSA();
    }
#endif
  }
  rescaler_last_cpuinfo_used = DEDUP_vP8_GetCPUInfo;
}
