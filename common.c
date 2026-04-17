/*
 * 文件：common.c
 * 说明：通用工具函数实现文件
 * 
 * 本文件实现了系统中常用的工具函数。
 * 
 * 主要功能包括：
 * - 字符串处理（去除换行符、安全复制）
 * - 输入处理（读取行、整数、验证等）
 * - 输入验证（性别、日期、手机号格式验证）
 */

#include "common.h"

/*
 * 函数：trim_newline - 去除字符串末尾的换行符
 * 参数：s - 待处理的字符串指针
 * 返回：无
 *
 * 功能描述：
 * 该函数用于移除字符串末尾的所有换行符（'\n'）和回车符（'\r'）。
 * 在使用 fgets() 函数读取用户输入时，输入缓冲区中的换行符会被一并读入，
 * 当用户输入"张三"后按下回车键，fgets() 实际读取的内容为"张三\n"。
 * 调用此函数后，字符串将变为"张三"，便于后续处理和比较操作。
 *
 * 处理逻辑：
 * 1. 首先检查输入指针是否为空，避免空指针解引用导致程序崩溃
 * 2. 获取字符串长度，从末尾开始向前遍历
 * 3. 连续删除末尾的'\n'和'\r'字符，直到遇到其他字符或字符串开头
 *
 * 使用场景：
 * - 处理 fgets() 读取的用户输入
 * - 清理文件读取的文本行
 * - 标准化字符串数据格式
 */
void trim_newline(char *s)
{
    if (!s)
        return;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
    {
        s[--len] = '\0';
    }
}

/*
 * 说明：清空输入缓冲区
 * 读取并丢弃 stdin 中直到换行符或 EOF 的所有字符
 * 用于在读取输入后清理缓冲区，避免影响后续输入
 * 
 * 函数用途：
 * scanf() 或 strtol() 读取数字后，换行符会留在输入缓冲区中。
 * 此函数读取并丢弃缓冲区中直到换行符或 EOF 的所有字符，
 * 避免残留的换行符影响后续输入函数的读取。
 * 
 * 使用场景：
 * - 在用 scanf() 或 strtol() 读取数字后
 * - 在混合使用不同类型的输入函数时
 */
void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

/*
 * 说明：读取一行用户输入
 * 参数：prompt 提示信息（可为 NULL，为 NULL 时不显示提示）
 * 参数：buf 存储输入的缓冲区
 * 参数：size 缓冲区大小
 * 
 * 处理流程：
 * 1. 如果提供了提示信息，则显示它（如"请输入姓名："）
 * 2. 使用 fgets 读取一行输入（包括回车键）
 * 3. 去除末尾的换行符（调用 trim_newline 函数）
 * 
 * 函数用途：
 * 封装 fgets() 读取操作，自动显示提示信息并去除末尾换行符。
 * 调用者只需传入提示文本和缓冲区，无需手动处理换行符。
 * 
 * 使用示例：
 *   char name[50];
 *   read_line("请输入您的姓名：", name, sizeof(name));
 *   /* 用户输入"张三"并按回车后，name 中的内容为"张三"（不含换行符） */
 */
void read_line(const char *prompt, char *buf, int size)
{
    if (prompt)
        printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL)
    {
        buf[0] = '\0';
        return;
    }
    trim_newline(buf);
}

/*
 * 说明：安全地复制字符串，防止缓冲区溢出
 * 参数：dst 目标缓冲区（要存放字符串的地方）
 * 参数：src 源字符串（原来的字符串）
 * 参数：n 目标缓冲区大小（最多能放多少个字符）
 * 
 * 函数用途：
 * 限制字符串复制的最大长度，防止目标缓冲区溢出。
 * strncpy() 不保证目标字符串以'\0'结尾，此函数确保结果始终为空终止字符串。
 * 
 * 使用示例：
 *   char smallBuf[10];
 *   safe_copy(smallBuf, "这是一个很长的字符串", sizeof(smallBuf));
 *   /* smallBuf 中的内容为截断后的字符串，且以空字符结尾，不会发生缓冲区溢出 */
 */
void safe_copy(char *dst, const char *src, size_t n)
{
    if (!dst || !src || n == 0)
        return;
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
}

/*
 * 说明：验证性别输入是否有效
 * 参数：gender 性别字符串
 * 返回值：1 表示有效（"男"或"女"），0 表示无效
 * 
 * 函数用途：
 * 验证性别字符串是否为系统接受的标准值（"男"或"女"）。
 * 拒绝非标准输入（如"男性"、"M"、"F"等），确保数据格式一致。
 *
 * 注意：此函数进行精确字符串匹配，不接受带空格或其他变体。
 */
