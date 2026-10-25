#pragma once
#include "Processor.h"
#include "Theme.h"
#include "vendor/qrcodegen/qrcodegen.hpp"

inline juce::StringArray projectionPhoneUrls(const juce::String& urls){
 juce::StringArray result;for(auto url:juce::StringArray::fromLines(urls))if(Projection::isPhoneAddress(juce::URL(url).getDomain()))result.add(url);return result;
}

inline juce::Image projectionQrImage(const juce::String& url){
 if(url.isEmpty())return {};
 const auto qr=qrcodegen::QrCode::encodeText(url.toRawUTF8(),qrcodegen::QrCode::Ecc::MEDIUM);
 const int unit=6,size=(qr.getSize()+8)*unit;
 juce::Image image(juce::Image::RGB,size,size,true);juce::Graphics g(image);g.fillAll(juce::Colours::white);g.setColour(juce::Colours::black);
 for(int y=0;y<qr.getSize();++y)for(int x=0;x<qr.getSize();++x)if(qr.getModule(x,y))g.fillRect((x+4)*unit,(y+4)*unit,unit,unit);
 return image;
}

class ProjectionQR:public juce::Component,private juce::Timer{
public:
 ProjectionQR(CueProcessor& p,std::function<void()> close):processor_(p),close_(std::move(close)){
  setComponentID("projection-qr-panel");for(auto*c:std::initializer_list<juce::Component*>{&title_,&help_,&address_,&back_,&copy_,&networks_})addAndMakeVisible(c);
  title_.setFont(style::font(28,true));help_.setFont(style::font(17));address_.setFont(style::font(14));address_.setJustificationType(juce::Justification::centred);address_.setMinimumHorizontalScale(1);
  networks_.setComponentID("projection-network");copy_.setComponentID("projection-copy-selected");back_.setComponentID("projection-qr-back");
  back_.onClick=[this]{setVisible(false);if(close_)close_();};copy_.onClick=[this]{if(selected_.isNotEmpty())juce::SystemClipboard::copyTextToClipboard(selected_);};networks_.onChange=[this]{select();};
  refresh();startTimer(500);
 }
 void paint(juce::Graphics&g)override{
  g.fillAll(getLookAndFeel().findColour(juce::ListBox::backgroundColourId));
  if(image_.isValid()){const int modules=image_.getWidth()/6;const int unit=std::max(1,std::min(qrBounds_.getWidth(),qrBounds_.getHeight())/modules);const int size=unit*modules;auto box=juce::Rectangle<int>(size,size).withCentre(qrBounds_.getCentre());g.setImageResamplingQuality(juce::Graphics::lowResamplingQuality);g.drawImage(image_,box.toFloat());}
 }
 void resized()override{auto r=getLocalBounds().reduced(30);auto top=r.removeFromTop(42);back_.setBounds(top.removeFromRight(180));title_.setBounds(top);r.removeFromTop(14);help_.setBounds(r.removeFromTop(72));r.removeFromTop(12);networks_.setBounds(r.removeFromTop(36));r.removeFromTop(8);auto bottom=r.removeFromBottom(40);copy_.setBounds(bottom.withSizeKeepingCentre(220,40));r.removeFromBottom(8);address_.setBounds(r.removeFromBottom(44));qrBounds_=r.reduced(8);repaint();}
 juce::String selectedUrl()const{return selected_;}
 juce::Image qrImage()const{return image_;}
private:
 juce::String tr(const char*zh,const char*en)const{return U(processor_.state().settings.language?en:zh);}
 void timerCallback()override{if(isVisible())refresh();}
 void refresh(){
  title_.setText(tr("手机扫码看歌词","Scan to read lyrics"),juce::dontSendNotification);back_.setButtonText(tr("返回设置","Back to settings"));copy_.setButtonText(tr("复制当前地址","Copy selected address"));
  const auto urls=processor_.projection().urls();if(urls!=last_){last_=urls;auto previous=selected_;choices_=projectionPhoneUrls(urls);networks_.clear(juce::dontSendNotification);for(int i=0;i<choices_.size();++i)networks_.addItem(juce::URL(choices_[i]).getDomain(),i+1);networks_.setSelectedId(std::max(0,choices_.indexOf(previous))+1,juce::dontSendNotification);select();}
  const bool available=!choices_.isEmpty();networks_.setEnabled(available);copy_.setEnabled(available);
  help_.setText(!processor_.projection().active()?tr("投屏已关闭，二维码已失效。返回设置重新开启。","Projection stopped. Return to settings to start a new session."):!available?tr("没有可用的局域网地址。请连接与手机相同的 Wi-Fi 或路由器，再重新开启投屏。","No LAN address. Connect this computer and phone to the same network, then restart projection."):tr("手机与电脑连接同一网络，用相机扫码后在浏览器打开。转动手机可切换横竖屏；如打不开，请切换下方网卡地址，并检查防火墙。只投歌词，不传伴奏。","Connect phone and computer to the same network. Scan with the camera and open in a browser. Rotate for landscape or portrait. If unavailable, try another network address or check the firewall. Lyrics only; no audio."),juce::dontSendNotification);
 }
 void select(){const int index=networks_.getSelectedId()-1;selected_=index>=0&&index<choices_.size()?choices_[index]:juce::String{};image_=projectionQrImage(selected_);address_.setText(selected_,juce::dontSendNotification);repaint();}
 CueProcessor& processor_;std::function<void()>close_;juce::Label title_,help_,address_;juce::TextButton back_,copy_;juce::ComboBox networks_;juce::StringArray choices_;juce::String last_{"unset"},selected_;juce::Image image_;juce::Rectangle<int>qrBounds_;
};
