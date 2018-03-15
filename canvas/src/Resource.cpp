/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <Resources.h>
#include <SkBitmap.h>
#include <SkData.h>
#include <SkImage.h>
#include <SkImageGenerator.h>

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst) {
  std::unique_ptr<SkImageGenerator> gen(SkImageGenerator::MakeFromEncoded(std::move(data)));
  if (gen) {
    SkImageInfo imageInfo = gen->getInfo()
      .makeColorType(SkColorType::kRGBA_8888_SkColorType)
      .makeColorSpace(nullptr);
    return dst->tryAllocPixels(imageInfo) && gen->getPixels(imageInfo, dst->getPixels(), dst->rowBytes(), nullptr);
  } else {
    return false;
  }
}