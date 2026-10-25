#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
// Shared MIDI connection: all instances send through one pair of dedicated ports.
class HostSeekBridge final: private juce::Timer, private juce::MidiInputCallback {
public:
 HostSeekBridge(){refresh();startTimer(1000);}
 ~HostSeekBridge()override{stopTimer();if(input_)input_->stop();}
 bool ready()const{return code_.load()==1&&juce::Time::getMillisecondCounterHiRes()-heartbeat_.load()<3000;}
 int status()const{return juce::Time::getMillisecondCounterHiRes()-heartbeat_.load()<3000?code_.load():0;}
 juce::String diagnostics()const{juce::String s="bridge="+juce::String(status())+" input="+juce::String(input_?1:0)+" output="+juce::String(output_?1:0);for(auto d:juce::MidiInput::getAvailableDevices())s+="\nIN "+d.name;for(auto d:juce::MidiOutput::getAvailableDevices())s+="\nOUT "+d.name;return s;}
 bool send(double seconds){if(!ready()||!output_||!std::isfinite(seconds)||seconds<0||seconds>604800)return false;auto text=juce::String(seconds,3);juce::MemoryBlock bytes;const unsigned char header[]={0x7d,0x4c,0x59,1};bytes.append(header,4);bytes.append(text.toRawUTF8(),text.getNumBytesAsUTF8());output_->sendMessageNow(juce::MidiMessage::createSysExMessage(bytes.getData(),(int)bytes.getSize()));return true;}
private:
 void refresh(){if(!output_)for(auto d:juce::MidiOutput::getAvailableDevices())if(d.name=="Lyra Control"){output_=juce::MidiOutput::openDevice(d.identifier);break;}if(!input_)for(auto d:juce::MidiInput::getAvailableDevices())if(d.name=="Lyra Feedback"){input_=juce::MidiInput::openDevice(d.identifier,this);if(input_)input_->start();break;}}
 void timerCallback()override{if(status()==0){if(input_)input_->stop();input_.reset();output_.reset();}refresh();}
 void handleIncomingMidiMessage(juce::MidiInput*,const juce::MidiMessage&m)override{if(!m.isSysEx()||m.getSysExDataSize()!=5)return;auto*d=m.getSysExData();if(d[0]!=0x7d||d[1]!=0x4c||d[2]!=0x59||d[3]!=2)return;code_.store(d[4]);heartbeat_.store(juce::Time::getMillisecondCounterHiRes());}
 std::unique_ptr<juce::MidiInput> input_;std::unique_ptr<juce::MidiOutput> output_;std::atomic<int> code_{0};std::atomic<double> heartbeat_{0};
};
