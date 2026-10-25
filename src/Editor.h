#pragma once
#include "Processor.h"
#include "Reader.h"
#include "Localization.h"
#include "Guide.h"
#include "AlignmentPanel.h"
#include "ProjectionQR.h"
class CueEditor:public juce::AudioProcessorEditor,private juce::ListBoxModel,private juce::Timer,public juce::FileDragAndDropTarget {
public:
 explicit CueEditor(CueProcessor&);
 ~CueEditor()override;
 void paint(juce::Graphics&)override;void resized()override;
 bool isInterestedInFileDrag(const juce::StringArray&)override{return true;}
 void filesDropped(const juce::StringArray&,int,int)override;
 void importFile(const juce::File&);
 void saveProject(std::function<void(bool)> done={});
 bool keyPressed(const juce::KeyPress&)override;
private:
 juce::String tr(const char* text)const{return cue::translate(U(text),cached_.settings.language);}
 void updateAppearance();
 void setPure(bool);
 juce::TextButton recordTab_,editTab_,timeTab_,styleTab_,pureButton_,quickProjection_;
 juce::ComboBox readingView_;
 int workspace_=0;bool pure_=false;
 void showProjectionQR();
 std::unique_ptr<ProjectionQR> projectionQR_;
 juce::TextButton projectionScan_;
 void updateTransport();
 void timerCallback()override;
 int getNumRows()override;void paintListBoxItem(int,juce::Graphics&,int,int,bool)override;
 void selectedRowsChanged(int)override;void listBoxItemDoubleClicked(int,const juce::MouseEvent&)override;
 void listBoxItemClicked(int,const juce::MouseEvent&)override;
 juce::String getTooltipForRow(int)override;
 void refresh();void chooseImport();void exportLyrics();void openTextEditor();void stamp();void stampWord();void modify(std::function<void(cue::State&)>);
 void setupSlider(juce::Slider&,double,double,double,const juce::String&);
 CueProcessor& processor_;style::Theme theme_;Reader reader_;
 juce::ListBox list_;juce::Label title_,subtitle_,status_,clock_,listTitle_,settingsTitle_,lineHelp_;
 juce::TextButton import_,save_,export_,edit_,undo_,redo_,readerButton_,play_,home_,stamp_,wordStamp_,add_,remove_,applyLine_,shift_,reset_,full_,previous_,next_;
 struct SettingsSurface:juce::Component {
  std::function<void(juce::Graphics&)> preview;
  void paint(juce::Graphics&g)override{
   auto& laf=getLookAndFeel();
   g.fillAll(laf.findColour(juce::TextEditor::backgroundColourId));
   g.setColour(laf.findColour(juce::ListBox::backgroundColourId));
   g.fillRect(0,0,196,getHeight());
   if(preview)preview(g);
  }
 } settingsPage_;
 juce::TextButton settingsButton_,backButton_,copyContact_;
 juce::TextButton generalNav_,projectionNav_,helpNav_,aboutNav_;
 int settingsSection_=0;
 juce::Label appearanceTitle_,themeLabel_,languageLabel_,guideTitle_,contact_;
 juce::ComboBox themeChoice_,languageChoice_,helpTopic_;
 juce::StringArray helpSections_;
 juce::TextEditor guide_;
 int appliedTheme_=-1,appliedLanguage_=-1;
 std::unique_ptr<AlignmentPanel> alignmentPanel_;
 juce::TextButton ktvButton_;
 juce::TextButton originSet_,originReset_,projectionToggle_,projectionCopy_,applySinger_;
 juce::TextEditor originValue_,projectionAddress_,singerText_;
 juce::ComboBox singerColour_;
 juce::Label originLabel_,projectionLabel_;
 juce::ComboBox mode_,alignment_,palette_;
 juce::ToggleButton mirror_;
 juce::Slider offset_,font_,spacing_,context_,speed_,seek_,scale_;
 juce::TextEditor lineText_,lineStart_,shiftValue_;
 std::unique_ptr<juce::FileChooser> chooser_;

 cue::State cached_;uint64_t revision_=~uint64_t{};int selected_=-1,wordIndex_=0;bool refreshing_=false,stamping_=false;
 juce::File projectFile_;juce::String notice_;double noticeUntil_=0;
 void notify(const juce::String& text);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueEditor)
};
