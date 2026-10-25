"""One-time mechanically checked port of LDDC's GPL-3.0-only QRC primitive.
Keeps the nonstandard QRC S-box and key permutation verbatim.
"""
from pathlib import Path
import ast
root=Path(__file__).resolve().parents[2]
tree=ast.parse((Path(__file__).parent/'tripledes.py').read_text('utf-8'))
func={n.name:n for n in tree.body if isinstance(n,ast.FunctionDef)}
def expr(n): return ast.unparse(n).replace('//','/')
sbox=ast.literal_eval(next(n.value for n in tree.body if isinstance(n,ast.Assign) and n.targets[0].id=='sbox'))
qtree=ast.parse((Path(__file__).parent/'qmc1.py').read_text('utf-8'))
key=ast.literal_eval(next(n.value for n in qtree.body if isinstance(n,ast.Assign)))
out=['''// Adapted from LDDC, Copyright (C) 2024-2025 沉默の金 <cmzj@cmzj.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/chenmozhijin/LDDC/tree/main/LDDC/core/decryptor
#include "Lyrics.h"
#include <array>
#include <cstdint>
#include <stdexcept>
namespace cue { namespace {
using u32=uint32_t; using Block=std::array<uint8_t,8>;
using Schedule=std::array<std::array<uint8_t,6>,16>;
u32 bitnum(const uint8_t* a,int b,int c){return ((a[(b/32)*4+3-(b%32)/8]>>(7-b%8))&1u)<<c;}
u32 bitnum_intr(u32 a,int b,int c){return ((a>>(31-b))&1u)<<c;}
u32 bitnum_intl(u32 a,int b,int c){return ((a<<b)&0x80000000u)>>c;}
int sbox_bit(int a){return (a&32)|((a&31)>>1)|((a&1)<<4);}
''']
out+=['const u32 sbox[8][64]={'+','.join('{'+','.join(map(str,row))+'}' for row in sbox)+'};']
ret=next(n for n in func['initial_permutation'].body if isinstance(n,ast.Return))
out+=['std::pair<u32,u32> initial_permutation(const uint8_t* input_data){return {'+','.join(expr(x) for x in ret.value.elts)+'};}']
out+=['Block inverse_permutation(u32 s0,u32 s1){Block data{};']
for n in func['inverse_permutation'].body:
 if isinstance(n,ast.Assign) and isinstance(n.targets[0],ast.Subscript):out += [expr(n.targets[0])+' = '+expr(n.value)+';']
out+=['return data;}','u32 f(u32 state,const std::array<uint8_t,6>& key){']
for n in func['f'].body:
 if isinstance(n,ast.Assign):
  name=expr(n.targets[0])
  if name in ('t1','t2'):out+=['u32 '+name+' = '+expr(n.value)+';']
  elif name=='lrgstate' and isinstance(n.value,ast.Tuple):out+=['std::array<u32,6> lrgstate={'+','.join(expr(x) for x in n.value.elts)+'};']
  elif name=='lrgstate':out+=['for(int i=0;i<6;++i)lrgstate[i]^=key[i];']
  elif name=='state':out+=['state = '+expr(n.value)+';']
 elif isinstance(n,ast.Return):out+=['return '+expr(n.value)+';']
out+=['}','Schedule schedule(const uint8_t* key,bool decrypt){Schedule schedule{};']
for n in func['key_schedule'].body:
 if isinstance(n,ast.Assign) and isinstance(n.value,ast.Tuple):out+=['const int '+n.targets[0].id+'[]={'+','.join(expr(x) for x in n.value.elts)+'};']
out+=['''u32 c=0,d=0;
for(int i=0;i<28;++i){c|=bitnum(key,key_perm_c[i],31-i);d|=bitnum(key,key_perm_d[i],31-i);}
for(int i=0;i<16;++i){c=((c<<key_rnd_shift[i])|(c>>(28-key_rnd_shift[i])))&0xfffffff0u;d=((d<<key_rnd_shift[i])|(d>>(28-key_rnd_shift[i])))&0xfffffff0u;
int togen=decrypt?15-i:i;
for(int j=0;j<24;++j)schedule[togen][j/8]|=(uint8_t)bitnum_intr(c,key_compression[j],7-j%8);
for(int j=24;j<48;++j)schedule[togen][j/8]|=(uint8_t)bitnum_intr(d,key_compression[j]-27,7-j%8);
}return schedule;}
Block crypt(const uint8_t* input,const Schedule& key){auto [s0,s1]=initial_permutation(input);for(int i=0;i<15;++i){auto prev=s1;s1=f(s1,key[i])^s0;s0=prev;}s0=f(s1,key[15])^s0;return inverse_permutation(s0,s1);}
''']
out+=['const uint8_t qmc[]={'+','.join(map(str,key))+'};','''}
juce::MemoryBlock decodeQrc(const juce::MemoryBlock& input,bool local){
 auto bytes=input;
 if(local){if(bytes.getSize()<19)throw std::runtime_error("QRC 文件过短");auto*p=(uint8_t*)bytes.getData();for(size_t i=0;i<bytes.getSize();++i)p[i]^=qmc[(i>0x7fff?i%0x7fff:i)&0x7f];bytes.removeSection(0,11);}
 if(bytes.isEmpty()||bytes.getSize()%8!=0)throw std::runtime_error("QRC 加密数据长度无效");
 static const uint8_t key[]="!@#)(*$%123ZXC!@!@#)(NHL";
 static const std::array<Schedule,3> keys={schedule(key+16,true),schedule(key+8,false),schedule(key,true)};
 auto*p=(uint8_t*)bytes.getData();for(size_t i=0;i<bytes.getSize();i+=8){auto b=crypt(p+i,keys[0]);b=crypt(b.data(),keys[1]);b=crypt(b.data(),keys[2]);std::memcpy(p+i,b.data(),8);}return bytes;
}
}
''']
(root/'src/QrcDecode.cpp').write_text('\n'.join(out),'utf-8')
print('Ported QRC primitive to C++')
