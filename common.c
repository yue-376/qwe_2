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
 * 说明：去除字符串末尾的换行符
 * 参数：s 要处理的字符串
 * 
 * 处理流程：
 * 1. 检查指针是否为空（防止程序崩溃）
 * 2. 从字符串末尾开始，删除所有的 '\n' 和 '\r' 字符
 * 
 * 为什么需要这个函数？
 * 当你用 fgets() 读取用户输入时，它会连回车键一起读进来，
 * 比如你输入"张三"然后按回车，fgets 会读成"张三\n"。
 * 这个函数的作用就是把末尾那个多余的"\n"去掉，只保留"张三"。
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
 * 为什么需要这个函数？
 * 当你让用户输入一个数字，用户输入"123"然后按回车，
 * 程序读取了"123"，但回车键产生的'\n'还留在输入缓冲区里。
 * 下一次你用 getchar() 读取字符时，会直接读到那个残留的'\n'，
 * 这会导致程序跳过用户的输入，直接执行下一步。
 * 
 * 这个函数的作用是清理干净缓冲区里残留的字符，
 * 确保下次读取输入时能正确获取用户的新输入。
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
 * 1. 如果提供了提示信息，则显示它（比如"请输入姓名："）
 * 2. 使用 fgets 读取一行输入（包括回车键）
 * 3. 去除末尾的换行符（调用 trim_newline 函数）
 * 
 * 为什么需要这个函数？
 * 直接用 fgets() 读取输入有两个问题：
 * - 每次都要写提示信息，很麻烦
 * - 读进来的字符串末尾会有换行符，需要手动去掉
 * 
 * 这个函数把这两件事都自动化了，你只需要调用 read_line()，
 * 它就会帮你显示提示、读取输入、去掉换行符。
 * 
 * 使用示例：
 *   char name[50];
 *   read_line("请输入您的姓名：", name, sizeof(name));
 *   // 用户输入"张三"后，name 里就是"张三"（没有换行符）
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
 * 为什么需要这个函数？
 * 如果你有一个只能装 10 个字符的数组 char name[10]，
 * 有人想把"张三是一个非常非常长的名字"（20 个字符）放进去，
 * 如果直接用 strcpy()，多出来的字符会溢出到旁边的内存，
 * 这可能导致程序崩溃、数据损坏，甚至被黑客利用（这就是著名的"缓冲区溢出攻击"）。
 * 
 * 这个函数的作用：
 * - 它会先检查 dst 和 src 是否为空指针（防止程序崩溃）
 * - 它只复制 n-1 个字符，留一个位置给字符串结束符'\0'
 * - 确保无论源字符串多长，都不会超出目标缓冲区的容量
 * 
 * 使用示例：
 *   char smallBuf[10];
 *   safe_copy(smallBuf, "这是一个很长的字符串", sizeof(smallBuf));
 *   // smallBuf 里会是"这是一个很"（自动截断，不会溢出）
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
 * 为什么需要这个函数？
 * 用户输入是不可靠的！有人可能输入：
 * - "男性"、"女性"（多了个"性"字）
 * - "M"、"F"（英文缩写）
 * - "未知"、"其他"（系统不支持的值）
 * - 甚至是一串乱码
 * 
 * 这个函数只接受标准的"男"或"女"，
 * 其他任何输入都会被拒绝，确保数据库里的数据格式统一。
 * 
 * 注意：这个函数只做简单的字符串比较，不做大小写转换，
 * 所以"男"有效，但"男 "（带空格）就无效。
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
 * 1. 长度必须为 10 个字符（比如"2024-01-15"正好 10 个字符）
 * 2. 格式必须为 YYYY-MM-DD（第 5 和第 8 位必须是'-'连字符）
 * 3. 其他位置必须是数字（不能是字母或其他符号）
 * 4. 年份范围：1900-2100（太早或太晚的日期都不接受）
 * 5. 月份范围：1-12（没有 13 月或 0 月）
 * 6. 日期范围：根据月份和闰年计算
 *    - 1 月、3 月、5 月等：最多 31 天
 *    - 4 月、6 月等：最多 30 天
 *    - 2 月：平年 28 天，闰年 29 天
 * 
 * 为什么需要这个函数？
 * 用户可能会输入各种奇怪的日期：
 * - "2024/01/15"（用了斜杠而不是连字符）
 * - "2024-13-01"（不存在的 13 月）
 * - "2024-02-30"（2 月哪有 30 天？）
 * - "二零二四年一月十五日"（中文日期，系统不认识）
 * 
 * 这个函数会严格检查每一个细节，确保只有合法有效的日期才能通过。
 * 这样可以避免数据库里出现"2024-02-30"这种错误数据。
 * 
 * 闰年判断规则（重要！）：
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
 * 
 * 为什么需要这个函数？
 * 用户可能会输入各种格式的手机号：
 * - "138-1234-5678"（带连字符）
 * - "138 1234 5678"（带空格）
 * - "1381234567"（只有 10 位，少了一位）
 * - "23812345678"（不是以 1 开头）
 * - "13812345678abc"（后面带了字母）
 * 
 * 这个函数会严格检查，确保只有标准格式的 11 位手机号才能通过。
 * 这样可以保证数据库里的手机号格式统一，方便后续联系患者或医生。
 * 
 * 注意：这个函数只检查格式，不检查号码是否真实存在，
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
 * 
 * 这个函数的作用是什么？
 * 当你需要用户在多步骤表单中输入数据时，这个函数允许用户在每一步输入"0"返回上一步重新修改。
 * 
 * 工作流程：
 * 1. 显示提示信息，让用户输入
 * 2. 如果用户输入"0"，立即返回 0（表示要返回上一步）
 * 3. 否则调用验证函数检查输入是否合法
 * 4. 如果合法，保存到 buf 并返回 1（成功）
 * 5. 如果不合法，显示错误信息，让用户重新输入（循环回到第 1 步）
 * 
 * 使用示例：
 *   char gender[10];
 *   if (!read_line_with_validate("请输入性别:", gender, sizeof(gender), 
 *                                validate_gender, "性别必须是\"男\"或\"女\"")) {
 *       // 用户输入了"0"，需要返回上一步
 *       return;
 *   }
 *   // 到这里 gender 里已经是验证过的有效值了
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
 * 
 * 这个函数的作用是什么？
 * 当你需要用户输入一个数字，而且这个数字必须在某个范围内时，
 * 这个函数就会派上用场。它会自动帮你做以下事情：
 * - 显示提示信息
 * - 读取用户输入
 * - 检查是否是有效的整数
 * - 检查是否在指定范围内
 * - 如果无效，告诉用户并让用户重新输入
 * 
 * 工作流程：
 * 1. 显示提示信息，等待用户输入
 * 2. 如果输入为空，提示"输入不能为空"
 * 3. 尝试把输入的字符串转换成整数
 * 4. 检查转换是否成功（排除"abc"这种非数字输入）
 * 5. 检查数字是否在 min~max 范围内
 * 6. 如果以上都通过，返回该数字；否则提示错误并回到第 1 步
 * 
 * 特点：
 * - 循环读取直到用户输入有效的整数，不会退出程序
 * - 自动处理各种无效输入（字母、小数、超出范围等）
 * - 返回值一定是有效的，调用者不需要再次验证
 * 
 * 使用示例：
 *   int age = read_int("请输入年龄 (1-150): ", 1, 150);
 *   // 用户无论输入"abc"、"-5"、"200"还是"25.5"都会被拒绝
 *   // 只有输入 1~150 之间的整数才会被接受
 *   // 到这里 age 一定是一个 1~150 之间的有效整数
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
 * 说明：暂停程序，等待用户按回车键继续
 * 
 * 这个函数的作用是什么？
 * 当程序显示了一大段信息（比如患者病历），但屏幕只能显示这么多时，
 * 后面的内容会立即出现，用户还没来得及看完，信息就被刷掉了。
 * 
 * 这个函数就是用来解决这个问题的：
 * - 它显示"按回车继续..."的提示
 * - 然后停下来等待用户按下回车键
 * - 用户看完信息后按回车，程序才继续执行
 * 
 * 使用场景：
 * - 显示完重要信息后，让用户有时间阅读
 * - 在执行危险操作前，让用户确认已经准备好
 * - 在分步操作中，给用户一个喘息的机会
 * 
 * 注意：这个函数假设输入缓冲区是空的，
 * 如果前面有残留的换行符，它会立即返回而不会等待。
 * 所以通常在调用它之前要先清空输入缓冲区。
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
