// Lyra transport bridge. Cubase MIDI Remote API v1, ES5.
var api = require('midiremote_api_v1');
var driver = api.makeDeviceDriver('AmuMusic', 'Lyra', 'Amu music');
var midiIn = driver.mPorts.makeMidiInput('Lyra Control');
var midiOut = driver.mPorts.makeMidiOutput('Lyra Feedback');
driver.makeDetectionUnit().detectPortPair(midiIn, midiOut).expectInputNameEquals('Lyra Control').expectOutputNameEquals('Lyra Feedback');
var page = driver.mMapping.makePage('Lyra lyrics');
var primary = page.mHostAccess.mTransport.mTimeDisplay.mPrimary.mTransportLocator;
var secondary = page.mHostAccess.mTransport.mTimeDisplay.mSecondary.mTransportLocator;
var mapping = null, active = false, recording = false, selected = null, lastBeat = 0;
var formats = [false, false];
function observe(which, locator) { locator.mOnChange = function(device, am, time, format) {
    mapping = am;
    // Seconds is formatted H:MM:SS.mmm, unlike bars/beats or frame timecode.
    formats[which] = /^-?\d+:\d{2}:\d{2}\.\d{3}$/.test(time);
    selected = formats[1] ? secondary : formats[0] ? primary : null;
}; }
observe(0, primary); observe(1, secondary);
page.mOnActivate = function(device, am) { mapping = am; active = true; };
page.mOnDeactivate = function() { active = false; mapping = null; };
page.mHostAccess.mTransport.mValue.mRecord.mOnProcessValueChange = function(device, am, value) { recording = value > 0; };
function status(device) { midiOut.sendMidi(device, [0xf0,0x7d,0x4c,0x59,2,!active?4:recording?3:selected?1:2,0xf7]); }
driver.mOnIdle = function(device) { var now = new Date().getTime(); if(now-lastBeat>=500){lastBeat=now;status(device);} };
function pad(n, size) { var s = String(n);while(s.length<size)s='0'+s;return s; }
function formatTime(seconds) { var ms=Math.round(seconds*1000);return pad(Math.floor(ms/3600000),2)+':'+pad(Math.floor(ms/60000)%60,2)+':'+pad(Math.floor(ms/1000)%60,2)+'.'+pad(ms%1000,3); }
midiIn.mOnSysex = function(device, msg) {
    if(msg.length<7||msg.length>24||msg[0]!==0xf0||msg[1]!==0x7d||msg[2]!==0x4c||msg[3]!==0x59||msg[4]!==1||msg[msg.length-1]!==0xf7)return;
    if(!active||!mapping||!selected||recording){status(device);return;}
    var text='';for(var i=5;i<msg.length-1;i++)text+=String.fromCharCode(msg[i]);
    if(!/^\d{1,6}\.\d{3}$/.test(text))return;
    var seconds=Number(text);if(!isFinite(seconds)||seconds<0||seconds>604800)return;
    selected.setTime(mapping,formatTime(seconds));
};
