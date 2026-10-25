#include "State.h"
#include <cmath>
#include <set>
namespace cue {
namespace {
juce::var object(){return juce::var(new juce::DynamicObject());}
void put(juce::var& v,const char* key,juce::var value){v.getDynamicObject()->setProperty(key,value);}
bool numeric(juce::var v){return (v.isInt()||v.isInt64()||v.isDouble())&&std::isfinite((double)v);}
double requiredNumber(const juce::var& v,const char* key,double low,double high){auto n=v[key];if(!numeric(n)||(double)n<low||(double)n>high)throw std::runtime_error("数值字段无效或超出范围");return (double)n;}
}
juce::String encodeState(const State&s) {
 auto root=object();put(root,"version",1);auto d=object();put(d,"title",s.document.title);put(d,"format",s.document.format);put(d,"offset",s.document.offset);put(d,"songStart",s.document.songStart);put(d,"timed",s.document.timed);juce::Array<juce::var> lines;
 for(auto&l:s.document.lines){auto row=object();put(row,"text",l.text);put(row,"singer",l.singer);put(row,"colour",l.colour);put(row,"start",l.start);put(row,"end",l.end);put(row,"id",l.id);put(row,"marked",l.marked);juce::Array<juce::var> words;for(auto&w:l.words){auto word=object();put(word,"text",w.text);put(word,"start",w.start);put(word,"end",w.end);words.add(word);}put(row,"words",words);lines.add(row);}put(d,"lines",lines);put(root,"document",d);
 auto o=object();put(o,"theme",s.settings.theme);put(o,"language",s.settings.language);put(o,"fontSize",s.settings.fontSize);put(o,"lineSpacing",s.settings.lineSpacing);put(o,"speed",s.settings.speed);put(o,"context",s.settings.context);put(o,"align",s.settings.align);put(o,"mirror",s.settings.mirror);put(o,"foreground",s.settings.foreground);put(o,"background",s.settings.background);put(o,"accent",s.settings.accent);put(root,"settings",o);return juce::JSON::toString(root,true);
}
bool decodeState(const juce::String& text,State& out,juce::String& error) {
 try {
  if(text.getNumBytesAsUTF8()>16*1024*1024)throw std::runtime_error("项目状态过大");
  auto root=juce::JSON::parse(text);if(!root.isObject()||!root["version"].isInt()||(int)root["version"]!=1)throw std::runtime_error("项目格式损坏或版本不支持");
  State s;auto d=root["document"];if(!d.isObject()||!d["timed"].isBool()||!d["title"].isString()||!d["format"].isString())throw std::runtime_error("文稿字段缺失");s.document.title=d["title"].toString();s.document.format=d["format"].toString();s.document.timed=(bool)d["timed"];s.document.offset=requiredNumber(d,"offset",-86400,86400);s.document.songStart=d.hasProperty("songStart")?requiredNumber(d,"songStart",0,604800):0;auto* lines=d["lines"].getArray();if(!lines||lines->size()>20000)throw std::runtime_error("文稿行数无效");std::set<juce::int64> ids;
  for(auto&row:*lines){Line l;if(!row["text"].isString())throw std::runtime_error("歌词文本字段缺失");l.text=row["text"].toString();if(row.hasProperty("singer")){if(!row["singer"].isString()||row["singer"].toString().length()>80)throw std::runtime_error("Invalid singer label");l.singer=row["singer"].toString();}if(row.hasProperty("colour")){auto c=row["colour"].toString();if(!row["colour"].isString()||(c.isNotEmpty()&&(c.length()!=8||!c.containsOnly("0123456789abcdefABCDEF"))))throw std::runtime_error("Invalid line colour");l.colour=c;}if(l.text.length()>10000)throw std::runtime_error("单行文本过长");l.start=requiredNumber(row,"start",-86400,604800);l.end=requiredNumber(row,"end",l.start,604800);if(row.hasProperty("id")){l.id=(juce::int64)requiredNumber(row,"id",0,1000000000);if(l.id&&!ids.insert(l.id).second)throw std::runtime_error("歌词行标识重复");}l.marked=row.hasProperty("marked")?(bool)row["marked"]:s.document.timed;if(row.hasProperty("marked")&&!row["marked"].isBool())throw std::runtime_error("标时状态无效");auto* words=row["words"].getArray();if(!words||words->size()>10000)throw std::runtime_error("字词数据无效");double previous=l.start;juce::String combined;
   for(auto&w:*words){Word word;if(!w["text"].isString())throw std::runtime_error("字词文本字段缺失");word.text=w["text"].toString();combined+=word.text;word.start=requiredNumber(w,"start",previous-.01,l.end+.1);word.end=requiredNumber(w,"end",word.start,l.end+.1);previous=word.start;l.words.push_back(word);}if(!words->isEmpty()&&combined!=l.text)throw std::runtime_error("字词时间与歌词文字不一致");s.document.lines.push_back(l);}
  auto o=root["settings"];for(auto key:{"theme","language"})if(o.hasProperty(key)&&(!o[key].isInt()||(int)o[key]<0||(int)o[key]>(juce::String(key)=="theme"?4:1)))throw std::runtime_error("外观或语言设置无效");s.settings.theme=o.hasProperty("theme")?(int)o["theme"]:0;s.settings.language=o.hasProperty("language")?(int)o["language"]:0;s.settings.fontSize=requiredNumber(o,"fontSize",18,100);s.settings.lineSpacing=requiredNumber(o,"lineSpacing",1,2.5);s.settings.speed=requiredNumber(o,"speed",5,150);s.settings.context=(int)requiredNumber(o,"context",0,5);s.settings.align=(int)requiredNumber(o,"align",0,2);if(!o["mirror"].isBool())throw std::runtime_error("镜像设置无效");s.settings.mirror=(bool)o["mirror"];
  for(auto pair:{std::pair<const char*,juce::String*>("foreground",&s.settings.foreground),{"background",&s.settings.background},{"accent",&s.settings.accent}}){auto c=o[pair.first].toString();if(c.length()!=8||!c.containsOnly("0123456789abcdefABCDEF"))throw std::runtime_error("颜色设置无效");*pair.second=c;}
  normalise(s.document);out=std::move(s);error.clear();return true;
 }catch(const std::exception&e){error=juce::String::fromUTF8(e.what());return false;}
}
bool shiftRange(Document&d,int first,int last,double delta,double scale) {
 if(first<0||last<first||last>=(int)d.lines.size()||!std::isfinite(delta)||!std::isfinite(scale)||scale<=0||scale>10)return false;
 auto copy=d;double anchor=d.lines[(size_t)first].start;
 auto transform=[&](double v){return anchor+(v-anchor)*scale+delta;};
 for(int i=first;i<=last;++i){auto&l=copy.lines[(size_t)i];l.start=transform(l.start);l.end=transform(l.end);if(l.start< -86400||l.end>604800)return false;for(auto&w:l.words){w.start=transform(w.start);w.end=transform(w.end);}}
 normalise(copy);d=std::move(copy);return true;
}
void editLine(Document&d,int i,juce::String text){if(i>=0&&i<(int)d.lines.size()&&d.lines[(size_t)i].text!=text){d.lines[(size_t)i].text=text;d.lines[(size_t)i].words.clear();}}
}
