#pragma once
#include <vector>
#include "types.h"

namespace neon {

// 与 web 版 levels.ts 对应的关卡数据
inline const std::vector<LevelDefinition> LEVELS = {
  {
    "霓虹启程", {"NNNNNNNNNN", "NBNNNNNNBN", "NNTNTTNTNN", "NGNNRRNNGN"},
    330, 132, {}, 0.18f,
  },
  {
    "脉冲回廊", {"..TTTTTT..", ".NBNRNNBN.", "NNTI..ITNN", ".NNEXXNN..", "..GNNNNG.."},
    365, 118, {1, 3}, 0.20f,
  },
  {
    "量子壁垒", {"I.NTTTTN.I", "NBT....TBN", "NNEXIIXENN", ".TRGNNGRT.", "..NBBBBN.."},
    395, 108, {2}, 0.22f,
  },
  {
    "核心风暴", {"IITTTTTTII", "TBNXXXXNBT", "NEXI..IXEN", "TTNNBBNNTT", ".EXNNNNXE.", "I..GRRG..I"},
    425, 98, {1, 4}, 0.24f,
  },
  {
    "彩虹矩阵", {"RGRGRGRGRG", "GRBGRGRBRG", "RGRGGGGRGR", "GRG....GRG", "RGRGRGRGRG"},
    440, 92, {1, 3}, 0.26f,
  },
};

// 英文关卡名(与 web 版 main.ts 一致)
inline const char* englishLevelNames[5] = { "Neon Dawn", "Pulse Corridor", "Quantum Barrier", "Core Storm", "Rainbow Matrix" };

} // namespace neon
