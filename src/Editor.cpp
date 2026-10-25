#include "Editor.h"
namespace {
void label(juce::Label&l,const juce::String&t){l.setText(t,juce::dontSendNotification);}
void button(juce::TextButton&b,const char*text,std::function<void()>fn){b.setButtonText(U(text));b.onClick=std::move(fn);}
bool numberText(const juce::String&s,double&out){try{auto v=s.trim().toStdString();size_t n=0;out=std::stod(v,&n);return n==v.size()&&std::isfinite(out);}catch(...){return false;}}
}
CueEditor::CueEditor(CueProcessor&p):AudioProcessorEditor(p),processor_(p),reader_(p){
 setLookAndFeel(&theme_);setWantsKeyboardFocus(true);setResizable(true,true);setResizeLimits(1180,780,1800,1200);setSize(1280,840);
 for(juce::Component*c:std::initializer_list<juce::Component*>{&reader_,&list_,&title_,&subtitle_,&status_,&clock_,&listTitle_,&settingsTitle_,&lineHelp_,&import_,&save_,&export_,&edit_,&undo_,&redo_,&readerButton_,&play_,&home_,&stamp_,&wordStamp_,&add_,&remove_,&applyLine_,&shift_,&reset_,&full_,&previous_,&next_,&mode_,&alignment_,&palette_,&mirror_,&offset_,&font_,&spacing_,&context_,&speed_,&seek_,&scale_,&lineText_,&lineStart_,&shiftValue_})addAndMakeVisible(c);
 label(title_,U("Lyra  录音歌词跟随系统"));label(subtitle_,tr("录音提词工作台"));label(listTitle_,tr("文稿与时间"));label(settingsTitle_,tr("阅读设置"));
 for(auto*l:{&title_,&subtitle_,&status_,&clock_,&listTitle_,&settingsTitle_,&lineHelp_})l->setFont(style::font(15));title_.setFont(style::font(21,true));clock_.setFont(juce::Font(juce::FontOptions("Consolas",25,juce::Font::plain)));
 title_.setColour(juce::Label::textColourId,theme_.ink);clock_.setJustificationType(juce::Justification::centredRight);
 list_.setModel(this);list_.setRowHeight(64);list_.setRowSelectedOnMouseDown(false);list_.setMultipleSelectionEnabled(true);list_.setOutlineThickness(0);list_.setTooltip(tr("双击歌词预览；按住 Shift 选择连续范围以校准时间"));
 mode_.addItem(tr("跟随宿主"),1);mode_.addItem(tr("时间轴预览"),2);mode_.addItem(tr("文章滚动"),3);mode_.setSelectedId((int)p.mode()+1,juce::dontSendNotification);
 mode_.onChange=[this]{processor_.setMode((CueProcessor::Mode)(mode_.getSelectedId()-1));refresh();};
 button(import_,"导入文稿",[this]{chooseImport();});import_.setColour(juce::TextButton::buttonColourId,theme_.accent);import_.setColour(juce::TextButton::textColourOffId,theme_.bg);
 button(save_,"保存项目",[this]{saveProject();});button(export_,"导出 LRC",[this]{exportLyrics();});button(edit_,"编辑全文",[this]{openTextEditor();});
 button(undo_,"撤销",[this]{processor_.undo();refresh();});button(redo_,"重做",[this]{processor_.redo();refresh();});
 button(readerButton_,"独立阅读窗",[this]{processor_.showReader();});
 button(full_,"全屏阅读",[this]{processor_.showReader(true);});
 button(play_,"开始",[this]{processor_.togglePlay();});button(home_,"回到开头",[this]{processor_.seek(0);});button(stamp_,"逐句打点",[this]{stamp();});button(wordStamp_,"逐字打点",[this]{stampWord();});
 button(previous_,"上一页",[this]{processor_.seek(std::max(0.0,processor_.displayTime()-reader_.getHeight()*.65/cached_.settings.speed));});button(next_,"下一页",[this]{processor_.seek(processor_.displayTime()+reader_.getHeight()*.65/cached_.settings.speed);});
 button(add_,"新增行",[this]{modify([this](cue::State&s){int at=juce::jlimit(0,(int)s.document.lines.size(),selected_+1);double t=s.document.timed?processor_.displayTime()-s.document.offset-s.document.songStart:at*5;s.document.lines.insert(s.document.lines.begin()+at,{tr("在这里输入新内容"),t,t+5,{}});cue::normalise(s.document);});});
 button(remove_,"删除行",[this]{auto rows=list_.getSelectedRows();modify([rows](cue::State&s){for(int i=rows.size()-1;i>=0;--i){int n=rows[i];if(n>=0&&n<(int)s.document.lines.size())s.document.lines.erase(s.document.lines.begin()+n);}});});
 button(applyLine_,"应用本句",[this]{if(selected_<0)return;double t;if(!numberText(lineStart_.getText(),t)){notify(tr("开始时间请输入秒数，例如 12.500"));return;}modify([&](cue::State&s){auto&d=s.document;if(selected_>=(int)d.lines.size())return;cue::editLine(d,selected_,lineText_.getText());if(d.timed&&!cue::shiftRange(d,selected_,selected_,t-d.lines[(size_t)selected_].start))notify(tr("时间超出范围"));});});
 button(shift_,"校准所选范围",[this]{double delta;if(!numberText(shiftValue_.getText(),delta)){notify(tr("平移量请输入秒数"));return;}auto rows=list_.getSelectedRows();if(rows.isEmpty()){notify(tr("请先选择要校准的歌词行"));return;}auto s=processor_.state();if(!cue::shiftRange(s.document,rows[0],rows[rows.size()-1],delta,scale_.getValue())){notify(tr("校准参数超出范围"));return;}if(!processor_.commit(s)){notify(processor_.lastError());return;}notify(tr("已校准所选范围，可撤销"));refresh();});
 button(reset_,"重置偏移",[this]{modify([this](cue::State&s){s.document.offset=0;});});
 setupSlider(offset_,-120,120,.01,tr("整体延后（秒）"));setupSlider(font_,18,100,1,tr("字号"));setupSlider(spacing_,1,2.5,.05,tr("行距"));setupSlider(context_,0,5,1,tr("共享默认上下文行数"));setupSlider(speed_,5,150,1,tr("滚动速度（像素/秒）"));setupSlider(scale_,.25,4,.01,tr("所选时间比例"));scale_.setValue(1,juce::dontSendNotification);setupSlider(seek_,0,300,.01,tr("预览位置"));seek_.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 offset_.onValueChange=[this]{if(!refreshing_)modify([this](cue::State&s){s.document.offset=offset_.getValue();});};
 font_.onValueChange=[this]{if(!refreshing_)modify([this](cue::State&s){s.settings.fontSize=font_.getValue();});};spacing_.onValueChange=[this]{if(!refreshing_)modify([this](cue::State&s){s.settings.lineSpacing=spacing_.getValue();});};context_.onValueChange=[this]{if(!refreshing_)modify([this](cue::State&s){s.settings.context=(int)context_.getValue();});};speed_.onValueChange=[this]{if(!refreshing_)modify([this](cue::State&s){double pos=processor_.displayTime()*s.settings.speed;s.settings.speed=speed_.getValue();processor_.seek(pos/s.settings.speed);});};
 seek_.onValueChange=[this]{if(!refreshing_&&processor_.mode()!=CueProcessor::Mode::host)processor_.seek(seek_.getValue());};
 alignment_.addItem(tr("左对齐"),1);alignment_.addItem(tr("居中"),2);alignment_.addItem(tr("右对齐"),3);alignment_.onChange=[this]{if(!refreshing_)modify([this](cue::State&s){s.settings.align=alignment_.getSelectedId()-1;});};
 palette_.onChange=[this]{if(!refreshing_)modify([this](cue::State&s){cue::applyPalette(s.settings,palette_.getSelectedId()-1);});};

 mirror_.setButtonText(tr("水平镜像"));mirror_.onClick=[this]{modify([this](cue::State&s){s.settings.mirror=mirror_.getToggleState();});};
 for(auto*t:{&lineText_,&lineStart_,&shiftValue_})t->setFont(style::font(14));lineText_.setTextToShowWhenEmpty(tr("选择一行以编辑文字"),theme_.muted);lineStart_.setTooltip(tr("本句开始时间，单位为秒"));shiftValue_.setText("0.00");shiftValue_.setTooltip(tr("所选范围整体平移秒数；负数提前"));
 settingsButton_.setComponentID("settings");themeChoice_.setComponentID("theme");languageChoice_.setComponentID("language");backButton_.setComponentID("back");copyContact_.setComponentID("contact");addAndMakeVisible(settingsButton_);settingsButton_.setButtonText(U("设置"));settingsButton_.onClick=[this]{if(projectionQR_)projectionQR_->setVisible(false);settingsPage_.setVisible(!settingsPage_.isVisible());if(settingsPage_.isVisible()){settingsPage_.toFront(false);backButton_.grabKeyboardFocus();}};
 addChildComponent(settingsPage_);
 settingsPage_.preview=[this](juce::Graphics&g){if(settingsSection_!=0)return;const int x=556,w=settingsPage_.getWidth()-x-36;g.setColour(theme_.muted);g.setFont(style::font(14));g.drawText(U(cached_.settings.language?"Reading preview":"阅读效果预览"),x,112,w,24,juce::Justification::left);auto r=juce::Rectangle<float>((float)x,146.f,(float)w,218.f);g.setColour(theme_.panel);g.fillRoundedRectangle(r,6.f);g.setColour(theme_.muted);g.setFont(style::font(19));g.drawText(U(cached_.settings.language?"Previous line":"上一句歌词"),x+20,174,w-40,30,juce::Justification::centred);g.setColour(theme_.accent);g.setFont(style::font(28,true));g.drawText(U(cached_.settings.language?"The current lyric":"当前歌词逐字跟随"),x+20,234,w-40,40,juce::Justification::centred);g.setColour(theme_.muted);g.setFont(style::font(19));g.drawText(U(cached_.settings.language?"Next line":"下一句歌词"),x+20,302,w-40,30,juce::Justification::centred);};
 int section=0;for(auto*b:{&generalNav_,&projectionNav_,&helpNav_,&aboutNav_}){settingsPage_.addAndMakeVisible(b);const int id=section++;b->setComponentID("settings-section-"+juce::String(id));b->getProperties().set("quiet",true);b->onClick=[this,id]{settingsSection_=id;resized();};}
 for(auto*b:{&save_,&readerButton_,&quickProjection_,&settingsButton_,&recordTab_,&editTab_,&timeTab_,&styleTab_,&backButton_})b->getProperties().set("quiet",true);
 for(juce::Component*c:std::initializer_list<juce::Component*>{&backButton_,&copyContact_,&appearanceTitle_,&themeLabel_,&languageLabel_,&guideTitle_,&contact_,&themeChoice_,&languageChoice_,&guide_})settingsPage_.addAndMakeVisible(c);
 backButton_.setButtonText(U("返回工作台"));backButton_.onClick=[this]{if(projectionQR_)projectionQR_->setVisible(false);settingsPage_.setVisible(false);settingsButton_.grabKeyboardFocus();};
 copyContact_.setButtonText(U("复制微信号"));copyContact_.onClick=[this]{juce::SystemClipboard::copyTextToClipboard("Amumusic1005");notify(tr("微信号已复制"));};
 label(appearanceTitle_,U("外观与语言"));appearanceTitle_.setFont(style::font(24,true));label(themeLabel_,U("软件主题"));label(languageLabel_,U("界面语言"));label(guideTitle_,U("产品使用说明"));guideTitle_.setFont(style::font(20,true));label(contact_,U("合作咨询微信：Amumusic1005"));
 languageChoice_.addItem(U("简体中文"),1);languageChoice_.addItem("English",2);
 languageChoice_.onChange=[this]{if(!refreshing_)modify([this](cue::State&s){s.settings.language=languageChoice_.getSelectedId()-1;});};
 themeChoice_.onChange=[this]{if(!refreshing_)modify([this](cue::State&s){cue::applyPalette(s.settings,themeChoice_.getSelectedId()-1);});};
 settingsPage_.addAndMakeVisible(helpTopic_);helpTopic_.setComponentID("help-topic");helpTopic_.onChange=[this]{int i=helpTopic_.getSelectedId()-1;if(i>=0&&i<helpSections_.size()){guide_.setText(helpSections_[i],false);guide_.setCaretPosition(0);}};
 guide_.setMultiLine(true);guide_.setReadOnly(true);guide_.setScrollbarsShown(true);guide_.setFont(style::font(18));guide_.setLineSpacing(1.45f);guide_.setIndents(24,20);guide_.setName("User guide");

 for(juce::Component*c:std::initializer_list<juce::Component*>{&originSet_,&originReset_,&originValue_,&originLabel_,&singerText_,&singerColour_,&applySinger_})addAndMakeVisible(c);
 addAndMakeVisible(ktvButton_);ktvButton_.setComponentID("ktv-align");button(ktvButton_,"KTV 自动对齐",[this]{alignmentPanel_=std::make_unique<AlignmentPanel>(processor_,[this]{if(alignmentPanel_)alignmentPanel_->setVisible(false);refresh();});addAndMakeVisible(*alignmentPanel_);alignmentPanel_->setBounds(getLocalBounds().reduced(16,84).withBottom(getHeight()-44));alignmentPanel_->toFront(true);});
 originSet_.setComponentID("song-origin");originValue_.setComponentID("song-origin-value");singerText_.setComponentID("singer");singerColour_.setComponentID("singer-colour");applySinger_.setComponentID("apply-singer");
 label(originLabel_,tr("歌曲起点（工程秒数）"));originValue_.setText("0.000");originValue_.setTooltip(tr("输入伴奏在工程中的起点秒数，按回车应用"));
 originValue_.onReturnKey=[this]{double t;if(!numberText(originValue_.getText(),t)||t<0||t>604800){notify(tr("起点请输入 0 到 604800 秒"));return;}modify([&](cue::State&s){s.document.songStart=t;});};
 button(originSet_,"取宿主位置",[this]{auto t=processor_.transport();if(!t.valid||t.age>1||t.seconds<0){notify(tr("宿主时间不可用，请先开始播放或切换时间轴预览"));return;}modify([&](cue::State&s){s.document.songStart=t.seconds;});});
 button(originReset_,"起点归零",[this]{modify([](cue::State&s){s.document.songStart=0;});});
 singerText_.setTextToShowWhenEmpty(tr("歌手 / 角色"),theme_.muted);singerText_.setInputRestrictions(80);
 button(applySinger_,"标注所选行",[this]{auto rows=list_.getSelectedRows();auto name=singerText_.getText();int colour=singerColour_.getSelectedId();static const char* colours[]={"","ff48b7a9","ffe9a23b","ffd77aaa","ff7aa8ee"};modify([&](cue::State&s){for(int i=0;i<rows.size();++i)if(rows[i]>=0&&rows[i]<(int)s.document.lines.size()){auto& l=s.document.lines[(size_t)rows[i]];l.singer=name;l.colour=colours[juce::jlimit(0,4,colour-1)];}});});
 for(juce::Component*c:std::initializer_list<juce::Component*>{&projectionScan_,&projectionToggle_,&projectionCopy_,&projectionAddress_,&projectionLabel_})settingsPage_.addAndMakeVisible(c);
 projectionToggle_.setComponentID("projection-toggle");projectionAddress_.setComponentID("projection-address");projectionAddress_.setReadOnly(true);projectionAddress_.setMultiLine(true);projectionAddress_.setFont(style::font(12));
 label(projectionLabel_,tr("歌词投屏 · 手机与电脑连接同一局域网"));
 button(projectionToggle_,"开启投屏",[this]{auto& server=processor_.projection();if(server.active())server.stop();else if(!server.start())notify(tr("投屏启动失败"));projectionAddress_.setText(server.urls(),false);updateTransport();if(server.active())showProjectionQR();});
 button(projectionCopy_,"复制投屏地址",[this]{juce::SystemClipboard::copyTextToClipboard(processor_.projection().urls());notify(tr("投屏地址已复制；请使用手机可访问的局域网地址"));});
 button(projectionScan_,"手机扫码",[this]{showProjectionQR();});projectionScan_.setComponentID("projection-scan");
 projectionAddress_.setText(processor_.projection().urls(),false);
 int tabIndex=0;for(auto*b:{&recordTab_,&editTab_,&timeTab_,&styleTab_}){addAndMakeVisible(b);const int id=tabIndex++;b->onClick=[this,id]{workspace_=id;resized();repaint();};}
 recordTab_.setButtonText(U("录音"));editTab_.setButtonText(U("歌词编辑"));timeTab_.setButtonText(U("时间校准"));styleTab_.setButtonText(U("阅读样式"));
 recordTab_.setComponentID("workspace-record");editTab_.setComponentID("workspace-edit");timeTab_.setComponentID("workspace-time");styleTab_.setComponentID("workspace-style");
 addAndMakeVisible(pureButton_);pureButton_.setComponentID("workspace-pure");button(pureButton_,"纯歌词",[this]{setPure(true);});
 addAndMakeVisible(quickProjection_);quickProjection_.setComponentID("quick-projection");button(quickProjection_,"手机投屏",[this]{if(!processor_.projection().active()&&!processor_.projection().start()){notify(tr("投屏启动失败"));return;}showProjectionQR();});
 reader_.setRestoreControlsCallback([this]{setPure(false);});
 addAndMakeVisible(readingView_);readingView_.setComponentID("reading-view");readingView_.onChange=[this]{reader_.setReadingOptions(readingView_.getSelectedId()-2,1.0,pure_);};reader_.setReadingOptions(3,1.0,false);
 refresh();if(getNumRows()>0)list_.selectRow(0);timerCallback();startTimerHz(15);
}
CueEditor::~CueEditor(){stopTimer();projectionQR_.reset();alignmentPanel_.reset();chooser_.reset();list_.setModel(nullptr);setLookAndFeel(nullptr);}
void CueEditor::setupSlider(juce::Slider&s,double low,double high,double step,const juce::String&name){s.setRange(low,high,step);s.setSliderStyle(juce::Slider::LinearHorizontal);s.setTextBoxStyle(juce::Slider::TextBoxRight,false,64,24);s.setName(name);s.setTooltip(name);}
void CueEditor::notify(const juce::String&s){notice_=cue::translate(s,cached_.settings.language);noticeUntil_=juce::Time::getMillisecondCounterHiRes()+7000;label(status_,notice_);}
void CueEditor::modify(std::function<void(cue::State&)>fn){auto s=processor_.state();fn(s);if(!processor_.commit(s))notify(processor_.lastError());refresh();}
void CueEditor::refresh(){auto selectedId=selected_>=0&&selected_<getNumRows()?cached_.document.lines[(size_t)selected_].id:0;int cursor=wordIndex_;if(revision_!=processor_.revision()&&!stamping_)wordIndex_=0;refreshing_=true;auto snapshot=processor_.snapshot();cached_=std::move(snapshot.first);revision_=snapshot.second;updateAppearance();list_.updateContent();if(selectedId){for(int i=0;i<getNumRows();++i)if(cached_.document.lines[(size_t)i].id==selectedId){selected_=i;list_.selectRow(i,false,false);break;}}list_.repaint();originValue_.setText(juce::String(cached_.document.songStart,3),false);offset_.setValue(cached_.document.offset,juce::dontSendNotification);font_.setValue(cached_.settings.fontSize,juce::dontSendNotification);spacing_.setValue(cached_.settings.lineSpacing,juce::dontSendNotification);context_.setValue(cached_.settings.context,juce::dontSendNotification);speed_.setValue(cached_.settings.speed,juce::dontSendNotification);alignment_.setSelectedId(cached_.settings.align+1,juce::dontSendNotification);palette_.setSelectedId(cached_.settings.theme+1,juce::dontSendNotification);mirror_.setToggleState(cached_.settings.mirror,juce::dontSendNotification);mode_.setSelectedId((int)processor_.mode()+1,juce::dontSendNotification);
 bool host=processor_.mode()==CueProcessor::Mode::host,article=processor_.mode()==CueProcessor::Mode::article;play_.setEnabled(!host);home_.setEnabled(!host);seek_.setEnabled(!host&&!article);previous_.setEnabled(article);next_.setEnabled(article);offset_.setEnabled(cached_.document.timed);shift_.setEnabled(cached_.document.timed);export_.setEnabled(cached_.document.timed);undo_.setEnabled(processor_.canUndo());redo_.setEnabled(processor_.canRedo());double end=1;for(auto&l:cached_.document.lines)end=std::max(end,l.end+cached_.document.offset+cached_.document.songStart);seek_.setRange(0,end,.01);label(listTitle_,tr("文稿 · ")+juce::String(cached_.document.lines.size())+tr(" 行"));label(subtitle_,(cached_.document.title==U("未命名文稿")?tr("未命名文稿"):cached_.document.title)+(processor_.dirty()?" *":""));selectedRowsChanged(selected_);if(stamping_)wordIndex_=cursor;refreshing_=false;updateTransport();resized();repaint();}
