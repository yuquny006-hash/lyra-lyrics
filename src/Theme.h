#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Appearance.h"
namespace style {
inline const juce::Colour bg{0xff182126},panel{0xff222e34},raised{0xff2c3a41},ink{0xffebf0ed},muted{0xffa5b4ba},accent{0xff6ed5c3},line{0xff35464f};
inline juce::Font font(float size,bool bold=false){return juce::Font(juce::FontOptions("Microsoft YaHei UI",size,bold?juce::Font::bold:juce::Font::plain));}
class Theme:public juce::LookAndFeel_V4 {
public:
 juce::Colour bg,panel,raised,ink,muted,accent,accent2,line,shadow;
 static constexpr float R=5.f, R_PANEL=8.f;
 Theme(){apply(0);}
 void apply(int id){auto& p=cue::palette(id);bg=juce::Colour(p.bg);panel=juce::Colour(p.panel);raised=juce::Colour(p.raised);ink=juce::Colour(p.ink);muted=juce::Colour(p.muted);accent=juce::Colour(p.accent);accent2=juce::Colour(p.accent2);line=juce::Colour(p.line);shadow=juce::Colour(p.shadow);
  setColour(juce::TextButton::buttonColourId,raised);
  setColour(juce::TextButton::textColourOffId,ink);
  setColour(juce::TextButton::buttonOnColourId,accent);
  setColour(juce::TextButton::textColourOnId,bg);
  setColour(juce::Label::textColourId,ink);
  setColour(juce::ComboBox::backgroundColourId,raised);
  setColour(juce::ComboBox::textColourId,ink);
  setColour(juce::ComboBox::outlineColourId,line);
  setColour(juce::PopupMenu::backgroundColourId,panel);
  setColour(juce::PopupMenu::textColourId,ink);
  setColour(juce::PopupMenu::highlightedBackgroundColourId,raised);
  setColour(juce::TextEditor::backgroundColourId,bg);
  setColour(juce::TextEditor::textColourId,ink);
  setColour(juce::TextEditor::highlightColourId,accent.withAlpha(.3f));
  setColour(juce::TextEditor::outlineColourId,line);
  setColour(juce::TextEditor::focusedOutlineColourId,accent);
  setColour(juce::Slider::thumbColourId,accent);
  setColour(juce::Slider::trackColourId,accent);
  setColour(juce::Slider::backgroundColourId,line);
  setColour(juce::Slider::textBoxTextColourId,ink);
  setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  setColour(juce::ListBox::backgroundColourId,panel);
  setColour(juce::ToggleButton::textColourId,ink);
  setColour(juce::ToggleButton::tickColourId,accent);
  setColour(juce::ToggleButton::tickDisabledColourId,muted);
  setColour(juce::ScrollBar::thumbColourId,muted.withAlpha(.6f));
 }
 juce::Font getTextButtonFont(juce::TextButton&,int)override{return font(16);}
 juce::Font getComboBoxFont(juce::ComboBox&)override{return font(16);}
 juce::Font getLabelFont(juce::Label&label)override{return label.getFont();}
 juce::Font getPopupMenuFont()override{return font(16);}
 void drawButtonBackground(juce::Graphics&g,juce::Button&b,const juce::Colour&c,bool over,bool down)override{
  auto r=b.getLocalBounds().toFloat().reduced(.5f);
  if(b.getProperties().getWithDefault("quiet",false)){
   if(over||down){g.setColour(raised);g.fillRoundedRectangle(r,R);}
   if(b.getProperties().getWithDefault("selected",false)){g.setColour(accent);g.fillRect(10.f,r.getBottom()-2.f,r.getWidth()-20.f,2.f);}
   if(b.hasKeyboardFocus(true)){g.setColour(accent);g.drawRoundedRectangle(r,R,1.5f);}return;
  }
  juce::Colour fill=c;
  if(!b.isEnabled()) fill=fill.withMultipliedAlpha(.4f);
  else if(down) fill=fill.brighter(.14f);
  else if(over) fill=fill.brighter(.07f);
  g.setColour(fill); g.fillRoundedRectangle(r,R);
  if(b.hasKeyboardFocus(true)&&b.isEnabled()){g.setColour(accent);g.drawRoundedRectangle(r,R,2.f);}
 }
 void drawComboBox(juce::Graphics&g,int w,int h,bool,int,int,int,int,juce::ComboBox&box)override{
  auto r=juce::Rectangle<float>(0,0,(float)w,(float)h).reduced(.5f);
  g.setColour(raised); g.fillRoundedRectangle(r,R);
  g.setColour(box.hasKeyboardFocus(true)?accent:line); g.drawRoundedRectangle(r,R,1.f);
  juce::Path arrow; arrow.startNewSubPath((float)w-18,(float)h/2-2); arrow.lineTo((float)w-13,(float)h/2+3); arrow.lineTo((float)w-8,(float)h/2-2);
  g.setColour(ink); g.strokePath(arrow,juce::PathStrokeType(1.4f));
 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int w,int h,float sliderPos,float,float,const juce::Slider::SliderStyle style,juce::Slider&s)override{
  if(style==juce::Slider::LinearBar||style==juce::Slider::LinearBarVertical){juce::LookAndFeel_V4::drawLinearSlider(g,x,y,w,h,sliderPos,0,0,style,s);return;}
  const float th=4.f; juce::Rectangle<float> track((float)x,(float)y+(float)h*.5f-th/2.f,(float)w,th);
  g.setColour(line); g.fillRoundedRectangle(track,th/2.f);
  float fw=juce::jmax(0.f,sliderPos-(float)x);
  if(fw>0.f){ g.setColour(accent); g.fillRoundedRectangle(track.withWidth(fw),th/2.f); }
  float tx=sliderPos, ty=(float)y+(float)h*.5f;
  g.setColour(accent); g.fillEllipse(juce::Rectangle<float>(tx-7,ty-7,14,14));
  g.setColour(bg); g.fillEllipse(juce::Rectangle<float>(tx-2.5f,ty-2.5f,5,5));
  if(s.hasKeyboardFocus(true)){ g.setColour(accent); g.drawEllipse(juce::Rectangle<float>(tx-9,ty-9,18,18),2.f); }
 }
 void drawScrollbar(juce::Graphics&g,juce::ScrollBar&bar,int x,int y,int w,int h,bool isVertical,int thumbStart,int thumbSize,bool,bool)override{
  juce::Rectangle<float> r;
  if(isVertical) r=juce::Rectangle<float>((float)x+((float)w-8)/2.f,(float)thumbStart,8.f,(float)thumbSize);
  else r=juce::Rectangle<float>((float)thumbStart,(float)y+((float)h-8)/2.f,(float)thumbSize,8.f);
  g.setColour(muted.withAlpha(.55f)); g.fillRoundedRectangle(r,4.f);
 }
 void drawTextEditorOutline(juce::Graphics&g,int w,int h,juce::TextEditor&te)override{
  auto r=juce::Rectangle<float>(0,0,(float)w,(float)h).reduced(.5f);
  g.setColour(te.hasKeyboardFocus(true)?accent:line); g.drawRoundedRectangle(r,6.f,1.f);
  if(te.hasKeyboardFocus(true)){ g.setColour(accent.withAlpha(.22f)); g.drawRoundedRectangle(r,6.f,2.f); }
 }
 void drawToggleButton(juce::Graphics&g,juce::ToggleButton&b,bool,bool)override{
  const float box=18.f; float hgt=(float)b.getHeight();
  juce::Rectangle<float> r(2.f,(hgt-box)/2.f,box,box);
  g.setColour(b.getToggleState()?accent:raised); g.fillRoundedRectangle(r,5.f);
  g.setColour(b.getToggleState()?bg:line); g.drawRoundedRectangle(r,5.f,1.f);
  if(b.getToggleState()){ juce::Path t; t.startNewSubPath(r.getX()+4.f,r.getCentreY()); t.lineTo(r.getX()+8.f,r.getBottom()-4.f); t.lineTo(r.getRight()-4.f,r.getY()+4.f); g.setColour(bg); g.strokePath(t,juce::PathStrokeType(2.f)); }
  g.setColour(b.isEnabled()?ink:muted); g.setFont(font(14)); g.drawText(b.getButtonText(),r.getRight()+8.f,0.f,b.getWidth()-box-12.f,hgt,juce::Justification::left);
 }
};
}
