# Third party notices

CueFlow application source: AGPL-3.0-or-later, except explicitly attributed GPL-3.0-only QRC files combined under GPLv3 section 13.

JUCE 8.0.12: Copyright Raw Material Software Limited. Used under AGPLv3. Source: https://github.com/juce-framework/JUCE/tree/8.0.12 . Full JUCE notices are included in LICENSES/JUCE.md, with module notices preserved in dependency source.

QRC decoding: src/QrcDecode.cpp and research/format-reference/*.py derive from LDDC, Copyright (C) 2024-2025 沉默の金 <cmzj@cmzj.org>, GPL-3.0-only. Source: https://github.com/chenmozhijin/LDDC/tree/main/LDDC/core/decryptor . The generated C++ preserves QRC's modified DES permutation and S-box; it is not a general encryption implementation.

JUCE incorporates zlib, HarfBuzz, SheenBidi and other components with their own notices. See JUCE source and LICENSES/JUCE.md. VST is a trademark of Steinberg Media Technologies GmbH. CueFlow is an independent project and is not affiliated with Steinberg or PreSonus.

pluginval (Tracktion, GPLv3) is used as a development test tool, not shipped as an application dependency. Source: https://github.com/Tracktion/pluginval .

## Optional local alignment runtime (0.4)

The release ships CueFlow's `engine/align.py` and a setup script. Python, alignment dependencies and model weights are downloaded separately; their binaries and model files are not bundled in the release. Package metadata in the configured development runtime identifies:

- stable-ts 2.19.1: MIT. Source and license: https://github.com/jianfch/stable-ts .
- openai-whisper 20250625: MIT. Source, model information and license: https://github.com/openai/whisper .
- PyTorch (torch) 2.5.1+cpu: BSD-3-Clause. Source and notices: https://github.com/pytorch/pytorch/tree/v2.5.1 . PyTorch includes further third-party notices in its installed distribution.
- SoundFile 0.13.1: BSD-3-Clause. Source and notices: https://github.com/bastibe/python-soundfile . Its libsndfile dependency has separate licensing and notices; see the installed wheel and https://github.com/libsndfile/libsndfile .
- CPython 3.12.10: Python Software Foundation License Version 2 and associated notices. License: https://docs.python.org/3.12/license.html . The embedded runtime preserves its `LICENSE.txt`.

The setup also installs torchaudio, NumPy, Numba, llvmlite and transitive dependencies. Their individual license files and notices remain in the installed runtime; this list is not a replacement for those notices. Any future redistribution of the runtime, model weights or compiled dependency binaries requires preserving their applicable licenses and notices.


QR Code generator v1.8.0 (C++), Project Nayuki, MIT. Vendored unmodified in src/vendor/qrcodegen. Source: https://github.com/nayuki/QR-Code-generator/tree/v1.8.0/cpp ; license: LICENSES/qrcodegen-MIT.txt. QR images are generated locally.

CLAP wrapper: free-audio/clap-juce-extensions, MIT, pinned commit 7adee3a1bd4684d4caa5601100e364abccff4b4d. Submodules: clap 29ffcc273be7c7c651f6c9953b99e69700e2387a and clap-helpers a61bcdf0ecc2c8db1e80bfe8bf9cb7e8d9fd2bbc. Sources and original notices are included in Source/third_party/clap-juce-extensions-source.zip. Copies of the three licenses are in LICENSES.

Optional unsigned AAX development builds use JUCE's bundled AAX SDK under its GPLv3 option. Preserve the AAX SDK LICENSE.txt included in the JUCE source archive. AAX development output is not distributed as a signed Pro Tools product.
