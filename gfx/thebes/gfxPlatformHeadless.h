/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GFX_PLATFORM_HEADLESS_H
#define GFX_PLATFORM_HEADLESS_H

#include "gfxPlatform.h"
#include "nsTArray.h"
#include "mozilla/gfx/gfxVars.h"

class gfxFontconfigUtils;

class gfxPlatformHeadless : public gfxPlatform {
public:
    gfxPlatformHeadless();
    virtual ~gfxPlatformHeadless();

    virtual already_AddRefed<gfxASurface>
      CreateOffscreenSurface(const IntSize& aSize,
                             gfxImageFormat aFormat) override;

    virtual already_AddRefed<mozilla::gfx::ScaledFont>
      GetScaledFontForFont(mozilla::gfx::DrawTarget* aTarget, gfxFont *aFont) override;

    virtual nsresult GetFontList(nsIAtom *aLangGroup,
                                 const nsACString& aGenericFamily,
                                 nsTArray<nsString>& aListOfFonts) override;

    virtual nsresult UpdateFontList() override;

    virtual void
    GetCommonFallbackFonts(uint32_t aCh, uint32_t aNextCh,
                           Script aRunScript,
                           nsTArray<const char*>& aFontList) override;

    virtual gfxPlatformFontList* CreatePlatformFontList() override;

    virtual nsresult GetStandardFamilyName(const nsAString& aFontName,
                                           nsAString& aFamilyName) override;

    gfxFontGroup*
    CreateFontGroup(const mozilla::FontFamilyList& aFontFamilyList,
                    const gfxFontStyle *aStyle,
                    gfxTextPerfMetrics* aTextPerf,
                    gfxUserFontSet *aUserFontSet,
                    gfxFloat aDevToCssSize) override;

    /**
     * Look up a local platform font using the full font face name (needed to
     * support @font-face src local() )
     */
    virtual gfxFontEntry* LookupLocalFont(const nsAString& aFontName,
                                          uint16_t aWeight,
                                          int16_t aStretch,
                                          uint8_t aStyle) override;

    /**
     * Activate a platform font (needed to support @font-face src url() )
     *
     */
    virtual gfxFontEntry* MakePlatformFont(const nsAString& aFontName,
                                           uint16_t aWeight,
                                           int16_t aStretch,
                                           uint8_t aStyle,
                                           const uint8_t* aFontData,
                                           uint32_t aLength) override;

    /**
     * Check whether format is supported on a platform or not (if unclear,
     * returns true).
     */
    virtual bool IsFontFormatSupported(nsIURI *aFontURI,
                                         uint32_t aFormatFlags) override;

    virtual void FlushContentDrawing() override;

    FT_Library GetFTLibrary() override;

    static bool UseFcFontList() { return false; }

    virtual gfxImageFormat GetOffscreenFormat() override;

    bool SupportsApzWheelInput() const override {
      return true;
    }

    bool SupportsApzTouchInput() const override;

    void FontsPrefsChanged(const char *aPref) override;

    bool SupportsPluginDirectBitmapDrawing() override {
      return true;
    }

    bool AccelerateLayersByDefault() override {
      return false;
    }

#ifdef GL_PROVIDER_GLX
    already_AddRefed<mozilla::gfx::VsyncSource> CreateHardwareVsyncSource() override;
#endif

#ifdef MOZ_X11
    Display* GetCompositorDisplay() {
      return nullptr;
    }
#endif // MOZ_X11

protected:
    static gfxFontconfigUtils *sFontconfigUtils;

};

#endif /* GFX_PLATFORM_HEADLESS_H */
