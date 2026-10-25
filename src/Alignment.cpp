#include "Alignment.h"
namespace cue {
bool alignedState(const State&base,const juce::var&result,State&out,juce::String&error){
 auto fail=[&](const char*message){error=U(message);return false;};
 if(!result.isObject()||!result["version"].isInt()||(int)result["version"]!=1)return fail("Invalid alignment result / 对齐结果无效");
 auto* rows=result["lines"].getArray();if(!rows||rows->isEmpty()||rows->size()>2000)return fail("Invalid alignment line count / 对齐行数无效");
 auto root=juce::JSON::parse(encodeState(base));auto document=root["document"];document.getDynamicObject()->setProperty("format","KTV-AUTO");document.getDynamicObject()->setProperty("timed",true);
 juce::Array<juce::var> lines;double previous=-1;size_t oldIndex=0;
 for(const auto&row:*rows){
  if(!row.isObject()||!row["text"].isString()||row["text"].toString().isEmpty()||!row["words"].isArray()||row["words"].size()==0)return fail("Missing word timing / 缺少逐字时间");
  auto copy=juce::JSON::parse(juce::JSON::toString(row));auto obj=copy.getDynamicObject();obj->setProperty("marked",true);obj->setProperty("id",0);obj->setProperty("singer","");obj->setProperty("colour","");
  if((double)row["start"]<previous)return fail("Alignment is out of order / 对齐时间顺序异常");previous=(double)row["start"];
  for(size_t i=oldIndex;i<base.document.lines.size();++i)if(base.document.lines[i].text.trim()==row["text"].toString()){obj->setProperty("singer",base.document.lines[i].singer);obj->setProperty("colour",base.document.lines[i].colour);oldIndex=i+1;break;}
  lines.add(copy);
 }
 document.getDynamicObject()->setProperty("lines",lines);
 State validated;if(!decodeState(juce::JSON::toString(root),validated,error))return false;
 out=std::move(validated);error.clear();return true;
}
juce::File AlignmentJob::configFile(){return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("AmuMusic/CueFlow/alignment.json");}
bool AlignmentJob::start(const juce::File&audio,const juce::String&lyrics,const juce::String&language,const juce::String&model,juce::String&error){
 if(running_){error=U("Alignment is already running / 已在对齐中");return false;}
 auto config=juce::JSON::parse(configFile().loadFileAsString());
 auto pythonPath=config["python"].toString(),scriptPath=config["script"].toString(),modelPath=config["models"].toString();
 if(!juce::File::isAbsolutePath(pythonPath)||!juce::File::isAbsolutePath(scriptPath)||!juce::File::isAbsolutePath(modelPath)||!juce::File(pythonPath).existsAsFile()||!juce::File(scriptPath).existsAsFile()){
  error=U("Local alignment engine is not installed. Run setup-alignment.ps1 from the release package / 请先运行发行包中的 setup-alignment.ps1 安装本地对齐引擎");return false;
 }
 if(!audio.existsAsFile()||lyrics.trim().isEmpty()||lyrics.length()>30000||(language!="zh"&&language!="en")||(model!="small"&&model!="base")){error=U("Choose reference vocals and valid sung lyrics / 请选择参考人声并填写演唱歌词");return false;}
 cancel();finished_=false;result_=juce::var();progress_=0;message_=U("Starting local alignment / 正在启动本地对齐");
 folder_=juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("AmuMusic/CueFlow/alignment-jobs").getChildFile(juce::Uuid().toString());
 if(folder_.createDirectory().failed()){error=U("Cannot create alignment work folder / 无法创建对齐工作目录");return false;}
 resultFile_=folder_.getChildFile("result.json");progressFile_=folder_.getChildFile("progress.json");auto request=folder_.getChildFile("request.json");
 juce::var data(new juce::DynamicObject());auto set=[&](const char*k,const juce::var&v){data.getDynamicObject()->setProperty(k,v);};
 set("audio",audio.getFullPathName());set("lyrics",lyrics);set("language",language);set("model",model);set("model_dir",modelPath);set("result",resultFile_.getFullPathName());set("progress",progressFile_.getFullPathName());
 if(!request.replaceWithText(juce::JSON::toString(data),false,false)||!process_.start(juce::StringArray{pythonPath,scriptPath,"--request",request.getFullPathName()},0)){error=U("Could not start local alignment / 无法启动对齐进程");return false;}
 running_=true;error.clear();return true;
}
void AlignmentJob::poll(){
 if(!running_)return;
 if(progressFile_.getSize()<65536){auto status=juce::JSON::parse(progressFile_.loadFileAsString());if(status.isObject()){message_=status["message"].toString();progress_=(double)status["progress"];}}
 if(process_.isRunning())return;
 running_=false;
 if(process_.getExitCode()!=0||!resultFile_.existsAsFile()||resultFile_.getSize()>16*1024*1024){message_=U("Alignment failed; original lyrics unchanged / 对齐失败，原歌词未改动。 ")+message_;finished_=false;return;}
 result_=juce::JSON::parse(resultFile_.loadFileAsString());finished_=result_.isObject();progress_=finished_?1:0;
 if(!finished_)message_=U("Invalid alignment result / 对齐结果无效");
}
void AlignmentJob::cancel(){if(running_&&process_.isRunning()){process_.kill();process_.waitForProcessToFinish(1000);}running_=false;finished_=false;result_=juce::var();progress_=0;message_=U("Cancelled; original lyrics unchanged / 已取消，原歌词未改动");}
}
