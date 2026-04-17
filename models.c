/*
 * 文件：models.c
 * 说明：数据模型管理函数实现文件
 * 
 * 本文件实现了医院管理系统中数据模型的核心管理功能，主要包括两大类操作：
 * 
 * 一、内存释放功能（防止内存泄漏）
 *    系统使用链表存储所有数据（患者、医生、挂号记录等），这些节点都是在程序运行时
 *    动态分配的内存。当程序退出或重新加载数据时，必须释放这些内存，否则会造成内存泄漏。
 *    本文件为每种数据类型都提供了专门的释放函数。
 * 
 * 二、数据库初始化功能
 *    在程序启动时，需要将所有链表指针初始化为 NULL，表示空数据库。
 *    在释放所有数据后，也需要重新初始化，使数据库回到干净的初始状态。
 * 
 * 三、账号管理功能
 *    提供账号查找、登录验证和创建新账号的功能，是用户认证系统的基础。
 * 
 * 数据结构说明：
 * - Patient: 患者信息（病历号、姓名、性别、出生日期、电话、医保类型等）
 * - Doctor: 医生信息（工号、姓名、科室、职称等）
 * - Registration: 挂号记录（挂号号、患者 ID、医生 ID、科室、日期、类型、状态）
 * - Visit: 看诊记录（看诊号、挂号号、诊断结果、检查项目、处方）
 * - Exam: 检查记录（检查号、患者 ID、医生 ID、检查编码、名称、执行时间、费用、结果）
 * - Ward: 病房信息（病房 ID、类型、科室、床位数、占用床位数、维修床位数）
 * - Inpatient: 住院记录（住院号、患者 ID、病房 ID、床位号、入院日期、预计出院日期、总费用）
 * - Drug: 药品信息（药品 ID、通用名、商品名、别名、类型、科室、价格、库存）
 * - DrugLog: 药品出入库日志（日志 ID、药品 ID、操作类型、数量、操作员姓名、日期）
 * - Account: 用户账号（用户名、密码、角色、关联 ID）
 */

#include "models.h"

/*
 * 函数：free_patients - 释放患者链表所有节点
 * 
 * 功能说明：
 *   遍历整个患者链表，逐个释放每个节点占用的内存。
 *   这是防止内存泄漏的关键函数，必须在程序退出或重新加载数据前调用。
 * 
 * 参数：
 *   head - 患者链表的头指针
 * 
 * 工作原理：
 *   1. 使用 while 循环遍历链表，直到头指针为 NULL（表示到达链表末尾）
 *   2. 在释放当前节点之前，先用临时变量 t 保存当前节点的地址
 *   3. 然后将头指针移动到下一个节点（head = head->next）
 *   4. 最后释放临时变量 t 指向的节点
 *   
 *   使用临时变量的原因：如果直接 free(head)，将无法访问 head->next，
 *   会导致链表断裂，后续节点无法释放，造成内存泄漏。
 * 
 * 内存管理提示：
 *   - 每个 Patient 节点都是通过 malloc() 动态分配的
 *   - 释放后这些内存会归还给系统，不能再访问
 *   - 释放完成后，链表头指针仍然指向原来的位置（成为悬空指针），
 *     所以调用此函数后应该将头指针设为 NULL
 */
void free_patients(Patient *head)
{
    while (head)
    {
        Patient *t = head;      /* 保存当前节点地址，防止丢失 */
        head = head->next;      /* 先移动到下一个节点，因为当前节点马上要被释放了 */
        free(t);                /* 释放当前节点占用的内存 */
    }
}

/*
 * 函数：free_doctors - 释放医生链表所有节点
 * 
 * 功能说明：
 *   与 free_patients 完全相同，只是操作的对象是医生链表。
 *   遍历并释放所有医生节点，防止内存泄漏。
 * 
 * 参数：
 *   head - 医生链表的头指针
 */
void free_doctors(Doctor *head)
{
    while (head)
    {
        Doctor *t = head;       /* 保存当前医生节点 */
        head = head->next;      /* 移动到下一个医生节点 */
        free(t);                /* 释放当前节点 */
    }
}

/*
 * 函数：free_regs - 释放挂号记录链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有挂号记录节点。挂号记录是患者就诊的第一步，
 *   包含患者 ID、医生 ID、科室、日期、类型和状态等信息。
 * 
 * 参数：
 *   head - 挂号记录链表的头指针
 */
