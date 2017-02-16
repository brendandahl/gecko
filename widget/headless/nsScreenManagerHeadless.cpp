/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsScreenManagerHeadless.h"


nsScreenManagerHeadless::nsScreenManagerHeadless()
{
  mOneScreen = new HeadlessScreen(nullptr);
}


nsScreenManagerHeadless::~nsScreenManagerHeadless()
{
}

// addref, release, QI
NS_IMPL_ISUPPORTS(nsScreenManagerHeadless, nsIScreenManager)

NS_IMETHODIMP
nsScreenManagerHeadless::ScreenForId(uint32_t aId, nsIScreen **outScreen)
{
  NS_IF_ADDREF(*outScreen = mOneScreen.get());
  return NS_OK;
}

NS_IMETHODIMP
nsScreenManagerHeadless::ScreenForRect(int32_t aX, int32_t aY,
                                  int32_t aWidth, int32_t aHeight,
                                  nsIScreen **aOutScreen)
{
  NS_IF_ADDREF(*aOutScreen = mOneScreen.get());
  return NS_OK;
}


NS_IMETHODIMP
nsScreenManagerHeadless::GetPrimaryScreen(nsIScreen * *aPrimaryScreen)
{
  NS_IF_ADDREF(*aPrimaryScreen = mOneScreen.get());
  return NS_OK;
}

NS_IMETHODIMP
nsScreenManagerHeadless::GetNumberOfScreens(uint32_t *aNumberOfScreens)
{
  *aNumberOfScreens = 1;
  return NS_OK;

}

NS_IMETHODIMP
nsScreenManagerHeadless::GetSystemDefaultScale(float *aDefaultScale)
{
  *aDefaultScale = 1;
  return NS_OK;
}

NS_IMETHODIMP
nsScreenManagerHeadless::ScreenForNativeWidget(void *aWidget, nsIScreen **outScreen)
{
  *outScreen = nullptr;
  return NS_OK;
}

HeadlessScreen::HeadlessScreen(void *nativeScreen)
{
}

HeadlessScreen::~HeadlessScreen()
{
}

NS_IMETHODIMP
HeadlessScreen::GetId(uint32_t *outId)
{
  *outId = 1;
  return NS_OK;
}

NS_IMETHODIMP
HeadlessScreen::GetRect(int32_t *outLeft,  int32_t *outTop,
                      int32_t *outWidth, int32_t *outHeight)
{
  nsIntRect r(0, 0, 640, 409);
  *outLeft = r.x;
  *outTop = r.y;
  *outWidth = r.width;
  *outHeight = r.height;
  return NS_OK;
}

NS_IMETHODIMP
HeadlessScreen::GetAvailRect(int32_t *outLeft,  int32_t *outTop,
                           int32_t *outWidth, int32_t *outHeight)
{
  return GetRect(outLeft, outTop, outWidth, outHeight);
}

NS_IMETHODIMP
HeadlessScreen::GetPixelDepth(int32_t *aPixelDepth)
{
  // Default in fingerprint resist mode.
  *aPixelDepth = 24;
  return NS_OK;
}

NS_IMETHODIMP
HeadlessScreen::GetColorDepth(int32_t *aColorDepth)
{
  return GetPixelDepth(aColorDepth);
}

NS_IMETHODIMP
HeadlessScreen::GetRotation(uint32_t* aRotation)
{
  NS_WARNING("Attempt to get screen rotation through nsIScreen::GetRotation().  Nothing should know or care this in sandboxed contexts.  If you want *orientation*, use hal.");
  return NS_ERROR_NOT_AVAILABLE;
}

NS_IMETHODIMP
HeadlessScreen::SetRotation(uint32_t aRotation)
{
  NS_WARNING("Attempt to set screen rotation through nsIScreen::GetRotation().  Nothing should know or care this in sandboxed contexts.  If you want *orientation*, use hal.");
  return NS_ERROR_NOT_AVAILABLE;
}
