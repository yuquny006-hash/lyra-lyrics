#include "Karaoke.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <iostream>

int main() {
    juce::ScopedJuceInitialiser_GUI gui;
    int failed=0;
    const auto check=[&](bool ok,const char* name){if(!ok){++failed;std::cerr<<"FAIL "<<name<<'\n';}};
    using cue::karaoke::progress;
    check(progress(1,2,6)==0,"before start");
    check(progress(2,2,6)==0,"at start");
    check(progress(4,2,6)==.5,"sustained midpoint");
    check(progress(6,2,6)==1&&progress(7,2,6)==1,"end and after");
    check(progress(4,2,6)==.5&&progress(3,2,6)==.25,"seek is absolute");
    check(progress(1,2,2)==0&&progress(2,2,2)==1,"zero duration");
    const std::vector<cue::Word> words={{U("长音"),2,6},{"  office ",6,8},{U("换行 中文"),8,12}};
    for (auto alignment : {juce::Justification::left,juce::Justification::centred,juce::Justification::right}) {
        juce::AttributedString tagged,plain;
        auto font=juce::Font(juce::FontOptions(42));
        tagged.setJustification(alignment);plain.setJustification(alignment);
        for(size_t i=0;i<words.size();++i){tagged.append(words[i].text,font,cue::karaoke::tag(i));plain.append(words[i].text,font,juce::Colours::white);}
        juce::TextLayout layout,reference;
        layout.createLayout(tagged,180);reference.createLayout(plain,180);
        check(layout.getNumLines()>1,"native wrapping");
        check(layout.getNumLines()==reference.getNumLines()&&std::abs(layout.getHeight()-reference.getHeight())<.01,"tagging preserves layout");
        const auto render=[&](double time){juce::Image image(juce::Image::ARGB,220,300,true);juce::Graphics g(image);cue::karaoke::draw(g,layout,{10,10,200,280},alignment,words,time,juce::Colours::white,juce::Colours::red);return image;};
        const auto reds=[](const juce::Image& image){int n=0;for(int y=0;y<image.getHeight();++y)for(int x=0;x<image.getWidth();++x){auto p=image.getPixelAt(x,y);if(p.getAlpha()>100&&p.getRed()>150&&p.getGreen()<100)++n;}return n;};
        const auto equal=[](const juce::Image& a,const juce::Image& b){for(int y=0;y<a.getHeight();++y)for(int x=0;x<a.getWidth();++x)if(a.getPixelAt(x,y)!=b.getPixelAt(x,y))return false;return true;};
        auto before=render(1),quarter=render(3),middle=render(4),end=render(6);
        check(reds(before)==0,"no premature colour");
        check(reds(quarter)>0&&reds(middle)>reds(quarter)&&reds(end)>reds(middle),"continuous partial glyph sweep");
        check(equal(middle,render(4)),"pause renders identically");
        render(12);check(equal(quarter,render(3)),"backseek resets exact coverage");
        juce::Image expected(juce::Image::ARGB,220,300,true);
        { juce::Graphics g(expected);reference.draw(g,{10,10,200,280}); }
        if (!equal(before,expected)) {
            int different=0; for(int y=0;y<before.getHeight();++y)for(int x=0;x<before.getWidth();++x)if(before.getPixelAt(x,y)!=expected.getPixelAt(x,y))++different;
            std::cerr<<"Alignment "<<alignment<<" different pixels "<<different<<" widths "<<layout.getWidth()<<" / "<<reference.getWidth()<<" heights "<<layout.getHeight()<<" / "<<reference.getHeight()<<'\n';
        }
        check(equal(before,expected),"whitespace shaping and alignment match native text");
    }
    std::cout<<(failed?"Karaoke tests failed\n":"Karaoke tests passed\n");
    return failed?1:0;
}
