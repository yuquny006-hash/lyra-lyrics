#pragma once
#include "State.h"
namespace cue {
bool alignedState(const State& base,const juce::var& result,State& out,juce::String& error);
class AlignmentJob {
public:
 ~AlignmentJob(){cancel();}
 bool start(const juce::File& audio,const juce::String& lyrics,const juce::String& language,const juce::String& model,juce::String& error);
 void poll();void cancel();
 bool running()const{return running_;}bool finished()const{return finished_;}
 juce::var result()const{return result_;}juce::String message()const{return message_;}double progress()const{return progress_;}
 static juce::File configFile();
private:
 juce::ChildProcess process_;
 juce::File folder_,resultFile_,progressFile_;
 bool running_=false,finished_=false;
 double progress_=0;
 juce::String message_;juce::var result_;
};
}
