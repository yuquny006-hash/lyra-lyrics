#pragma once
#include "Editor.h"

// Call on the JUCE message thread after ScopedJuceInitialiser_GUI.
template<class Check> void runSeekInteractionTests(Check&& check) {
 auto event=[](juce::Component& c,juce::Point<float> point,bool drag=false,int clicks=1){
  auto now=juce::Time::getCurrentTime();
  return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,
   juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,&c,&c,now,point,now,clicks,drag);
 };
 CueProcessor p;auto state=p.state();state.document=*cue::parseText("[00:01]One","lrc").document;
 state.settings.language=1;p.commit(state);p.setMode(CueProcessor::Mode::preview);
 Reader reader(p);reader.setSize(640,400);reader.setReadingOptions(0,1,false);
 auto paint=[&]{reader.createComponentSnapshot(reader.getLocalBounds());};
 auto tap=[&](juce::Point<float> point,bool drag=false){reader.mouseDown(event(reader,point));reader.mouseUp(event(reader,point,drag));};
 paint();tap({320,200});check(p.displayTime()==1,"reader painted lyric tap seeks");
 p.seek(0);tap({320,200},true);check(p.displayTime()==0,"reader drag does not seek");
 tap({2,200});check(p.displayTime()==0,"reader clipped margin is not a lyric target");
 paint();reader.mouseDown(event(reader,{320,200}));state=p.state();state.document.offset=2;p.commit(state);
 reader.mouseUp(event(reader,{320,200}));check(p.displayTime()==0&&reader.getTooltip().contains("changed"),"reader rejects revision changed between press and release with feedback");
 state.settings.mirror=true;p.commit(state);paint();tap({320,200});check(p.displayTime()==3,"mirrored reader lyric tap seeks");
 p.seek(0);reader.setReadingOptions(0,1,true);paint();tap({320,200});check(!reader.isPure()&&p.displayTime()==0,"pure reader tap restores controls without seek");
 reader.setReadingOptions(0,1,true);paint();tap({320,200},true);check(reader.isPure()&&p.displayTime()==0,"pure reader drag does not restore or seek");
 reader.setReadingOptions(0,1,false);p.setMode(CueProcessor::Mode::article);paint();tap({320,200});check(p.displayTime()==0,"article reader has no timed click targets");
 p.setMode(CueProcessor::Mode::preview);CueEditor editor(p);juce::ListBox* list=nullptr;
 for(auto*c:editor.getChildren())if(auto*l=dynamic_cast<juce::ListBox*>(c))list=l;
 check(list!=nullptr,"seek test finds lyric list");
 if(list){
  auto* model=list->getListBoxModel();auto click=event(*list,{30,30});
  model->listBoxItemClicked(0,click);check(p.displayTime()==3,"record workspace single click seeks");
  for(auto id:{"workspace-edit","workspace-time"}){
   for(auto*c:editor.getChildren())if(c->getComponentID()==id)if(auto*b=dynamic_cast<juce::Button*>(c))b->onClick();
   p.seek(0);model->listBoxItemClicked(0,click);check(p.displayTime()==0,"edit and timing single click only selects");
  }
  model->listBoxItemDoubleClicked(0,event(*list,{30,30},false,2));check(p.displayTime()==3,"double click seeks in timing workspace");
  p.seek(0);model->listBoxItemDoubleClicked(0,event(*list,{30,30},true,2));check(p.displayTime()==0,"dragged double click does not seek");
  state=p.state();state.document.offset=4;p.commit(state);model->listBoxItemDoubleClicked(0,event(*list,{30,30},false,2));
  check(p.displayTime()==0,"list cached revision rejects stale click");
 }
}
