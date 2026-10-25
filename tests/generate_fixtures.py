"""Generate original lyric test fixtures using independent LDDC reference crypto.
Only used during development; never needed by the application.
"""
from pathlib import Path
import zlib
root = Path(__file__).resolve().parents[1]
source = (root/'research/format-reference/tripledes.py').read_text('utf-8')
source = source.replace('from LDDC.common.data.cache import cache', '').replace('@cache.memoize()', '')
ns = {}
exec(compile(source, 'LDDC reference tripledes', 'exec'), ns)
qmc = {}
exec(compile((root/'research/format-reference/qmc1.py').read_text('utf-8'), 'LDDC reference qmc1', 'exec'), qmc)
directory = root/'tests/fixtures'
directory.mkdir(exist_ok=True)
krc = '[1000,1500]<0,500,0>你<500,1000,0>好\n[4000,1000]<0,1000,0>世界'
key = b'@Gaw^2tGQ61-\xce\xd2ni'
packed = zlib.compress(krc.encode())
(directory/'original.krc').write_bytes(b'krc1'+bytes(c^key[i%16] for i,c in enumerate(packed)))
xml = '<QrcInfos><Lyric_1 LyricContent="[1000,1500]你(1000,500)好(1500,1000)&#10;[4000,1000]世界(4000,1000)"/></QrcInfos>'
packed = zlib.compress(xml.encode())
packed += b'\0' * (-len(packed)%8)
schedule = ns['tripledes_key_setup'](b'!@#)(*$%123ZXC!@!@#)(NHL', ns['ENCRYPT'])
encrypted = b''.join(ns['tripledes_crypt'](packed[i:i+8], schedule) for i in range(0,len(packed),8))
(directory/'cloud.qrc').write_text(encrypted.hex(), 'utf-8')
local = bytearray(b'[offset:0]\n'+encrypted)
qmc['qmc1_decrypt'](local)
(directory/'local.qrc').write_bytes(local)
(directory/'original.yrc').write_text('[1000,1500](1000,500,0)你(1500,1000,0)好\n[4000,1000](4000,1000,0)世界', 'utf-8')
print('Wrote 4 original fixtures')
