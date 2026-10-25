#include "Editor.h"
class CueApplication:public juce::JUCEApplication {
public:
 const juce::String getApplicationName()override{return "CueFlow";}
 const juce::String getApplicationVersion()override{return "0.5.4";}
 bool moreThanOneInstanceAllowed()override{return true;}
 void initialise(const juce::String&cmd)override{
  processor_=std::make_unique<CueProcessor>();
  auto preferences=juce::JSON::parse(appearanceFile().loadFileAsString());auto initial=processor_->state();
  if(preferences["theme"].isInt()&&(int)preferences["theme"]>=0&&(int)preferences["theme"]<5)cue::applyPalette(initial.settings,(int)preferences["theme"]);
  if(preferences["language"].isInt()&&(int)preferences["language"]>=0&&(int)preferences["language"]<2)initial.settings.language=(int)preferences["language"];
  processor_->commit(initial);processor_->markSaved();processor_->setMode(CueProcessor::Mode::preview);window_=std::make_unique<Window>(*processor_,[this]{systemRequestedQuit();});
  auto file=juce::File(cmd.unquoted());if(cmd.isNotEmpty()&&file.existsAsFile())window_->editor->importFile(file);
 }
 void shutdown()override{if(processor_){auto state=processor_->state();juce::var preferences(new juce::DynamicObject());preferences.getDynamicObject()->setProperty("theme",state.settings.theme);preferences.getDynamicObject()->setProperty("language",state.settings.language);auto file=appearanceFile();file.getParentDirectory().createDirectory();juce::TemporaryFile temporary(file);if(temporary.getFile().replaceWithText(juce::JSON::toString(preferences)))temporary.overwriteTargetFileWithTemporary();}window_.reset();processor_.reset();}
 void systemRequestedQuit()override{
  if(closing_)return;if(!processor_->dirty()){quit();return;}closing_=true;
  auto tr=[this](const char* text){return cue::translate(U(text),processor_->state().settings.language);};
  juce::AlertWindow::showYesNoCancelBox(juce::MessageBoxIconType::QuestionIcon,tr("保存当前文稿？"),tr("当前项目有未保存的修改。"),tr("保存并退出"),tr("不保存"),tr("取消"),nullptr,juce::ModalCallbackFunction::create([this](int choice){closing_=false;if(choice==1)window_->editor->saveProject([this](bool ok){if(ok)quit();});else if(choice==2)quit();}));
 }
private:
 static juce::File appearanceFile(){return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("AmuMusic/CueFlow/appearance.json");}
 struct Window:juce::DocumentWindow,private juce::Timer {
  CueEditor*editor;std::function<void()>close;CueProcessor& processor;
  Window(CueProcessor&p,std::function<void()>fn):DocumentWindow(U("Lyra 录音歌词跟随系统"),style::bg,DocumentWindow::allButtons),close(std::move(fn)),processor(p){setUsingNativeTitleBar(true);editor=new CueEditor(p);setContentOwned(editor,true);setResizable(true,true);setResizeLimits(1180,780,1800,1200);centreWithSize(1280,840);setVisible(true);startTimer(500);}
  void timerCallback()override{setName(cue::translate(U("Lyra 录音歌词跟随系统"),processor.state().settings.language));}
  void closeButtonPressed()override{close();}
 };
 std::unique_ptr<CueProcessor>processor_;std::unique_ptr<Window>window_;bool closing_=false;
};
START_JUCE_APPLICATION(CueApplication)
