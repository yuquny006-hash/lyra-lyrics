#include "State.h"
#include "Alignment.h"
#include <iostream>
#include <cmath>
using namespace cue;
int failures=0, checks=0;
void check(bool b,const char* name) { ++checks; if(!b) {++failures; std::cerr<<"FAIL "<<name<<"\n";} }
bool near(double a,double b) { return std::abs(a-b)<0.00001; }
int main(int argc,char** argv) {
 auto l=parseText("[ti:测试]\n[offset:200]\n[00:01.00][00:05.00]你好\n[00:03.50]世界", "lrc");
 check(bool(l),"LRC parses");
 if(l) {auto& d=*l.document; check(d.lines.size()==3,"multi timestamps"); check(d.title==juce::String::fromUTF8("测试"),"unicode title"); check(near(d.offset,-.2),"LRC positive offset advances"); check(d.lines[0].words.empty(),"LRC no fabricated words"); check(locate(d,3.4).line==1,"offset query"); check(locate(d,5).line==2 && locate(d,1).line==0,"backward seek"); check(locate(d,-2).line==-1,"negative time preroll"); }
 auto y=parseText("[1000,1500](1000,500,0)你(1500,1000,0)好\n[4000,1000](4000,1000,0)世界", "yrc");
 check(bool(y),"YRC parses");
 if(y) {auto& d=*y.document;check(d.lines[0].words.size()==2,"YRC words");check(d.lines[0].text==juce::String::fromUTF8("你好"),"YRC text");check(locate(d,1.75).word==1 && near(locate(d,1.75).progress,.25),"word progress");check(!locate(d,3).active,"interlude");check(locate(d,4).line==1 && locate(d,1.1).line==0,"loop relocation");}
 auto k=parseText("[1000,1500]<0,500,0>你<500,1000,0>好", "krc");
 check(bool(k),"KRC plain parses"); if(k) check(near(k.document->lines[0].words.back().start,1.5),"KRC relative start");
 auto q=parseText("<QrcInfos><Lyric_1 LyricContent=\"[1000,1500]你(1000,500)好(1500,1000)\"/></QrcInfos>","qrc");
 check(bool(q),"QRC XML parses"); if(q) check(q.document->lines[0].words.size()==2,"QRC word timing");
 check(!parseText("[bad]oops","lrc"),"invalid timed file rejected");
 check(!parseText("","txt"),"empty document rejected");
 {juce::MemoryBlock b;bool loaded=juce::File::getCurrentWorkingDirectory().getChildFile("tests/fixtures/original.krc").loadFileAsData(b);check(loaded&&b.getSize()>8,"encrypted fixture available");if(b.getSize()>8){b.setSize(b.getSize()-4);check(!parseLyrics(b,"krc"),"truncated zlib checksum rejected");}}
 check(bool(parseText("[sign:]\n[total:2000]\n[1000,500]<0,500,0>词","krc")),"metadata variants");
 check(bool(parseText("[1000,500]词(说明)(1000,500)","qrc")),"parentheses in lyric");
 check(bool(parseText("[00:01.00]翻译","qrc")),"QRC line timing variant");
 auto plain=parseText("你好 😀\n第二行","txt"); check(bool(plain)&&!plain.document->timed,"plain article");
 if(y) {
  State original{*y.document,{}}; original.settings.mirror=true; original.document.offset=1.25;
  State restored; juce::String err; check(decodeState(encodeState(original),restored,err),"state roundtrip");
  check(restored.document.lines.size()==2 && restored.settings.mirror && near(restored.document.offset,1.25),"state content restored");
  check(!decodeState("{\"version\":99}",restored,err),"unknown version rejected");
  check(restored.document.lines.size()==2,"invalid state retains current");
  auto broken=juce::JSON::parse(encodeState(original));auto& first=broken["document"]["lines"].getArray()->getReference(0);first.getDynamicObject()->removeProperty("text");check(!decodeState(juce::JSON::toString(broken),restored,err),"missing line text rejected");
  broken=juce::JSON::parse(encodeState(original));broken["document"]["lines"].getArray()->getReference(0).getDynamicObject()->setProperty("text","different");check(!decodeState(juce::JSON::toString(broken),restored,err),"word mapping mismatch rejected");
  auto d=*y.document; check(shiftRange(d,0,1,2,2),"time transform accepts"); check(near(d.lines[0].start,3)&&near(d.lines[0].words[1].start,4)&&near(d.lines[1].start,9),"range anchored scaling");
  check(!shiftRange(d,0,1,0,-1),"negative scale rejected");
  editLine(d,0,"改词");check(d.lines[0].words.empty(),"edit invalidates stale word mapping");
 }
 if(argc>1) {juce::File dir(juce::String::fromUTF8(argv[1])); for(auto f:dir.findChildFiles(juce::File::findFiles,false)) {juce::MemoryBlock data;f.loadFileAsData(data);auto r=parseLyrics(data,f.getFileExtension());check(bool(r),("fixture "+f.getFileName()).toRawUTF8());if(r)check(!r.document->lines.empty(),"fixture nonempty");else std::cerr<<r.error.toStdString()<<"\n";} }
 {
  State s; s.document=*parseText("[00:01]Hello\n[00:05]World","lrc").document;
  auto json=juce::JSON::parse(encodeState(s));json["document"].getDynamicObject()->setProperty("songStart",30.0);
  auto& row=json["document"]["lines"].getArray()->getReference(0);
  row.getDynamicObject()->setProperty("singer","Singer A");row.getDynamicObject()->setProperty("colour","ffe9a23b");
  State restored;juce::String error;check(decodeState(juce::JSON::toString(json),restored,error),"new session fields accepted");
  check(locate(restored.document,30).line==-1&&locate(restored.document,31).line==0,"song origin maps host position");
  auto saved=juce::JSON::parse(encodeState(restored));check(saved["document"]["songStart"]==juce::var(30.0),"origin persists");
  check(saved["document"]["lines"][0]["singer"].toString()=="Singer A","singer persists");
  check(saved["document"]["lines"][0]["colour"].toString()=="ffe9a23b","line colour persists");
  check(exportLrc(restored.document).contains("[00:01.000]"),"export stays relative to song");
  json["document"].getDynamicObject()->setProperty("songStart",-1.0);check(!decodeState(juce::JSON::toString(json),restored,error),"negative origin rejected");
 }
 {
  State base,out;juce::String error;base.document.songStart=30;base.document.title="Song";base.document.lines.push_back({"Hi",0,1,{}});base.document.lines[0].singer="Lead";
  auto result=juce::JSON::parse(R"({"version":1,"lines":[{"text":"Hi","start":1,"end":2,"words":[{"text":"H","start":1,"end":1.5},{"text":"i","start":1.5,"end":2}]}]})");
  check(alignedState(base,result,out,error)&&out.document.lines[0].words.size()==2,"automatic alignment result applies");
  check(out.document.songStart==30&&out.document.title=="Song"&&out.document.lines[0].singer=="Lead","alignment retains song origin title and labels");
  check(base.document.lines[0].words.empty(),"alignment conversion does not mutate original");
  auto saved=encodeState(out);result["lines"][0]["words"][0].getDynamicObject()->setProperty("text","Wrong");
  check(!alignedState(base,result,out,error)&&encodeState(out)==saved,"mismatched alignment keeps previous output");
  result["lines"][0].getDynamicObject()->setProperty("words",juce::Array<juce::var>{});check(!alignedState(base,result,out,error),"alignment without word timing rejected");
 }
 std::cout<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;
}
