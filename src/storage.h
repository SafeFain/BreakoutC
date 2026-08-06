#pragma once
#include "types.h"

namespace neon {

// 与 web 版 storage.ts 对应:7 字段 KV 文本持久化,含逐字段校验与默认值
StorageData loadStorage();
void saveStorage(const StorageData& data);

} // namespace neon
