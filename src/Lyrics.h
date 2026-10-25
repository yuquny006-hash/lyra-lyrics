#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <optional>
inline juce::String U(const char* text) { return juce::String::fromUTF8(text); }
namespace cue {
struct Word { juce::String text; double start = 0, end = 0; };
struct Line { juce::String text; double start = 0, end = 0; std::vector<Word> words; juce::int64 id=0; bool marked=true; juce::String singer, colour; };
struct Document { juce::String title = U("未命名文稿"), format = "TXT"; std::vector<Line> lines; double offset = 0, songStart = 0; bool timed = false; };
struct ParseResult { std::optional<Document> document; juce::String error; explicit operator bool() const { return document.has_value(); } };
struct Position { int line = -1, word = -1; double progress = 0; bool active = false; };
ParseResult parseLyrics(const juce::MemoryBlock&, juce::String extension);
ParseResult parseText(const juce::String&, juce::String extension);
inline ParseResult parseText(const char* s,juce::String extension) { return parseText(U(s),extension); }
Position locate(const Document&, double seconds);
void normalise(Document&);
juce::String exportLrc(const Document&);
juce::String formatTime(double);
juce::MemoryBlock decodeQrc(const juce::MemoryBlock&, bool local);
}
