#include "Processor.h"
#include "Editor.h"
#include "SeekInteractionTests.h"
#include <iostream>
struct Head : juce::AudioPlayHead {
 double t=0;bool playing=true;
 juce::Optional<PositionInfo> getPosition()const override {PositionInfo p;p.setTimeInSeconds(t);p.setIsPlaying(playing);return p;}
};
int main(int argc,char** argv) {
 juce::ScopedJuceInitialiser_GUI gui;
 if(argc>1&&juce::String(argv[1])=="--projection-fixture"){
  if(argc!=3){std::cerr<<"Usage: --projection-fixture <url-file>\n";return 2;}
  CueProcessor fixture;auto state=fixture.state();state.document=*cue::parseText("[00:01]One\n[00:03]Two\n[00:05]Three","lrc").document;fixture.commit(state);fixture.setMode(CueProcessor::Mode::preview);
  if(!fixture.projection().start()){std::cerr<<"Projection fixture failed to start\n";return 2;}
  juce::File output(juce::String::fromUTF8(argv[2]));output.getParentDirectory().createDirectory();
  if(!output.replaceWithText(fixture.projection().localUrl())){std::cerr<<"Cannot write projection fixture URL\n";return 2;}
  std::cout<<"Projection fixture: "<<fixture.projection().localUrl()<<std::endl;
  juce::MessageManager::getInstance()->runDispatchLoopUntil(30000);fixture.projection().stop();return 0;
 }

 if(argc>1&&juce::String(argv[1])=="--bridge-probe"){HostSeekBridge bridge;for(auto d:juce::MidiOutput::getAvailableDevices())std::cout<<"MIDI OUT: "<<d.name<<std::endl;juce::MessageManager::getInstance()->runDispatchLoopUntil(3000);std::cout<<"BRIDGE STATUS: "<<bridge.status()<<std::endl;if(argc>2)std::cout<<"SEND: "<<bridge.send(juce::String(argv[2]).getDoubleValue())<<std::endl;return bridge.ready()?0:2;}
 int failed=0,checks=0;
 auto check=[&](bool b,const char* n){++checks;if(!b){++failed;std::cerr<<"FAIL "<<n<<"\n";}};
 runSeekInteractionTests(check);
 {CueProcessor click;auto s=click.state();s.document=*cue::parseText("[00:01]One\n[00:03]Two","lrc").document;s.document.songStart=10;s.document.offset=.5;click.commit(s);click.setMode(CueProcessor::Mode::preview);check(click.seekLine(1,click.revision())&&click.displayTime()==13.5,"lyric seek includes song start and offset");check(!click.seekLine(0,click.revision()+1)&&click.displayTime()==13.5,"stale lyric click rejected");check(!click.seekLine(999,click.revision()),"invalid row rejected");click.setMode(CueProcessor::Mode::host);check(!click.seekLine(0,click.revision())&&click.mode()==CueProcessor::Mode::host,"unconnected bridge does not fake a host seek");}
 CueProcessor p;Head h;p.setPlayHead(&h);p.prepareToPlay(48000,256);
 juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer midi;
 for(int c=0;c<2;++c)for(int s=0;s<256;++s)audio.setSample(c,s,float(s-c)/512);
 h.t=4.25;p.processBlock(audio,midi);
 check(p.transport().valid&&p.transport().seconds==4.25,"host position");
 check(audio.getSample(1,200)==199.0f/512,"float passthrough");
 h.t=1;h.playing=false;p.processBlock(audio,midi);check(p.transport().seconds==1&&!p.transport().playing,"seek and pause");
 juce::AudioBuffer<double> doubles(2,32);doubles.clear();doubles.setSample(0,4,.123456789);p.processBlock(doubles,midi);check(doubles.getSample(0,4)==.123456789,"double passthrough");
 auto doc=cue::parseText("[00:01]第一句\n[00:03]第二句","lrc");check(bool(doc),"test document");
 if(doc){auto s=p.state();s.document=*doc.document;p.commit(s);p.markSaved();s.document.offset=2;p.commit(s);check(p.dirty(),"edit dirty");p.undo();check(p.state().document.offset==0&&!p.dirty(),"undo saved state");p.redo();check(p.state().document.offset==2,"redo");}
 juce::MemoryBlock saved;p.getStateInformation(saved);CueProcessor restored;restored.setStateInformation(saved.getData(),(int)saved.getSize());check(restored.state().document.lines.size()==2&&restored.state().document.offset==2,"host restores embedded lyrics");
 restored.setStateInformation("broken",6);check(restored.state().document.lines.size()==2,"invalid host state preserved");
 auto before=restored.state();auto invalid=before;invalid.document.offset=std::numeric_limits<double>::infinity();restored.commit(invalid);check(restored.state().document.offset==before.document.offset,"invalid UI mutation rejected");
 p.setMode(CueProcessor::Mode::preview);p.seek(2.5);check(std::abs(p.displayTime()-2.5)<.02,"manual seek");p.setMode(CueProcessor::Mode::host);check(p.displayTime()==1,"host mode resumes host");
 for(int i=0;i<3;++i){std::unique_ptr<juce::AudioProcessorEditor> e(p.createEditor());check(e!=nullptr,"editor lifecycle");e->setSize(1120,720);
  if(i==0&&argc>1){CueProcessor visual;auto s=visual.state();s.document=*cue::parseText(juce::File::getCurrentWorkingDirectory().getChildFile(U("samples/录音演示.yrc")).loadFileAsString(),"yrc").document;visual.commit(s);visual.setMode(CueProcessor::Mode::preview);visual.seek(1.7);CueEditor display(visual);auto shot=display.createComponentSnapshot(display.getLocalBounds());juce::File f(juce::String::fromUTF8(argv[1]));f.getParentDirectory().createDirectory();juce::FileOutputStream stream(f);stream.setPosition(0);stream.truncate();juce::PNGImageFormat png;check(stream.openedOk()&&png.writeImageToStream(shot,stream)&&stream.getStatus().wasOk(),"UI snapshot");}
 }
 {
  CueProcessor manual;manual.setMode(CueProcessor::Mode::preview);auto s=manual.state();s.document=*cue::parseText("A\nB\nC","txt").document;manual.commit(s);manual.seek(20);CueEditor editor(manual);
  juce::ListBox* list=nullptr;juce::TextButton* stamp=nullptr;for(auto*c:editor.getChildren()){if(auto*l=dynamic_cast<juce::ListBox*>(c))list=l;if(auto*b=dynamic_cast<juce::TextButton*>(c))if(b->getButtonText()==U("逐句打点"))stamp=b;}
  if(list&&stamp){list->selectRow(0);stamp->onClick();auto after=manual.state().document;int row=list->getSelectedRow();check(row>=0&&after.lines[(size_t)row].text=="B","line stamp advances to next original line after reorder");check(cue::locate(after,6).line==-1,"unstamped article lines not advertised as timed");}
 }
 {
  CueProcessor manual;manual.setMode(CueProcessor::Mode::preview);auto s=manual.state();s.document=*cue::parseText("ABC\nDEF","txt").document;manual.commit(s);CueEditor editor(manual);juce::ListBox* list=nullptr;juce::TextButton* word=nullptr;juce::TextButton* edit=nullptr;juce::TextEditor* text=nullptr;
  for(auto*c:editor.getChildren()){if(auto*l=dynamic_cast<juce::ListBox*>(c))list=l;if(auto*b=dynamic_cast<juce::TextButton*>(c)){if(b->getButtonText()==U("逐字打点"))word=b;if(b->getButtonText()==U("应用本句"))edit=b;}if(auto*t=dynamic_cast<juce::TextEditor*>(c))if(t->getText()=="ABC")text=t;}
  if(list&&word&&edit&&text){manual.seek(20);word->onClick();manual.seek(21);word->onClick();auto d=manual.state().document;check(d.lines[0].text=="ABC"&&d.lines[0].words[0].end==21&&d.lines[0].words[1].start==21,"word stamping stays on original row");text->setText("XYZ");edit->onClick();manual.seek(22);word->onClick();d=manual.state().document;check(d.lines[0].words.size()==2&&d.lines[0].words[0].text=="X","word cursor resets after text edit");}
  auto previous=manual.state().document;auto invalid=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("cueflow-invalid",".txt");invalid.replaceWithText(juce::String::repeatedString("x",10001));editor.importFile(invalid);check(manual.state().document.lines[0].text==previous.lines[0].text,"failed import preserves document");invalid.deleteFile();
  cue::State large;large.document.title="Large project";for(int i=0;i<2300;++i)large.document.lines.push_back({juce::String::repeatedString("x",2000),(double)i,(double)i+1,{}});auto file=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("cueflow-large",".cueflow");file.replaceWithText(cue::encodeState(large));check(file.getSize()>4*1024*1024,"large project fixture exceeds lyric limit");editor.importFile(file);check(manual.state().document.lines.size()==2300,"saved project over 4 MB reopens");file.deleteFile();
 }

 {
  CueProcessor localized;auto state=localized.state();state.document=*cue::parseText("[00:01]保持歌词\n[00:04]Second line","lrc").document;localized.commit(state);CueEditor editor(localized);
  std::function<juce::Component*(juce::Component&,const juce::String&)> find=[&](juce::Component& root,const juce::String& id)->juce::Component*{if(root.getComponentID()==id)return &root;for(auto*c:root.getChildren())if(auto*r=find(*c,id))return r;return nullptr;};
  auto* language=dynamic_cast<juce::ComboBox*>(find(editor,"language"));auto* theme=dynamic_cast<juce::ComboBox*>(find(editor,"theme"));auto* settings=dynamic_cast<juce::Button*>(find(editor,"settings"));auto* back=dynamic_cast<juce::Button*>(find(editor,"back"));
  check(language&&theme&&settings&&back,"settings controls available");
  if(language&&theme&&settings&&back){
   settings->onClick();check(theme->getParentComponent()->isVisible(),"settings page opens");
   for(int section=0;section<4;++section){auto* nav=dynamic_cast<juce::Button*>(find(editor,"settings-section-"+juce::String(section)));check(nav!=nullptr,"settings category available without snapshots");if(nav){nav->onClick();check(theme->isVisible()==(section==0),"settings category visibility without snapshots");}}dynamic_cast<juce::Button*>(find(editor,"settings-section-0"))->onClick();
   language->setSelectedId(2,juce::sendNotificationSync);check(localized.state().settings.language==1&&settings->getButtonText()=="Settings"&&back->getButtonText()=="Back to studio","language changes visible controls");
   check(localized.state().document.lines[0].text==U("保持歌词"),"language leaves script intact");
   for(int i=0;i<5;++i){theme->setSelectedId(i+1,juce::sendNotificationSync);check(localized.state().settings.theme==i&&localized.state().settings.background==juce::Colour(cue::palette(i).bg).toString(),"theme applies to reader and state");}
   check(cue::guide(0).contains(U("逐字打点"))&&cue::guide(1).contains("Amumusic1005"),"bilingual guide includes workflow and contact");
   juce::MemoryBlock savedAppearance;localized.getStateInformation(savedAppearance);CueProcessor second;second.setStateInformation(savedAppearance.getData(),(int)savedAppearance.getSize());check(second.state().settings.theme==4&&second.state().settings.language==1,"host restores theme and language");
   CueProcessor separate;check(separate.state().settings.theme==0&&separate.state().settings.language==0,"appearance isolated across instances");
   if(argc>1){auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();auto snapshot=[&](const juce::String& name){auto shot=editor.createComponentSnapshot(editor.getLocalBounds());juce::FileOutputStream out(folder.getChildFile(name));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(out.openedOk()&&png.writeImageToStream(shot,out)&&out.getStatus().wasOk(),"appearance snapshot");};snapshot("settings-en.png");back->onClick();for(int i=0;i<5;++i){theme->setSelectedId(i+1,juce::sendNotificationSync);snapshot("theme-"+juce::String(i)+".png");}settings->onClick();language->setSelectedId(1,juce::sendNotificationSync);snapshot("settings-zh.png");for(int section=0;section<4;++section){auto* nav=dynamic_cast<juce::Button*>(find(editor,"settings-section-"+juce::String(section)));check(nav!=nullptr,"settings category available");if(nav){nav->onClick();snapshot("settings-category-"+juce::String(section)+".png");check(theme->isVisible()==(section==0),"appearance fields restricted to appearance page");auto* topic=find(editor,"help-topic");check(topic&&topic->isVisible()==(section==2),"help topics restricted to guide page");}}dynamic_cast<juce::Button*>(find(editor,"settings-section-0"))->onClick();}
   language->setSelectedId(1,juce::sendNotificationSync);check(settings->getButtonText()==U("设置"),"language switches back to Chinese");back->onClick();check(!theme->getParentComponent()->isVisible(),"settings page closes");
   settings->onClick();dynamic_cast<juce::Button*>(find(editor,"settings-section-1"))->onClick();auto* toggle=dynamic_cast<juce::Button*>(find(editor,"projection-toggle"));auto* scan=dynamic_cast<juce::Button*>(find(editor,"projection-scan"));if(toggle&&scan){toggle->onClick();auto* qr=find(editor,"projection-qr-panel");check(qr&&qr->isVisible(),"start projection opens QR panel");editor.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));check(qr&&!qr->isVisible()&&theme->getParentComponent()->isVisible(),"Escape returns from QR to settings");scan->onClick();qr=find(editor,"projection-qr-panel");settings->onClick();check(qr&&!qr->isVisible()&&!theme->getParentComponent()->isVisible(),"closing settings hides QR overlay");localized.projection().stop();}
  }
 }
 {
  cue::State state,decoded;juce::String error;state.settings.theme=4;state.settings.language=1;check(cue::decodeState(cue::encodeState(state),decoded,error)&&decoded.settings.theme==4&&decoded.settings.language==1,"project appearance roundtrip");
  auto legacy=juce::JSON::parse(cue::encodeState(state));legacy["settings"].getDynamicObject()->removeProperty("theme");legacy["settings"].getDynamicObject()->removeProperty("language");check(cue::decodeState(juce::JSON::toString(legacy),decoded,error)&&decoded.settings.language==0,"legacy project compatible");
  auto bad=juce::JSON::parse(cue::encodeState(state));bad["settings"].getDynamicObject()->setProperty("theme",8);check(!cue::decodeState(juce::JSON::toString(bad),decoded,error),"invalid theme rejected");bad["settings"].getDynamicObject()->setProperty("theme",0);bad["settings"].getDynamicObject()->setProperty("language",2);check(!cue::decodeState(juce::JSON::toString(bad),decoded,error),"invalid language rejected");
 }

 {
  CueProcessor sync;Head host;sync.setPlayHead(&host);auto state=sync.state();state.document=*cue::parseText("[00:01]First\n[00:05]Second","lrc").document;state.document.songStart=30;sync.commit(state);host.t=31;sync.processBlock(audio,midi);
  CueEditor editor(sync);juce::Button* origin=nullptr;juce::Button* stamp=nullptr;juce::Button* annotation=nullptr;juce::TextEditor* singer=nullptr;
  for(auto*c:editor.getChildren()){if(c->getComponentID()=="song-origin")origin=dynamic_cast<juce::Button*>(c);if(c->getComponentID()=="apply-singer")annotation=dynamic_cast<juce::Button*>(c);if(c->getComponentID()=="singer")singer=dynamic_cast<juce::TextEditor*>(c);if(auto*b=dynamic_cast<juce::Button*>(c))if(b->getButtonText()==U("逐句打点"))stamp=b;}
  check(origin&&stamp&&annotation&&singer,"sync workflow controls");
  if(origin&&stamp&&annotation&&singer){singer->setText("Lead");annotation->onClick();check(sync.state().document.lines[0].singer=="Lead","selected line singer annotation");host.t=40;sync.processBlock(audio,midi);stamp->onClick();bool found=false;for(auto&l:sync.state().document.lines)if(l.text=="First")found=l.start==10;check(found,"host stamp subtracts song origin");origin->onClick();check(sync.state().document.songStart==40,"capture host origin");}
 }
 {
  CueProcessor server;auto state=server.state();state.document=*cue::parseText("[00:01]First\n[00:05]Second","lrc").document;state.document.songStart=30;state.document.lines[0].singer="Singer A";server.commit(state);Head host;server.setPlayHead(&host);host.t=31;server.processBlock(audio,midi);
  auto& screen=server.projection();check(!screen.active(),"projection opt in");check(screen.start(),"projection listener starts");
  auto request=[&](const juce::String& url,const juce::String& method="GET"){juce::URL parsed(url);juce::StreamingSocket socket;juce::String response;if(!socket.connect("127.0.0.1",parsed.getPort(),1500))return response;auto req=method+" /"+parsed.getSubPath()+" HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";socket.write(req.toRawUTF8(),(int)req.getNumBytesAsUTF8());auto until=juce::Time::getMillisecondCounterHiRes()+2500;while(juce::Time::getMillisecondCounterHiRes()<until){if(socket.waitUntilReady(true,100)<=0)continue;char buffer[4096];int n=socket.read(buffer,sizeof(buffer),false);if(n<=0)break;response+=juce::String::fromUTF8(buffer,n);}return response;};
  auto url=screen.localUrl();auto html=request(url);check(html.startsWith("HTTP/1.1 200")&&html.contains("Amu music"),"real HTTP reader served");check(!html.contains("id=\"full\"")&&!html.contains("requestFullscreen")&&html.contains("value=\"player\""),"phone has player preset and no fullscreen action");
  auto payload=request(url+"state");auto json=juce::JSON::parse(payload.fromFirstOccurrenceOf("\r\n\r\n",false,false));check((double)json["time"]==1&&(int)json["current"]==0&&(bool)json["playing"],"projection uses host origin and clock");check(json["rows"][0]["singer"].toString()=="Singer A","projection includes singer");
  host.t=35;host.playing=false;server.processBlock(audio,midi);screen.publish();payload=request(url+"state");json=juce::JSON::parse(payload.fromFirstOccurrenceOf("\r\n\r\n",false,false));check((int)json["current"]==1&&!(bool)json["playing"],"projection follows seek and pause");
  host.t=31;server.processBlock(audio,midi);screen.publish();payload=request(url+"state");json=juce::JSON::parse(payload.fromFirstOccurrenceOf("\r\n\r\n",false,false));check((int)json["current"]==0,"projection follows loop backwards");
  state=server.state();state.document=*cue::parseText("[00:01]Lead\n[00:01]Harmony\n[00:05]Next","lrc").document;state.settings.context=0;server.commit(state);host.t=1;server.processBlock(audio,midi);screen.publish();payload=request(url+"state");json=juce::JSON::parse(payload.fromFirstOccurrenceOf("\r\n\r\n",false,false));check(json["rows"].size()==3&&(int)json["groupEnd"]==1,"projection keeps simultaneous lines with zero context");
  juce::String many;for(int i=0;i<21;++i)many+="[00:"+juce::String(i*2).paddedLeft('0',2)+"]Line "+juce::String(i)+"\n";state=server.state();state.document=*cue::parseText(many,"lrc").document;state.document.songStart=0;state.settings.context=0;server.commit(state);host.t=20;server.processBlock(audio,midi);screen.publish();payload=request(url+"state");json=juce::JSON::parse(payload.fromFirstOccurrenceOf("\r\n\r\n",false,false));check(json["rows"].size()==11&&(int)json["rows"][0]["index"]==5,"phone receives player neighbors even when shared context is zero");
  auto base=url.upToLastOccurrenceOf("/",false,false).upToLastOccurrenceOf("/",true,false);check(request(base+"wrong/state").startsWith("HTTP/1.1 404"),"wrong token rejected");check(request(url+"state","POST").startsWith("HTTP/1.1 404"),"projection has no control endpoint");
  {server.setMode(CueProcessor::Mode::preview);server.seek(20);juce::URL target(url);juce::StreamingSocket connection;check(connection.connect("127.0.0.1",target.getPort(),1000),"timeout test connects");auto body=juce::String("{\"row\":0,\"revision\":\"")+juce::String(server.revision())+"\"}";auto requestText="POST /"+target.getSubPath()+"seek HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Type: application/json\r\nX-CueFlow-Seek: 1\r\nContent-Length: "+juce::String(body.getNumBytesAsUTF8())+"\r\n\r\n"+body;connection.write(requestText.toRawUTF8(),(int)requestText.getNumBytesAsUTF8());juce::String reply;if(connection.waitUntilReady(true,1500)>0){char buffer[4096];int n=connection.read(buffer,sizeof(buffer),false);if(n>0)reply=juce::String::fromUTF8(buffer,n);}check(reply.startsWith("HTTP/1.1 408"),"stalled message thread cancels phone seek");juce::MessageManager::getInstance()->runDispatchLoopUntil(350);check(server.displayTime()==20,"expired phone seek cannot jump later");}
  {CueEditor e(server);}check(screen.active(),"projection survives editor close");
  check(projectionPhoneUrls("http://224.0.0.1:1/a/\nhttp://255.255.255.255:1/a/\nhttp://0.0.0.0:1/a/\nhttp://127.0.0.1:1/a/\nhttp://169.254.1.2:1/a/\nhttp://192.168.3.145:1/a/")==juce::StringArray{"http://192.168.3.145:1/a/"},"phone addresses exclude loopback and self-assigned interfaces");
  ProjectionQR qr(server,[]{});qr.setSize(1000,650);qr.setVisible(true);auto selectedUrl=qr.selectedUrl();check(selectedUrl.isEmpty()||(!selectedUrl.contains("127.0.0.1")&&screen.urls().contains(selectedUrl)),"phone QR uses a current LAN address only");
  auto encoded=projectionQrImage(url);check(encoded.isValid()&&encoded.getPixelAt(0,0)==juce::Colours::white,"QR includes white quiet zone");
  if(argc>1){auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();auto write=[&](const juce::String&name,juce::Image image){juce::FileOutputStream out(folder.getChildFile(name));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(png.writeImageToStream(image,out),"QR screenshot saved");};write("qr-code.png",encoded);folder.getChildFile("qr-url.txt").replaceWithText(url);style::Theme theme;qr.setLookAndFeel(&theme);write("qr-panel.png",qr.createComponentSnapshot(qr.getLocalBounds()));qr.setLookAndFeel(nullptr);}
  screen.stop();check(!screen.active(),"projection stops");juce::MessageManager::getInstance()->runDispatchLoopUntil(550);check(qr.selectedUrl().isEmpty()&&!qr.qrImage().isValid(),"stopped projection clears QR");check(screen.start()&&screen.localUrl()!=url,"restart rotates screen URL");juce::MessageManager::getInstance()->runDispatchLoopUntil(550);check(selectedUrl.isEmpty()||qr.selectedUrl()!=selectedUrl,"QR refreshes token after restart");screen.stop();
 }
 {
  CueProcessor imported;Head host;imported.setPlayHead(&host);host.t=2;imported.processBlock(audio,midi);imported.setMode(CueProcessor::Mode::article);CueEditor editor(imported);
  auto snapshot=imported.snapshot();auto newer=snapshot.first;newer.document.title="New session";imported.commit(newer);check(!imported.commitIfRevision(snapshot.first,snapshot.second)&&imported.state().document.title=="New session","stale alignment cannot overwrite changed project");auto fresh=imported.snapshot();fresh.first.document.title="Applied";check(imported.commitIfRevision(fresh.first,fresh.second),"current alignment revision can commit");
  auto file=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("cueflow-timed",".yrc");file.replaceWithText("[1000,2000](1000,1000,0)A(2000,1000,0)B");editor.importFile(file);file.deleteFile();
  check(imported.mode()==CueProcessor::Mode::host,"timed import leaves article mode and follows available host");
  juce::Button* align=nullptr;for(auto*c:editor.getChildren())if(c->getComponentID()=="ktv-align")align=dynamic_cast<juce::Button*>(c);check(align!=nullptr,"automatic alignment entry available");
  if(align){align->onClick();AlignmentPanel* panel=nullptr;for(auto*c:editor.getChildren())if(auto*p=dynamic_cast<AlignmentPanel*>(c))panel=p;check(panel&&panel->isVisible(),"automatic alignment panel opens");
   if(argc>1&&panel){auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();auto image=editor.createComponentSnapshot(editor.getLocalBounds());juce::FileOutputStream out(folder.getChildFile("ktv-panel.png"));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(png.writeImageToStream(image,out),"automatic alignment panel screenshot");}
  }
 }
 if(argc>1){
  auto file=juce::File::getCurrentWorkingDirectory().getChildFile(U("user-output/童真年代/童真年代-KTV.cueflow"));
  if(file.existsAsFile()){CueProcessor song;CueEditor editor(song);editor.importFile(file);check(song.state().document.lines.size()==44&&song.state().document.timed,"real song project validates");song.setMode(CueProcessor::Mode::preview);song.seek(50.75);juce::MessageManager::getInstance()->runDispatchLoopUntil(60);auto image=editor.createComponentSnapshot(editor.getLocalBounds());auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();juce::FileOutputStream out(folder.getChildFile("real-song.png"));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(png.writeImageToStream(image,out),"real song KTV snapshot");auto yrc=cue::parseText(file.withFileExtension("yrc").loadFileAsString(),"yrc");check(yrc&&yrc.document->lines.size()==44,"real song YRC validates");}
 }
 {
  CueProcessor workspace;auto state=workspace.state();state.document=*cue::parseText("[00:01]First\n[00:05]Second","lrc").document;workspace.commit(state);
  auto editor=std::make_unique<CueEditor>(workspace);
  auto byId=[&](const char*id)->juce::Button*{for(auto*c:editor->getChildren())if(c->getComponentID()==id)return dynamic_cast<juce::Button*>(c);return nullptr;};
  auto*record=byId("workspace-record");auto*edit=byId("workspace-edit");auto*timing=byId("workspace-time");auto*style=byId("workspace-style");auto*pure=byId("workspace-pure");auto*align=byId("ktv-align");
  check(record&&edit&&timing&&style&&pure&&align,"workspace controls available");
  if(record&&edit&&timing&&style&&pure&&align){
   check(!align->isVisible(),"record workspace hides timing tools");timing->onClick();check(align->isVisible(),"timing tools open on demand");record->onClick();check(!align->isVisible(),"return to record hides timing tools");pure->onClick();check(!record->isVisible()&&!pure->isVisible(),"main pure mode hides navigation");editor->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));check(record->isVisible(),"Escape restores main controls");
   if(argc>1){auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();for(auto*tab:{record,edit,timing,style}){tab->onClick();editor->setSize(1180,780);auto image=editor->createComponentSnapshot(editor->getLocalBounds());juce::FileOutputStream out(folder.getChildFile(tab->getComponentID()+".png"));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(png.writeImageToStream(image,out),"workspace minimum-size screenshot");}}
  }
  workspace.showReader();auto*window=workspace.readerWindow();check(window&&window->isVisible(),"processor opens singer window");
  const auto before=cue::encodeState(workspace.state());if(window){window->setReadingOptions(1,1.4,true);check(window->reader().isPure()&&window->reader().readingLayout()==1&&cue::encodeState(workspace.state())==before,"reader options remain local");window->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));check(!window->reader().isPure(),"reader Escape restores controls");check(!window->transportEnabled(),"host reader progress is read only");}
  editor.reset();check(workspace.readerWindow()==window&&window->isVisible(),"singer window survives editor destruction");Head liveHost;workspace.setPlayHead(&liveHost);liveHost.t=4;workspace.processBlock(audio,midi);juce::MessageManager::getInstance()->runDispatchLoopUntil(80);check(!window->transportEnabled()&&std::abs(window->progressValue()-4)<.01,"singer window follows live host with editor closed");workspace.setMode(CueProcessor::Mode::preview);workspace.seek(3);juce::MessageManager::getInstance()->runDispatchLoopUntil(80);check(window->transportEnabled()&&std::abs(window->progressValue()-3)<.01,"reader continues updating after editor close");
  workspace.showReader();check(workspace.readerWindow()==window,"reopen reuses singer window");
  window->toggleFullscreen();check(window->isFullscreen(),"reader enters fullscreen");window->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));check(!window->isFullscreen(),"Escape exits ordinary fullscreen");
  {CueProcessor article;auto a=article.state();a.document=*cue::parseText("Short script","txt").document;article.commit(a);article.setMode(CueProcessor::Mode::article);article.seek(100);article.togglePlay();Reader local(article);local.setSize(700,400);local.setReadingOptions(1,.5,false);auto image=local.createComponentSnapshot(local.getLocalBounds());check(image.isValid()&&article.running()&&article.displayTime()>=100,"local article layout cannot pause or rewind shared transport");}

  if(argc>1){auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();for(bool pureView:{false,true}){window->setReadingOptions(1,1,pureView);auto*c=window->getContentComponent();auto image=c->createComponentSnapshot(c->getLocalBounds());juce::FileOutputStream out(folder.getChildFile(pureView?"reader-pure.png":"reader-controls.png"));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(png.writeImageToStream(image,out),"reader view screenshot");}}
 }
 {
  CueProcessor player;auto state=player.state();juce::String lyrics;for(int i=0;i<21;++i)lyrics+="[00:"+juce::String(i*2).paddedLeft('0',2)+"]Lyric line "+juce::String(i)+"\n";state.document=*cue::parseText(lyrics,"lrc").document;player.commit(state);player.setMode(CueProcessor::Mode::preview);player.seek(20.5);
  Reader reader(player);reader.setSize(800,700);const auto before=cue::encodeState(player.state());reader.setReadingOptions(2,1,true);auto three=reader.createComponentSnapshot(reader.getLocalBounds());reader.setReadingOptions(3,1,true);auto many=reader.createComponentSnapshot(reader.getLocalBounds());
  const auto bands=[&](const juce::Image&image){int count=0,gap=100;auto bg=juce::Colour::fromString(state.settings.background);for(int y=0;y<image.getHeight();++y){bool ink=false;for(int x=30;x<image.getWidth()-30;++x)if(image.getPixelAt(x,y)!=bg){ink=true;break;}if(ink){if(gap>=8)++count;gap=0;}else ++gap;}return count;};
  check(bands(many)>=7&&bands(many)>bands(three),"player renders more visible lyric lines than three-line preset");check(cue::encodeState(player.state())==before&&reader.readingLayout()==3,"player preset does not mutate song or shared settings");
  state.document.lines[0].singer="Lead";player.commit(state);player.seek(0);reader.setReadingOptions(3,1,false);auto labeled=reader.createComponentSnapshot(reader.getLocalBounds());int overlapInk=0;for(int y=63;y<70;++y)for(int x=30;x<770;++x)if(labeled.getPixelAt(x,y)!=juce::Colour::fromString(state.settings.background))++overlapInk;check(overlapInk==0,"player first lyric leaves space below singer label");

  CueEditor editor(player);juce::ComboBox*view=nullptr;juce::Button*pure=nullptr;Reader*mainReader=nullptr;for(auto*c:editor.getChildren()){if(c->getComponentID()=="reading-view")view=dynamic_cast<juce::ComboBox*>(c);if(c->getComponentID()=="workspace-pure")pure=dynamic_cast<juce::Button*>(c);if(auto*r=dynamic_cast<Reader*>(c))mainReader=r;}
  check(view&&view->getSelectedId()==5&&mainReader&&mainReader->readingLayout()==3,"main defaults to player view");if(view&&pure&&mainReader){view->setSelectedId(3,juce::sendNotificationSync);pure->onClick();check(mainReader->isPure()&&mainReader->readingLayout()==1,"pure mode preserves selected lyric layout");editor.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));}
  if(argc>1){auto folder=juce::File(juce::String::fromUTF8(argv[1])).getParentDirectory();juce::FileOutputStream out(folder.getChildFile("player-many.png"));out.setPosition(0);out.truncate();juce::PNGImageFormat png;check(png.writeImageToStream(many,out),"player density snapshot");}
 }
 if(argc>2&&juce::String(argv[2])=="--align-smoke"&&!failed){
  auto request=juce::JSON::parse(juce::File::getCurrentWorkingDirectory().getChildFile(U("user-output/童真年代/request.json")).loadFileAsString());cue::AlignmentJob job;juce::String error;
  bool started=job.start(juce::File(request["audio"].toString()),request["lyrics"].toString(),"zh","small",error);check(started,"native child process starts with Unicode audio path");
  if(started){job.cancel();check(!job.running()&&!job.finished(),"native cancellation discards result");started=job.start(juce::File(request["audio"].toString()),request["lyrics"].toString(),"zh","small",error);check(started,"native worker restarts after cancellation");}
  auto until=juce::Time::getMillisecondCounterHiRes()+180000;while(job.running()&&juce::Time::getMillisecondCounterHiRes()<until){juce::MessageManager::getInstance()->runDispatchLoopUntil(100);job.poll();}
  check(job.finished()&&job.result()["lines"].size()==44,"native worker returns complete real song result");if(!job.finished())std::cerr<<job.message()<<'\n';job.cancel();
 }
 if(argc>2&&juce::String(argv[2])=="--serve"&&!failed){
  CueProcessor demo;auto state=demo.state();state.document=*cue::parseText(juce::File::getCurrentWorkingDirectory().getChildFile(U("samples/录音演示.yrc")).loadFileAsString(),"yrc").document;state.document.lines[0].singer=U("主唱 · Amu");demo.commit(state);demo.setMode(CueProcessor::Mode::preview);demo.seek(1.7);demo.projection().start();
  auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("research");folder.getChildFile("projection-demo-url.txt").replaceWithText(demo.projection().localUrl());
  const auto until=juce::Time::getMillisecondCounterHiRes()+180000;while(juce::Time::getMillisecondCounterHiRes()<until&&!folder.getChildFile("projection-stop").existsAsFile())juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
 }
 p.releaseResources();std::cout<<checks<<" checks, "<<failed<<" failures\n";return failed?1:0;
}