void free_regs(Registration *head)
{
    while (head)
    {
        Registration *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_visits - 释放看诊记录链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有看诊记录节点。看诊记录包含医生的诊断结果、
 *   检查项目安排和处方信息，是医疗过程的核心记录。
 * 
 * 参数：
 *   head - 看诊记录链表的头指针
 */
void free_visits(Visit *head)
{
    while (head)
    {
        Visit *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_exams - 释放检查记录链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有检查记录节点。检查记录包含各项医学检查的详细信息，
 *   如检查编码、名称、执行时间、费用和检查结果等。
 * 
 * 参数：
 *   head - 检查记录链表的头指针
 */
void free_exams(Exam *head)
{
    while (head)
    {
        Exam *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_wards - 释放病房链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有病房节点。病房信息包括病房类型、所属科室、
 *   总床位数、已占用床位数和维修中的床位数等。
 * 
 * 参数：
 *   head - 病房链表的头指针
 */
void free_wards(Ward *head)
{
    while (head)
    {
        Ward *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_inpatients - 释放住院记录链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有住院记录节点。住院记录包含患者的住院详细信息，
 *   如病房 ID、床位号、入院日期、预计出院日期和累计费用等。
 * 
 * 参数：
 *   head - 住院记录链表的头指针
 */
void free_inpatients(Inpatient *head)
{
    while (head)
    {
        Inpatient *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_drugs - 释放药品链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有药品节点。药品信息包括通用名、商品名、别名、
 *   类型、归属科室、价格和库存数量等。
 * 
 * 参数：
 *   head - 药品链表的头指针
 */
void free_drugs(Drug *head)
{
    while (head)
    {
        Drug *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_druglogs - 释放药品日志链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有药品出入库日志节点。药品日志记录了药品的每一次入库和出库操作，
 *   包括操作类型（入库/出库）、数量、操作员姓名和操作日期，用于药品追溯和管理。
 * 
 * 参数：
 *   head - 药品日志链表的头指针
 */
void free_druglogs(DrugLog *head)
{
    while (head)
    {
        DrugLog *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：free_accounts - 释放账号链表所有节点
 * 
 * 功能说明：
 *   遍历并释放所有用户账号节点。账号信息包含用户名、密码、角色和关联的用户 ID。
 *   
 * 安全提示：
 *   - 账号数据包含敏感信息（密码），实际应用中应该加密存储
 *   - 释放账号内存时，密码也会随之清除，这是一种简单的安全措施
 * 
 * 参数：
 *   head - 账号链表的头指针
 */
void free_accounts(Account *head)
{
    while (head)
    {
        Account *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 函数：init_database - 初始化数据库结构
 * 
 * 功能说明：
 *   将数据库结构体中的所有链表头指针设置为 NULL，表示一个空的数据库。
 *   这是程序启动时必须执行的第一步操作，确保后续操作不会访问到随机内存地址。
 * 
 * 参数：
 *   db - 指向数据库结构体的指针
 * 
 * 初始化的必要性：
 *   - C 语言中，局部变量的初始值是未定义的（可能是任意随机值）
 *   - 如果不初始化为 NULL，这些指针会指向不可预测的内存地址
 *   - 当程序尝试遍历这些"野指针"时，会导致程序崩溃或数据损坏
 * 
 * 调用时机：
 *   1. 程序启动后，创建 Database 变量后立即调用
 *   2. 释放所有数据后（free_database 内部也会调用）
 *   3. 需要清空数据库重新开始的时候
 */
void init_database(Database *db)
{
    /* 将所有链表头指针设为 NULL，表示空链表 */
    db->patients = NULL;      /* 患者链表 */
    db->doctors = NULL;       /* 医生链表 */
    db->registrations = NULL; /* 挂号记录链表 */
    db->visits = NULL;        /* 看诊记录链表 */
    db->exams = NULL;         /* 检查记录链表 */
    db->wards = NULL;         /* 病房链表 */
    db->inpatients = NULL;    /* 住院记录链表 */
    db->drugs = NULL;         /* 药品链表 */
    db->drugLogs = NULL;      /* 药品日志链表 */
    db->accounts = NULL;      /* 账号链表 */
}

/*
 * 函数：free_database - 释放数据库所有内存
 * 
 * 功能说明：
 *   依次调用各个类型的释放函数，清理数据库中所有动态分配的内存，
 *   然后重新初始化数据库结构，使其回到干净的初始状态。
 *   
 * 这是程序退出前必须执行的操作，防止内存泄漏。
 * 
 * 参数：
 *   db - 指向数据库结构体的指针
 * 
 * 释放顺序说明：
 *   按照数据结构定义的顺序依次释放，理论上顺序不重要，
 *   因为各链表之间是独立的（节点间没有交叉引用）。
 *   
 * 重要提示：
 *   - 此函数只释放内存，不删除磁盘上的数据文件
 *   - 下次启动程序时，数据会从文件重新加载
 *   - 释放后数据库处于"空"状态，可以重新加载新数据
 */
void free_database(Database *db)
{
    /* 依次释放所有类型的链表节点 */
    free_patients(db->patients);      /* 释放所有患者记录 */
    free_doctors(db->doctors);        /* 释放所有医生记录 */
    free_regs(db->registrations);     /* 释放所有挂号记录 */
    free_visits(db->visits);          /* 释放所有看诊记录 */
    free_exams(db->exams);            /* 释放所有检查记录 */
    free_wards(db->wards);            /* 释放所有病房记录 */
    free_inpatients(db->inpatients);  /* 释放所有住院记录 */
    free_drugs(db->drugs);            /* 释放所有药品记录 */
    free_druglogs(db->drugLogs);      /* 释放所有药品日志记录 */
    free_accounts(db->accounts);      /* 释放所有账号记录 */
    
    /* 重新初始化数据库结构，将所有指针重置为 NULL */
    /* 这是一个好习惯，可以避免悬空指针问题 */
    init_database(db);
}

/* ==================== 账号管理函数实现 ==================== */

/*
 * 函数：find_account - 根据用户名查找账号
 * 
 * 功能说明：
 *   在账号链表中遍历查找指定用户名的账号记录。
 *   这是用户登录、注册时检查用户名是否存在的核心函数。
 * 
 * 参数：
 *   db - 数据库指针，用于访问账号链表
 *   username - 要查找的用户名（字符串）
 * 
 * 返回值：
 *   - 找到：返回指向该账号节点的指针
 *   - 未找到：返回 NULL
 * 
 * 工作原理：
 *   1. 从账号链表头开始逐个遍历
 *   2. 使用 strcmp 比较每个节点的 username 字段
 *   3. 找到匹配项立即返回该节点指针
 *   4. 如果遍历完整个链表都没找到，返回 NULL
 * 
 * 时间复杂度：O(n)，其中 n 是账号数量
 * 优化建议：如果账号很多，可以考虑使用哈希表提高查找效率
 */
Account *find_account(Database *db, const char *username)
{
    Account *curr = db->accounts;  /* 从链表头开始 */
    while (curr)  /* 遍历直到链表末尾（curr 为 NULL） */
    {
        if (strcmp(curr->username, username) == 0)  /* 找到匹配的 username */
        {
            return curr;  /* 返回该账号节点 */
        }
        curr = curr->next;  /* 移动到下一个节点 */
    }
    return NULL;  /* 遍历结束仍未找到，返回 NULL */
}

/*
 * 函数：authenticate_user - 验证用户登录
 * 
 * 功能说明：
 *   验证用户名和密码是否匹配，用于用户登录时的身份认证。
 *   只有用户名存在且密码完全匹配时才认证成功。
 * 
 * 参数：
 *   db - 数据库指针
 *   username - 用户输入的用户名
 *   password - 用户输入的密码
 * 
 * 返回值：
 *   - 认证成功：返回指向该账号节点的指针
 *   - 认证失败：返回 NULL（用户名不存在或密码错误）
 * 
 * 安全说明：
 *   - 当前实现使用明文密码，实际应用中应该使用加密存储（如 bcrypt、PBKDF2）
 *   - 密码比较使用简单的字符串比较，容易受到时序攻击
 *   - 建议在正式系统中增加登录失败次数限制、验证码等安全机制
 */
Account *authenticate_user(Database *db, const char *username, const char *password)
{
    /* 先根据用户名查找账号 */
    Account *acc = find_account(db, username);
    /* 账号存在且密码匹配时返回账号指针，否则返回 NULL */
    if (acc && strcmp(acc->password, password) == 0)
    {
        return acc;  /* 返回账号信息，供调用者设置会话 */
    }
    return NULL;  /* 认证失败 */
}

/*
 * 函数：create_account - 创建新账号
 * 
 * 功能说明：
 *   在系统中注册一个新的用户账号，包括用户名、密码、角色和关联 ID。
 *   这是用户注册功能的核心函数，会在账号链表中添加一个新节点。
 * 
 * 参数：
 *   db - 数据库指针
 *   username - 用户名（必须是唯一的）
 *   password - 密码（明文存储，实际应用中应加密）
 *   role - 用户角色（患者/医生/管理员）
 *   linkedId - 关联的用户 ID（患者的病历号或医生的工号，管理员为 0）
 * 
 * 返回值：
 *   1 - 创建成功
 *   0 - 创建失败（用户名已存在或内存分配失败）
 * 
 * 处理流程：
 *   1. 首先检查用户名是否已被占用，避免重复注册
 *   2. 为新账号分配内存空间
 *   3. 初始化所有字段（用户名、密码、角色、关联 ID）
 *   4. 将新节点插入到账号链表头部（最高效的插入方式）
 * 
 * 安全注意事项：
 *   - 当前实现使用明文密码，存在安全风险
 *   - 没有密码强度检查，建议增加最小长度、复杂度等验证
 *   - 没有防止暴力破解的机制（如登录失败锁定）
 *   
 * 性能优化：
 *   - 采用"头插法"将新节点插入链表头部，时间复杂度 O(1)
 *   - 如果采用尾插法或其他位置插入，需要遍历链表，效率较低
 */
int create_account(Database *db, const char *username, const char *password, UserRole role, int linkedId)
{
    /* 第一步：检查用户名是否已被占用 */
    /* 用户名必须唯一，不能重复注册 */
    if (find_account(db, username) != NULL)
    {
        return 0;  /* 用户名已存在，返回失败 */
    }
    
    /* 第二步：为新账号分配内存 */
    /* malloc 会从堆区分配一块指定大小的内存 */
    Account *newAcc = (Account *)malloc(sizeof(Account));
    if (!newAcc)
    {
        return 0;  /* 内存分配失败（系统内存不足），返回失败 */
    }
    
    /* 第三步：初始化账号的各个字段 */
    /* 使用 strncpy 而不是 strcpy，防止缓冲区溢出 */
    /* sizeof(newAcc->username) - 1 确保留出空间给结尾的 '\\0' */
    strncpy(newAcc->username, username, sizeof(newAcc->username) - 1);
    newAcc->username[sizeof(newAcc->username) - 1] = '\0';  /* 强制加上字符串结束符 */
    
    strncpy(newAcc->password, password, sizeof(newAcc->password) - 1);
    newAcc->password[sizeof(newAcc->password) - 1] = '\0';  /* 强制加上字符串结束符 */
    
    newAcc->role = role;       /* 设置用户角色 */
    newAcc->linkedId = linkedId;  /* 设置关联的用户 ID */
    newAcc->next = NULL;       /* 新节点的 next 先设为 NULL */
    
    /* 将新节点插入到账号链表头部 */
    /* 这是最高效的插入方式，不需要遍历整个链表 */
    /* 新节点的 next 指针指向原来的头节点，然后更新头指针指向新节点 */
    newAcc->next = db->accounts;  /* 新节点的 next 指向原来的头节点 */
    db->accounts = newAcc;        /* 更新头指针，让它指向新节点 */
    
    return 1;  /* 创建成功 */
}

/*
 * 函数：get_role_name - 获取角色名称字符串
 * 
 * 功能说明：
 *   将 UserRole 枚举值转换为可读的中文角色名称。
 *   用于在界面上显示用户角色，而不是显示数字代码。
 * 
 * 参数：
 *   role - 用户角色枚举值（ROLE_PATIENT/ROLE_DOCTOR/ROLE_MANAGER）
 * 
 * 返回值：
 *   对应的中文字符串：
 *   - ROLE_PATIENT -> "患者"
 *   - ROLE_DOCTOR -> "医生"
 *   - ROLE_MANAGER -> "管理员"
 *   - 其他值 -> "未知角色"
 * 
 * 使用场景：
 *   - 登录成功后显示欢迎信息："欢迎，xxx（角色）"
 *   - 在报表中显示用户的角色类型
 *   - 调试时打印用户信息
 */
const char *get_role_name(UserRole role)
{
    switch (role)
    {
        case ROLE_PATIENT:
            return "患者";      /* 患者角色 */
        case ROLE_DOCTOR:
            return "医生";      /* 医生角色 */
        case ROLE_MANAGER:
            return "管理员";    /* 管理员角色 */
        default:
            return "未知角色";  /* 防御性编程：处理意外的角色值 */
    }
}
