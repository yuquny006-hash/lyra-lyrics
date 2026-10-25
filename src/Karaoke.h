#pragma once
#include "Lyrics.h"
#include <juce_graphics/juce_graphics.h>
#include <cmath>

namespace cue::karaoke {
inline double progress(double time, double start, double end) {
    if (!std::isfinite(time) || !std::isfinite(start) || !std::isfinite(end)) return 0;
    if (end <= start) return time >= start ? 1 : 0;
    return juce::jlimit(0.0, 1.0, (time - start) / (end - start));
}
// Colour tags partition the already shaped glyph stream. JUCE 8 shapes the full
// string using font ranges before applying colour ranges, preserving ligatures,
// fallback fonts, whitespace positions and native line breaking.
inline juce::Colour tag(size_t word) { return juce::Colour(0xff000000u | (juce::uint32)(word + 1)); }
inline void draw(juce::Graphics& g, const juce::TextLayout& layout,
                 juce::Rectangle<float> area, juce::Justification justification,
                 const std::vector<Word>& words, double time,
                 juce::Colour foreground, juce::Colour accent) {
    const auto origin = justification.appliedToRectangle(
        juce::Rectangle<float>(layout.getWidth(), layout.getHeight()), area).getPosition();
    std::vector<float> widths(words.size()), consumed(words.size());
    for (const auto& line : layout)
        for (const auto* run : line.runs) {
            const auto index = (size_t)((run->colour.getARGB() & 0xffffffu) - 1u);
            if (index < widths.size()) widths[index] += run->getRunBoundsX().getLength();
        }
    for (const auto& line : layout) for (const auto* run : line.runs) {
        const auto index = (size_t)((run->colour.getARGB() & 0xffffffu) - 1u);
        std::vector<juce::uint16> glyphs;
        std::vector<juce::Point<float>> positions;
        for (const auto& glyph : run->glyphs) {
            glyphs.push_back((juce::uint16)glyph.glyphCode);
            positions.push_back(glyph.anchor);
        }
        const auto transform = juce::AffineTransform::translation(origin + line.lineOrigin);
        const auto paint = [&](juce::Colour colour) {
            auto& context = g.getInternalContext();
            context.setFont(run->font);
            context.setFill(juce::FillType(colour));
            context.drawGlyphs(glyphs, positions, transform);
        };
        juce::Graphics::ScopedSaveState saved(g);
        paint(foreground);
        if (index >= words.size()) continue;
        const auto bounds = run->getRunBoundsX();
        const auto remaining = (float)progress(time, words[index].start, words[index].end) * widths[index] - consumed[index];
        consumed[index] += bounds.getLength();
        if (remaining <= 0) continue;
        if (remaining < bounds.getLength()) {
            // A floating point path keeps the sweep subpixel smooth, including
            // across a single sustained Chinese character.
            juce::Path clip;
            clip.addRectangle(area.getX() - 10000.f, area.getY() - 10000.f,
                origin.x + line.lineOrigin.x + bounds.getStart() + remaining - area.getX() + 10000.f,
                area.getHeight() + 20000.f);
            g.reduceClipRegion(clip);
        }
        paint(accent);
    }
}
}
