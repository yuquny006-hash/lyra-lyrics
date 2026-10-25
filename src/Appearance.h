#pragma once
#include "State.h"
namespace cue {
struct Palette { const char* zh; const char* en; juce::uint32 bg,panel,raised,ink,muted,accent,accent2,line,shadow; };
inline const Palette palettes[] = {
 {"钛银", "Titanium", 0xffe9edf1,0xfff4f6f8,0xffdde3e9,0xff222b38,0xff56657a,0xff315fd1,0xff0e9f8f,0xffcbd3dd,0x33202a3a},
 {"午夜蓝", "Midnight", 0xff141d30,0xff1b2740,0xff2a3854,0xffeef3fd,0xff9fb0cb,0xff8fb4ff,0xff5ad6c4,0xff36445f,0x55101830},
 {"松石", "Mineral",  0xff162826,0xff1f352f,0xff30483f,0xffecf4ec,0xffa6c0b3,0xff9bceb2,0xffe0b15a,0xff3b554b,0x55102018},
 {"暮紫", "Dusk",     0xff272333,0xff332d43,0xff473d58,0xfff4eefb,0xffc0b4d2,0xffc4a5ef,0xff6fd0c0,0xff574965,0x551a1428},
 {"黑曜", "Obsidian", 0xff181818,0xff232323,0xff353535,0xfff5f2eb,0xffb9b5ac,0xffe8ba72,0xff7fb6ff,0xff48443e,0x55000000}
};
inline const Palette& palette(int id){return palettes[juce::jlimit(0,4,id)];}
inline void applyPalette(Settings& s,int id){s.theme=juce::jlimit(0,4,id);auto& p=palette(s.theme);s.background=juce::Colour(p.bg).toString();s.foreground=juce::Colour(p.ink).toString();s.accent=juce::Colour(p.accent).toString();}
}
