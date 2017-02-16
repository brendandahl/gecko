/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define PANGO_ENABLE_BACKEND
#define PANGO_ENABLE_ENGINE

#include "gfxPlatformHeadless.h"
#include "prenv.h"

#include "gfxFcPlatformFontList.h"
#include "gfxFontconfigUtils.h"
#include "gfxFontconfigFonts.h"
#include "gfxUserFontSet.h"
#include "gfxUtils.h"
#include "gfxPrefs.h"
#include "VsyncSource.h"
#include "mozilla/gfx/Logging.h"
#include "gfxImageSurface.h"

using namespace mozilla;
using namespace mozilla::gfx;

gfxFontconfigUtils *gfxPlatformHeadless::sFontconfigUtils = nullptr;

gfxPlatformHeadless::gfxPlatformHeadless()
{
    sFontconfigUtils = gfxFontconfigUtils::GetFontconfigUtils();
    uint32_t canvasMask = BackendTypeBit(BackendType::CAIRO);
    uint32_t contentMask = BackendTypeBit(BackendType::CAIRO);
#ifdef USE_SKIA
    canvasMask |= BackendTypeBit(BackendType::SKIA);
    contentMask |= BackendTypeBit(BackendType::SKIA);
#endif
    InitBackendPrefs(canvasMask, BackendType::CAIRO,
                     contentMask, BackendType::CAIRO);
}

gfxPlatformHeadless::~gfxPlatformHeadless()
{
    gfxFontconfigUtils::Shutdown();
    sFontconfigUtils = nullptr;
    gfxPangoFontGroup::Shutdown();
}

void
gfxPlatformHeadless::FlushContentDrawing()
{
}

already_AddRefed<gfxASurface>
gfxPlatformHeadless::CreateOffscreenSurface(const IntSize& aSize,
                                       gfxImageFormat aFormat)
{
    if (!Factory::AllowedSurfaceSize(aSize)) {
        return nullptr;
    }

    RefPtr<gfxASurface> newSurface;
    newSurface = new gfxImageSurface(aSize, aFormat);

    if (newSurface->CairoStatus()) {
        newSurface = nullptr; // surface isn't valid for some reason
    }

    if (newSurface) {
        gfxUtils::ClearThebesSurface(newSurface);
    }

    return newSurface.forget();
}

nsresult
gfxPlatformHeadless::GetFontList(nsIAtom *aLangGroup,
                            const nsACString& aGenericFamily,
                            nsTArray<nsString>& aListOfFonts)
{
    return sFontconfigUtils->GetFontList(aLangGroup,
                                         aGenericFamily,
                                         aListOfFonts);
}

nsresult
gfxPlatformHeadless::UpdateFontList()
{
    return sFontconfigUtils->UpdateFontList();
}

void
gfxPlatformHeadless::GetCommonFallbackFonts(uint32_t aCh, uint32_t aNextCh,
                                       Script aRunScript,
                                       nsTArray<const char*>& aFontList)
{
    // TODO: we may want to call into the platform dependent code here
}

gfxPlatformFontList*
gfxPlatformHeadless::CreatePlatformFontList()
{
    gfxPlatformFontList* list = new gfxFcPlatformFontList();
    if (NS_SUCCEEDED(list->InitFontList())) {
        return list;
    }
    gfxPlatformFontList::Shutdown();
    return nullptr;
}

nsresult
gfxPlatformHeadless::GetStandardFamilyName(const nsAString& aFontName, nsAString& aFamilyName)
{
    return sFontconfigUtils->GetStandardFamilyName(aFontName, aFamilyName);
}

gfxFontGroup *
gfxPlatformHeadless::CreateFontGroup(const FontFamilyList& aFontFamilyList,
                                const gfxFontStyle* aStyle,
                                gfxTextPerfMetrics* aTextPerf,
                                gfxUserFontSet* aUserFontSet,
                                gfxFloat aDevToCssSize)
{
    return new gfxPangoFontGroup(aFontFamilyList, aStyle,
                                 aUserFontSet, aDevToCssSize);
}

gfxFontEntry*
gfxPlatformHeadless::LookupLocalFont(const nsAString& aFontName,
                                uint16_t aWeight,
                                int16_t aStretch,
                                uint8_t aStyle)
{
    return gfxPangoFontGroup::NewFontEntry(aFontName, aWeight,
                                           aStretch, aStyle);
}

gfxFontEntry*
gfxPlatformHeadless::MakePlatformFont(const nsAString& aFontName,
                                 uint16_t aWeight,
                                 int16_t aStretch,
                                 uint8_t aStyle,
                                 const uint8_t* aFontData,
                                 uint32_t aLength)
{
    // passing ownership of the font data to the new font entry
    return gfxPangoFontGroup::NewFontEntry(aFontName, aWeight,
                                           aStretch, aStyle,
                                           aFontData, aLength);
}

FT_Library
gfxPlatformHeadless::GetFTLibrary()
{
    return gfxPangoFontGroup::GetFTLibrary();
}

bool
gfxPlatformHeadless::IsFontFormatSupported(nsIURI *aFontURI, uint32_t aFormatFlags)
{
    // check for strange format flags
    NS_ASSERTION(!(aFormatFlags & gfxUserFontSet::FLAG_FORMAT_NOT_USED),
                 "strange font format hint set");

    // accept supported formats
    // Pango doesn't apply features from AAT TrueType extensions.
    // Assume that if this is the only SFNT format specified,
    // then AAT extensions are required for complex script support.
    if (aFormatFlags & gfxUserFontSet::FLAG_FORMATS_COMMON) {
        return true;
    }

    // reject all other formats, known and unknown
    if (aFormatFlags != 0) {
        return false;
    }

    // no format hint set, need to look at data
    return true;
}

gfxImageFormat
gfxPlatformHeadless::GetOffscreenFormat()
{
    return SurfaceFormat::X8R8G8B8_UINT32;
}

void gfxPlatformHeadless::FontsPrefsChanged(const char *aPref)
{
}

already_AddRefed<ScaledFont>
gfxPlatformHeadless::GetScaledFontForFont(DrawTarget* aTarget, gfxFont *aFont)
{
    switch (aTarget->GetBackendType()) {
    case BackendType::CAIRO:
    case BackendType::SKIA:
        if (aFont->GetType() == gfxFont::FONT_TYPE_FONTCONFIG) {
            gfxFontconfigFontBase* fcFont = static_cast<gfxFontconfigFontBase*>(aFont);
            return Factory::CreateScaledFontForFontconfigFont(
                    fcFont->GetCairoScaledFont(),
                    fcFont->GetPattern(),
                    fcFont->GetAdjustedSize());
        }
        MOZ_FALLTHROUGH;
    default:
        return GetScaledFontForFontWithCairoSkia(aTarget, aFont);
    }
}

#ifdef GL_PROVIDER_GLX

already_AddRefed<gfx::VsyncSource>
gfxPlatformHeadless::CreateHardwareVsyncSource()
{
  return gfxPlatform::CreateHardwareVsyncSource();
}

bool
gfxPlatformHeadless::SupportsApzTouchInput() const
{
  int value = gfxPrefs::TouchEventsEnabled();
  return value == 1 || value == 2;
}

#endif
