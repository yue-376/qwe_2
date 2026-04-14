/*
  * 文件：common.h
 * 说明：通用工具函数和常量定义头文件
 * 
 * 本文件定义了系统中使用的：
 * - 常用字符串长度常量
 * - 布尔类型枚举
 * - 输入输出工具函数声明
 * - 输入验证函数声明
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ==================== 字符串长度常量定义 ==================== */
#define NAME_LEN 32       /* 姓名字段最大长度 */
#define PHONE_LEN 24      /* 电话号码字段最大长度 */
#define DATE_LEN 16       /* 日期字段最大长度 (YYYY-MM-DD 格式) */
#define SMALL_LEN 16      /* 小文本字段最大长度（如科室、类型等） */
#define TEXT_LEN 128      /* 中等文本字段最大长度（如诊断结果） */
#define LONG_LEN 128      /* 长文本字段最大长度 */
#define MAX_LINE 512      /* 文件读取时单行最大长度 */

/* ==================== 布尔类型定义 ==================== */
typedef enum
{
    FALSE = 0,  /* 假 */
    TRUE = 1    /* 真 */
} Bool;

/* ==================== 输入输出工具函数 ==================== */

/*
 * 说明：去除字符串末尾的换行符
 * 参数：s 要处理的字符串
 */
void trim_newline(char *s);

/*
 * 说明：清空输入缓冲区
 * 读取并丢弃 stdin 中直到换行符的所有字符
 */
void clear_input_buffer(void);

/*
 * 说明：读取一行用户输入
 * 参数：prompt 提示信息（可为 NULL）
 * 参数：buf 存储输入的缓冲区
 * 参数：size 缓冲区大小
 */
void read_line(const char *prompt, char *buf, int size);

/*
 * 说明：安全地复制字符串，防止缓冲区溢出
 * 参数：dst 目标缓冲区
 * 参数：src 源字符串
 * 参数：n 目标缓冲区大小
 */
void safe_copy(char *dst, const char *src, size_t n);

/*
 * 说明：读取指定范围内的整数
 * 参数：prompt 提示信息
 * 参数：minv 最小值
 * 参数：maxv 最大值
 * 返回值：有效的整数值
 */
int read_int(const char *prompt, int minv, int maxv);

/*
 * 说明：读取指定范围内的浮点数
 * 参数：prompt 提示信息
 * 参数：minv 最小值
 * 参数：maxv 最大值
 * 返回值：有效的浮点数值
 */
double read_double(const char *prompt, double minv, double maxv);

/*
 * 说明：读取字符串，可选择是否允许为空
 * 参数：prompt 提示信息
 * 参数：buf 存储输入的缓冲区
 * 参数：n 缓冲区大小
 * 参数：allow_empty 是否允许空字符串
 */
void read_string(const char *prompt, char *buf, size_t n, int allow_empty);

/*
 * 说明：显示是/否提示，等待用户选择
 * 参数：prompt 提示信息
 * 返回值：1 表示是，0 表示否
 */
int ask_yes_no(const char *prompt);

/*
 * 说明：检查字符串是否以指定前缀开头
 * 参数：s 要检查的字符串
 * 参数：prefix 前缀字符串
 * 返回值：1 表示是，0 表示否
 */
int starts_with(const char *s, const char *prefix);

/*
 * 说明：暂停程序，等待用户按回车键继续
 */
void pause_and_wait(void);

/*
 * 说明：忽略大小写比较两个字符串
 * 参数：a 第一个字符串
 * 参数：b 第二个字符串
 * 返回值：1 表示相等，0 表示不等
 */
int str_equal_ignore_case(const char *a, const char *b);

/*
 * 说明：估算 UTF-8 字符串终端显示宽度（ASCII=1，非 ASCII 按 2 处理）
 * 参数：s UTF-8 字符串
 * 返回值：显示宽度
 */
int utf8_display_width(const char *s);

/*
 * 说明：解析当前 UTF-8 字符的字节长度和显示宽度
 * 参数：p 指向当前字符首字节
 * 参数：bytes 输出参数，字符字节长度
 * 参数：width 输出参数，字符显示宽度
 */
void utf8_char_info(const unsigned char *p, int *bytes, int *width);

/*
 * 说明：按显示宽度左对齐打印 UTF-8 字符串
 * 参数：s 字符串
 * 参数：colWidth 目标列宽
 */
void print_utf8_cell(const char *s, int colWidth);

/*
 * 说明：按显示宽度打印 UTF-8 字符串，超宽时截断并补齐空格
 * 参数：s 字符串
 * 参数：colWidth 目标列宽
 */
void print_utf8_cell_fit(const char *s, int colWidth);

/* ==================== 输入验证函数 ==================== */

/*
 * 说明：验证性别输入是否有效
 * 参数：gender 性别字符串
 * 返回值：1 表示有效（"男"或"女"），0 表示无效
 */
int validate_gender(const char *gender);

/*
 * 说明：验证日期格式是否为 YYYY-MM-DD
 * 参数：date 日期字符串
 * 返回值：1 表示有效，0 表示无效
 */
int validate_date(const char *date);

/*
 * 说明：验证手机号是否为 11 位数字且以 1 开头
 * 参数：phone 手机号字符串
 * 返回值：1 表示有效，0 表示无效
 */
int validate_phone(const char *phone);

/*
 * 说明：读取带验证的输入，支持输入"0"返回上一步
 * 参数：prompt 提示信息
 * 参数：buf 存储输入的缓冲区
 * 参数：size 缓冲区大小
 * 参数：validate_func 验证函数指针
 * 参数：error_msg 验证失败时的错误提示
 * 返回值：1 表示成功，0 表示用户选择返回
 */
int read_line_with_validate(const char *prompt, char *buf, int size, 
                            int (*validate_func)(const char *), 
                            const char *error_msg);

#endif
