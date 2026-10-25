#include <clap/clap.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
static const void* CLAP_ABI extension(const clap_host_t*,const char*){return nullptr;}
static void CLAP_ABI request(const clap_host_t*){}
static uint32_t CLAP_ABI count(const clap_input_events_t*){return 0;}
static const clap_event_header_t* CLAP_ABI event(const clap_input_events_t*,uint32_t){return nullptr;}
static bool CLAP_ABI push(const clap_output_events_t*,const clap_event_header_t*){return true;}
struct Data {std::vector<char> bytes;size_t position=0;};
static int64_t CLAP_ABI write(const clap_ostream_t*s,const void*p,uint64_t n){auto&d=*static_cast<Data*>(s->ctx);auto c=static_cast<const char*>(p);d.bytes.insert(d.bytes.end(),c,c+n);return (int64_t)n;}
static int64_t CLAP_ABI read(const clap_istream_t*s,void*p,uint64_t n){auto&d=*static_cast<Data*>(s->ctx);auto size=std::min<uint64_t>(n,d.bytes.size()-d.position);std::memcpy(p,d.bytes.data()+d.position,(size_t)size);d.position+=(size_t)size;return (int64_t)size;}
int main(int argc,char**argv){
 if(argc!=2)return 2;
#ifdef _WIN32
 auto module=LoadLibraryA(argv[1]);auto entry=module?reinterpret_cast<const clap_plugin_entry_t*>(GetProcAddress(module,"clap_entry")):nullptr;
#else
 auto module=dlopen(argv[1],RTLD_NOW);auto entry=module?static_cast<const clap_plugin_entry_t*>(dlsym(module,"clap_entry")):nullptr;
#endif
 if(!entry||!entry->init(argv[1])){std::cerr<<"FAIL module load/init\n";return 1;}
 auto factory=static_cast<const clap_plugin_factory_t*>(entry->get_factory(CLAP_PLUGIN_FACTORY_ID));
 if(!factory||factory->get_plugin_count(factory)!=1)return 1;
 auto descriptor=factory->get_plugin_descriptor(factory,0);
 if(!descriptor||std::strcmp(descriptor->id,"studio.cueflow.prompter"))return 1;
 clap_host_t host{CLAP_VERSION,nullptr,"Lyra smoke host","Amu music","","1",extension,request,request,request};
 auto p=factory->create_plugin(factory,&host,descriptor->id);
 if(!p||!p->init(p)){std::cerr<<"FAIL plugin init\n";return 1;}
 auto ports=static_cast<const clap_plugin_audio_ports_t*>(p->get_extension(p,CLAP_EXT_AUDIO_PORTS));
 clap_audio_port_info_t info{};
 if(!ports||ports->count(p,true)!=1||!ports->get(p,0,true,&info)||info.channel_count!=2)return 1;
 auto state=static_cast<const clap_plugin_state_t*>(p->get_extension(p,CLAP_EXT_STATE));
 Data data;clap_ostream_t output{&data,write};clap_istream_t input{&data,read};
 if(!state||!state->save(p,&output)||data.bytes.empty()||!state->load(p,&input)){std::cerr<<"FAIL state roundtrip\n";return 1;}
 if(!p->activate(p,48000,16,256)||!p->start_processing(p))return 1;
 float left[256],right[256],ol[256]{},orr[256]{};
 for(int i=0;i<256;++i){left[i]=i/512.0f;right[i]=-left[i];}
 float*ins[]{left,right};float*outs[]{ol,orr};
 clap_audio_buffer_t in{};in.data32=ins;in.channel_count=2;
 clap_audio_buffer_t out{};out.data32=outs;out.channel_count=2;
 clap_input_events_t events{nullptr,count,event};clap_output_events_t emitted{nullptr,push};
 clap_event_transport_t transport{};transport.header.size=sizeof transport;transport.header.type=CLAP_EVENT_TRANSPORT;
 transport.flags=CLAP_TRANSPORT_HAS_SECONDS_TIMELINE|CLAP_TRANSPORT_IS_PLAYING;
 clap_process_t process{};process.frames_count=256;process.transport=&transport;process.audio_inputs=&in;process.audio_outputs=&out;process.audio_inputs_count=1;process.audio_outputs_count=1;process.in_events=&events;process.out_events=&emitted;
 for(int pass=0;pass<3;++pass){transport.song_pos_seconds=(int64_t)((pass==0?32.4:16.22)*CLAP_SECTIME_FACTOR);if(pass==2)transport.flags=CLAP_TRANSPORT_HAS_SECONDS_TIMELINE;process.steady_time=pass*256;if(p->process(p,&process)==CLAP_PROCESS_ERROR)return 1;for(int i=0;i<256;++i)if(ol[i]!=left[i]||orr[i]!=right[i]){std::cerr<<"FAIL passthrough\n";return 1;}}
 p->stop_processing(p);p->deactivate(p);p->destroy(p);entry->deinit();
#ifdef _WIN32
 FreeLibrary(module);
#else
 dlclose(module);
#endif
 std::cout<<"PASS CLAP factory, identity, stereo ports, state roundtrip, transport play/seek/stop processing and audio passthrough\n";
}