int validate_gender(const char *gender)
{
    if (!gender || strlen(gender) == 0)
        return 0;
    if (strcmp(gender, "男") == 0 || strcmp(gender, "女") == 0)
        return 1;
    return 0;
}

/*
 * 说明：验证日期格式是否为 YYYY-MM-DD
 * 参数：date 日期字符串
 * 返回值：1 表示有效，0 表示无效
 * 
 * 验证规则：
 * 1. 长度必须为 10 个字符（如"2024-01-15"正好 10 个字符）
 * 2. 格式必须为 YYYY-MM-DD（第 5 和第 8 位必须是'-'连字符）
 * 3. 其他位置必须是数字（不能是字母或其他符号）
 * 4. 年份范围：1900-2100（太早或太晚的日期都不接受）
 * 5. 月份范围：1-12（没有 13 月或 0 月）
 * 6. 日期范围：根据月份和闰年计算
 *    - 1 月、3 月、5 月等：最多 31 天
 *    - 4 月、6 月等：最多 30 天
 *    - 2 月：平年 28 天，闰年 29 天
 * 
 * 函数用途：
 * 验证日期字符串是否符合 YYYY-MM-DD 格式且为有效日期。
 * 检查项目包括：格式、长度、字符类型、年月日范围及闰年规则。
 * 拒绝无效日期（如 2024-02-30、2024-13-01 等），确保数据有效性。
 *
 * 闰年判断规则：
 * - 能被 4 整除但不能被 100 整除的年份是闰年（如 2024 年）
 * - 或者能被 400 整除的年份也是闰年（如 2000 年）
 * - 闰年的 2 月有 29 天，平年只有 28 天
 */
int validate_date(const char *date)
{
    if (!date || strlen(date) != 10)
        return 0;
    /* 检查格式：YYYY-MM-DD */
    if (date[4] != '-' || date[7] != '-')
        return 0;
    for (int i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
            continue;
        if (!isdigit((unsigned char)date[i]))
            return 0;
    }
    /* 解析年、月、日 */
    int year = atoi(date);
    int month = atoi(date + 5);
    int day = atoi(date + 8);
    
    /* 验证年份范围 */
    if (year < 1900 || year > 2100)
        return 0;
    
    /* 验证月份范围 */
    if (month < 1 || month > 12)
        return 0;
    
    /* 根据月份验证日期范围 */
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    /* 检查闰年 */
    if (month == 2)
    {
        int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (is_leap)
            days_in_month[2] = 29;
    }
    
    if (day < 1 || day > days_in_month[month])
        return 0;
    
    return 1;
}

/*
 * 说明：验证手机号是否为 11 位数字且以 1 开头
 * 参数：phone 手机号字符串
 * 返回值：1 表示有效，0 表示无效
 * 
 * 验证规则（针对中国大陆手机号）：
 * 1. 必须是 11 位数字（不多不少正好 11 位）
 * 2. 第一位必须是'1'（中国手机号都以 1 开头）
 * 3. 所有位都必须是数字（不能有字母、空格或其他符号）
 * 函数用途：
 * 验证中国大陆手机号格式：11 位数字且以 1 开头。
 * 拒绝带分隔符、长度错误或首位非 1 的输入，确保号码格式统一。
 *
 * 注意：此函数仅验证格式，不验证号码是否真实有效。
 * 也就是说"11111111111"能通过验证（虽然这不是一个真实的号码）。
 */
int validate_phone(const char *phone)
{
    if (!phone)
        return 0;
    size_t len = strlen(phone);
    if (len != 11)
        return 0;
    if (phone[0] != '1')
        return 0;
    for (size_t i = 0; i < len; i++)
    {
        if (!isdigit((unsigned char)phone[i]))
            return 0;
    }
    return 1;
}

/*
 * 说明：读取带验证的输入，支持输入"0"返回上一步
 * 参数：prompt 提示信息（显示给用户的提示）
 * 参数：buf 存储输入的缓冲区（成功验证后会把结果存在这里）
 * 参数：size 缓冲区大小（防止溢出）
 * 参数：validate_func 验证函数指针（用来检查输入是否合法的函数）
 * 参数：error_msg 验证失败时的错误提示（告诉用户哪里错了）
 * 返回值：1 表示成功（输入有效并已保存），0 表示用户选择返回（输入了"0"）
 * 函数用途：
 * 读取带验证的输入，支持用户输入"0"返回上一步。
 * 循环读取并验证用户输入，直到输入有效或用户选择返回。
 *
 * 工作流程：
 * 1. 显示提示信息读取输入
 * 2. 若输入"0"则返回 0（表示返回上一步）
 * 3. 调用验证函数检查输入合法性
 * 4. 验证通过则保存到 buf 并返回 1
 * 5. 验证失败则显示错误信息并重新读取
 *   /* 此时 buf 中已保存验证通过的有效值 */
 */
