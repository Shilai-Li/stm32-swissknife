/**
 * @file cjson_tests.c
 * @brief cJSON 组件测试用例
 * 
 * 测试 cJSON 库的基本功能：
 * - JSON 解析
 * - JSON 创建
 * - JSON 打印
 */

#include "cjson.h"
#include "uart.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>

/* ==== Helper Functions ==== */

/**
 * @brief 打印字符串到串口
 */
static void print(const char *str) {
    UART_Send((const uint8_t *)str, strlen(str));
}

/**
 * @brief 打印带换行的字符串
 */
static void println(const char *str) {
    print(str);
    print("\r\n");
}

/**
 * @brief 打印分隔线
 */
static void print_separator(void) {
    println("----------------------------------------");
}

/* ==== Test Cases ==== */

/**
 * @brief 测试 JSON 解析功能
 */
static void test_json_parse(void) {
    println("[TEST] JSON Parse");
    
    const char *json_str = "{\"device\":\"STM32F103\",\"temp\":25.5,\"active\":true,\"count\":100}";
    
    print("Input: ");
    println(json_str);
    
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        println("[FAIL] Parse failed!");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            print("Error before: ");
            println(error_ptr);
        }
        return;
    }
    
    // 获取字符串字段
    cJSON *device = cJSON_GetObjectItem(json, "device");
    if (cJSON_IsString(device) && device->valuestring != NULL) {
        print("  device: ");
        println(device->valuestring);
    }
    
    // 获取数字字段 (浮点)
    cJSON *temp = cJSON_GetObjectItem(json, "temp");
    if (cJSON_IsNumber(temp)) {
        char buf[32];
        snprintf(buf, sizeof(buf), "  temp: %.1f", temp->valuedouble);
        println(buf);
    }
    
    // 获取布尔字段
    cJSON *active = cJSON_GetObjectItem(json, "active");
    if (cJSON_IsBool(active)) {
        print("  active: ");
        println(cJSON_IsTrue(active) ? "true" : "false");
    }
    
    // 获取数字字段 (整数)
    cJSON *count = cJSON_GetObjectItem(json, "count");
    if (cJSON_IsNumber(count)) {
        char buf[32];
        snprintf(buf, sizeof(buf), "  count: %d", count->valueint);
        println(buf);
    }
    
    cJSON_Delete(json);
    println("[PASS] JSON Parse");
}

/**
 * @brief 测试 JSON 创建功能
 */
static void test_json_create(void) {
    println("[TEST] JSON Create");
    
    // 创建根对象
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        println("[FAIL] Failed to create root object!");
        return;
    }
    
    // 添加各种类型的字段
    cJSON_AddStringToObject(root, "name", "STM32-SwissKnife");
    cJSON_AddNumberToObject(root, "version", 1.0);
    cJSON_AddBoolToObject(root, "debug", 1);
    cJSON_AddNumberToObject(root, "uptime_ms", 12345);
    
    // 添加嵌套对象
    cJSON *sensors = cJSON_AddObjectToObject(root, "sensors");
    cJSON_AddNumberToObject(sensors, "temperature", 23.5);
    cJSON_AddNumberToObject(sensors, "humidity", 65);
    
    // 添加数组
    cJSON *gpio_pins = cJSON_AddArrayToObject(root, "gpio_pins");
    cJSON_AddItemToArray(gpio_pins, cJSON_CreateNumber(13));
    cJSON_AddItemToArray(gpio_pins, cJSON_CreateNumber(14));
    cJSON_AddItemToArray(gpio_pins, cJSON_CreateNumber(15));
    
    // 打印 JSON (带格式化)
    char *json_formatted = cJSON_Print(root);
    if (json_formatted != NULL) {
        println("Formatted output:");
        println(json_formatted);
        cJSON_free(json_formatted);
    }
    
    // 打印 JSON (不带格式化，节省空间)
    char *json_compact = cJSON_PrintUnformatted(root);
    if (json_compact != NULL) {
        println("Compact output:");
        println(json_compact);
        cJSON_free(json_compact);
    }
    
    cJSON_Delete(root);
    println("[PASS] JSON Create");
}

/**
 * @brief 测试 JSON 数组解析
 */
static void test_json_array(void) {
    println("[TEST] JSON Array");
    
    const char *json_str = "{\"values\":[10,20,30,40,50]}";
    
    print("Input: ");
    println(json_str);
    
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        println("[FAIL] Parse failed!");
        return;
    }
    
    cJSON *values = cJSON_GetObjectItem(json, "values");
    if (!cJSON_IsArray(values)) {
        println("[FAIL] 'values' is not an array!");
        cJSON_Delete(json);
        return;
    }
    
    int size = cJSON_GetArraySize(values);
    char buf[64];
    snprintf(buf, sizeof(buf), "  Array size: %d", size);
    println(buf);
    
    print("  Values: ");
    cJSON *element = NULL;
    cJSON_ArrayForEach(element, values) {
        if (cJSON_IsNumber(element)) {
            snprintf(buf, sizeof(buf), "%d ", element->valueint);
            print(buf);
        }
    }
    println("");
    
    cJSON_Delete(json);
    println("[PASS] JSON Array");
}

/**
 * @brief 测试使用预分配缓冲区打印 JSON（避免动态内存分配）
 */
static void test_json_preallocated(void) {
    println("[TEST] JSON Preallocated Print");
    
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "msg", "Hello");
    cJSON_AddNumberToObject(root, "val", 42);
    
    // 使用预分配的缓冲区
    char buffer[128];
    if (cJSON_PrintPreallocated(root, buffer, sizeof(buffer), 0)) {
        print("  Output: ");
        println(buffer);
        println("[PASS] JSON Preallocated Print");
    } else {
        println("[FAIL] Buffer too small!");
    }
    
    cJSON_Delete(root);
}

/**
 * @brief 显示版本信息
 */
static void show_version(void) {
    println("=== cJSON Component Test ===");
    print("Version: ");
    println(cJSON_Version());
    print_separator();
}

/* ==== Main Entry ==== */

void User_Entry(void) {
    // 初始化延时
    Delay_Init();
    
    // 等待串口稳定
    Delay_ms(500);
    
    // 显示版本
    show_version();
    
    // 运行测试
    test_json_parse();
    print_separator();
    
    test_json_create();
    print_separator();
    
    test_json_array();
    print_separator();
    
    test_json_preallocated();
    print_separator();
    
    println("=== All Tests Completed ===");
    
    // 主循环
    while (1) {
        Delay_ms(1000);
    }
}
