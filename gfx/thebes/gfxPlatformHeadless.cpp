/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define PANGO_ENABLE_BACKEND
#define PANGO_ENABLE_ENGINE

#include "gfxPlatformHeadless.h"
#include "prenv.h"

#include "nsUnicharUtils.h"
#include "nsUnicodeProperties.h"
#include "gfx2DGlue.h"
#include "gfxFcPlatformFontList.h"
#include "gfxFontconfigUtils.h"
#include "gfxFontconfigFonts.h"
#include "gfxConfig.h"
#include "gfxContext.h"
#include "gfxUserFontSet.h"
#include "gfxUtils.h"
#include "gfxFT2FontBase.h"
#include "gfxPrefs.h"
#include "VsyncSource.h"
#include "mozilla/Atomics.h"
#include "mozilla/Monitor.h"
#include "base/task.h"
#include "base/thread.h"
#include "base/message_loop.h"
#include "mozilla/gfx/Logging.h"

#include "mozilla/gfx/2D.h"

#include "cairo.h"

#include "gfxImageSurface.h"

#include <fontconfig/fontconfig.h>

#include "nsMathUtils.h"

#define GDK_PIXMAP_SIZE_MAX 32767

#define GFX_PREF_MAX_GENERIC_SUBSTITUTIONS "gfx.font_rendering.fontconfig.max_generic_substitutions"

using namespace mozilla;
using namespace mozilla::gfx;
using namespace mozilla::unicode;

gfxFontconfigUtils *gfxPlatformHeadless::sFontconfigUtils = nullptr;

gfxPlatformHeadless::gfxPlatformHeadless()
{
    mMaxGenericSubstitutions = UNINITIALIZED_VALUE;

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

// xxx - this is ubuntu centric, need to go through other distros and flesh
// out a more general list
static const char kFontDejaVuSans[] = "DejaVu Sans";
static const char kFontDejaVuSerif[] = "DejaVu Serif";
static const char kFontEmojiOneMozilla[] = "EmojiOne Mozilla";
static const char kFontFreeSans[] = "FreeSans";
static const char kFontFreeSerif[] = "FreeSerif";
static const char kFontTakaoPGothic[] = "TakaoPGothic";
static const char kFontDroidSansFallback[] = "Droid Sans Fallback";
static const char kFontWenQuanYiMicroHei[] = "WenQuanYi Micro Hei";
static const char kFontNanumGothic[] = "NanumGothic";

void
gfxPlatformHeadless::GetCommonFallbackFonts(uint32_t aCh, uint32_t aNextCh,
                                       Script aRunScript,
                                       nsTArray<const char*>& aFontList)
{
    if (aNextCh == 0xfe0fu) {
      // if char is followed by VS16, try for a color emoji glyph
      aFontList.AppendElement(kFontEmojiOneMozilla);
    }

    aFontList.AppendElement(kFontDejaVuSerif);
    aFontList.AppendElement(kFontFreeSerif);
    aFontList.AppendElement(kFontDejaVuSans);
    aFontList.AppendElement(kFontFreeSans);

    if (!IS_IN_BMP(aCh)) {
        uint32_t p = aCh >> 16;
        if (p == 1) { // try color emoji font, unless VS15 (text style) present
            if (aNextCh != 0xfe0fu && aNextCh != 0xfe0eu) {
                aFontList.AppendElement(kFontEmojiOneMozilla);
            }
        }
    }

    // add fonts for CJK ranges
    // xxx - this isn't really correct, should use the same CJK font ordering
    // as the pref font code
    if (aCh >= 0x3000 &&
        ((aCh < 0xe000) ||
         (aCh >= 0xf900 && aCh < 0xfff0) ||
         ((aCh >> 16) == 2))) {
        aFontList.AppendElement(kFontTakaoPGothic);
        aFontList.AppendElement(kFontDroidSansFallback);
        aFontList.AppendElement(kFontWenQuanYiMicroHei);
        aFontList.AppendElement(kFontNanumGothic);
    }
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
    // only checking for generic substitions, pass other changes up
    if (strcmp(GFX_PREF_MAX_GENERIC_SUBSTITUTIONS, aPref)) {
        gfxPlatform::FontsPrefsChanged(aPref);
        return;
    }

    mMaxGenericSubstitutions = UNINITIALIZED_VALUE;
}

void
gfxPlatformHeadless::GetPlatformCMSOutputProfile(void *&mem, size_t &size)
{
    mem = nullptr;
    size = 0;
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