int CueEditor::getNumRows(){return (int)cached_.document.lines.size();}
void CueEditor::paintListBoxItem(int row,juce::Graphics&g,int w,int h,bool selected){if(row<0||row>=getNumRows())return;auto&l=cached_.document.lines[(size_t)row];if(selected){g.setColour(theme_.raised);g.fillRoundedRectangle(4,2,(float)w-8,(float)h-4,7);g.setColour(theme_.accent);g.fillRoundedRectangle(4,10,3,h-20,1.5f);}g.setFont(style::font(13));g.setColour(theme_.muted);g.drawText(juce::String(row+1).paddedLeft('0',2)+"   "+(cached_.document.timed&&l.marked?cue::formatTime(l.start+cached_.document.offset+cached_.document.songStart):tr("未标时")),16,8,w-30,18,juce::Justification::left);g.setColour(theme_.ink);g.setFont(style::font(17));g.drawText((l.singer.isEmpty()?juce::String{}:"["+l.singer+"] ")+l.text,16,30,w-30,24,juce::Justification::left,true);}
void CueEditor::selectedRowsChanged(int row){if(row!=selected_)wordIndex_=0;selected_=row;bool ok=row>=0&&row<getNumRows();applyLine_.setEnabled(ok);remove_.setEnabled(ok);stamp_.setEnabled(ok);wordStamp_.setEnabled(ok);if(ok){auto&l=cached_.document.lines[(size_t)row];lineText_.setText(l.text,false);lineStart_.setText(juce::String(l.start,3),false);singerText_.setText(l.singer,false);static const char* colours[]={"","ff48b7a9","ffe9a23b","ffd77aaa","ff7aa8ee"};int colour=1;for(int i=0;i<5;++i)if(l.colour==colours[i])colour=i+1;singerColour_.setSelectedId(colour,juce::dontSendNotification);label(lineHelp_,cached_.document.timed?tr("开始秒数 · 修改文字会清除本句字级时间"):tr("普通文本 · 可切换文章滚动"));}else {lineText_.clear();lineStart_.clear();label(lineHelp_,tr("选择歌词，可编辑文字和开始时间"));}}
void CueEditor::listBoxItemClicked(int row,const juce::MouseEvent&e){
 if(workspace_!=0||row<0||row>=getNumRows()||!e.mods.isLeftButtonDown()||e.mods.isAnyModifierKeyDown()||e.mouseWasDraggedSinceMouseDown()||e.getNumberOfClicks()>1)return;
 if(!processor_.seekLine(row,revision_))notify(processor_.seekStatus());
}
void CueEditor::listBoxItemDoubleClicked(int row,const juce::MouseEvent&e){
 if(row<0||row>=getNumRows()||!e.mods.isLeftButtonDown()||e.mods.isAnyModifierKeyDown()||e.mouseWasDraggedSinceMouseDown())return;
 if(!processor_.seekLine(row,revision_))notify(processor_.seekStatus());
}
juce::String CueEditor::getTooltipForRow(int row){
 if(row<0||row>=getNumRows())return {};
 if(!cached_.document.timed||!cached_.document.lines[(size_t)row].marked)return U(cached_.settings.language?"This line has no timestamp":"本句尚未标时");
 if(!processor_.canSeekLyrics())return processor_.seekStatus();
 return U(cached_.settings.language?(workspace_==0?"Click to seek to this lyric":"Select to edit; double-click to seek"):(workspace_==0?"单击跳转到本句":"单击选择编辑；双击跳转"));
}
void CueEditor::timerCallback(){if(revision_!=processor_.revision())refresh();updateTransport();}
void CueEditor::showProjectionQR(){projectionQR_=std::make_unique<ProjectionQR>(processor_,[this]{settingsSection_=1;settingsPage_.setVisible(true);resized();settingsPage_.toFront(false);projectionScan_.grabKeyboardFocus();updateTransport();});addAndMakeVisible(*projectionQR_);projectionQR_->setBounds(settingsPage_.getBounds());projectionQR_->toFront(true);}
void CueEditor::updateTransport(){projectionScan_.setButtonText(tr("手机扫码"));projectionScan_.setEnabled(processor_.projection().active());projectionToggle_.setButtonText(processor_.projection().active()?tr("关闭投屏"):tr("开启投屏"));projectionCopy_.setEnabled(processor_.projection().active());double t=processor_.displayTime();label(clock_,cue::formatTime(t));play_.setButtonText(processor_.running()?tr("暂停"):tr("开始"));refreshing_=true;if(!seek_.isMouseButtonDown())seek_.setValue(t,juce::dontSendNotification);refreshing_=false;auto transport=processor_.transport();bool timed=cached_.document.timed;juce::String mode=processor_.mode()==CueProcessor::Mode::host?(transport.valid?(transport.age>1?tr("等待宿主更新时间"):transport.playing?tr("宿主播放中"):tr("宿主已暂停")):tr("等待宿主 · 独立运行请选预览")):processor_.mode()==CueProcessor::Mode::article?tr("文章滚动 · 空格开始/暂停"):tr("时间轴预览 · 不控制宿主播放");if(juce::Time::getMillisecondCounterHiRes()>noticeUntil_)label(status_,mode+"    /    "+cached_.document.format+"    /    "+(timed?tr("已标时"):tr("未标时：请逐句打点或导入带时间歌词")));}
void CueEditor::setPure(bool pure){pure_=pure;reader_.setReadingOptions(reader_.readingLayout(),1.0,pure);if(pure){settingsPage_.setVisible(false);if(projectionQR_)projectionQR_->setVisible(false);if(alignmentPanel_)alignmentPanel_->setVisible(false);}resized();repaint();reader_.grabKeyboardFocus();}
void CueEditor::paint(juce::Graphics&g){
 g.fillAll(theme_.bg);if(pure_)return;
 g.setColour(theme_.panel);g.fillRect(16,148,256,getHeight()-202);if(workspace_!=0)g.fillRect(getWidth()-320,148,304,getHeight()-202);
 g.setColour(theme_.line);g.drawHorizontalLine(78,24.f,(float)getWidth()-24);g.drawHorizontalLine(getHeight()-42,24.f,(float)getWidth()-24);
 g.setColour(theme_.muted);g.setFont(style::font(14));for(auto*s:{&offset_,&font_,&spacing_,&context_,&speed_,&scale_})if(s->isVisible())g.drawText(s->getName(),s->getX(),s->getY()-22,s->getWidth(),18,juce::Justification::left);
 if(shiftValue_.isVisible())g.drawText(tr("平移秒数"),shiftValue_.getX(),shiftValue_.getY()-22,100,18,juce::Justification::left);
 if(alignment_.isVisible())g.drawText(tr("对齐"),alignment_.getX(),alignment_.getY()-22,120,18,juce::Justification::left);
 if(palette_.isVisible())g.drawText(tr("主题"),palette_.getX(),palette_.getY()-22,120,18,juce::Justification::left);
 g.setColour(theme_.ink);g.setFont(style::font(17,true));g.drawText("Amu music",getWidth()-156,getHeight()-34,136,24,juce::Justification::centredRight);
}
void CueEditor::resized(){int w=getWidth(),h=getHeight();
 // Each workspace explicitly owns its visible tools; overlays retain their own visibility.
 for(auto*c:getChildren()){if(dynamic_cast<juce::ResizableCornerComponent*>(c)){auto*peer=getPeer();c->setVisible(!pure_&&(!peer||(!peer->isFullScreen()&&!peer->isKioskMode())));}else if(c!=&settingsPage_&&c!=projectionQR_.get()&&c!=alignmentPanel_.get())c->setVisible(false);}
 reader_.setVisible(true);if(pure_){reader_.setBounds(getLocalBounds());return;}
 auto show=[](juce::Component&c,int x,int y,int width,int height){c.setBounds(x,y,width,height);c.setVisible(true);};
 show(title_,24,12,400,30);show(subtitle_,24,44,310,24);
 int x=w-638;for(auto*b:{&import_,&save_,&readerButton_,&quickProjection_,&settingsButton_}){int bw=b==&settingsButton_?72:b==&readerButton_?140:126;show(*b,x,25,bw,36);x+=bw+8;}
 x=24;int ti=0;for(auto*b:{&recordTab_,&editTab_,&timeTab_,&styleTab_}){int bw=ti==0?80:128;show(*b,x,94,bw,36);b->setColour(juce::TextButton::buttonColourId,ti==workspace_?theme_.accent:theme_.raised);b->getProperties().set("selected",ti==workspace_);b->setColour(juce::TextButton::textColourOffId,ti==workspace_?theme_.accent:theme_.muted);x+=bw+8;++ti;}
 show(readingView_,536,94,176,36);show(pureButton_,w-134,94,110,36);show(clock_,w-344,94,190,36);
 show(listTitle_,28,162,230,26);show(list_,24,198,240,h-262);
 const int rx=w-300,rw=264;const int readerRight=workspace_==0?w-24:w-340;
 show(reader_,292,158,readerRight-292,h-286);
 show(mode_,292,h-102,174,34);int sx=480;
 if(processor_.mode()!=CueProcessor::Mode::host){show(play_,sx,h-102,76,34);sx+=84;show(home_,sx,h-102,112,34);sx+=120;}
 if(processor_.mode()==CueProcessor::Mode::article){show(previous_,sx,h-102,92,34);show(next_,sx+100,h-102,92,34);}
 else show(seek_,sx,h-99,std::max(40,readerRight-sx),28);
 if(workspace_!=0){show(settingsTitle_,rx,163,rw,28);label(settingsTitle_,workspace_==1?tr("歌词编辑"):workspace_==2?tr("时间校准"):tr("阅读样式"));}
 if(workspace_==1){
 show(edit_,rx,210,rw,36);show(add_,rx,258,128,34);show(remove_,rx+136,258,128,34);
 show(lineText_,rx,312,rw,38);show(lineStart_,rx,360,108,34);show(applyLine_,rx+116,360,148,34);show(lineHelp_,rx,400,rw,46);lineHelp_.setFont(style::font(12));
 show(singerText_,rx,460,140,34);show(singerColour_,rx+148,460,116,34);show(applySinger_,rx,504,rw,34);
 show(export_,rx,h-156,rw,34);show(undo_,rx,h-108,128,34);show(redo_,rx+136,h-108,128,34);
 }else if(workspace_==2){
 show(ktvButton_,rx,210,rw,38);show(originLabel_,rx,265,rw,24);show(originValue_,rx,296,rw,32);show(originSet_,rx,338,128,32);show(originReset_,rx+136,338,128,32);
 show(stamp_,rx,386,128,34);show(wordStamp_,rx+136,386,128,34);show(offset_,rx,458,rw,28);show(reset_,rx,496,rw,30);
 show(shiftValue_,rx,562,100,30);show(scale_,rx+112,562,152,30);show(shift_,rx,604,rw,34);show(undo_,rx,h-108,128,34);show(redo_,rx+136,h-108,128,34);
 }else if(workspace_==3){
 int sy=230;for(auto*s:{&font_,&spacing_,&context_}){show(*s,rx,sy,rw,28);sy+=64;}
 if(processor_.mode()==CueProcessor::Mode::article){show(speed_,rx,sy,rw,28);sy+=64;}
 show(alignment_,rx,sy+10,128,34);show(palette_,rx+136,sy+10,128,34);show(mirror_,rx,sy+60,rw,32);show(full_,rx,h-108,rw,34);
 }
 show(status_,24,h-35,w-200,24);
 if(alignmentPanel_)alignmentPanel_->setBounds(getLocalBounds().reduced(16,84).withBottom(h-44));
 settingsPage_.setBounds(16,84,w-32,h-132);int pw=settingsPage_.getWidth(),ph=settingsPage_.getHeight();
 for(auto*c:settingsPage_.getChildren())c->setVisible(false);
 int navY=90,index=0;const char* zh[]={"外观与语言","手机投屏","使用说明","关于 Lyra"};const char* en[]={"Appearance","Phone screen","User guide","About Lyra"};
 for(auto*b:{&generalNav_,&projectionNav_,&helpNav_,&aboutNav_}){show(*b,16,navY,164,42);b->setButtonText(U(cached_.settings.language?en[index]:zh[index]));b->getProperties().set("selected",index==settingsSection_);b->setColour(juce::TextButton::textColourOffId,index==settingsSection_?theme_.accent:theme_.muted);navY+=54;++index;}
 const int cx=232,cw=juce::jmin(760,pw-cx-36);
 show(appearanceTitle_,cx,28,cw-170,36);appearanceTitle_.setText(U(cached_.settings.language?en[settingsSection_]:zh[settingsSection_]),juce::dontSendNotification);
 show(backButton_,pw-188,30,160,32);
 if(settingsSection_==0){show(themeLabel_,cx,112,220,24);show(themeChoice_,cx,146,280,38);show(languageLabel_,cx,222,220,24);show(languageChoice_,cx,256,280,38);}
 else if(settingsSection_==1){show(projectionLabel_,cx,112,cw,32);show(projectionToggle_,cx,174,150,36);show(projectionScan_,cx+164,174,150,36);show(projectionCopy_,cx,326,180,34);show(projectionAddress_,cx,236,cw,64);projectionAddress_.setText(processor_.projection().urls(),false);}
 else if(settingsSection_==2){show(helpTopic_,cx,96,cw,36);show(guide_,cx,150,cw,ph-182);}
 else{show(guideTitle_,cx,112,cw,36);guideTitle_.setText(U(cached_.settings.language?"Lyra / Recording lyrics companion":"Lyra / 录音歌词跟随系统"),juce::dontSendNotification);show(contact_,cx,190,cw,32);show(copyContact_,cx,242,180,36);}
 if(projectionQR_)projectionQR_->setBounds(settingsPage_.getBounds());
 settingsPage_.repaint();
}
void CueEditor::chooseImport(){chooser_=std::make_unique<juce::FileChooser>(tr("导入歌词、文章或 CueFlow 项目"),juce::File{},"*.lrc;*.krc;*.qrc;*.yrc;*.txt;*.cueflow");auto safe=juce::Component::SafePointer<CueEditor>(this);chooser_->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&c){if(safe&&c.getResult().existsAsFile())safe->importFile(c.getResult());});}
void CueEditor::filesDropped(const juce::StringArray&f,int,int){if(!f.isEmpty())importFile(juce::File(f[0]));}
void CueEditor::importFile(const juce::File&f){bool project=f.hasFileExtension("cueflow");if(f.getSize()>(project?16:4)*1024*1024){notify(project?tr("项目超过 16 MB 限制"):tr("歌词超过 4 MB，请拆分后重试"));return;}auto s=processor_.state();juce::String error;if(project){if(!cue::decodeState(f.loadFileAsString(),s,error)){notify(error);return;}if(!processor_.commit(s)){notify(processor_.lastError());return;}processor_.markSaved();projectFile_=f;}
 else{juce::MemoryBlock data;if(!f.loadFileAsData(data)){notify(tr("无法读取文件"));return;}auto result=cue::parseLyrics(data,f.getFileExtension());if(!result){notify(result.error);return;}s.document=*result.document;if(s.document.title==U("未命名文稿"))s.document.title=f.getFileNameWithoutExtension();if(!processor_.commit(s)){notify(processor_.lastError());return;}projectFile_={};if(!s.document.timed)processor_.setMode(CueProcessor::Mode::article);}
 if(s.document.timed)processor_.setMode(processor_.wrapperType!=juce::AudioProcessor::wrapperType_Undefined||processor_.transport().valid?CueProcessor::Mode::host:CueProcessor::Mode::preview);else processor_.setMode(CueProcessor::Mode::article);
 wordIndex_=0;processor_.seek(0);refresh();if(getNumRows()>0)list_.selectRow(0);notify(tr("已导入：")+f.getFileName()+tr("；替换前内容可撤销"));}
