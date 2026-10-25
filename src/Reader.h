#pragma once
#include "Processor.h"
#include "Theme.h"
#include "Localization.h"
class Reader:public juce::Component,public juce::SettableTooltipClient,private juce::Timer {
public:
 explicit Reader(CueProcessor& p);
 void paint(juce::Graphics&)override;
 void mouseUp(const juce::MouseEvent&)override;
 void mouseDown(const juce::MouseEvent&)override;
 void setReadingOptions(int layout,double scale,bool pure);
 int readingLayout()const{return readingLayout_;}
 double readingScale()const{return readingScale_;}
 bool isPure()const{return pure_;}
 void setRestoreControlsCallback(std::function<void()> f){restoreControls_=std::move(f);}
 void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&)override;
 bool keyPressed(const juce::KeyPress&)override;
 void setFullscreenCallback(std::function<void()> f){fullscreen_=std::move(f);}
private:
 juce::String tr(const char* text)const{return cue::translate(U(text),state_.settings.language);}
 void timerCallback()override;
 juce::TextLayout layout(const cue::Line&,double,bool,float);
 CueProcessor& processor_;cue::State state_;uint64_t revision_=~uint64_t{};std::function<void()>fullscreen_;
 int readingLayout_=-1;double readingScale_=1;bool pure_=false;std::function<void()>restoreControls_;
 std::vector<juce::TextLayout> article_;int articleWidth_=0;uint64_t articleRevision_=~uint64_t{};
 struct LyricHit {juce::Rectangle<float> bounds;int row;uint64_t revision;};
 std::vector<LyricHit> lyricHits_;int pressedRow_=-1;uint64_t pressedRevision_=~uint64_t{};bool pressedPure_=false;
 juce::String seekNotice_;double seekNoticeUntil_=0;
};
class ReaderWindow:public juce::DocumentWindow,private juce::Timer {
public:
 ReaderWindow(CueProcessor& p,juce::LookAndFeel* look=nullptr);
 ~ReaderWindow()override;
 void closeButtonPressed()override{setVisible(false);}
 void toggleFullscreen();
 bool isFullscreen()const{return full_;}
 Reader& reader(){return reader_;}
 void setReadingOptions(int layout,double scale,bool pure);
 bool transportEnabled()const{return progress_.isEnabled();}
 double progressValue()const{return progress_.getValue();}
 bool keyPressed(const juce::KeyPress&k)override{if(k==juce::KeyPress::escapeKey&&!reader_.isPure()&&full_){toggleFullscreen();return true;}return reader_.keyPressed(k);}
private:
 class View:public juce::Component{public:std::function<void()>onResize;void resized()override{if(onResize)onResize();}void paint(juce::Graphics&g)override{g.fillAll(getLookAndFeel().findColour(juce::ListBox::backgroundColourId));}};
 void layoutControls();void refreshControls();
 void timerCallback()override;
 CueProcessor& processor_;
 style::Theme theme_;
 bool full_=false;juce::Rectangle<int> normalBounds_;View view_;Reader reader_;
 juce::ComboBox layoutChoice_;int toolbarLanguage_=-1;
 juce::TextButton smaller_,larger_,reset_,pureButton_,play_,fullscreenButton_;
 juce::Slider progress_;juce::Label time_;
};
