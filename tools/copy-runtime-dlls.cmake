# 构建后处理:把可执行文件依赖的 Windows 运行库(.dll)复制到产物目录。
# 由 CMakeLists.txt 在 POST_BUILD 阶段调用:
#   cmake -DDEST=<exe 目录> -DDLLS=<$<TARGET_RUNTIME_DLLS:...>> -P copy-runtime-dlls.cmake
# 静态链接 SDL(或非 Windows 平台)时 DLLS 为空,直接跳过,不报错。
if(NOT DLLS)
  return()
endif()
if(NOT DEST)
  message(FATAL_ERROR "copy-runtime-dlls.cmake: 缺少 -DDEST")
endif()
file(COPY ${DLLS} DESTINATION "${DEST}")
