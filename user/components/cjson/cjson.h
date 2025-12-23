/**
 * @file cjson.h
 * @brief cJSON 组件包装头文件
 * 
 * 这是 cJSON 库的包装头文件，用于简化在项目中引用 cJSON。
 * 直接 include 此文件即可使用 cJSON 的所有功能。
 * 
 * cJSON 是一个超轻量级的 JSON 解析器，适用于嵌入式系统。
 * 
 * @version 1.7.19
 * @see https://github.com/DaveGamble/cJSON
 * 
 * 使用示例:
 * @code
 *   #include "cjson.h"
 *   
 *   // 解析 JSON 字符串
 *   const char *json_str = "{\"name\":\"STM32\",\"value\":123}";
 *   cJSON *json = cJSON_Parse(json_str);
 *   if (json != NULL) {
 *       cJSON *name = cJSON_GetObjectItem(json, "name");
 *       if (cJSON_IsString(name)) {
 *           printf("name: %s\n", name->valuestring);
 *       }
 *       cJSON *value = cJSON_GetObjectItem(json, "value");
 *       if (cJSON_IsNumber(value)) {
 *           printf("value: %d\n", value->valueint);
 *       }
 *       cJSON_Delete(json);
 *   }
 *   
 *   // 创建 JSON 对象
 *   cJSON *root = cJSON_CreateObject();
 *   cJSON_AddStringToObject(root, "device", "STM32F103");
 *   cJSON_AddNumberToObject(root, "temperature", 25.5);
 *   char *output = cJSON_Print(root);
 *   printf("%s\n", output);
 *   cJSON_free(output);
 *   cJSON_Delete(root);
 * @endcode
 * 
 * 嵌入式优化提示:
 * - 如果内存紧张，可以使用 cJSON_PrintUnformatted() 代替 cJSON_Print()
 * - 可以使用 cJSON_InitHooks() 设置自定义的内存分配函数
 * - 可以使用 cJSON_PrintPreallocated() 避免动态内存分配
 * - 可以修改 CJSON_NESTING_LIMIT 限制嵌套深度以减少栈使用
 */

#ifndef __CJSON_COMPONENT_H__
#define __CJSON_COMPONENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 包含官方 cJSON 头文件 */
#include "csrc/cJSON.h"

/*
 * cJSON 版本信息
 * 可以使用 cJSON_Version() 获取字符串格式的版本号
 */
#define CJSON_COMPONENT_VERSION_MAJOR  CJSON_VERSION_MAJOR
#define CJSON_COMPONENT_VERSION_MINOR  CJSON_VERSION_MINOR
#define CJSON_COMPONENT_VERSION_PATCH  CJSON_VERSION_PATCH

#ifdef __cplusplus
}
#endif

#endif /* __CJSON_COMPONENT_H__ */
