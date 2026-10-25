#pragma once
#include "Processor.h"
#include "Alignment.h"

class AlignmentPanel : public juce::Component, private juce::Timer {
public:
 AlignmentPanel(CueProcessor&, std::function<void()> onClose);
 ~AlignmentPanel() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 juce::String tr(const char* zh,const char* en) const;
 void chooseAudio(); void start(); void apply(); void close(); void updateControls();
 void timerCallback() override;
 CueProcessor& processor_; std::function<void()> onClose_;
 cue::AlignmentJob job_; cue::State base_; uint64_t revision_=0;
 bool reviewed_=false; double progress_=0;
 juce::File audio_; std::unique_ptr<juce::FileChooser> chooser_;
 juce::Label title_,help_,audioPath_,lyricsLabel_,previewLabel_,status_,languageLabel_,modelLabel_;
 juce::TextEditor lyrics_,preview_;
 juce::ComboBox language_,model_;
 juce::TextButton choose_,start_,cancel_,apply_,back_;
 juce::ProgressBar progressBar_{progress_};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlignmentPanel)
};
