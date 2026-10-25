// Adapted from LDDC, Copyright (C) 2024-2025 沉默の金 <cmzj@cmzj.org>
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

const u32 sbox[8][64]={{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,3,13,4,7,15,2,8,15,12,0,1,10,6,9,11,5,0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12},{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,3,15,0,6,10,10,13,8,9,4,5,11,12,7,2,14},{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3},{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13},{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12},{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}};
std::pair<u32,u32> initial_permutation(const uint8_t* input_data){return {bitnum(input_data, 57, 31) | bitnum(input_data, 49, 30) | bitnum(input_data, 41, 29) | bitnum(input_data, 33, 28) | bitnum(input_data, 25, 27) | bitnum(input_data, 17, 26) | bitnum(input_data, 9, 25) | bitnum(input_data, 1, 24) | bitnum(input_data, 59, 23) | bitnum(input_data, 51, 22) | bitnum(input_data, 43, 21) | bitnum(input_data, 35, 20) | bitnum(input_data, 27, 19) | bitnum(input_data, 19, 18) | bitnum(input_data, 11, 17) | bitnum(input_data, 3, 16) | bitnum(input_data, 61, 15) | bitnum(input_data, 53, 14) | bitnum(input_data, 45, 13) | bitnum(input_data, 37, 12) | bitnum(input_data, 29, 11) | bitnum(input_data, 21, 10) | bitnum(input_data, 13, 9) | bitnum(input_data, 5, 8) | bitnum(input_data, 63, 7) | bitnum(input_data, 55, 6) | bitnum(input_data, 47, 5) | bitnum(input_data, 39, 4) | bitnum(input_data, 31, 3) | bitnum(input_data, 23, 2) | bitnum(input_data, 15, 1) | bitnum(input_data, 7, 0),bitnum(input_data, 56, 31) | bitnum(input_data, 48, 30) | bitnum(input_data, 40, 29) | bitnum(input_data, 32, 28) | bitnum(input_data, 24, 27) | bitnum(input_data, 16, 26) | bitnum(input_data, 8, 25) | bitnum(input_data, 0, 24) | bitnum(input_data, 58, 23) | bitnum(input_data, 50, 22) | bitnum(input_data, 42, 21) | bitnum(input_data, 34, 20) | bitnum(input_data, 26, 19) | bitnum(input_data, 18, 18) | bitnum(input_data, 10, 17) | bitnum(input_data, 2, 16) | bitnum(input_data, 60, 15) | bitnum(input_data, 52, 14) | bitnum(input_data, 44, 13) | bitnum(input_data, 36, 12) | bitnum(input_data, 28, 11) | bitnum(input_data, 20, 10) | bitnum(input_data, 12, 9) | bitnum(input_data, 4, 8) | bitnum(input_data, 62, 7) | bitnum(input_data, 54, 6) | bitnum(input_data, 46, 5) | bitnum(input_data, 38, 4) | bitnum(input_data, 30, 3) | bitnum(input_data, 22, 2) | bitnum(input_data, 14, 1) | bitnum(input_data, 6, 0)};}
Block inverse_permutation(u32 s0,u32 s1){Block data{};
data[3] = bitnum_intr(s1, 7, 7) | bitnum_intr(s0, 7, 6) | bitnum_intr(s1, 15, 5) | bitnum_intr(s0, 15, 4) | bitnum_intr(s1, 23, 3) | bitnum_intr(s0, 23, 2) | bitnum_intr(s1, 31, 1) | bitnum_intr(s0, 31, 0);
data[2] = bitnum_intr(s1, 6, 7) | bitnum_intr(s0, 6, 6) | bitnum_intr(s1, 14, 5) | bitnum_intr(s0, 14, 4) | bitnum_intr(s1, 22, 3) | bitnum_intr(s0, 22, 2) | bitnum_intr(s1, 30, 1) | bitnum_intr(s0, 30, 0);
data[1] = bitnum_intr(s1, 5, 7) | bitnum_intr(s0, 5, 6) | bitnum_intr(s1, 13, 5) | bitnum_intr(s0, 13, 4) | bitnum_intr(s1, 21, 3) | bitnum_intr(s0, 21, 2) | bitnum_intr(s1, 29, 1) | bitnum_intr(s0, 29, 0);
data[0] = bitnum_intr(s1, 4, 7) | bitnum_intr(s0, 4, 6) | bitnum_intr(s1, 12, 5) | bitnum_intr(s0, 12, 4) | bitnum_intr(s1, 20, 3) | bitnum_intr(s0, 20, 2) | bitnum_intr(s1, 28, 1) | bitnum_intr(s0, 28, 0);
data[7] = bitnum_intr(s1, 3, 7) | bitnum_intr(s0, 3, 6) | bitnum_intr(s1, 11, 5) | bitnum_intr(s0, 11, 4) | bitnum_intr(s1, 19, 3) | bitnum_intr(s0, 19, 2) | bitnum_intr(s1, 27, 1) | bitnum_intr(s0, 27, 0);
data[6] = bitnum_intr(s1, 2, 7) | bitnum_intr(s0, 2, 6) | bitnum_intr(s1, 10, 5) | bitnum_intr(s0, 10, 4) | bitnum_intr(s1, 18, 3) | bitnum_intr(s0, 18, 2) | bitnum_intr(s1, 26, 1) | bitnum_intr(s0, 26, 0);
data[5] = bitnum_intr(s1, 1, 7) | bitnum_intr(s0, 1, 6) | bitnum_intr(s1, 9, 5) | bitnum_intr(s0, 9, 4) | bitnum_intr(s1, 17, 3) | bitnum_intr(s0, 17, 2) | bitnum_intr(s1, 25, 1) | bitnum_intr(s0, 25, 0);
data[4] = bitnum_intr(s1, 0, 7) | bitnum_intr(s0, 0, 6) | bitnum_intr(s1, 8, 5) | bitnum_intr(s0, 8, 4) | bitnum_intr(s1, 16, 3) | bitnum_intr(s0, 16, 2) | bitnum_intr(s1, 24, 1) | bitnum_intr(s0, 24, 0);
return data;}
u32 f(u32 state,const std::array<uint8_t,6>& key){
u32 t1 = bitnum_intl(state, 31, 0) | (state & 4026531840) >> 1 | bitnum_intl(state, 4, 5) | bitnum_intl(state, 3, 6) | (state & 251658240) >> 3 | bitnum_intl(state, 8, 11) | bitnum_intl(state, 7, 12) | (state & 15728640) >> 5 | bitnum_intl(state, 12, 17) | bitnum_intl(state, 11, 18) | (state & 983040) >> 7 | bitnum_intl(state, 16, 23);
u32 t2 = bitnum_intl(state, 15, 0) | (state & 61440) << 15 | bitnum_intl(state, 20, 5) | bitnum_intl(state, 19, 6) | (state & 3840) << 13 | bitnum_intl(state, 24, 11) | bitnum_intl(state, 23, 12) | (state & 240) << 11 | bitnum_intl(state, 28, 17) | bitnum_intl(state, 27, 18) | (state & 15) << 9 | bitnum_intl(state, 0, 23);
std::array<u32,6> lrgstate={t1 >> 24 & 255,t1 >> 16 & 255,t1 >> 8 & 255,t2 >> 24 & 255,t2 >> 16 & 255,t2 >> 8 & 255};
for(int i=0;i<6;++i)lrgstate[i]^=key[i];
state = sbox[0][sbox_bit(lrgstate[0] >> 2)] << 28 | sbox[1][sbox_bit((lrgstate[0] & 3) << 4 | lrgstate[1] >> 4)] << 24 | sbox[2][sbox_bit((lrgstate[1] & 15) << 2 | lrgstate[2] >> 6)] << 20 | sbox[3][sbox_bit(lrgstate[2] & 63)] << 16 | sbox[4][sbox_bit(lrgstate[3] >> 2)] << 12 | sbox[5][sbox_bit((lrgstate[3] & 3) << 4 | lrgstate[4] >> 4)] << 8 | sbox[6][sbox_bit((lrgstate[4] & 15) << 2 | lrgstate[5] >> 6)] << 4 | sbox[7][sbox_bit(lrgstate[5] & 63)];
return bitnum_intl(state, 15, 0) | bitnum_intl(state, 6, 1) | bitnum_intl(state, 19, 2) | bitnum_intl(state, 20, 3) | bitnum_intl(state, 28, 4) | bitnum_intl(state, 11, 5) | bitnum_intl(state, 27, 6) | bitnum_intl(state, 16, 7) | bitnum_intl(state, 0, 8) | bitnum_intl(state, 14, 9) | bitnum_intl(state, 22, 10) | bitnum_intl(state, 25, 11) | bitnum_intl(state, 4, 12) | bitnum_intl(state, 17, 13) | bitnum_intl(state, 30, 14) | bitnum_intl(state, 9, 15) | bitnum_intl(state, 1, 16) | bitnum_intl(state, 7, 17) | bitnum_intl(state, 23, 18) | bitnum_intl(state, 13, 19) | bitnum_intl(state, 31, 20) | bitnum_intl(state, 26, 21) | bitnum_intl(state, 2, 22) | bitnum_intl(state, 8, 23) | bitnum_intl(state, 18, 24) | bitnum_intl(state, 12, 25) | bitnum_intl(state, 29, 26) | bitnum_intl(state, 5, 27) | bitnum_intl(state, 21, 28) | bitnum_intl(state, 10, 29) | bitnum_intl(state, 3, 30) | bitnum_intl(state, 24, 31);
}
Schedule schedule(const uint8_t* key,bool decrypt){Schedule schedule{};
const int key_rnd_shift[]={1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
const int key_perm_c[]={56,48,40,32,24,16,8,0,57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35};
const int key_perm_d[]={62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,60,52,44,36,28,20,12,4,27,19,11,3};
const int key_compression[]={13,16,10,23,0,4,2,27,14,5,20,9,22,18,11,3,25,7,15,6,26,19,12,1,40,51,30,36,46,54,29,39,50,44,32,47,43,48,38,55,33,52,45,41,49,35,28,31};
u32 c=0,d=0;
for(int i=0;i<28;++i){c|=bitnum(key,key_perm_c[i],31-i);d|=bitnum(key,key_perm_d[i],31-i);}
for(int i=0;i<16;++i){c=((c<<key_rnd_shift[i])|(c>>(28-key_rnd_shift[i])))&0xfffffff0u;d=((d<<key_rnd_shift[i])|(d>>(28-key_rnd_shift[i])))&0xfffffff0u;
int togen=decrypt?15-i:i;
for(int j=0;j<24;++j)schedule[togen][j/8]|=(uint8_t)bitnum_intr(c,key_compression[j],7-j%8);
for(int j=24;j<48;++j)schedule[togen][j/8]|=(uint8_t)bitnum_intr(d,key_compression[j]-27,7-j%8);
}return schedule;}
Block crypt(const uint8_t* input,const Schedule& key){auto [s0,s1]=initial_permutation(input);for(int i=0;i<15;++i){auto prev=s1;s1=f(s1,key[i])^s0;s0=prev;}s0=f(s1,key[15])^s0;return inverse_permutation(s0,s1);}

const uint8_t qmc[]={195,74,214,202,144,103,247,82,216,161,102,98,159,91,9,0,195,94,149,35,159,19,17,126,216,146,63,188,144,187,116,14,195,71,116,61,144,170,63,81,216,244,17,132,159,222,149,29,195,198,9,213,159,250,102,249,216,240,247,160,144,161,214,243,195,243,214,161,144,160,247,240,216,249,102,250,159,213,9,198,195,29,149,222,159,132,17,244,216,81,63,170,144,61,116,71,195,14,116,187,144,188,63,146,216,126,17,19,159,35,149,94,195,0,9,91,159,98,102,161,216,82,247,103,144,202,214,74};
}
juce::MemoryBlock decodeQrc(const juce::MemoryBlock& input,bool local){
 auto bytes=input;
 if(local){if(bytes.getSize()<19)throw std::runtime_error("QRC 文件过短");auto*p=(uint8_t*)bytes.getData();for(size_t i=0;i<bytes.getSize();++i)p[i]^=qmc[(i>0x7fff?i%0x7fff:i)&0x7f];bytes.removeSection(0,11);}
 if(bytes.isEmpty()||bytes.getSize()%8!=0)throw std::runtime_error("QRC 加密数据长度无效");
 static const uint8_t key[]="!@#)(*$%123ZXC!@!@#)(NHL";
 static const std::array<Schedule,3> keys={schedule(key+16,true),schedule(key+8,false),schedule(key,true)};
 auto*p=(uint8_t*)bytes.getData();for(size_t i=0;i<bytes.getSize();i+=8){auto b=crypt(p+i,keys[0]);b=crypt(b.data(),keys[1]);b=crypt(b.data(),keys[2]);std::memcpy(p+i,b.data(),8);}return bytes;
}
}
