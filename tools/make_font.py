#!/usr/bin/env python3
"""从系统 NotoSansCJK-Bold.ttc 提取 SC face 并子集化为游戏内置字体(跨平台用)。"""
import glob
import os
import re
import sys

from fontTools.ttLib import TTFont
from fontTools import subset

SRC = "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc"
OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "fonts", "NotoSansSC-Bold-subset.otf")

# 1) 从源码提取全部非 ASCII 字符
chars = set()
for path in glob.glob(os.path.join(os.path.dirname(__file__), "..", "src", "*.h")) + \
            glob.glob(os.path.join(os.path.dirname(__file__), "..", "src", "*.cpp")):
    text = open(path, encoding="utf-8", errors="ignore").read()
    for ch in text:
        if ord(ch) > 127:
            chars.add(ch)

# 2) 手工符号表(源码中为 \xe2\x86\xbb 等转义,需显式补充)
chars.update("↻♪♥◆★✦×·←→—…↑↓《》「」『』【】、。，！？；：（）￥")

# 3) ASCII 可打印(数字/字母/标点)
chars.update(chr(c) for c in range(33, 127))

text = "".join(sorted(chars))
print(f"字符集: {len(chars)} 字符")

font = TTFont(SRC, fontNumber=2)
options = subset.Options()
options.layout_features = []
options.name_IDs = [1, 2, 3, 4, 5, 6]
options.recalc_bounds = True
options.notdef_outline = True
sub = subset.Subsetter(options)
sub.populate(text=text)
sub.subset(font)
font.save(OUT)
print(f"输出: {os.path.abspath(OUT)} ({os.path.getsize(OUT) // 1024} KB)")