int read_line_with_validate(const char *prompt, char *buf, int size, 
                            int (*validate_func)(const char *), 
                            const char *error_msg)
{
    char temp[256];
    while (1)
    {
        read_line(prompt, temp, sizeof(temp));
        /* 检查用户是否想返回 */
        if (strcmp(temp, "0") == 0)
        {
            return 0;
        }
        /* 验证输入 */
        if (validate_func(temp))
        {
            safe_copy(buf, temp, size);
            return 1;
        }
        printf("%s\n", error_msg);
    }
}

/*
 * 说明：读取指定范围内的整数
 * 参数：prompt 提示信息（显示给用户的提示）
 * 参数：min 最小值（允许输入的最小数字）
 * 参数：max 最大值（允许输入的最大数字）
 * 返回值：有效的整数值（一定在 min~max 范围内）
 * 函数用途：
 * 读取指定范围内的整数，循环提示直到用户输入有效值。
 * 自动处理空输入、非数字、小数和超出范围等无效情况。
 *
 * 工作流程：
 * 1. 显示提示信息读取输入
 * 2. 检查输入是否为空
 * 3. 使用 strtol() 转换为整数
 * 4. 验证转换是否完全成功且值在 min~max 范围内
 * 5. 验证失败则提示错误并重新读取
 *
 * 特点：
 * - 返回值保证在指定范围内，调用者无需再次验证
 * - 支持负数范围（如 -10 到 10）
 *   /* 返回值保证在 min 到 max 范围内 */
 */
int read_int(const char *prompt, int min, int max)
{
    char line[64];
    char *end;
    long value;
    while (1)
    {
        read_line(prompt, line, sizeof(line));
        if (strlen(line) == 0) {
            printf("输入不能为空，请输入 %d ~ %d 的整数。\n", min, max);
            continue;
        }
        value = strtol(line, &end, 10);
        if (*line != '\0' && *end == '\0' && value >= min && value <= max)
        {
            return (int)value;
        }
        printf("输入无效，请输入 %d ~ %d 的整数。\n", min, max);
    }
}

/*
/*
 * 说明：暂停程序，等待用户按回车键继续
 *
 * 函数用途：
 * 显示"按回车继续..."提示并等待用户按键，用于分页显示信息时控制输出节奏。
 * 调用前需确保输入缓冲区为空（通常先调用 clear_input_buffer()）。
 *
 * 使用场景：
 * - 显示长列表或详细信息后，给用户时间阅读
 * - 执行重要操作前，等待用户确认已准备好
 */
 */
void pause_and_wait(void)
{
    printf("\n按回车继续...");
    getchar();
}

/*
 * 说明：忽略大小写比较两个字符串
 * 参数：a 第一个字符串
 * 参数：b 第二个字符串
 * 返回值：1 表示相等，0 表示不等
 */
int str_equal_ignore_case(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int utf8_display_width(const char *s)
{
    int width = 0;
    const unsigned char *p = (const unsigned char *)s;
    if (!s)
        return 0;
    while (*p)
    {
        int bytes;
        int chWidth;
        utf8_char_info(p, &bytes, &chWidth);
        width += chWidth;
        p += bytes;
    }
    return width;
}

void utf8_char_info(const unsigned char *p, int *bytes, int *width)
{
    if (*p < 0x80)
    {
        *bytes = 1;
        *width = 1;
    }
    else if ((*p & 0xE0) == 0xC0)
    {
        *bytes = (p[1] ? 2 : 1);
        *width = 2;
    }
    else if ((*p & 0xF0) == 0xE0)
    {
        *bytes = (p[1] && p[2] ? 3 : 1);
        *width = 2;
    }
    else if ((*p & 0xF8) == 0xF0)
    {
        *bytes = (p[1] && p[2] && p[3] ? 4 : 1);
        *width = 2;
    }
    else
    {
        *bytes = 1;
        *width = 1;
    }
}

void print_utf8_cell(const char *s, int colWidth)
{
    int width;
    if (!s)
        s = "";
    width = utf8_display_width(s);
    printf("%s", s);
    while (width < colWidth)
    {
        putchar(' ');
        width++;
    }
}

void print_utf8_cell_fit(const char *s, int colWidth)
{
    int width = 0;
    const unsigned char *p;
    int bytes;
    int chWidth;
    if (!s)
        s = "";
    p = (const unsigned char *)s;
    while (*p)
    {
        utf8_char_info(p, &bytes, &chWidth);
        if (width + chWidth > colWidth)
            break;
        printf("%.*s", bytes, (const char *)p);
        width += chWidth;
        p += bytes;
    }
    while (width < colWidth)
    {
        putchar(' ');
        width++;
    }
}
