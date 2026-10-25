#include "Reader.h"
#include "Karaoke.h"
Reader::Reader(CueProcessor&p):processor_(p){setWantsKeyboardFocus(true);setTitle(tr("提词阅读区"));startTimerHz(60);}
void Reader::timerCallback(){if(revision_!=processor_.revision()){auto snapshot=processor_.snapshot();state_=std::move(snapshot.first);revision_=snapshot.second;}repaint();}
juce::TextLayout Reader::layout(const cue::Line&l,double /*time*/,bool current,float width){juce::AttributedString a;const double density=readingLayout_==3&&!current?.82:1.0;auto font=style::font((float)(state_.settings.fontSize*readingScale_*density),current);auto fg=juce::Colour::fromString(state_.settings.foreground);if(l.colour.isNotEmpty())fg=juce::Colour::fromString(l.colour);a.setJustification(state_.settings.align==0?juce::Justification::left:state_.settings.align==2?juce::Justification::right:juce::Justification::centred);a.setLineSpacing((float)(state_.settings.fontSize*readingScale_*(state_.settings.lineSpacing-1)*.35));
 if(l.words.empty()||!current||processor_.mode()==CueProcessor::Mode::article)a.append(l.text,font,current?fg:fg.withAlpha(.48f));
 else for(size_t i=0;i<l.words.size();++i)a.append(l.words[i].text,font,cue::karaoke::tag(i));
 juce::TextLayout result;result.createLayout(a,std::max(50.f,width));return result;
}
void Reader::paint(juce::Graphics&g){
 if(revision_!=processor_.revision()){auto snapshot=processor_.snapshot();state_=std::move(snapshot.first);revision_=snapshot.second;}
 lyricHits_.clear();
 setTooltip(juce::Time::getMillisecondCounterHiRes()<seekNoticeUntil_?seekNotice_:U(state_.settings.language?(pure_?"Tap to restore controls":"Click a timed lyric to seek"):(pure_?"轻点恢复控件":"点击已标时歌词跳转")));
 auto area=getLocalBounds().toFloat();auto bg=juce::Colour::fromString(state_.settings.background),ac=juce::Colour::fromString(state_.settings.accent);g.fillAll(bg);
 if(state_.settings.mirror)g.addTransform(juce::AffineTransform::scale(-1,1).translated((float)getWidth(),0));
 auto&d=state_.document;auto content=area.reduced(26,46);float width=content.getWidth();
 if(d.lines.empty()){g.setColour(juce::Colour::fromString(state_.settings.foreground).withAlpha(.7f));g.setFont(style::font(24));g.drawFittedText(tr("把下一句话，留在眼前"),content.toNearestInt().withHeight(60).withY(getHeight()/2-50),juce::Justification::centred,2);g.setFont(style::font(14));g.drawFittedText(tr("导入歌词，或粘贴你的文稿\n支持 LRC · KRC · QRC · YRC · TXT"),content.toNearestInt().withHeight(70).withY(getHeight()/2+12),juce::Justification::centred,3);return;}
 double time=processor_.displayTime();
 if(!d.timed&&processor_.mode()!=CueProcessor::Mode::article){g.setColour(ac);g.setFont(style::font(20));g.drawFittedText(tr("歌词尚未标时：导入带时间歌词，或跟随伴奏逐句打点"),content.toNearestInt(),juce::Justification::centred,4);return;}
 if(processor_.mode()==CueProcessor::Mode::article){
  if(articleWidth_!=(int)width||articleRevision_!=revision_){article_.clear();for(auto&l:d.lines)article_.push_back(layout(l,-1,true,width));articleWidth_=(int)width;articleRevision_=revision_;}
  float total=0;for(auto&l:article_)total+=l.getHeight()+(float)(state_.settings.fontSize*readingScale_)*(float)state_.settings.lineSpacing;
  float scroll=(float)(time*state_.settings.speed);float maxScroll=std::max(0.f,total-content.getHeight()*.5f);scroll=juce::jlimit(0.f,maxScroll,scroll);
  float y=content.getY()+content.getHeight()*.35f-scroll;
  for(auto&l:article_){if(y+l.getHeight()>0&&y<getHeight())l.draw(g,{content.getX(),y,width,l.getHeight()});y+=l.getHeight()+(float)(state_.settings.fontSize*readingScale_)*(float)state_.settings.lineSpacing;}
  if(!pure_){g.setColour(ac.withAlpha(.45f));g.fillRect(8.f,content.getY()+content.getHeight()*.35f,3.f,(float)(state_.settings.fontSize*readingScale_));}
 }else{
  auto pos=cue::locate(d,time);int current=std::max(0,pos.line);int groupEnd=current;while(groupEnd+1<(int)d.lines.size()&&d.lines[(size_t)groupEnd+1].start==d.lines[(size_t)current].start)++groupEnd;
  if(!pure_&&d.lines[(size_t)current].singer.isNotEmpty())content=content.withTop(std::max(content.getY(),70.f));
  int start=std::max(0,current-state_.settings.context),end=std::min((int)d.lines.size()-1,groupEnd+state_.settings.context);
  if(readingLayout_==3){start=std::max(0,current-5);end=std::min((int)d.lines.size()-1,groupEnd+5);while(start>0&&d.lines[(size_t)start-1].start==d.lines[(size_t)start].start)--start;while(end+1<(int)d.lines.size()&&d.lines[(size_t)end+1].start==d.lines[(size_t)end].start)++end;}
  else if(readingLayout_>=0){start=current;end=groupEnd;
   if(readingLayout_==2&&current>0){start=current-1;while(start>0&&d.lines[(size_t)start-1].start==d.lines[(size_t)start].start)--start;}
   if(readingLayout_>=1&&groupEnd+1<(int)d.lines.size()){end=groupEnd+1;while(end+1<(int)d.lines.size()&&d.lines[(size_t)end+1].start==d.lines[(size_t)end].start)++end;}
  }std::vector<juce::TextLayout> rows;float before=0,currentHeight=0;const float gap=readingLayout_==3?10.f:20.f;
  for(int i=start;i<=end;++i){auto l=layout(d.lines[(size_t)i],time-d.offset-d.songStart,i>=current&&i<=groupEnd,width);float h=l.getHeight()+gap;if(i<current)before+=h;else if(i<=groupEnd)currentHeight+=h;rows.push_back(std::move(l));}
  float y=content.getCentreY()-currentHeight*.5f-before;
  if(readingLayout_==3&&start==0)y=std::min(y,content.getY());
  const auto& singer=d.lines[(size_t)current].singer;if(!pure_&&singer.isNotEmpty()){g.setColour(ac);g.setFont(style::font(16,true));g.drawText(singer,26,40,(int)width,22,juce::Justification::centred);}
  {juce::Graphics::ScopedSaveState clipped(g);g.reduceClipRegion(content.toNearestInt());for(size_t r=0;r<rows.size();++r){auto&l=rows[r];const int index=start+(int)r;const auto&line=d.lines[(size_t)index];auto bounds=juce::Rectangle<float>(content.getX(),y,width,l.getHeight());if(line.marked){auto hit=bounds.getIntersection(content).getIntersection(area);if(!hit.isEmpty()){if(state_.settings.mirror)hit.setX((float)getWidth()-hit.getRight());lyricHits_.push_back({hit,index,revision_});}}if(index>=current&&index<=groupEnd&&!line.words.empty()){auto fg=juce::Colour::fromString(line.colour.isNotEmpty()?line.colour:state_.settings.foreground);cue::karaoke::draw(g,l,bounds,state_.settings.align==0?juce::Justification::left:state_.settings.align==2?juce::Justification::right:juce::Justification::centred,line.words,time-d.offset-d.songStart,fg,ac);}else l.draw(g,bounds);y+=l.getHeight()+gap;}}
  if(!pure_){g.setColour(ac);g.setFont(style::font(12));auto hint=pos.line<0?tr("准备进入第一句"):!pos.active?(current==(int)d.lines.size()-1?tr("文稿结束"):tr("间奏 · 等待下一句")):tr("当前句 ")+juce::String(current+1)+" / "+juce::String(d.lines.size());if(juce::Time::getMillisecondCounterHiRes()<seekNoticeUntil_)hint=seekNotice_;g.drawText(hint,20,14,getWidth()-40,22,juce::Justification::centred);
  const auto&line=d.lines[(size_t)current];double progress=juce::jlimit(0.0,1.0,(time-d.offset-d.songStart-line.start)/std::max(.001,line.end-line.start));g.setColour(ac.withAlpha(.15f));g.fillRoundedRectangle(26.f,getHeight()-24.f,width,3,1.5f);g.setColour(ac);g.fillRoundedRectangle(26.f,getHeight()-24.f,width*(float)progress,3,1.5f);}
 }
}
void Reader::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&w){if(processor_.mode()==CueProcessor::Mode::article)processor_.seek(std::max(0.0,processor_.displayTime()-w.deltaY*120/state_.settings.speed));}
bool Reader::keyPressed(const juce::KeyPress&k){if(k==juce::KeyPress::escapeKey&&pure_){if(restoreControls_)restoreControls_();else setReadingOptions(readingLayout_,readingScale_,false);return true;}if(k==juce::KeyPress::F11Key){if(fullscreen_)fullscreen_();return true;}if(processor_.mode()==CueProcessor::Mode::host)return false;if(k==juce::KeyPress::spaceKey){processor_.togglePlay();return true;}if(k==juce::KeyPress::homeKey){processor_.seek(0);return true;}if(k==juce::KeyPress::pageDownKey||k==juce::KeyPress::pageUpKey){double step=(getHeight()*.65)/state_.settings.speed;processor_.seek(std::max(0.0,processor_.displayTime()+(k==juce::KeyPress::pageDownKey?step:-step)));return true;}return false;}
void Reader::setReadingOptions(int mode,double scale,bool pure){readingLayout_=juce::jlimit(-1,3,mode);double nextScale=std::isfinite(scale)?juce::jlimit(.5,3.0,scale):1.0;if(nextScale!=readingScale_)articleWidth_=-1;readingScale_=nextScale;pure_=pure;repaint();}
void Reader::mouseDown(const juce::MouseEvent&e){
 pressedRow_=-1;pressedPure_=pure_;
 if(!e.mods.isLeftButtonDown()||pure_)return;
 for(const auto&hit:lyricHits_)if(hit.bounds.contains(e.position)){pressedRow_=hit.row;pressedRevision_=hit.revision;break;}
}
void Reader::mouseUp(const juce::MouseEvent&e){
 const int row=pressedRow_;pressedRow_=-1;
 if(!e.mods.isLeftButtonDown()||e.mouseWasDraggedSinceMouseDown())return;
 if(pressedPure_){if(pure_){if(restoreControls_)restoreControls_();else setReadingOptions(readingLayout_,readingScale_,false);}return;}
 if(pure_||row<0)return;
 if(!processor_.seekLine(row,pressedRevision_)){seekNotice_=processor_.seekStatus();seekNoticeUntil_=juce::Time::getMillisecondCounterHiRes()+7000;setTooltip(seekNotice_);}else{seekNotice_.clear();seekNoticeUntil_=0;}
 repaint();
}
ReaderWindow::ReaderWindow(CueProcessor&p,juce::LookAndFeel*):DocumentWindow(U("Lyra · 阅读窗口"),style::bg,DocumentWindow::allButtons),processor_(p),reader_(p){
 theme_.apply(p.state().settings.theme);setLookAndFeel(&theme_);setUsingNativeTitleBar(true);setResizable(true,true);setResizeLimits(760,400,10000,10000);
 setContentNonOwned(&view_,false);view_.addAndMakeVisible(reader_);view_.onResize=[this]{layoutControls();};
 for(auto*b:{&smaller_,&larger_,&reset_,&pureButton_,&play_,&fullscreenButton_})view_.addAndMakeVisible(b);
 view_.addAndMakeVisible(progress_);view_.addAndMakeVisible(time_);time_.setFont(style::font(14));
 view_.addAndMakeVisible(layoutChoice_);layoutChoice_.setComponentID("reader-layout");layoutChoice_.onChange=[this]{setReadingOptions(layoutChoice_.getSelectedId()-2,reader_.readingScale(),false);};
 smaller_.onClick=[this]{setReadingOptions(reader_.readingLayout(),reader_.readingScale()-.1,false);};larger_.onClick=[this]{setReadingOptions(reader_.readingLayout(),reader_.readingScale()+.1,false);};reset_.onClick=[this]{setReadingOptions(-1,1.0,false);};pureButton_.onClick=[this]{setReadingOptions(reader_.readingLayout(),reader_.readingScale(),true);};
 play_.onClick=[this]{processor_.togglePlay();refreshControls();};fullscreenButton_.onClick=[this]{toggleFullscreen();};
 progress_.setSliderStyle(juce::Slider::LinearHorizontal);progress_.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);progress_.setRange(0,1,.001);progress_.setTitle(U("歌曲进度"));progress_.onValueChange=[this]{if(processor_.mode()!=CueProcessor::Mode::host)processor_.seek(progress_.getValue());};
 reader_.setFullscreenCallback([this]{toggleFullscreen();});reader_.setRestoreControlsCallback([this]{setReadingOptions(reader_.readingLayout(),reader_.readingScale(),false);});
 reader_.setReadingOptions(3,1.0,false);centreWithSize(960,640);refreshControls();startTimerHz(20);
}
ReaderWindow::~ReaderWindow(){stopTimer();clearContentComponent();setLookAndFeel(nullptr);}
void ReaderWindow::setReadingOptions(int mode,double scale,bool pure){reader_.setReadingOptions(mode,scale,pure);layoutControls();refreshControls();if(isShowing())reader_.grabKeyboardFocus();}
void ReaderWindow::layoutControls(){
 auto bounds=view_.getLocalBounds();bool visible=!reader_.isPure();
 for(auto*b:{&smaller_,&larger_,&reset_,&pureButton_,&play_,&fullscreenButton_})b->setVisible(visible);
 layoutChoice_.setVisible(visible);progress_.setVisible(visible);time_.setVisible(visible);
 if(visible){auto top=bounds.removeFromTop(52).reduced(8,6);layoutChoice_.setBounds(top.removeFromLeft(204).reduced(3,0));for(auto*b:{&smaller_,&larger_})b->setBounds(top.removeFromLeft(50).reduced(3,0));reset_.setBounds(top.removeFromLeft(80).reduced(3,0));pureButton_.setBounds(top.removeFromLeft(100).reduced(3,0));fullscreenButton_.setBounds(top.removeFromLeft(90).reduced(3,0));auto bottom=bounds.removeFromBottom(52).reduced(8,6);play_.setBounds(bottom.removeFromLeft(100).reduced(3,0));time_.setBounds(bottom.removeFromRight(160));progress_.setBounds(bottom);}
 reader_.setBounds(bounds);
}
void ReaderWindow::refreshControls(){
 auto state=processor_.state();theme_.apply(state.settings.theme);setBackgroundColour(theme_.bg);view_.repaint();auto tr=[&](const char*s){return cue::translate(U(s),state.settings.language);};
 setName(tr("Lyra · 阅读窗口"));smaller_.setButtonText("A-");larger_.setButtonText("A+");reset_.setButtonText(tr("恢复默认"));pureButton_.setButtonText(tr("纯歌词"));fullscreenButton_.setButtonText(tr("全屏"));
 if(toolbarLanguage_!=state.settings.language){toolbarLanguage_=state.settings.language;layoutChoice_.clear(juce::dontSendNotification);int i=1;for(auto*name:{"共享默认","单行","当前 + 下一句","上下文三行","播放器视图"})layoutChoice_.addItem(tr(name),i++);}layoutChoice_.setSelectedId(reader_.readingLayout()+2,juce::dontSendNotification);
 bool editable=processor_.mode()!=CueProcessor::Mode::host;play_.setEnabled(editable);progress_.setEnabled(editable);play_.setButtonText(editable?(processor_.running()?tr("暂停"):tr("播放")):tr("跟随宿主"));
 double duration=1;for(auto&line:state.document.lines)duration=std::max(duration,line.end+state.document.offset+state.document.songStart);double position=processor_.displayTime();progress_.setRange(0,duration,.001);if(!progress_.isMouseButtonDown())progress_.setValue(juce::jlimit(0.0,duration,position),juce::dontSendNotification);
 auto stamp=[](double t){int seconds=(int)std::max(0.0,t);return juce::String(seconds/60)+":"+juce::String(seconds%60).paddedLeft('0',2);};time_.setText(stamp(position)+" / "+stamp(duration),juce::dontSendNotification);
}
void ReaderWindow::timerCallback(){refreshControls();if(!isVisible())return;bool visible=false;for(auto&d:juce::Desktop::getInstance().getDisplays().displays)if(d.userArea.getIntersection(getBounds()).getWidth()>80&&d.userArea.getIntersection(getBounds()).getHeight()>80){visible=true;break;}if(!visible){full_=false;setUsingNativeTitleBar(true);centreWithSize(960,640);}}
void ReaderWindow::toggleFullscreen(){if(!full_){normalBounds_=getBounds();auto*display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(getBounds());if(display){setUsingNativeTitleBar(false);setBounds(display->totalArea);full_=true;}}else{setUsingNativeTitleBar(true);setBounds(normalBounds_);full_=false;}}
