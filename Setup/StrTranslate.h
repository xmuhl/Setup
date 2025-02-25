// StrTranslate.h: 用于在宽字符和多字节字符之间转换。

#pragma once

// 将多字节转换成宽字符字符串
LPWSTR MultiToWChar(LPSTR pMultiByteStr, int iMultiByte);

//从宽字节转化为多字节
LPSTR WCharToMulti(CString pWCharStr);