void CueEditor::saveProject(std::function<void(bool)>done){auto write=[safe=juce::Component::SafePointer<CueEditor>(this),done](juce::File f){if(!safe){if(done)done(false);return;}if(f==juce::File{}){if(done)done(false);return;}f=f.withFileExtension("cueflow");juce::TemporaryFile temp(f);bool ok=temp.getFile().replaceWithText(cue::encodeState(safe->processor_.state()),false,false,"\n")&&temp.overwriteTargetFileWithTemporary();if(ok){safe->projectFile_=f;safe->processor_.markSaved();safe->refresh();}safe->notify(ok?safe->tr("项目已保存"):safe->tr("保存失败，请检查目录权限"));if(done)done(ok);};
 if(projectFile_!=juce::File{}){write(projectFile_);return;}chooser_=std::make_unique<juce::FileChooser>(tr("保存 CueFlow 项目"),juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Untitled.cueflow"),"*.cueflow");chooser_->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[write](const juce::FileChooser&c){write(c.getResult());});}
void CueEditor::exportLyrics(){for(auto&l:cached_.document.lines){if(!l.marked){notify(tr("还有未标时的行，请完成打点后再导出；可随时保存项目"));return;}if(l.start+cached_.document.offset<0){notify(tr("存在负时间，请调整偏移后再导出 LRC"));return;}}chooser_=std::make_unique<juce::FileChooser>(tr("导出逐行 LRC（字级时间保留在项目文件）"),juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("lyrics.lrc"),"*.lrc");auto safe=juce::Component::SafePointer<CueEditor>(this);chooser_->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[safe](const juce::FileChooser&c){if(!safe||c.getResult()==juce::File{})return;auto f=c.getResult().withFileExtension("lrc");bool ok=f.replaceWithText(cue::exportLrc(safe->processor_.state().document),false,false,"\n");safe->notify(ok?safe->tr("已导出逐行 LRC，字级时间请保存项目"):safe->tr("导出失败"));});}
void CueEditor::stamp(){if(selected_<0||selected_>=getNumRows())return;if(processor_.mode()==CueProcessor::Mode::host&&(!processor_.transport().valid||processor_.transport().age>1)){notify(tr("宿主时间不可用，请先开始播放或切换时间轴预览"));return;}auto s=processor_.state();int row=selected_;auto nextId=s.document.lines[(size_t)std::min(row+1,getNumRows()-1)].id;auto&d=s.document;auto&l=d.lines[(size_t)row];double t=processor_.displayTime()-d.offset-d.songStart,delta=t-l.start;l.start=t;l.end+=delta;for(auto&w:l.words){w.start+=delta;w.end+=delta;}l.marked=true;d.timed=true;d.format="MANUAL";if(!processor_.commit(s)){notify(processor_.lastError());return;}refresh();for(int i=0;i<getNumRows();++i)if(cached_.document.lines[(size_t)i].id==nextId){list_.selectRow(i);break;}notify(tr("已记录本句开始时间，未打点行保持未标时"));}
void CueEditor::stampWord(){if(selected_<0||selected_>=getNumRows())return;auto s=processor_.state();auto&l=s.document.lines[(size_t)selected_];auto text=l.text;int count=text.length();if(count==0)return;double t=processor_.displayTime()-s.document.offset-s.document.songStart;if(processor_.mode()==CueProcessor::Mode::host&&(!processor_.transport().valid||processor_.transport().age>1)){notify(tr("需要有效宿主时间，或切换到预览模式"));return;}
 if(wordIndex_>=count){if(!l.words.empty()&&t>=l.words.back().start){l.words.back().end=t;l.end=t;if(!processor_.commit(s)){notify(processor_.lastError());return;}}wordIndex_=0;refresh();notify(tr("本句逐字打点已结束，再次点击会重新标注"));return;}
 if(wordIndex_==0){l.words.clear();l.start=t;}else {if((int)l.words.size()>wordIndex_)l.words.resize((size_t)wordIndex_);if(l.words.empty()||t<l.words.back().start){notify(tr("时间已回跳，请重新选择本句后打点"));return;}l.words.back().end=t;}
 l.words.push_back({text.substring(wordIndex_,wordIndex_+1),t,t+1});int nextCursor=wordIndex_+1;l.end=std::max(l.end,t+1);l.marked=true;s.document.timed=true;s.document.format="MANUAL";
 // Preserve the undispatched suffix as a single pending segment so text is never lost.
 if(nextCursor<count)l.words.push_back({text.substring(nextCursor),t+1,l.end});if(!processor_.commit(s)){notify(processor_.lastError());return;}wordIndex_=nextCursor;stamping_=true;refresh();stamping_=false;notify(tr("已标记第 ")+juce::String(wordIndex_)+tr(" 个字；最后再按一次记录结束"));
 // The next click discards the pending suffix before appending a real timestamp.
}
void CueEditor::openTextEditor(){struct TextDialog:juce::Component{style::Theme theme;juce::TextEditor text;juce::TextButton apply{U("应用文稿")},cancel{U("取消")};TextDialog(int id){theme.apply(id);setLookAndFeel(&theme);addAndMakeVisible(text);addAndMakeVisible(apply);addAndMakeVisible(cancel);text.setMultiLine(true);text.setReturnKeyStartsNewLine(true);text.setFont(style::font(18));setSize(720,520);}~TextDialog(){setLookAndFeel(nullptr);}void resized()override{text.setBounds(20,20,getWidth()-40,getHeight()-86);apply.setBounds(getWidth()-236,getHeight()-50,100,32);cancel.setBounds(getWidth()-124,getHeight()-50,100,32);}};
 auto*d=new TextDialog(cached_.settings.theme);d->apply.setButtonText(tr("应用文稿"));d->cancel.setButtonText(tr("取消"));juce::String content;for(auto&l:cached_.document.lines)content+=l.text+"\n";d->text.setText(content,false);auto safe=juce::Component::SafePointer<CueEditor>(this);auto sd=juce::Component::SafePointer<TextDialog>(d);d->apply.onClick=[safe,sd]{if(!safe||!sd)return;auto parsed=cue::parseText(sd->text.getText(),"txt");if(!parsed){safe->notify(parsed.error);return;}auto s=safe->processor_.state();auto old=s.document;auto next=*parsed.document;next.title=old.title;
 if(next.lines.size()==old.lines.size()){next=old;for(size_t i=0;i<old.lines.size();++i)cue::editLine(next,(int)i,parsed.document->lines[i].text);}else safe->notify(safe->tr("行数变化，已切换普通文章；原时间可通过撤销恢复"));s.document=next;if(!safe->processor_.commit(s)){safe->notify(safe->processor_.lastError());return;}if(!next.timed)safe->processor_.setMode(CueProcessor::Mode::article);safe->refresh();if(auto*w=sd->findParentComponentOfClass<juce::DialogWindow>())w->exitModalState(1);};d->cancel.onClick=[sd]{if(sd)if(auto*w=sd->findParentComponentOfClass<juce::DialogWindow>())w->exitModalState(0);};juce::DialogWindow::LaunchOptions o;o.content.setOwned(d);o.dialogTitle=tr("编辑全文 · 行数不变时保留未修改行的时间");o.dialogBackgroundColour=theme_.panel;o.escapeKeyTriggersCloseButton=true;o.useNativeTitleBar=true;o.resizable=true;o.launchAsync();}
bool CueEditor::keyPressed(const juce::KeyPress&k){if(k==juce::KeyPress::escapeKey&&pure_){setPure(false);return true;}if(k==juce::KeyPress::escapeKey&&projectionQR_&&projectionQR_->isVisible()){projectionQR_->setVisible(false);settingsSection_=1;settingsPage_.setVisible(true);resized();settingsPage_.toFront(false);projectionScan_.grabKeyboardFocus();return true;}if(k==juce::KeyPress::escapeKey&&settingsPage_.isVisible()){settingsPage_.setVisible(false);settingsButton_.grabKeyboardFocus();return true;}if(k.getModifiers().isCommandDown()&&k.getKeyCode()=='S'){saveProject();return true;}if(k.getModifiers().isCommandDown()&&k.getKeyCode()=='Z'){processor_.undo();return true;}if(k==juce::KeyPress::spaceKey&&processor_.mode()!=CueProcessor::Mode::host){processor_.togglePlay();return true;}return false;}

void CueEditor::updateAppearance(){
 const bool themeChanged=appliedTheme_!=cached_.settings.theme,languageChanged=appliedLanguage_!=cached_.settings.language;
 if(themeChanged){appliedTheme_=cached_.settings.theme;theme_.apply(appliedTheme_);for(auto* field:{&lineText_,&lineStart_,&shiftValue_,&guide_,&originValue_,&singerText_,&projectionAddress_})field->applyColourToAllText(theme_.ink,true);title_.setColour(juce::Label::textColourId,theme_.ink);import_.setColour(juce::TextButton::buttonColourId,theme_.accent);import_.setColour(juce::TextButton::textColourOffId,theme_.bg);sendLookAndFeelChange();}
 if(languageChanged){appliedLanguage_=cached_.settings.language;notice_.clear();noticeUntil_=0;
  std::function<void(juce::Component&)> translateChildren=[&](juce::Component& parent){for(auto*c:parent.getChildren()){if(auto*b=dynamic_cast<juce::Button*>(c))b->setButtonText(cue::translate(b->getButtonText(),appliedLanguage_));if(auto*l=dynamic_cast<juce::Label*>(c))l->setText(cue::translate(l->getText(),appliedLanguage_),juce::dontSendNotification);if(auto*s=dynamic_cast<juce::Slider*>(c)){s->setName(cue::translate(s->getName(),appliedLanguage_));s->setTooltip(s->getName());}translateChildren(*c);}};translateChildren(*this);
  readingView_.clear(juce::dontSendNotification);{int id=1;for(auto*name:{"共享默认","单行","当前 + 下一句","上下文三行","播放器视图"})readingView_.addItem(tr(name),id++);}readingView_.setSelectedId(reader_.readingLayout()+2,juce::dontSendNotification);
  singerColour_.clear(juce::dontSendNotification);{int i=1;for(auto* name:{"默认颜色","青绿","琥珀","玫瑰","天蓝"})singerColour_.addItem(tr(name),i++);}
  mode_.clear(juce::dontSendNotification);mode_.addItem(tr("跟随宿主"),1);mode_.addItem(tr("时间轴预览"),2);mode_.addItem(tr("文章滚动"),3);
  alignment_.clear(juce::dontSendNotification);alignment_.addItem(tr("左对齐"),1);alignment_.addItem(tr("居中"),2);alignment_.addItem(tr("右对齐"),3);
  palette_.clear(juce::dontSendNotification);themeChoice_.clear(juce::dontSendNotification);for(int i=0;i<5;++i){auto name=U(appliedLanguage_?cue::palettes[i].en:cue::palettes[i].zh);palette_.addItem(name,i+1);themeChoice_.addItem(name,i+1);}
  helpSections_.clear();helpTopic_.clear(juce::dontSendNotification);
  auto remaining=cue::guide(appliedLanguage_).replace("\r","");
  while(remaining.isNotEmpty()){auto block=remaining.upToFirstOccurrenceOf("\n\n",false,false).trim();remaining=remaining.contains("\n\n")?remaining.fromFirstOccurrenceOf("\n\n",false,false):juce::String{};if(block.isNotEmpty()){helpSections_.add(block);helpTopic_.addItem(block.upToFirstOccurrenceOf("\n",false,false).substring(0,52),helpSections_.size());}}
  helpTopic_.setSelectedId(juce::jmin(2,helpSections_.size()),juce::sendNotificationSync);
 }
 title_.setText(U(cached_.settings.language?"Lyra  Recording lyrics system":"Lyra  录音歌词跟随系统"),juce::dontSendNotification);
 themeChoice_.setSelectedId(cached_.settings.theme+1,juce::dontSendNotification);languageChoice_.setSelectedId(cached_.settings.language+1,juce::dontSendNotification);
 singerText_.setTextToShowWhenEmpty(tr("歌手 / 角色"),theme_.muted);originValue_.setTooltip(tr("输入伴奏在工程中的起点秒数，按回车应用"));lineText_.setTextToShowWhenEmpty(tr("选择一行以编辑文字"),theme_.muted);list_.setTooltip(tr("双击歌词预览；按住 Shift 选择连续范围以校准时间"));lineStart_.setTooltip(tr("本句开始时间，单位为秒"));shiftValue_.setTooltip(tr("所选范围整体平移秒数；负数提前"));
}
