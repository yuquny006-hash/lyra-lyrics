#pragma once
#include "Lyrics.h"
namespace cue {
struct Settings { double fontSize = 40, lineSpacing = 1.5, speed = 30; int context = 2, align = 1, theme = 0, language = 0; bool mirror = false; juce::String foreground = "ff222b38", background = "ffe9edf1", accent = "ff315fd1"; };
struct State { Document document; Settings settings; };
juce::String encodeState(const State&);
bool decodeState(const juce::String&, State&, juce::String& error);
bool shiftRange(Document&, int first, int last, double delta, double scale = 1.0);
void editLine(Document&, int index, juce::String text);
}
