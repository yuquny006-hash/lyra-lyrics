"""Local, explicit-transcript alignment worker. Never uploads audio or modifies inputs."""
from pathlib import Path
import argparse
import contextlib
import json
import math
import os
import sys
import time
import traceback
import unicodedata


def meaningful(char):
    return not char.isspace() and unicodedata.category(char)[0] not in 'PZC'


def key(char):
    return unicodedata.normalize('NFKC', char).casefold()


def map_alignment(lines, tokens, duration):
    """Map forced-aligned tokens back to exact user text; never silently replace lyrics."""
    chars=[]
    split_count=0
    previous=-1.0
    for token in tokens:
        start,end=float(token['start']),float(token['end'])
        if not math.isfinite(start+end) or start<0 or end<start or end>duration+.15 or start<previous-.05:
            raise ValueError('Invalid or non-monotonic alignment timestamps / 时间无效或顺序异常')
        previous=start
        text=[c for c in token['word'] if meaningful(c)]
        if len(text)>1:split_count+=1
        for i,c in enumerate(text):
            chars.append((key(c),start+(end-start)*i/len(text),start+(end-start)*(i+1)/len(text)))
    expected=[key(c) for line in lines for c in line if meaningful(c)]
    if expected!=[x[0] for x in chars]:
        raise ValueError('Aligned text differs from supplied lyrics; result not applied / 对齐文字与歌词不一致，未应用')
    if not chars or sum(e-s<.02 for _,s,e in chars)/len(chars)>.2:
        raise ValueError('Too many words lack reliable timing; check the vocal and sung lyrics / 过多字词没有有效时间，请检查干声和实唱歌词')
    warnings=[]
    if split_count:
        warnings.append(f'{split_count} multi-character tokens use proportional within-token timing; review long notes / {split_count} 个多字词内部按时长分配，请检查拖音')
    cursor=0
    rows=[]
    for line in lines:
        words=[]
        next_time=chars[cursor][1] if cursor<len(chars) else duration
        for c in line:
            if meaningful(c):
                _,start,end=chars[cursor];cursor+=1;next_time=end
            else:start=end=next_time
            words.append({'text':c,'start':round(start,6),'end':round(end,6)})
        if not any(meaningful(c) for c in line):raise ValueError('A lyric line contains no sung characters')
        rows.append({'text':line,'start':min(w['start'] for w in words),'end':max(w['end'] for w in words),'words':words})
    short=sum(e-s<.02 for _,s,e in chars)
    if short:warnings.append(f'{short} near-zero word durations need review / {short} 个字时长过短，需要检查')
    long=sum(e-s>3 for _,s,e in chars)
    if long:warnings.append(f'{long} words last over 3 seconds; check sustained notes / {long} 个字超过3秒，请检查拖音')
    return rows,warnings


def atomic_json(path,data):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    temp=path.with_suffix(path.suffix+'.tmp')
    temp.write_text(json.dumps(data,ensure_ascii=False,allow_nan=False),encoding='utf-8')
    os.replace(temp,path)


def run(request):
    started=time.monotonic()
    progress_path=request['progress']
    def progress(stage,value=-1):atomic_json(progress_path,{'message':stage,'progress':value,'elapsed':time.monotonic()-started})
    lines=[x.strip() for x in request['lyrics'].splitlines() if x.strip()]
    if not lines or len(lines)>2000 or sum(len(x) for x in lines)>30000:raise ValueError('Lyrics must contain 1–2000 lines and at most 30000 characters')
    model_name=request.get('model','small')
    if model_name not in ('base','small'):raise ValueError('Unsupported model')
    language=request.get('language','zh')
    if language not in ('zh','en'):raise ValueError('Unsupported language')
    audio_path=Path(request['audio'])
    if not audio_path.is_file() or audio_path.stat().st_size>2*1024**3:raise ValueError('Audio file missing or exceeds 2 GB')
    progress('Reading reference vocals / 读取参考人声',.02)
    import numpy as np
    import torch
    import torchaudio
    import soundfile as sf
    import stable_whisper
    torch.set_num_threads(min(6,max(1,(os.cpu_count() or 4)//2)))
    info=sf.info(str(audio_path))
    if info.duration<.5 or info.duration>1800:raise ValueError('Reference audio must be between 0.5 seconds and 30 minutes')
    waveform,rate=sf.read(str(audio_path),dtype='float32',always_2d=True)
    waveform=waveform.mean(axis=1)
    if not np.all(np.isfinite(waveform)) or np.max(np.abs(waveform))<.00001:raise ValueError('Reference audio is silent or invalid / 参考音频为空或无效')
    if rate!=16000:waveform=torchaudio.functional.resample(torch.from_numpy(waveform),rate,16000).numpy()
    progress('Loading local model (first use downloads model) / 加载模型，首次使用会下载',.06)
    model=stable_whisper.load_model(model_name,device='cpu',download_root=request['model_dir'])
    progress('Aligning vocals with lyrics; may take several minutes / 正在逐字对齐，可能需要几分钟',-1)
    result=model.align(waveform,'\n'.join(lines),language=language,original_split=True,verbose=None,
                       regroup=False,nonspeech_skip=None,max_word_dur=15,word_dur_factor=None,
                       failure_threshold=.35)
    if result is None:raise ValueError('Alignment failed. Check reference vocals and sung lyrics / 对齐失败，请检查干声与实唱歌词')
    raw=result.to_dict()
    atomic_json(str(request['result'])+'.raw.json',raw)
    tokens=[w for segment in raw['segments'] for w in segment.get('words',[])]
    progress('Checking lyric mapping / 检查歌词与时间',.94)
    rows,warnings=map_alignment(lines,tokens,len(waveform)/16000)
    output={'version':1,'engine':'stable-ts 2.19.1 / Whisper','model':model_name,'language':language,
            'duration':len(waveform)/16000,'lines':rows,'warnings':warnings,'elapsed':time.monotonic()-started}
    atomic_json(request['result'],output)
    atomic_json(str(request['result'])+'.raw.json',raw)
    progress('Ready for review; not yet applied / 已完成，请检查后应用',1)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--request',required=True);args=parser.parse_args()
    request=json.loads(Path(args.request).read_text(encoding='utf-8-sig'))
    log=Path(request['result']).with_suffix('.log')
    with log.open('w',encoding='utf-8') as stream,contextlib.redirect_stdout(stream),contextlib.redirect_stderr(stream):
        try:run(request)
        except Exception as error:
            traceback.print_exc()
            atomic_json(request['progress'],{'message':str(error),'progress':0,'error':True})
            return 1
    return 0

if __name__=='__main__':sys.exit(main())
