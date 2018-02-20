/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <Resources.h>
#include <SkBitmap.h>
// #include "SkCommandLineFlags.h"
#include <SkData.h>
#include <SkImage.h>
#include <SkImageGenerator.h>
// #include <SkOSFile.h>
// #include <SkOSPath.h>
// #include <SkStream.h>
// #include <SkTypeface.h>

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst) {
    std::unique_ptr<SkImageGenerator> gen(SkImageGenerator::MakeFromEncoded(std::move(data)));
    return gen && dst->tryAllocPixels(gen->getInfo()) &&
        gen->getPixels(gen->getInfo().makeColorSpace(nullptr), dst->getPixels(), dst->rowBytes(),
                       nullptr);
}