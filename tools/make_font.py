#!/usr/bin/env python3
"""从系统 Noto Sans CJK Bold 提取简体中文 face 并子集化为游戏内置字体(跨平台用)。

用法:
    python tools/make_font.py [--src 字体路径]

未指定 --src 时按顺序查找:
    1. 环境变量 NOTO_CJK_BOLD
    2. Linux 系统字体 /usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc
    3. 仓库内下载源 tools/fonts-src/NotoSansCJKsc-Bold.otf(Windows/macOS 用)

.ttc 取简体中文 face(fontNumber=2),.otf 直接读取。
依赖 fontTools(纯 Python 包)。
"""
import argparse
import glob
import os
import re
import sys

from fontTools.ttLib import TTFont
from fontTools import subset

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "assets", "fonts", "NotoSansSC-Bold-subset.otf")

CANDIDATES = [
    os.environ.get("NOTO_CJK_BOLD", ""),
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc",
    os.path.join(HERE, "fonts-src", "NotoSansCJKsc-Bold.otf"),
]


def find_source(explicit):
    """返回 (路径, fontNumber);找不到则退出。"""
    paths = ([explicit] if explicit else []) + CANDIDATES
    for path in paths:
        if path and os.path.exists(path):
            # .ttc 的 face 顺序:0=JP 1=KR 2=SC 3=TC 4=HK
            return path, (2 if path.lower().endswith(".ttc") else -1)
    sys.exit(
        "找不到 Noto Sans CJK 源字体。请用 --src 指定,或下载:\n"
        "  https://raw.githubusercontent.com/notofonts/noto-cjk/main/"
        "Sans/OTF/SimplifiedChinese/NotoSansCJKsc-Bold.otf\n"
        "保存到 tools/fonts-src/NotoSansCJKsc-Bold.otf"
    )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", help="Noto Sans CJK 源字体(.ttc 或 .otf)")
    args = ap.parse_args()
    src, font_number = find_source(args.src)
    print(f"源字体: {src}" + (" (face 2/SC)" if font_number == 2 else ""))

    # 1) 从源码提取全部非 ASCII 字符
    chars = set()
    sources = glob.glob(os.path.join(HERE, "..", "src", "*.h")) + \
              glob.glob(os.path.join(HERE, "..", "src", "*.cpp"))
    for path in sources:
        with open(path, encoding="utf-8", errors="ignore") as f:
            text = f.read()
        for ch in text:
            if ord(ch) > 127:
                chars.add(ch)
        # 以 UTF-8 字节转义书写的字符(如 \xe7\xac\xac)不会出现在文本中,解码后补充
        for m in re.finditer(r"(?:\\x[0-9a-fA-F]{2})+", text):
            raw = bytes(int(h, 16) for h in re.findall(r"\\x([0-9a-fA-F]{2})", m.group(0)))
            try:
                chars.update(raw.decode("utf-8"))
            except UnicodeDecodeError:
                pass

    # 2) 手工符号表(源码中为 \xe2\x86\xbb 等转义,需显式补充)
    #    仅收录 Noto Sans CJK 实际有的符号:✦(U+2726)、↻(U+21BB) 不存在,已改用 ＊/♻
    chars.update("♪♥◆★＊♻×·←→—…↑↓《》「」『』【】、。，！？；：（）￥")

    # 3) ASCII 可打印(含空格 0x20——缺空格会让 SDL_ttf 用 .notdef 豆腐块占位)
    chars.update(chr(c) for c in range(32, 127))

    text = "".join(sorted(chars))
    print(f"字符集: {len(chars)} 字符")

    font = TTFont(src, fontNumber=font_number)
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


if __name__ == "__main__":
    main()
