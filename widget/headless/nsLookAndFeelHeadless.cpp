/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsLookAndFeelHeadless.h"

using mozilla::LookAndFeel;

nsLookAndFeelHeadless::nsLookAndFeelHeadless()
{
}

nsLookAndFeelHeadless::~nsLookAndFeelHeadless()
{
}

nsresult
nsLookAndFeelHeadless::NativeGetColor(ColorID aID, nscolor& aColor)
{
  NS_WARNING("Native colors not supported by headless look and feel.");
  aColor = NS_RGB(0x00, 0x00, 0x00);
  return NS_OK;
}

nsresult
nsLookAndFeelHeadless::GetIntImpl(IntID aID, int32_t &aResult)
{
  nsresult res = nsXPLookAndFeel::GetIntImpl(aID, aResult);
  if (NS_SUCCEEDED(res)) {
    return res;
  }
  // printf("!!!TODO nsLookAndFeelHeadless::GetIntImpl\n");
  aResult = 0;
  return NS_ERROR_FAILURE;
}

nsresult
nsLookAndFeelHeadless::GetFloatImpl(FloatID aID, float &aResult)
{
  nsresult res = NS_OK;
  res = nsXPLookAndFeel::GetFloatImpl(aID, aResult);
  if (NS_SUCCEEDED(res)) {
    return res;
  }
  // printf("!!!TODO nsLookAndFeelHeadless::GetFloatImpl\n");
  aResult = -1.0;
  return NS_ERROR_FAILURE;
}

bool
nsLookAndFeelHeadless::GetFontImpl(FontID aID, nsString& aFontName,
               gfxFontStyle& aFontStyle,
               float aDevPixPerCSSPixel)
{
  // printf("!!!TODO nsLookAndFeelHeadless::GetFontImpl\n");
  return true;
}

char16_t
nsLookAndFeelHeadless::GetPasswordCharacterImpl()
{
  printf("!!!TODO nsLookAndFeelHeadless::GetPasswordCharacterImpl\n");
  return L'\x0020';
}

void
nsLookAndFeelHeadless::RefreshImpl()
{
  nsXPLookAndFeel::RefreshImpl();
}

bool
nsLookAndFeelHeadless::GetEchoPasswordImpl() {
  return false;
}
