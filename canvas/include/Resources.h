/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Resources_DEFINED
#define Resources_DEFINED

#include <SkImage.h>
#include <SkString.h>

class SkBitmap;
class SkData;

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst);

#endif  // Resources_DEFINED
