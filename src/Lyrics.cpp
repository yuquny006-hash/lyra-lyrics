#include "Lyrics.h"
#include <regex>
#include <cmath>
#include <juce_core/zip/juce_zlib.h>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
namespace cue {
namespace {
constexpr size_t maxBytes=4*1024*1024;
juce::String fromBytes(const juce::MemoryBlock& b) {
 if(b.getSize()>maxBytes) throw std::runtime_error("文件超过 4 MB 限制");
 auto* p=static_cast<const char*>(b.getData()); auto n=(int)b.getSize();
 if(n>=2 && ((unsigned char)p[0]==0xff || (unsigned char)p[0]==0xfe)) return juce::String::createStringFromData(p,n);
 if(juce::CharPointer_UTF8::isValidString(p,n)) return juce::String::fromUTF8(p,n).trimCharactersAtStart(juce::String::charToString(0xfeff));
#ifdef _WIN32
 int count=MultiByteToWideChar(936,MB_ERR_INVALID_CHARS,p,n,nullptr,0);
 if(count>0) {std::vector<wchar_t> out((size_t)count+1);MultiByteToWideChar(936,MB_ERR_INVALID_CHARS,p,n,out.data(),count);return juce::String(out.data());}
#endif
 throw std::runtime_error("无法识别文本编码，请另存为 UTF-8");
}
juce::MemoryBlock inflate(const juce::MemoryBlock& data) {
 using namespace juce::zlibNamespace;
 juce::MemoryBlock out(maxBytes);uLongf length=maxBytes;
 z_stream stream{};if(inflateInit(&stream)!=Z_OK)throw std::runtime_error("无法初始化歌词解压器");
 stream.next_in=(const Bytef*)data.getData();stream.avail_in=(uInt)data.getSize();stream.next_out=(Bytef*)out.getData();stream.avail_out=(uInt)maxBytes;
 int result=juce::zlibNamespace::inflate(&stream,Z_FINISH);length=stream.total_out;inflateEnd(&stream);
 if(result==Z_BUF_ERROR&&length==maxBytes)throw std::runtime_error("歌词解压后超过 4 MB 限制");
 if(result!=Z_STREAM_END||length==0)throw std::runtime_error("歌词压缩数据损坏或校验失败");
 out.setSize(length);return out;
}
double number(const std::ssub_match& m) {return std::stod(m.str());}
juce::String js(const std::string& s) {return juce::String::fromUTF8(s.data(),(int)s.size());}
}
void normalise(Document& d) {
 juce::int64 id=0;for(auto&l:d.lines)id=std::max(id,l.id);for(auto&l:d.lines)if(l.id==0)l.id=++id;
 if(d.timed)std::stable_sort(d.lines.begin(),d.lines.end(),[](const Line&a,const Line&b){if(a.marked!=b.marked)return a.marked;if(!a.marked)return false;return a.start<b.start;});
 for(size_t i=0;i<d.lines.size();++i){auto& l=d.lines[i];
  if(l.end<=l.start){size_t j=i+1;while(j<d.lines.size()&&d.lines[j].start<=l.start)++j;l.end=j<d.lines.size()?d.lines[j].start:l.start+5;}
 }
}
ParseResult parseLyrics(const juce::MemoryBlock& b,juce::String extension) {
 try {
  if(b.isEmpty()||b.getSize()>maxBytes)throw std::runtime_error("文件为空或超过 4 MB 限制");
  auto ext=extension.trimCharactersAtStart(".").toLowerCase();juce::MemoryBlock decoded=b;
  if(b.getSize()>=4 && std::memcmp(b.getData(),"krc1",4)==0){
   const unsigned char key[]={64,71,97,119,94,50,116,71,81,54,49,45,206,210,110,105};decoded.removeSection(0,4);auto* p=(unsigned char*)decoded.getData();for(size_t i=0;i<decoded.getSize();++i)p[i]^=key[i%16];decoded=inflate(decoded);ext="krc";
  } else if(ext=="qrc") {
   const auto* p=(const unsigned char*)b.getData();
   if(b.getSize()>11 && p[0]==0x98 && p[1]==0x25)decoded=inflate(decodeQrc(b,true));
   else {auto text=fromBytes(b).trim();if(text.containsOnly("0123456789abcdefABCDEF \r\n\t") && text.length()>=16){juce::MemoryBlock raw;raw.loadFromHexString(text);decoded=inflate(decodeQrc(raw,false));}}
  }
  return parseText(fromBytes(decoded),ext);
 }catch(const std::exception& e){return {{},juce::String::fromUTF8(e.what())};}
}
ParseResult parseText(const juce::String& text,juce::String extension) {
 try {
  if(text.getNumBytesAsUTF8()>maxBytes || text.trim().isEmpty())throw std::runtime_error("文稿为空或过大");
  Document d;auto ext=extension.trimCharactersAtStart(".").toLowerCase();d.format=ext.toUpperCase();d.timed=ext!="txt";
  auto content=text;
  if(ext=="qrc" && text.trimStart().startsWithChar('<')) {
   auto xml=juce::parseXML(text);if(!xml)throw std::runtime_error("QRC XML 损坏");
   std::function<juce::String(const juce::XmlElement&)> find=[&](const juce::XmlElement& e)->juce::String {if(e.hasAttribute("LyricContent"))return e.getStringAttribute("LyricContent");for(auto* child:e.getChildIterator()){auto s=find(*child);if(s.isNotEmpty())return s;}return {};};
   content=find(*xml);if(content.isEmpty())throw std::runtime_error("QRC 中没有 LyricContent 歌词内容");
  }
  static const std::regex lrcTag(R"(\[(\d+):(\d{1,2}(?:\.\d+)?)\])"), head(R"(^\[(-?\d+),(\d+)\])"), kword(R"(<(\d+),(\d+),\d+>(.*?)(?=<\d+,\d+,\d+>|$))"), yword(R"(\((\d+),(\d+),\d+\)(.*?)(?=\(\d+,\d+,\d+\)|$))"), qword(R"((.*?)\((\d+),(\d+)\))"),metadata(R"(^\[[a-zA-Z][\w-]*:.*\]$)");
  if(ext=="qrc"&&std::regex_search(content.toStdString(),lrcTag)){auto result=parseText(content,"lrc");if(result)result.document->format="QRC/LRC";return result;}
  int ignored=0;
  for(auto raw:juce::StringArray::fromLines(content)) {
   raw=raw.trim();if(raw.isEmpty())continue;
   if(raw.startsWith("[ti:")){d.title=raw.substring(4).upToLastOccurrenceOf("]",false,false);continue;}
   if(raw.startsWith("[offset:")){d.offset=-raw.substring(8).getDoubleValue()/1000.0;continue;}
   if(std::regex_match(raw.toStdString(),metadata)||raw.startsWithChar('{'))continue;
   if(raw.length()>100000)throw std::runtime_error("单行内容过长，请拆分后导入");
   if(!d.timed){if(raw.length()>10000)throw std::runtime_error("单行超过 10000 字符，请拆分后导入");d.lines.push_back({raw,(double)d.lines.size()*5,(double)(d.lines.size()+1)*5,{},0,false});continue;}
   auto s=raw.toStdString();std::smatch m;
   if(ext=="lrc") {
    std::vector<double> starts;size_t finish=0;
    for(std::sregex_iterator it(s.begin(),s.end(),lrcTag),end;it!=end;++it){if((size_t)it->position()!=finish)break;double seconds=number((*it)[2]);if(seconds>=60)throw std::runtime_error("LRC 秒数必须小于 60");starts.push_back(number((*it)[1])*60+seconds);finish=(size_t)(it->position()+it->length());}
    if(starts.empty()){++ignored;continue;}auto words=js(s.substr(finish)).trim();
    for(double t:starts)d.lines.push_back({words,t,0,{}});
   }else if(ext=="krc"||ext=="yrc"||ext=="qrc") {
    if(!std::regex_search(s,m,head)){++ignored;continue;}
    Line l;l.start=number(m[1])/1000;l.end=l.start+number(m[2])/1000;auto body=s.substr(m.length());
    const auto& pattern=ext=="krc"?kword:ext=="yrc"?yword:qword;
    size_t covered=0;
    for(std::sregex_iterator it(body.begin(),body.end(),pattern),end;it!=end;++it){auto w=*it;
     if((size_t)w.position()!=covered)throw std::runtime_error("逐字歌词包含无法解析的时间标签");covered=(size_t)(w.position()+w.length());
     Word word;word.text=js(w[ext=="qrc"?1:3].str());word.start=number(w[ext=="qrc"?2:1])/1000+(ext=="krc"?l.start:0);word.end=word.start+number(w[ext=="qrc"?3:2])/1000;
     if(word.start<l.start-.01||word.end>l.end+.1||(!l.words.empty()&&word.start<l.words.back().start))throw std::runtime_error("字词时间超出句子范围或顺序错误");
     l.text+=word.text;l.words.push_back(word);
    }
    if(l.words.empty()){l.text=js(body);if(l.text.containsAnyOf("<>()"))throw std::runtime_error("无法识别的逐字时间格式");}
    else if(covered<body.size()){auto tail=js(body.substr(covered));if(tail.trim().isNotEmpty())throw std::runtime_error("逐字歌词尾部缺少时间");}
    d.lines.push_back(std::move(l));
   }else throw std::runtime_error("支持 LRC / KRC / QRC / YRC / TXT 文件");
   if(d.lines.size()>20000)throw std::runtime_error("歌词行数超过 20000 行限制");
  }
  if(d.lines.empty())throw std::runtime_error("没有可用歌词，请检查文件格式和时间标签");
  if(ignored>0)throw std::runtime_error("部分歌词行缺少有效时间标签，导入已取消以避免丢失内容");
  normalise(d);for(auto&l:d.lines)if(l.text.length()>10000||l.start< -86400||l.end>604800)throw std::runtime_error("歌词文本或时间超出支持范围");return {std::move(d),{}};
 }catch(const std::exception&e){return {{},juce::String::fromUTF8(e.what())};}
}
Position locate(const Document& d,double seconds) {
 Position p;if(!std::isfinite(seconds)||d.lines.empty())return p;double t=seconds-d.offset-d.songStart;
 auto markedEnd=std::find_if(d.lines.begin(),d.lines.end(),[](const Line&l){return !l.marked;});auto it=std::upper_bound(d.lines.begin(),markedEnd,t,[](double v,const Line&l){return v<l.start;});if(it==d.lines.begin())return p;
 --it;while(it!=d.lines.begin()&&(it-1)->start==it->start)--it;p.line=(int)(it-d.lines.begin());p.active=t>=it->start&&t<it->end;
 for(size_t i=0;i<it->words.size();++i){const auto&w=it->words[i];if(t>=w.start){p.word=(int)i;p.progress=juce::jlimit(0.0,1.0,(t-w.start)/std::max(.001,w.end-w.start));}}
 return p;
}
juce::String formatTime(double t){if(!std::isfinite(t))return "--:--.---";auto ms=(juce::int64)std::llround(std::abs(t)*1000);return (t<0?"-":"")+juce::String::formatted("%02d:%02d.%03d",(int)(ms/60000),(int)((ms/1000)%60),(int)(ms%1000));}
juce::String exportLrc(const Document& d){juce::String out="[ti:"+d.title.replaceCharacters("\r\n", "  ")+ "]\n";for(auto&l:d.lines)if(l.marked)out+="["+formatTime(std::max(0.0,l.start+d.offset))+"]"+l.text+"\n";return out;}
}
