#include "AlignmentPanel.h"
#include "Theme.h"

juce::String AlignmentPanel::tr(const char* zh,const char* en) const { return U(processor_.state().settings.language ? en : zh); }

AlignmentPanel::AlignmentPanel(CueProcessor& p,std::function<void()> onClose):processor_(p),onClose_(std::move(onClose)) {
 for(auto* c:std::initializer_list<juce::Component*>{&title_,&help_,&audioPath_,&lyricsLabel_,&previewLabel_,&status_,&languageLabel_,&modelLabel_,&lyrics_,&preview_,&language_,&model_,&choose_,&start_,&cancel_,&apply_,&back_,&progressBar_}) addAndMakeVisible(c);
 title_.setText(tr("自动 KTV 对齐","Automatic KTV alignment"),juce::dontSendNotification); title_.setFont(style::font(26,true));
 help_.setText(tr("选择参考人声，核对实际演唱的歌词，再生成逐字时间。已预先移除标题、括号段落名和前奏/间奏/尾奏；请检查并补回需要演唱的内容。处理在本机进行，应用前不会修改文稿。","Select reference vocals and check the lyrics actually sung. Titles, bracket headings and instrumental labels were filtered; restore any sung content. Processing is local; the script changes only when you apply."),juce::dontSendNotification);
 help_.setMinimumHorizontalScale(1.f);
 choose_.setButtonText(tr("选择参考人声…","Choose reference vocals…")); start_.setButtonText(tr("开始对齐","Start alignment")); cancel_.setButtonText(tr("取消处理","Cancel processing")); apply_.setButtonText(tr("应用逐字时间","Apply word timing")); back_.setButtonText(tr("返回工作台","Back to studio"));
 lyricsLabel_.setText(tr("实际演唱的歌词 · 每句一行","Sung lyrics · one line per phrase"),juce::dontSendNotification);
 previewLabel_.setText(tr("结果预览与提示","Results and warnings"),juce::dontSendNotification);
 languageLabel_.setText(tr("演唱语言","Sung language"),juce::dontSendNotification); modelLabel_.setText(tr("本地模型","Local model"),juce::dontSendNotification);
 language_.addItem(tr("中文","Chinese"),1); language_.addItem(tr("英文","English"),2); language_.setSelectedId(1);
 model_.addItem(tr("small · 推荐","small · recommended"),1); model_.addItem(tr("base · 更快","base · faster"),2); model_.setSelectedId(1);
 for(auto* e:{&lyrics_,&preview_}) { e->setMultiLine(true); e->setReturnKeyStartsNewLine(true); e->setScrollbarsShown(true); e->setFont(style::font(17)); }
 preview_.setReadOnly(true);
 const auto ink=juce::Colour(cue::palette(p.state().settings.theme).ink);
 for(auto* e:{&lyrics_,&preview_}){e->setColour(juce::TextEditor::textColourId,ink);e->applyColourToAllText(ink,true);}
 auto s=p.state(); juce::StringArray lines; size_t index=0;
 for(const auto& line:s.document.lines) {
  auto t=line.text.trim(); const bool first=index++==0;
  const bool bracket=(t.startsWith("[")&&t.endsWith("]"))||(t.startsWith(U("【"))&&t.endsWith(U("】")))||(t.startsWith("(")&&t.endsWith(")"))||(t.startsWith(U("（"))&&t.endsWith(U("）")));
  if(t.isEmpty()||bracket||t==U("前奏")||t==U("间奏")||t==U("尾奏")||(first&&s.document.title.isNotEmpty()&&t.startsWith(s.document.title))) continue;
  lines.add(line.text);
 }
 lyrics_.setText(lines.joinIntoString("\n"));
 audioPath_.setText(tr("尚未选择音频","No audio selected"),juce::dontSendNotification);
 status_.setText(tr("需要已配置的本地 stable-ts 和模型。时间相对于所选音频起点。","Requires configured local stable-ts and model. Timing is relative to the selected audio start."),juce::dontSendNotification);
 choose_.onClick=[this]{chooseAudio();}; start_.onClick=[this]{start();}; apply_.onClick=[this]{apply();}; back_.onClick=[this]{close();};
 cancel_.onClick=[this]{stopTimer();job_.cancel();reviewed_=false;progress_=0;status_.setText(tr("已取消，原文稿未修改","Cancelled; original script unchanged"),juce::dontSendNotification);updateControls();};
 auto invalidate=[this]{reviewed_=false;updateControls();}; lyrics_.onTextChange=invalidate; language_.onChange=invalidate; model_.onChange=invalidate;
 updateControls();
}
AlignmentPanel::~AlignmentPanel(){stopTimer();chooser_.reset();job_.cancel();}
void AlignmentPanel::paint(juce::Graphics& g){g.fillAll(getLookAndFeel().findColour(juce::ListBox::backgroundColourId));}
void AlignmentPanel::resized(){
 auto r=getLocalBounds().reduced(28); auto top=r.removeFromTop(42);back_.setBounds(top.removeFromRight(180));title_.setBounds(top);r.removeFromTop(8);help_.setBounds(r.removeFromTop(65));r.removeFromTop(12);
 auto audioRow=r.removeFromTop(38);choose_.setBounds(audioRow.removeFromLeft(250));audioRow.removeFromLeft(12);audioPath_.setBounds(audioRow);r.removeFromTop(14);
 auto options=r.removeFromTop(36);languageLabel_.setBounds(options.removeFromLeft(110));language_.setBounds(options.removeFromLeft(160));options.removeFromLeft(28);modelLabel_.setBounds(options.removeFromLeft(110));model_.setBounds(options.removeFromLeft(230));r.removeFromTop(14);
 auto buttons=r.removeFromBottom(40);apply_.setBounds(buttons.removeFromRight(200));buttons.removeFromRight(12);cancel_.setBounds(buttons.removeFromRight(170));start_.setBounds(buttons.removeFromLeft(200));r.removeFromBottom(10);status_.setBounds(r.removeFromBottom(44));progressBar_.setBounds(r.removeFromBottom(16));r.removeFromBottom(14);
 auto left=r.removeFromLeft((r.getWidth()-20)/2);r.removeFromLeft(20);lyricsLabel_.setBounds(left.removeFromTop(30));lyrics_.setBounds(left);previewLabel_.setBounds(r.removeFromTop(30));preview_.setBounds(r);
}
void AlignmentPanel::updateControls(){const bool busy=job_.running();choose_.setEnabled(!busy&&!chooser_);lyrics_.setReadOnly(busy);language_.setEnabled(!busy);model_.setEnabled(!busy);start_.setEnabled(!busy&&audio_.existsAsFile()&&lyrics_.getText().trim().isNotEmpty());cancel_.setEnabled(busy);apply_.setEnabled(!busy&&reviewed_);}
void AlignmentPanel::chooseAudio(){
 chooser_=std::make_unique<juce::FileChooser>(tr("选择参考人声音频","Select reference vocal audio"),audio_,"*.wav;*.flac;*.mp3;*.ogg;*.aif;*.aiff",true);
 juce::Component::SafePointer<AlignmentPanel> safe(this);
 chooser_->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser& chooser){if(!safe)return;auto selected=chooser.getResult();if(selected.existsAsFile()){safe->audio_=selected;safe->audioPath_.setText(selected.getFullPathName(),juce::dontSendNotification);safe->reviewed_=false;}safe->chooser_.reset();safe->updateControls();});updateControls();
}
void AlignmentPanel::start(){
 auto snapshot=processor_.snapshot();base_=std::move(snapshot.first);revision_=snapshot.second;reviewed_=false;preview_.clear();juce::String error;
 if(!job_.start(audio_,lyrics_.getText(),language_.getSelectedId()==2?"en":"zh",model_.getSelectedId()==2?"base":"small",error)){status_.setText(error,juce::dontSendNotification);updateControls();return;}
 progress_=-1;status_.setText(tr("正在本机对齐；长音频可能需要数分钟…","Aligning locally; long audio may take several minutes…"),juce::dontSendNotification);updateControls();startTimer(250);
}
void AlignmentPanel::timerCallback(){
 job_.poll();progress_=job_.progress();status_.setText(job_.message(),juce::dontSendNotification);
 if(job_.running())return;
 stopTimer();reviewed_=false;
 if(job_.finished()){
  cue::State candidate;juce::String error;auto result=job_.result();
  if(cue::alignedState(base_,result,candidate,error)){
   juce::String text;auto warningValues=result.getProperty("warnings",{});if(auto* warnings=warningValues.getArray())for(const auto& w:*warnings)text+=tr("提示：","Warning: ")+w.toString()+"\n";
   text+="\n";for(const auto& line:candidate.document.lines){text+=cue::formatTime(line.start)+" – "+cue::formatTime(line.end)+"  "+line.text+"\n";for(const auto& word:line.words)text+="  "+cue::formatTime(word.start)+"  "+word.text+"\n";}
   preview_.setText(text);reviewed_=true;progress_=1;status_.setText(tr("请核对逐字时间；点击应用后替换当前歌词，可撤销。","Review word timings. Apply replaces the lyrics and can be undone."),juce::dontSendNotification);
  }else status_.setText(error,juce::dontSendNotification);
 }
 updateControls();
}
void AlignmentPanel::apply(){
 if(!reviewed_||job_.running())return;
 if(processor_.revision()!=revision_){reviewed_=false;status_.setText(tr("文稿或设置已改变。请重新对齐，避免覆盖新的修改。","Script or settings changed. Run alignment again to avoid overwriting newer changes."),juce::dontSendNotification);updateControls();return;}
 cue::State output;juce::String error;
 if(!cue::alignedState(base_,job_.result(),output,error)||!processor_.commitIfRevision(output,revision_)){status_.setText(error.isNotEmpty()?error:processor_.lastError(),juce::dontSendNotification);return;}
 processor_.setMode(processor_.mode()==CueProcessor::Mode::host||processor_.transport().valid?CueProcessor::Mode::host:CueProcessor::Mode::preview);close();
}
void AlignmentPanel::close(){stopTimer();job_.cancel();auto callback=onClose_;if(callback)callback();}
