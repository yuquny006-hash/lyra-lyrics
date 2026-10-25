#pragma once
#include <juce_events/juce_events.h>
#include <juce_core/juce_core.h>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <memory>
class CueProcessor;
class Projection final : private juce::Thread, private juce::Timer {
public:
 explicit Projection(CueProcessor&);
 ~Projection() override;
 bool start(); void stop(); bool active() const { return port_ > 0; }
 juce::String urls() const;
 juce::String localUrl() const;
 static bool isPhoneAddress(const juce::String& ip){auto parts=juce::StringArray::fromTokens(ip,".","");if(parts.size()!=4)return false;for(auto part:parts)if(part.isEmpty()||!part.containsOnly("0123456789")||part.getIntValue()>255)return false;const int first=parts[0].getIntValue();return first>0&&first<224&&first!=127&&!(first==169&&parts[1].getIntValue()==254);}
 void publish();
private:
 void run() override;
 void timerCallback() override;
 struct SeekRequest { int row; uint64_t revision; bool done=false, accepted=false, started=false; juce::String message; };
 std::deque<std::shared_ptr<SeekRequest>> seeks_;
 std::condition_variable seekChanged_;
 CueProcessor& processor_;
 juce::StreamingSocket listener_;
 int port_=0;
 juce::String token_, snapshot_;
 std::mutex mutex_;
};
