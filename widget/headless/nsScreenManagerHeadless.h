/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsScreenManagerHeadless_h___
#define nsScreenManagerHeadless_h___

#include "nsIScreenManager.h"
#include "nsBaseScreen.h"

//------------------------------------------------------------------------

class nsScreenManagerHeadless : public nsIScreenManager
{
public:
  nsScreenManagerHeadless ( );

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCREENMANAGER

private:
  virtual ~nsScreenManagerHeadless();
protected:
    nsCOMPtr<nsIScreen> mOneScreen;
};

class HeadlessScreen : public nsBaseScreen
{
public:
    explicit HeadlessScreen(void* nativeScreen);
    ~HeadlessScreen();

    NS_IMETHOD GetId(uint32_t* aId) override;
    NS_IMETHOD GetRect(int32_t* aLeft, int32_t* aTop, int32_t* aWidth, int32_t* aHeight) override;
    NS_IMETHOD GetAvailRect(int32_t* aLeft, int32_t* aTop, int32_t* aWidth, int32_t* aHeight) override;
    NS_IMETHOD GetPixelDepth(int32_t* aPixelDepth) override;
    NS_IMETHOD GetColorDepth(int32_t* aColorDepth) override;
    NS_IMETHOD GetRotation(uint32_t* aRotation) override;
    NS_IMETHOD SetRotation(uint32_t  aRotation) override;
};

#endif  // nsScreenManagerHeadless_h___
