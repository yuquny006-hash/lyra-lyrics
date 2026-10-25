#include "Projection.h"
#include "Processor.h"
#include "ProjectionPage.h"
#include <chrono>
#include <map>
Projection::Projection(CueProcessor& p):Thread("CueFlow projection"),processor_(p){}
Projection::~Projection(){stop();}
bool Projection::start(){
 if(active())return true;
 if(!listener_.createListener(0,"0.0.0.0"))return false;
 port_=listener_.getBoundPort();token_=juce::Uuid().toString().removeCharacters("-");
 publish();startTimerHz(10);startThread();return true;
}
void Projection::stop(){stopTimer();signalThreadShouldExit();stopThread(-1);listener_.close();port_=0;token_.clear();std::lock_guard<std::mutex> lock(mutex_);seeks_.clear();}
juce::String Projection::localUrl()const{return active()?"http://127.0.0.1:"+juce::String(port_)+"/"+token_+"/":juce::String{};}
juce::String Projection::urls()const{
 if(!active())return {};juce::Array<juce::IPAddress> addresses;juce::IPAddress::findAllAddresses(addresses,false);juce::StringArray result;
 for(const auto& a:addresses){auto ip=a.toString();if(isPhoneAddress(ip))result.addIfNotAlreadyThere("http://"+ip+":"+juce::String(port_)+"/"+token_+"/");}
 result.add(localUrl());return result.joinIntoString("\n");
}
void Projection::timerCallback(){
 for(int i=0;i<4;++i){std::shared_ptr<SeekRequest> r;{std::lock_guard<std::mutex> lock(mutex_);if(seeks_.empty())break;r=seeks_.front();seeks_.pop_front();r->started=true;}
  const bool accepted=processor_.seekLine(r->row,r->revision);const auto message=processor_.seekStatus();
  {std::lock_guard<std::mutex> lock(mutex_);r->accepted=accepted;r->message=message;r->done=true;}seekChanged_.notify_all();
 }publish();
}
void Projection::publish(){
 const auto coherent=processor_.snapshot();const auto&s=coherent.first;const auto&d=s.document;const double time=processor_.displayTime();const auto pos=cue::locate(d,time);const auto host=processor_.transport();
 juce::DynamicObject::Ptr root=new juce::DynamicObject();auto put=[&](const char*k,juce::var v){root->setProperty(k,v);};
 put("revision",juce::String(coherent.second));put("canSeek",processor_.canSeekLyrics());put("seekStatus",processor_.seekStatus());put("title",d.title);put("time",time-d.songStart-d.offset);put("playing",processor_.running());put("fresh",processor_.mode()!=CueProcessor::Mode::host||(host.valid&&host.age<1));put("timed",d.timed);put("article",processor_.mode()==CueProcessor::Mode::article);put("language",s.settings.language);
 put("foreground",s.settings.foreground);put("background",s.settings.background);put("accent",s.settings.accent);put("mirror",s.settings.mirror);put("font",s.settings.fontSize);put("align",s.settings.align);put("current",pos.line);
 juce::Array<juce::var> rows;int current=std::max(0,pos.line),groupEnd=current;
 while(groupEnd+1<(int)d.lines.size()&&d.lines[(size_t)groupEnd+1].marked&&d.lines[(size_t)groupEnd+1].start==d.lines[(size_t)current].start)++groupEnd;
 put("groupEnd",pos.line<0?-1:groupEnd);put("context",s.settings.context);
 double duration=0;for(const auto& l:d.lines)if(l.marked)duration=std::max(duration,l.end);put("duration",duration);
 const int context=std::max(5,s.settings.context);int first=std::max(0,current-context),last=std::min((int)d.lines.size()-1,groupEnd+context);
 while(first>0&&d.lines[(size_t)first].marked&&d.lines[(size_t)first-1].marked&&d.lines[(size_t)first-1].start==d.lines[(size_t)first].start)--first;
 while(last+1<(int)d.lines.size()&&d.lines[(size_t)last].marked&&d.lines[(size_t)last+1].marked&&d.lines[(size_t)last+1].start==d.lines[(size_t)last].start)++last;
 for(int i=first;i<=last;++i){const auto& l=d.lines[(size_t)i];juce::DynamicObject::Ptr row=new juce::DynamicObject();row->setProperty("marked",l.marked);row->setProperty("text",l.text);row->setProperty("singer",l.singer);row->setProperty("colour",l.colour);row->setProperty("index",i);row->setProperty("start",l.start);row->setProperty("end",l.end);juce::Array<juce::var> words;
  for(const auto&w:l.words){juce::DynamicObject::Ptr word=new juce::DynamicObject();word->setProperty("text",w.text);word->setProperty("start",w.start);word->setProperty("end",w.end);words.add(juce::var(word.get()));}row->setProperty("words",words);rows.add(juce::var(row.get()));
 }
 put("rows",rows);auto json=juce::JSON::toString(juce::var(root.get()),true);std::lock_guard<std::mutex> lock(mutex_);snapshot_=std::move(json);
}
void Projection::run(){
 std::map<juce::String,double> lastSeek;
 while(!threadShouldExit()){
  if(listener_.waitUntilReady(true,100)<=0)continue;
  std::unique_ptr<juce::StreamingSocket> client(listener_.waitForNextConnection());if(!client)continue;
  // Bound connection time, headers and payload; no chunking or keep-alive.
  juce::String request;const auto deadline=juce::Time::getMillisecondCounterHiRes()+350;
  bool complete=false, malformed=false;int headerBytes=0,bodyLength=0;
  juce::StringPairArray headers;juce::String line;
  while(!threadShouldExit()&&request.getNumBytesAsUTF8()<8192&&juce::Time::getMillisecondCounterHiRes()<deadline){
   if(client->waitUntilReady(true,25)<=0)continue;char buffer[1024];int n=client->read(buffer,sizeof(buffer),false);if(n<=0)break;
   for(int i=0;i<n;++i)if(buffer[i]==0)malformed=true;
   request+=juce::String::fromUTF8(buffer,n);if(request.getNumBytesAsUTF8()>8192)malformed=true;if(malformed)break;
   if(headerBytes==0&&request.contains("\r\n\r\n")){
    const auto head=request.upToFirstOccurrenceOf("\r\n\r\n",false,false);headerBytes=(int)head.getNumBytesAsUTF8()+4;
    auto lines=juce::StringArray::fromLines(head);line=lines[0];
    for(int i=1;i<lines.size();++i){auto at=lines[i].indexOfChar(':');if(at<=0){malformed=true;break;}auto key=lines[i].substring(0,at).trim().toLowerCase();if(headers.containsKey(key)){malformed=true;break;}headers.set(key,lines[i].substring(at+1).trim());}
    const auto length=headers["content-length"];if(headers.containsKey("transfer-encoding")||(!length.isEmpty()&&(!length.containsOnly("0123456789")||length.length()>4)))malformed=true;
    bodyLength=length.getIntValue();if(bodyLength>512)malformed=true;
   }
   if(malformed)break;if(headerBytes>0&&(int)request.getNumBytesAsUTF8()>=headerBytes+bodyLength){complete=true;break;}
  }
  const auto prefix="/"+token_+"/";juce::String body="Not found",status="404 Not Found",type="text/plain; charset=utf-8";
  if(complete&&!malformed&&line=="GET "+prefix+" HTTP/1.1"){body=U(cueProjectionPage);status="200 OK";type="text/html; charset=utf-8";}
  else if(complete&&!malformed&&line=="GET "+prefix+"state HTTP/1.1"){std::lock_guard<std::mutex> lock(mutex_);body=snapshot_;status="200 OK";type="application/json; charset=utf-8";}
  else if(complete&&!malformed&&line=="POST "+prefix+"seek HTTP/1.1"){
   type="application/json; charset=utf-8";status="400 Bad Request";body=R"({"accepted":false,"message":"Invalid seek request"})";
   const auto origin=headers["origin"],host=headers["host"];
   const bool sameOrigin=host.isNotEmpty()&&origin=="http://"+host&&headers["x-cueflow-seek"]=="1"&&headers["content-type"].toLowerCase()=="application/json";
   if(!sameOrigin){status="403 Forbidden";body=R"({"accepted":false,"message":"Same-origin request required"})";}
   else {
    auto payload=juce::JSON::parse(request.fromFirstOccurrenceOf("\r\n\r\n",false,false));auto row=payload["row"],rev=payload["revision"];const auto revision=rev.toString();uint64_t parsed=0;bool valid=revision.isNotEmpty()&&revision.length()<=20&&revision.containsOnly("0123456789");
    for(auto c:revision){const auto digit=(uint64_t)(c-'0');if(parsed>(UINT64_MAX-digit)/10){valid=false;break;}parsed=parsed*10+digit;}
    if((row.isInt()||row.isInt64())&&(juce::int64)row>=0&&(juce::int64)row<=1000000&&rev.isString()&&valid){
     auto r=std::make_shared<SeekRequest>();r->row=(int)row;r->revision=parsed;
     std::unique_lock<std::mutex> lock(mutex_);
     const auto now=juce::Time::getMillisecondCounterHiRes();const auto address=client->getHostName();
     for(auto it=lastSeek.begin();it!=lastSeek.end();)if(now-it->second>2000)it=lastSeek.erase(it);else ++it;
     const auto previous=lastSeek.find(address);
     if(seeks_.size()>=16||lastSeek.size()>=64||(previous!=lastSeek.end()&&now-previous->second<200)){status="429 Too Many Requests";body=R"({"accepted":false,"message":"Seek queue full; try again"})";}
     else {lastSeek[address]=now;seeks_.push_back(r);seekChanged_.wait_for(lock,std::chrono::milliseconds(250),[&]{return r->done;});
      const bool expired=!r->done&&!r->started;
      if(expired){seeks_.erase(std::remove(seeks_.begin(),seeks_.end(),r),seeks_.end());r->done=true;r->message="Seek timed out and was cancelled; tap again / 定位超时已取消，请重新点击";}
      juce::DynamicObject::Ptr result=new juce::DynamicObject();result->setProperty("queued",!r->done);result->setProperty("accepted",r->done&&r->accepted);result->setProperty("message",r->done?r->message:juce::String("Queued; playback position is not confirmed"));body=juce::JSON::toString(juce::var(result.get()),true);status=expired?"408 Request Timeout":!r->done?"202 Accepted":r->accepted?"200 OK":"409 Conflict";
     }
    }
   }
  }else if(!complete||malformed){status="400 Bad Request";body="Malformed or oversized request";}
  auto response="HTTP/1.1 "+status+"\r\nContent-Type: "+type+"\r\nContent-Length: "+juce::String(body.getNumBytesAsUTF8())+"\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nReferrer-Policy: no-referrer\r\nContent-Security-Policy: default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src 'self'; frame-ancestors 'none'\r\nConnection: close\r\n\r\n"+body;
  auto bytes=response.toUTF8();int sent=0,total=(int)response.getNumBytesAsUTF8();const auto until=juce::Time::getMillisecondCounterHiRes()+1000;
  while(sent<total&&!threadShouldExit()&&juce::Time::getMillisecondCounterHiRes()<until){if(client->waitUntilReady(false,50)<=0)continue;int n=client->write(bytes.getAddress()+sent,std::min(1024,total-sent));if(n<=0)break;sent+=n;}
 }
}
