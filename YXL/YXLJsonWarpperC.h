#ifndef _YXL_JSON_WARPPER_C_H_
#define _YXL_JSON_WARPPER_C_H_

#ifdef __cplusplus
extern "C"
{
#endif

void* CJsonLoad(const char* data, const int len);

void* CJsonGetRoot(void* json);

void* CJsonGetChild(void* par, const char* child);

int CJsonGetInt(void* node);

void CJsonGetInt2(void* node, int* dst);

void CJsonRelease(void* json);

#ifdef __cplusplus
}
#endif

#endif
