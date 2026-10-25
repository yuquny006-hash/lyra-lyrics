#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "State.h"
#include <mutex>
#include <optional>
#include "Projection.h"
#include "HostSeekBridge.h"
class ReaderWindow;
class CueProcessor : public juce::AudioProcessor, public juce::ChangeBroadcaster {
public:
 enum class Mode { host, preview, article };
 struct Transport { double seconds=0; bool playing=false,valid=false; double age=0; };
 CueProcessor();
 ~CueProcessor() override;
 void showReader(bool fullscreen=false);
 ReaderWindow* readerWindow()const{return readerWindow_.get();}
 Projection& projection(){return *projection_;}
 const juce::String getName()const override{return "CueFlow";}
 void prepareToPlay(double,int)override{}
 void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 void processBlock(juce::AudioBuffer<double>&,juce::MidiBuffer&)override;
 void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&)override{publishClock();}
 void processBlockBypassed(juce::AudioBuffer<double>&,juce::MidiBuffer&)override{publishClock();}
 bool supportsDoublePrecisionProcessing()const override{return true;}
 juce::AudioProcessorEditor* createEditor()override;
 bool hasEditor()const override{return true;}
 bool acceptsMidi()const override{return false;}
 bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 0;}
 int getNumPrograms()override{return 1;} int getCurrentProgram()override{return 0;} void setCurrentProgram(int)override{}
 const juce::String getProgramName(int)override{return {};}
 void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;
 void setStateInformation(const void*,int)override;
 cue::State state()const;
 std::pair<cue::State,uint64_t> snapshot()const;
 bool commitIfRevision(const cue::State&,uint64_t);
 bool commit(const cue::State&);
 void undo();void redo();bool canUndo()const;bool canRedo()const;
 void markSaved();bool dirty()const;
 juce::String lastError()const;
 Transport transport()const;
 void setMode(Mode);Mode mode()const{return mode_;}
 bool seekLine(int row,uint64_t expectedRevision);
 bool canSeekLyrics()const;
 juce::String seekStatus()const;
 void seek(double);void togglePlay();bool running()const;
 double displayTime()const;
 uint64_t revision()const{return revision_.load();}
private:
 juce::SharedResourcePointer<HostSeekBridge> seekBridge_;
 juce::String seekNotice_;
 std::atomic<bool> hostRecording_{false};
 std::atomic<double> hostOrigin_{0};
 std::unique_ptr<Projection> projection_;
 std::unique_ptr<ReaderWindow> readerWindow_;
 void publishClock();
 bool commitChecked(const cue::State&,std::optional<uint64_t>);
 mutable std::mutex mutex_;
 cue::State state_;
 juce::String saved_,error_;
 std::vector<juce::String> undo_,redo_;
 std::atomic<uint64_t> revision_{0};
 std::atomic<unsigned> sequence_{0};
 std::atomic<double> hostTime_{0},hostStamp_{0};
 std::atomic<bool> hostPlaying_{false},hostValid_{false};
 Mode mode_=Mode::host;
 double manualTime_=0,manualStamp_=0;
 bool manualRunning_=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueProcessor)
};
