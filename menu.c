#include "menu.h"
#include "data.h"
#include "auth.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ==================== 全局登录会话 ==================== */
UserSession g_session = {0, ROLE_PATIENT, 0, ""};

/* ==================== 登录与注册功能 ==================== */

/*
 * 说明：注册菜单函数
 * 参数：db 数据库指针
 */
void register_menu(Database *db) {
    char username[32], password[64], confirm[64];
    int roleChoice, linkedId = 0;
    UserRole role;
    
    printf("\n=== 用户注册 ===\n");
    
    printf("请输入用户名：");
    read_line("", username, sizeof(username));
    if (strlen(username) == 0) {
        printf("用户名不能为空。\n");
        return;
    }
    
    // 检查用户名是否已存在
    if (find_account(db, username) != NULL) {
        printf("用户名已存在。\n");
        return;
    }
    
    // 循环输入密码直到满足要求
    while (1) {
        printf("请输入密码（至少 4 位）：");
        read_line("", password, sizeof(password));
        if (strlen(password) < 4) {
            printf("密码长度至少为 4 位，请重新输入。\n");
            continue;
        }
        
        // 确认密码
        printf("请确认密码：");
        read_line("", confirm, sizeof(confirm));
        if (strcmp(password, confirm) != 0) {
            printf("两次输入的密码不一致，请重新输入密码。\n");
            continue;  // 返回到重新输入密码的步骤
        }
        break;
    }
    
    printf("\n选择角色：\n");
    printf("1. 患者\n");
    printf("2. 医生\n");
    printf("3. 管理员\n");
    roleChoice = read_int("请选择 (1-3): ", 1, 3);
    
    switch (roleChoice) {
        case 1:
            role = ROLE_PATIENT;
            printf("请输入关联的病历号（没有可填 0）: ");
            linkedId = read_int("", 0, 1000000);
            break;
        case 2:
            role = ROLE_DOCTOR;
            printf("请输入关联的医生工号: ");
            linkedId = read_int("", 1, 1000000);
            // 验证医生是否存在
            if (!find_doctor(db, linkedId)) {
                printf("警告：该工号对应的医生不存在，但仍可创建账号。\n");
            }
            break;
        case 3:
            role = ROLE_MANAGER;
            linkedId = 0;
            break;
        default:
            printf("无效的选择。\n");
            return;
    }
    
    if (create_account(db, username, password, role, linkedId)) {
        save_accounts(db, "./accounts.txt");
        printf("注册成功！您的角色是：%s\n", get_role_name(role));
    } else {
        printf("注册失败。\n");
    }
}

/*
 * 说明：登录菜单函数
 * 参数：db 数据库指针
 * 返回值：登录成功返回 1，失败返回 0
 */
int login_menu(Database *db) {
    char username[32], password[64];
    Account *acc;
    int choice;
    
    printf("\n=== 用户登录 ===\n");
    printf("1. 登录\n");
    printf("2. 注册新账号\n");
    printf("0. 退出程序\n");
    choice = read_int("请选择 (0-2): ", 0, 2);
    
    if (choice == 0) {
        return 0;
    }
    
    if (choice == 2) {
        register_menu(db);
        // 注册成功后直接返回，让主程序重新调用 login_menu 回到选择界面
        return -1;
    }
    
    printf("请输入用户名：");
    read_line("", username, sizeof(username));
    
    printf("请输入密码：");
    read_line("", password, sizeof(password));
    
    acc = authenticate_user(db, username, password);
    
    if (acc) {
        g_session.isLoggedIn = 1;
        g_session.role = acc->role;
        g_session.userId = acc->linkedId;
        strncpy(g_session.username, acc->username, sizeof(g_session.username) - 1);
        
        printf("\n登录成功！欢迎您，%s (%s)\n", acc->username, get_role_name(acc->role));
        return 1;
    } else {
        printf("用户名或密码错误。\n");
        // 登录失败时不退出，返回 -1 让调用者重新显示登录菜单
        return -1;
    }
}

/*
 * 说明：登出菜单
 */
static void logout_menu(void) {
    logout_user();
    printf("已成功登出。\n");
}

/* ==================== 患者角色菜单 ==================== */

/* 查看个人挂号记录 */
static void patient_view_registrations(Database *db) {
    Registration *r;
    int found = 0;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    printf("\n=== 我的挂号记录 ===\n");
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == g_session.userId) {
            printf("[%d] %s %s 医生%d %s %s\n", r->id, r->date, r->dept, r->doctorId, r->type, r->status);
            found = 1;
        }
    }
    
    if (!found) {
        printf("暂无挂号记录。\n");
    }
}

/* 查看个人检查记录 */
static void patient_view_exams(Database *db) {
    Exam *e;
    int found = 0;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    printf("\n=== 我的检查记录 ===\n");
    for (e = db->exams; e; e = e->next) {
        if (e->patientId == g_session.userId) {
            printf("[%d] %s %s %.2f %s\n", e->id, e->code, e->itemName, e->fee, e->result);
            found = 1;
        }
    }
    
    if (!found) {
        printf("暂无检查记录。\n");
    }
}

/* 查看个人住院记录 */
static void patient_view_inpatients(Database *db) {
    Inpatient *ip;
    int found = 0;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    printf("\n=== 我的住院记录 ===\n");
    for (ip = db->inpatients; ip; ip = ip->next) {
        if (ip->patientId == g_session.userId) {
            printf("[%d] 病房%d 床位%d %s ~ %s 费用%.2f\n", 
                   ip->id, ip->wardId, ip->bedNo, ip->admitDate, ip->expectedDischarge, ip->totalCost);
            found = 1;
        }
    }
    
    if (!found) {
        printf("暂无住院记录。\n");
    }
}

/*
 * 说明：患者角色菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录（保留参数以保持一致性）
 */
void patient_menu(Database *db, const char *dataDir) {
    int choice;
    (void)dataDir; /* 未使用的参数，保留以保持接口一致性 */
    
    while (1) {
        printf("\n========== 患者服务菜单 ==========\n");
        printf("欢迎，%s\n", g_session.username);
        printf("1. 查看我的挂号记录\n");
        printf("2. 查看我的检查记录\n");
        printf("3. 查看我的住院记录\n");
        printf("4. 修改个人信息\n");
        printf("0. 登出并返回登录界面\n");
        printf("请选择：");
        
        choice = read_int("", 0, 4);
        
        switch (choice) {
            case 1: patient_view_registrations(db); break;
            case 2: patient_view_exams(db); break;
            case 3: patient_view_inpatients(db); break;
            case 4: 
                printf("此功能开发中...\n"); 
                break;
            case 0: 
                logout_menu();
                return;
        }
    }
}

/* ==================== 医生角色菜单 ==================== */

/* 查看医生的患者挂号 */
static void doctor_view_patients(Database *db) {
    Registration *r;
    Patient *p;
    int count = 0;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_DOCTOR) {
        printf("权限不足。\n");
        return;
    }
    
    printf("\n=== 我的患者列表 ===\n");
    for (r = db->registrations; r; r = r->next) {
        if (r->doctorId == g_session.userId) {
            p = find_patient(db, r->patientId);
            if (p) {
                printf("[%d] %s %s %s %s %s\n", r->id, p->name, p->gender, r->date, r->dept, r->status);
                count++;
            }
        }
    }
    
    if (count == 0) {
        printf("暂无患者挂号。\n");
    } else {
        printf("共 %d 条记录。\n", count);
    }
}

/* 添加看诊记录 */
static void doctor_add_visit(Database *db, const char *dataDir) {
    int regId = 0;
    int step = 0;
    char diagnosis[TEXT_LEN], examItems[TEXT_LEN], prescription[TEXT_LEN];
    Registration *r = NULL;
    Visit *v;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_DOCTOR) {
        printf("权限不足。\n");
        return;
    }
    
    while (step < 4) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("挂号编号 (输入 0 返回上一步): ", 1, 1000000, &regId);
            if (ok) {
                r = find_registration(db, regId);
                if (!r) { 
                    printf("挂号记录不存在。\n"); 
                    ok = 0; 
                } else if (r->doctorId != g_session.userId) {
                    printf("这不是您的挂号记录。\n");
                    ok = 0;
                }
            }
        } else if (step == 1) ok = read_line_or_back("诊断结果 (输入 0 返回上一步): ", diagnosis, sizeof(diagnosis));
        else if (step == 2) ok = read_line_or_back("检查项目 (输入 0 返回上一步): ", examItems, sizeof(examItems));
        else ok = read_line_or_back("处方信息 (输入 0 返回上一步): ", prescription, sizeof(prescription));

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }
    v = (Visit*)malloc(sizeof(Visit));
    v->id = next_visit_id(db);
    v->regId = regId;
    strcpy(v->diagnosis, diagnosis);
    strcpy(v->examItems, examItems);
    strcpy(v->prescription, prescription);
    v->next = NULL;
    if (!db->visits) db->visits = v; else { Visit *q = db->visits; while (q->next) q = q->next; q->next = v; }
    strcpy(r->status, "已就诊");
    save_all(db, dataDir);
    printf("看诊记录已添加。\n");
}

/*
 * 说明：医生角色菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录
 */
void doctor_menu(Database *db, const char *dataDir) {
    int choice;
    
    while (1) {
        printf("\n========== 医生工作菜单 ==========\n");
        printf("欢迎，%s\n", g_session.username);
        printf("1. 查看我的患者\n");
        printf("2. 添加看诊记录\n");
        printf("3. 查看我的排班\n");
        printf("0. 登出并返回登录界面\n");
        printf("请选择：");
        
        choice = read_int("", 0, 3);
        
        switch (choice) {
            case 1: doctor_view_patients(db); break;
            case 2: doctor_add_visit(db, dataDir); break;
            case 3: 
                printf("此功能开发中...\n"); 
                break;
            case 0: 
                logout_menu();
                return;
        }
    }
}

/* ==================== 管理员角色菜单 ==================== */

/*
 * 说明：管理员角色菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录
 */
void manager_menu(Database *db, const char *dataDir) {
    int choice;
    
    while (1) {
        printf("\n========== 管理员菜单 ==========\n");
        printf("欢迎，%s\n", g_session.username);
        printf("1. 患者管理\n");
        printf("2. 医生管理\n");
        printf("3. 药品管理\n");
        printf("4. 病房管理\n");
        printf("5. 全院统计报表\n");
        printf("6. 用户账号管理\n");
        printf("0. 登出并返回登录界面\n");
        printf("A. 导入数据文件\n");
        printf("请选择：");
        
        char input[32];
        read_line("", input, sizeof(input));
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "A") == 0 || strcmp(input, "a") == 0) {
            char importDir[256];
            printf("请输入要导入的数据文件所在目录：");
            read_line("", importDir, sizeof(importDir));
            
            if (strlen(importDir) > 0) {
                int count = import_all(db, importDir);
                if (count > 0) {
                    printf("成功从 %s 导入了 %d 个数据文件。\n", importDir, count);
                    save_all(db, dataDir);
                    printf("数据已合并保存到当前数据库。\n");
                } else {
                    printf("未找到任何数据文件，请检查目录路径。\n");
                }
            } else {
                printf("取消导入操作。\n");
            }
            pause_and_wait();
            continue;
        }
        
        // 先验证输入是否为纯数字
        char *endptr;
        long val = strtol(input, &endptr, 10);
        
        // 检查是否有非数字字符（除了末尾的换行符等）
        if (*endptr != '\0') {
            printf("无效的选择，请输入 0-6 或 A。\n");
            pause_and_wait();
            continue;
        }
        
        choice = (int)val;
        
        // 验证输入是否为有效数字选项 (0-6)
        if (choice < 0 || choice > 6) {
            printf("无效的选择，请输入 0-6 或 A。\n");
            pause_and_wait();
            continue;
        }
        
        switch (choice) {
            case 1: 
                patient_management_menu(db, dataDir); 
                break;
            case 2: 
                printf("医生管理功能开发中...\n");
                pause_and_wait();
                break;
            case 3: 
                drug_management_menu(db, dataDir); 
                break;
            case 4: 
                printf("病房管理功能开发中...\n");
                pause_and_wait();
                break;
            case 5: 
                management_report(db); 
                pause_and_wait(); 
                break;
            case 6:
                printf("用户账号管理功能开发中...\n");
                pause_and_wait();
                break;
            case 0: 
                logout_menu();
                return;
        }
    }
}

/* ==================== 辅助统计函数 ==================== */
/* 统计患者总数 */
static int count_patients(Database *db) {
    int c = 0;
    Patient *p = db->patients;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}
/* 统计医生总数 */
static int count_doctors(Database *db) {
    int c = 0;
    Doctor *p = db->doctors;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}
/* 统计挂号记录总数 */
static int count_regs(Database *db) {
    int c = 0;
    Registration *p = db->registrations;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}
/* 统计住院患者总数 */
static int count_inpatients(Database *db) {
    int c = 0;
    Inpatient *p = db->inpatients;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}
/* 统计药品总数 */
static int count_drugs(Database *db) {
    int c = 0;
    Drug *p = db->drugs;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}
/*
 * 说明：读取一行输入，支持输入"0"返回上一步
 * 参数：prompt 提示信息
 * 参数：buf 存储输入的缓冲区
 * 参数：size 缓冲区大小
 * 返回值：1 表示成功读取，0 表示用户选择返回
 */
int read_line_or_back(const char *prompt, char *buf, int size) {
    read_line(prompt, buf, size);
    if (strcmp(buf, "0") == 0) {
        return 0;
    }
    return 1;
}

/* 读取性别输入并验证，支持输入"0"返回上一步 */
static int read_gender_with_back(const char *prompt, char *buf, int size) {
    char temp[256];
    while (1) {
        read_line(prompt, temp, sizeof(temp));
        if (strcmp(temp, "0") == 0) {
            return 0;
        }
        if (validate_gender(temp)) {
            safe_copy(buf, temp, size);
            return 1;
        }
        printf("性别输入无效，请输入\"男\"或\"女\"，或输入0返回上一步。\n");
    }
}

/* 读取日期输入并验证格式 YYYY-MM-DD，支持输入"0"返回上一步 */
static int read_date_with_back(const char *prompt, char *buf, int size) {
    char temp[256];
    while (1) {
        read_line(prompt, temp, sizeof(temp));
        if (strcmp(temp, "0") == 0) {
            return 0;
        }
        if (validate_date(temp)) {
            safe_copy(buf, temp, size);
            return 1;
        }
        printf("日期格式无效，请输入 YYYY-MM-DD 格式（如 2024-01-15），或输入0返回上一步。\n");
    }
}

/* 读取手机号输入并验证格式（11 位数字以 1 开头），支持输入"0"返回上一步 */
static int read_phone_with_back(const char *prompt, char *buf, int size) {
    char temp[256];
    while (1) {
        read_line(prompt, temp, sizeof(temp));
        if (strcmp(temp, "0") == 0) {
            return 0;
        }
        if (validate_phone(temp)) {
            safe_copy(buf, temp, size);
            return 1;
        }
        printf("手机号格式无效，请输入11位数字（以1开头），或输入0返回上一步。\n");
    }
}

/*
 * 说明：读取整数输入并验证范围，支持输入"0"返回上一步
 * 参数：prompt 提示信息
 * 参数：min 最小值
 * 参数：max 最大值
 * 参数：out 输出参数，存储读取的整数值
 * 返回值：1 表示成功读取，0 表示用户选择返回
 */
int read_int_or_back(const char *prompt, int min, int max, int *out) {
    char line[64];
    char *end;
    long value;
    while (1) {
        read_line(prompt, line, sizeof(line));
        if (strcmp(line, "0") == 0) return 0;
        value = strtol(line, &end, 10);
        if (*line != '\0' && *end == '\0' && value >= min && value <= max) {
            *out = (int)value;
            return 1;
        }
        printf("输入无效，请输入 %d ~ %d 的整数，或输入0返回上一步。\n", min, max);
    }
}

/*
 * 说明：列出所有患者信息
 * 参数：db 数据库指针
 */
static void list_patients(Database *db) {
    Patient *p = db->patients;
    printf("\n+----------+------------+------+--------------+---------------+----------------+\n");
    printf("| ");
    print_utf8_cell("病历号", 8); putchar(' ');
    printf("| ");
    print_utf8_cell("姓名", 10); putchar(' ');
    printf("| ");
    print_utf8_cell("性别", 4); putchar(' ');
    printf("| ");
    print_utf8_cell("出生日期", 12); putchar(' ');
    printf("| ");
    print_utf8_cell("联系方式", 13); putchar(' ');
    printf("| ");
    print_utf8_cell("医保", 14); putchar(' ');
    printf("|\n");
    printf("+----------+------------+------+--------------+---------------+----------------+\n");
    while (p) {
        printf("| %-8d ", p->id);
        printf("| "); print_utf8_cell(p->name, 10); putchar(' ');
        printf("| "); print_utf8_cell(p->gender, 4); putchar(' ');
        printf("| "); print_utf8_cell(p->birth, 12); putchar(' ');
        printf("| "); print_utf8_cell(p->phone, 13); putchar(' ');
        printf("| "); print_utf8_cell(p->insurance, 14); putchar(' ');
        printf("|\n");
        p = p->next;
    }
    printf("+----------+------------+------+--------------+---------------+----------------+\n");
}


/*
 * 说明：检查患者是否有关联的挂号、检查或住院记录
 * 参数：db 数据库指针
 * 参数：patientId 患者病历号
 * 返回值：1 表示有关联记录，0 表示无关联记录
 */
static int patient_has_related_records(Database *db, int patientId) {
    Registration *r;
    Exam *e;
    Inpatient *ip;
    for (r = db->registrations; r; r = r->next) if (r->patientId == patientId) return 1;
    for (e = db->exams; e; e = e->next) if (e->patientId == patientId) return 1;
    for (ip = db->inpatients; ip; ip = ip->next) if (ip->patientId == patientId) return 1;
    return 0;
}

/*
 * 说明：删除患者记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要删除的患者病历号
 * 2. 查找患者是否存在
 * 3. 检查是否有关联记录（挂号/检查/住院）
 * 4. 确认后删除并保存数据
 */
static void delete_patient(Database *db, const char *dataDir) {
    int id = read_int("要删除的患者病历号(输入0返回): ", 0, 1000000);
    Patient *prev = NULL;
    Patient *cur = db->patients;
    char confirm[16];
    if (id == 0) { printf("已返回上一步。\n"); return; }

    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }

    if (!cur) {
        printf("患者不存在。\n");
        return;
    }

    if (patient_has_related_records(db, id)) {
        printf("删除失败：该患者存在挂号/检查/住院关联记录，请先处理关联数据。\n");
        return;
    }

    printf("确认删除患者[%d] %s ? (y/n): ", cur->id, cur->name);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("已取消删除。\n");
        return;
    }

    if (prev) prev->next = cur->next;
    else db->patients = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：添加新患者记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 分配新患者节点内存
 * 2. 生成就诊病历号
 * 3. 逐步读取姓名、性别、出生日期、联系方式、医保类型
 * 4. 每步都支持返回上一步
 * 5. 添加到链表并保存数据
 */
static void add_patient(Database *db, const char *dataDir) {
    Patient *p = (Patient*)malloc(sizeof(Patient));
    int step = 0;
    p->id = next_patient_id(db);
    while (step < 5) {
        int ok = 0;
        if (step == 0) ok = read_line_or_back("姓名(输入0返回上一步): ", p->name, sizeof(p->name));
        else if (step == 1) ok = read_gender_with_back("性别 (男/女，输入 0 返回上一步): ", p->gender, sizeof(p->gender));
        else if (step == 2) ok = read_date_with_back("出生日期 (YYYY-MM-DD，输入 0 返回上一步): ", p->birth, sizeof(p->birth));
        else if (step == 3) ok = read_phone_with_back("联系方式 (11 位手机号，输入 0 返回上一步): ", p->phone, sizeof(p->phone));
        else ok = read_line_or_back("医保类型(输入0返回上一步): ", p->insurance, sizeof(p->insurance));

        if (ok) {
            step++;
        } else if (step == 0) {
            printf("已返回上一步。\n");
            free(p);
            return;
        } else {
            printf("已返回上一项输入。\n");
            step--;
        }
    }
    p->archived = 0;
    p->next = NULL;
    if (!db->patients) db->patients = p; else { Patient *q = db->patients; while (q->next) q = q->next; q->next = p; }
    save_all(db, dataDir);
    printf("添加成功，病历号=%d\n", p->id);
}

/* 统计指定患者在指定日期同一科室的挂号次数 */
static int count_patient_regs_same_day_dept(Database *db, int patientId, const char *date, const char *dept) {
    int cnt = 0; Registration *r = db->registrations;
    while (r) { if (r->patientId == patientId && strcmp(r->date, date) == 0 && strcmp(r->dept, dept) == 0) cnt++; r = r->next; }
    return cnt;
}
/* 统计指定患者在指定日期的挂号总次数 */
static int count_patient_regs_same_day(Database *db, int patientId, const char *date) {
    int cnt = 0; Registration *r = db->registrations;
    while (r) { if (r->patientId == patientId && strcmp(r->date, date) == 0) cnt++; r = r->next; }
    return cnt;
}

/*
 * 说明：添加挂号记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 逐步读取患者病历号、医生工号、科室、日期、挂号类型
 * 2. 验证患者和医生是否存在
 * 3. 检查挂号限制（同一天同一科室最多 1 次，同一天最多 3 次）
 * 4. 创建挂号记录并保存
 */
static void add_registration(Database *db, const char *dataDir) {
    Registration *r;
    int patientId = 0, doctorId = 0;
    int step = 0;
    char dept[SMALL_LEN], date[DATE_LEN], type[SMALL_LEN];
    while (step < 5) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("患者病历号(输入0返回上一步): ", 1, 1000000, &patientId);
            if (ok && !find_patient(db, patientId)) { printf("患者不存在。\n"); ok = 0; }
        } else if (step == 1) {
            ok = read_int_or_back("医生工号(输入0返回上一步): ", 1, 1000000, &doctorId);
            if (ok && !find_doctor(db, doctorId)) { printf("医生不存在。\n"); ok = 0; }
        } else if (step == 2) ok = read_line_or_back("科室(输入0返回上一步): ", dept, sizeof(dept));
        else if (step == 3) ok = read_date_with_back("挂号日期 (YYYY-MM-DD，输入 0 返回上一步): ", date, sizeof(date));
        else ok = read_line_or_back("挂号类型(普通/专家，输入0返回上一步): ", type, sizeof(type));

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }
    if (count_patient_regs_same_day_dept(db, patientId, date, dept) >= 1) { printf("同一患者同一天同一科室最多挂号1次。\n"); return; }
    if (count_patient_regs_same_day(db, patientId, date) >= 3) { printf("同一患者同一天最多挂号3次。\n"); return; }
    r = (Registration*)malloc(sizeof(Registration));
    r->id = next_registration_id(db);
    r->patientId = patientId;
    r->doctorId = doctorId;
    strcpy(r->dept, dept);
    strcpy(r->date, date);
    strcpy(r->type, type);
    strcpy(r->status, "未就诊");
    r->next = NULL;
    if (!db->registrations) db->registrations = r; else { Registration *q = db->registrations; while (q->next) q = q->next; q->next = r; }
    save_all(db, dataDir);
    printf("挂号成功，挂号编号=%d\n", r->id);
}

/*
 * 说明：检查挂号记录是否有关联的看诊记录
 * 参数：db 数据库指针
 * 参数：regId 挂号编号
 * 返回值：1 表示有关联记录，0 表示无关联记录
 */
static int registration_has_visit(Database *db, int regId) {
    Visit *v = db->visits;
    while (v) {
        if (v->regId == regId) return 1;
        v = v->next;
    }
    return 0;
}

/*
 * 说明：检查药品是否有出入库日志记录
 * 参数：db 数据库指针
 * 参数：drugId 药品编号
 * 返回值：1 表示有日志记录，0 表示无日志记录
 */
static int drug_has_logs(Database *db, int drugId) {
    DrugLog *l = db->drugLogs;
    while (l) {
        if (l->drugId == drugId) return 1;
        l = l->next;
    }
    return 0;
}

/*
 * 说明：删除挂号记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要删除的挂号编号
 * 2. 查找挂号记录是否存在
 * 3. 检查是否有关联看诊记录
 * 4. 确认后删除并保存数据
 */
static void delete_registration(Database *db, const char *dataDir) {
    int id = read_int("要删除的挂号编号(输入0返回): ", 0, 1000000);
    Registration *prev = NULL;
    Registration *cur = db->registrations;
    char confirm[16];
    if (id == 0) { printf("已返回上一步。\n"); return; }
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) { printf("挂号记录不存在。\n"); return; }
    if (registration_has_visit(db, id)) {
        printf("删除失败：该挂号记录已有关联看诊记录，请先删除看诊记录。\n");
        return;
    }
    printf("确认删除挂号[%d] 患者%d %s ? (y/n): ", cur->id, cur->patientId, cur->date);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) { printf("已取消删除。\n"); return; }
    if (prev) prev->next = cur->next; else db->registrations = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：添加看诊记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取挂号编号并验证是否存在
 * 2. 读取诊断结果、检查项目、处方信息
 * 3. 创建看诊记录并更新挂号状态为"已就诊"
 */
static void add_visit(Database *db, const char *dataDir) {
    int regId = 0;
    int step = 0;
    char diagnosis[TEXT_LEN], examItems[TEXT_LEN], prescription[TEXT_LEN];
    Registration *r = NULL;
    Visit *v;
    while (step < 4) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("挂号编号(输入0返回上一步): ", 1, 1000000, &regId);
            if (ok) {
                r = find_registration(db, regId);
                if (!r) { printf("挂号记录不存在。\n"); ok = 0; }
            }
        } else if (step == 1) ok = read_line_or_back("诊断结果(输入0返回上一步): ", diagnosis, sizeof(diagnosis));
        else if (step == 2) ok = read_line_or_back("检查项目(输入0返回上一步): ", examItems, sizeof(examItems));
        else ok = read_line_or_back("处方信息(输入0返回上一步): ", prescription, sizeof(prescription));

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }
    v = (Visit*)malloc(sizeof(Visit));
    v->id = next_visit_id(db);
    v->regId = regId;
    strcpy(v->diagnosis, diagnosis);
    strcpy(v->examItems, examItems);
    strcpy(v->prescription, prescription);
    v->next = NULL;
    if (!db->visits) db->visits = v; else { Visit *q = db->visits; while (q->next) q = q->next; q->next = v; }
    strcpy(r->status, "已就诊");
    save_all(db, dataDir);
    printf("看诊记录已添加。\n");
}

/*
 * 说明：删除看诊记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要删除的看诊编号
 * 2. 查找看诊记录
 * 3. 删除记录并恢复关联挂号状态为"未就诊"（如果没有其他看诊记录）
 */
static void delete_visit(Database *db, const char *dataDir) {
    int id = read_int("要删除的看诊编号(输入0返回): ", 0, 1000000);
    Visit *prev = NULL;
    Visit *cur = db->visits;
    Registration *r;
    Visit *v;
    char confirm[16];
    if (id == 0) { printf("已返回上一步。\n"); return; }
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) { printf("看诊记录不存在。\n"); return; }
    printf("确认删除看诊[%d] (挂号%d) ? (y/n): ", cur->id, cur->regId);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) { printf("已取消删除。\n"); return; }
    r = find_registration(db, cur->regId);
    if (prev) prev->next = cur->next; else db->visits = cur->next;
    free(cur);
    if (r) {
        int hasAnotherVisit = 0;
        for (v = db->visits; v; v = v->next) {
            if (v->regId == r->id) {
                hasAnotherVisit = 1;
                break;
            }
        }
        if (!hasAnotherVisit) strcpy(r->status, "未就诊");
    }
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：添加检查记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 逐步读取患者病历号、医生工号、检查编码、项目名称、执行时间、费用、结果
 * 2. 验证患者和医生是否存在
 * 3. 创建检查记录并保存
 */
static void add_exam(Database *db, const char *dataDir) {
    Exam *e = (Exam*)malloc(sizeof(Exam));
    int step = 0;
    char feeBuf[64];
    e->id = next_exam_id(db);
    while (step < 7) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("患者病历号(输入0返回上一步): ", 1, 1000000, &e->patientId);
            if (ok && !find_patient(db, e->patientId)) { printf("患者不存在。\n"); ok = 0; }
        } else if (step == 1) {
            ok = read_int_or_back("医生工号(输入0返回上一步): ", 1, 1000000, &e->doctorId);
            if (ok && !find_doctor(db, e->doctorId)) { printf("医生不存在。\n"); ok = 0; }
        } else if (step == 2) ok = read_line_or_back("检查编码(输入0返回上一步): ", e->code, sizeof(e->code));
        else if (step == 3) ok = read_line_or_back("检查项目名称(输入0返回上一步): ", e->itemName, sizeof(e->itemName));
        else if (step == 4) ok = read_date_with_back("执行时间 (YYYY-MM-DD，输入 0 返回上一步): ", e->execTime, sizeof(e->execTime));
        else if (step == 5) {
            ok = read_line_or_back("检查费用(输入0返回上一步): ", feeBuf, sizeof(feeBuf));
            if (ok) e->fee = atof(feeBuf);
        } else ok = read_line_or_back("检查结果(输入0返回上一步): ", e->result, sizeof(e->result));

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); free(e); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }
    e->next = NULL;
    if (!db->exams) db->exams = e; else { Exam *q = db->exams; while (q->next) q = q->next; q->next = e; }
    save_all(db, dataDir);
    printf("检查记录已添加。\n");
}

/*
 * 说明：删除检查记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void delete_exam(Database *db, const char *dataDir) {
    int id = read_int("要删除的检查编号(输入0返回): ", 0, 1000000);
    Exam *prev = NULL;
    Exam *cur = db->exams;
    char confirm[16];
    if (id == 0) { printf("已返回上一步。\n"); return; }
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) { printf("检查记录不存在。\n"); return; }
    printf("确认删除检查[%d] %s ? (y/n): ", cur->id, cur->itemName);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) { printf("已取消删除。\n"); return; }
    if (prev) prev->next = cur->next; else db->exams = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：添加住院记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 逐步读取患者病历号、病房编号、床位号、入院时间、预计出院时间、预估费用
 * 2. 验证患者和病房是否存在
 * 3. 检查病房是否有空闲床位
 * 4. 增加病房占用床位数并保存
 */
static void add_inpatient(Database *db, const char *dataDir) {
    Inpatient *ip = (Inpatient*)malloc(sizeof(Inpatient));
    Ward *w;
    int step = 0;
    char buf[64];
    ip->id = next_inpatient_id(db);
    while (step < 6) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("患者病历号(输入0返回上一步): ", 1, 1000000, &ip->patientId);
            if (ok && !find_patient(db, ip->patientId)) { printf("患者不存在。\n"); ok = 0; }
        } else if (step == 1) ok = read_int_or_back("病房编号(输入0返回上一步): ", 1, 1000000, &ip->wardId);
        else if (step == 2) ok = read_int_or_back("床位号(输入0返回上一步): ", 1, 1000, &ip->bedNo);
        else if (step == 3) ok = read_date_with_back("入院时间 (YYYY-MM-DD，输入 0 返回上一步): ", ip->admitDate, sizeof(ip->admitDate));
        else if (step == 4) ok = read_date_with_back("预计出院时间 (YYYY-MM-DD，输入 0 返回上一步): ", ip->expectedDischarge, sizeof(ip->expectedDischarge));
        else {
            ok = read_line_or_back("预估住院费用(输入0返回上一步): ", buf, sizeof(buf));
            if (ok) ip->totalCost = atof(buf);
        }

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); free(ip); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }
    w = find_ward(db, ip->wardId);
    if (!w) { printf("病房不存在。\n"); free(ip); return; }
    if (w->occupiedBeds + w->maintenanceBeds >= w->bedCount) { printf("病房没有空闲床位。\n"); free(ip); return; }
    w->occupiedBeds++;
    ip->next = NULL;
    if (!db->inpatients) db->inpatients = ip; else { Inpatient *q = db->inpatients; while (q->next) q = q->next; q->next = ip; }
    save_all(db, dataDir);
    printf("住院登记成功，编号=%d\n", ip->id);
}

/*
 * 说明：删除住院记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要删除的住院编号
 * 2. 查找住院记录
 * 3. 减少病房占用床位数
 * 4. 删除记录并保存
 */
static void delete_inpatient(Database *db, const char *dataDir) {
    int id = read_int("要删除的住院编号(输入0返回): ", 0, 1000000);
    Inpatient *prev = NULL;
    Inpatient *cur = db->inpatients;
    Ward *w;
    char confirm[16];
    if (id == 0) { printf("已返回上一步。\n"); return; }
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) { printf("住院记录不存在。\n"); return; }
    printf("确认删除住院[%d] 患者%d ? (y/n): ", cur->id, cur->patientId);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) { printf("已取消删除。\n"); return; }
    w = find_ward(db, cur->wardId);
    if (w && w->occupiedBeds > 0) w->occupiedBeds--;
    if (prev) prev->next = cur->next; else db->inpatients = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：挂号管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void registration_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 挂号管理 ---\n");
        printf("1. 新增挂号记录\n");
        printf("2. 删除挂号记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 2);
        if (choice == 0) return;
        if (choice == 1) add_registration(db, dataDir);
        else delete_registration(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 说明：看诊管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void visit_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 看诊管理 ---\n");
        printf("1. 新增看诊记录\n");
        printf("2. 删除看诊记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 2);
        if (choice == 0) return;
        if (choice == 1) add_visit(db, dataDir);
        else delete_visit(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 说明：检查管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void exam_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 检查管理 ---\n");
        printf("1. 新增检查记录\n");
        printf("2. 删除检查记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 2);
        if (choice == 0) return;
        if (choice == 1) add_exam(db, dataDir);
        else delete_exam(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 说明：住院管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void inpatient_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 住院管理 ---\n");
        printf("1. 新增住院记录\n");
        printf("2. 删除住院记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 2);
        if (choice == 0) return;
        if (choice == 1) add_inpatient(db, dataDir);
        else delete_inpatient(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 说明：患者管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
void patient_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 患者管理 ---\n");
        printf("1. 查看患者列表\n");
        printf("2. 新增患者\n");
        printf("3. 删除患者\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 3);
        if (choice == 0) return;
        if (choice == 1) list_patients(db);
        else if (choice == 2) add_patient(db, dataDir);
        else delete_patient(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 说明：药品出入库操作
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取药品编号并验证是否存在
 * 2. 读取操作类型（入库/出库）
 * 3. 读取数量（出库时检查库存是否充足）
 * 4. 读取操作人和日期
 * 5. 创建出入库日志并更新库存
 */
static void drug_inout(Database *db, const char *dataDir) {
    Drug *d = NULL;
    DrugLog *l;
    int drugId = 0, qty = 0;
    int step = 0;
    char op[SMALL_LEN], operatorName[NAME_LEN], date[DATE_LEN];
    while (step < 5) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("药品编号(输入0返回上一步): ", 1, 1000000, &drugId);
            if (ok) {
                d = find_drug(db, drugId);
                if (!d) { printf("药品不存在。\n"); ok = 0; }
            }
        } else if (step == 1) {
            ok = read_line_or_back("操作类型(入库/出库，输入0返回上一步): ", op, sizeof(op));
            if (ok && strcmp(op, "入库") != 0 && strcmp(op, "出库") != 0) {
                printf("操作类型仅支持“入库”或“出库”。\n");
                ok = 0;
            }
        } else if (step == 2) {
            ok = read_int_or_back("数量(输入0返回上一步): ", 1, 1000000, &qty);
            if (ok && strcmp(op, "出库") == 0 && d->stock < qty) {
                printf("库存不足。\n");
                ok = 0;
            }
        } else if (step == 3) ok = read_line_or_back("操作人(输入0返回上一步): ", operatorName, sizeof(operatorName));
        else ok = read_date_with_back("日期 (YYYY-MM-DD，输入 0 返回上一步): ", date, sizeof(date));

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }

    l = (DrugLog*)malloc(sizeof(DrugLog));
    l->id = next_druglog_id(db);
    l->drugId = drugId;
    strcpy(l->operation, op);
    l->quantity = qty;
    strcpy(l->operatorName, operatorName);
    strcpy(l->date, date);
    if (strcmp(op, "出库") == 0) d->stock -= qty; else d->stock += qty;
    l->next = NULL;
    if (!db->drugLogs) db->drugLogs = l; else { DrugLog *q = db->drugLogs; while (q->next) q = q->next; q->next = l; }
    save_all(db, dataDir);
    printf("操作成功，当前库存=%d\n", d->stock);
}

/*
 * 说明：添加新药品
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 逐步读取通用名、商品名、别名、类别、所属科室、单价、初始库存
 * 2. 创建药品记录并保存
 */
static void add_drug(Database *db, const char *dataDir) {
    Drug *d = (Drug*)malloc(sizeof(Drug));
    int step = 0;
    char priceBuf[64], stockBuf[64];
    d->id = next_drug_id(db);
    while (step < 7) {
        int ok = 0;
        if (step == 0) ok = read_line_or_back("通用名(输入0返回上一步): ", d->genericName, sizeof(d->genericName));
        else if (step == 1) ok = read_line_or_back("商品名(输入0返回上一步): ", d->brandName, sizeof(d->brandName));
        else if (step == 2) ok = read_line_or_back("别名(输入0返回上一步): ", d->alias, sizeof(d->alias));
        else if (step == 3) ok = read_line_or_back("类别(输入0返回上一步): ", d->type, sizeof(d->type));
        else if (step == 4) ok = read_line_or_back("所属科室(输入0返回上一步): ", d->dept, sizeof(d->dept));
        else if (step == 5) {
            ok = read_line_or_back("单价(输入0返回上一步): ", priceBuf, sizeof(priceBuf));
            if (ok) d->price = atof(priceBuf);
        } else {
            ok = read_line_or_back("初始库存(输入0返回上一步): ", stockBuf, sizeof(stockBuf));
            if (ok) d->stock = atoi(stockBuf);
        }

        if (ok) step++;
        else if (step == 0) { printf("已返回上一步。\n"); free(d); return; }
        else { printf("已返回上一项输入。\n"); step--; }
    }
    d->next = NULL;
    if (!db->drugs) db->drugs = d; else { Drug *q = db->drugs; while (q->next) q = q->next; q->next = d; }
    save_all(db, dataDir);
    printf("药品新增成功，药品编号=%d\n", d->id);
}

/*
 * 说明：删除药品记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要删除的药品编号
 * 2. 检查是否有出入库日志记录
 * 3. 确认后删除并保存
 */
static void delete_drug(Database *db, const char *dataDir) {
    int id = read_int("要删除的药品编号(输入0返回): ", 0, 1000000);
    Drug *prev = NULL;
    Drug *cur = db->drugs;
    char confirm[16];
    if (id == 0) { printf("已返回上一步。\n"); return; }
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) { printf("药品不存在。\n"); return; }
    if (drug_has_logs(db, id)) {
        printf("删除失败：该药品已有出入库记录，无法直接删除。\n");
        return;
    }
    printf("确认删除药品[%d] %s/%s ? (y/n): ", cur->id, cur->genericName, cur->brandName);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) { printf("已取消删除。\n"); return; }
    if (prev) prev->next = cur->next; else db->drugs = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：药品管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
void drug_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 药品管理 ---\n");
        printf("1. 药品出入库\n");
        printf("2. 新增药品\n");
        printf("3. 删除药品\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 3);
        if (choice == 0) return;
        if (choice == 1) drug_inout(db, dataDir);
        else if (choice == 2) add_drug(db, dataDir);
        else delete_drug(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 说明：患者视角报表 - 显示患者完整就诊记录
 * 参数：db 数据库指针
 */
static void patient_report(Database *db) {
    int pid = read_int("输入患者病历号(输入0返回): ", 0, 1000000);
    Patient *p = find_patient(db, pid);
    Registration *r;
    Exam *e;
    Inpatient *ip;
    if (pid == 0) { printf("已返回上一步。\n"); return; }
    if (!p) { printf("患者不存在。\n"); return; }
    printf("\n患者: %s(%d) %s %s %s %s\n", p->name, p->id, p->gender, p->birth, p->phone, p->insurance);
    printf("挂号记录:\n");
    for (r = db->registrations; r; r = r->next) if (r->patientId == pid) printf("  [%d] %s %s 医生%d %s %s\n", r->id, r->date, r->dept, r->doctorId, r->type, r->status);
    printf("检查记录:\n");
    for (e = db->exams; e; e = e->next) if (e->patientId == pid) printf("  [%d] %s %s %.2f %s\n", e->id, e->code, e->itemName, e->fee, e->result);
    printf("住院记录:\n");
    for (ip = db->inpatients; ip; ip = ip->next) if (ip->patientId == pid) printf("  [%d] 病房%d 床位%d %s ~ %s 费用%.2f\n", ip->id, ip->wardId, ip->bedNo, ip->admitDate, ip->expectedDischarge, ip->totalCost);
}

/*
 * 说明：医生视角报表 - 显示医生工作统计
 * 参数：db 数据库指针
 */
static void doctor_report(Database *db) {
    int did = read_int("输入医生工号(输入0返回): ", 0, 1000000);
    Doctor *d = find_doctor(db, did);
    Registration *r; int count = 0;
    if (did == 0) { printf("已返回上一步。\n"); return; }
    if (!d) { printf("医生不存在。\n"); return; }
    printf("\n医生: %s(%d) %s %s\n", d->name, d->id, d->dept, d->title);
    for (r = db->registrations; r; r = r->next) {
        if (r->doctorId == did) {
            printf("  挂号[%d] 患者%d %s %s %s\n", r->id, r->patientId, r->date, r->type, r->status);
            count++;
        }
    }
    printf("总接诊/挂号关联数量: %d\n", count);
}

/*
 * 说明：管理视角报表 - 显示全院运营数据统计
 * 参数：db 数据库指针
 */
void management_report(Database *db) {
    Ward *w; Drug *d; Inpatient *ip; Exam *e;
    double wardRate, inpatientIncome = 0, examIncome = 0;
    int totalBeds = 0, usedBeds = 0;
    char idBuf[32], stockBuf[32], priceBuf[32];
    for (w = db->wards; w; w = w->next) { totalBeds += w->bedCount; usedBeds += w->occupiedBeds; }
    for (ip = db->inpatients; ip; ip = ip->next) inpatientIncome += ip->totalCost;
    for (e = db->exams; e; e = e->next) examIncome += e->fee;
    wardRate = totalBeds ? (usedBeds * 100.0 / totalBeds) : 0;
    printf("\n=== 管理视角报表 ===\n");
    printf("患者总数: %d\n", count_patients(db));
    printf("医生总数: %d\n", count_doctors(db));
    printf("挂号记录总数: %d\n", count_regs(db));
    printf("住院人数: %d\n", count_inpatients(db));
    printf("药品种类: %d\n", count_drugs(db));
    printf("床位利用率: %.2f%%\n", wardRate);
    printf("住院费用汇总: %.2f\n", inpatientIncome);
    printf("检查费用汇总: %.2f\n", examIncome);
    printf("药品库存盘点(全部):\n");
    printf("+------+--------------------+--------------------+--------+----------+------------+\n");
    printf("| ");
    print_utf8_cell("ID", 4); putchar(' ');
    printf("| ");
    print_utf8_cell("通用名", 18); putchar(' ');
    printf("| ");
    print_utf8_cell("商品名", 18); putchar(' ');
    printf("| ");
    print_utf8_cell("库存", 6); putchar(' ');
    printf("| ");
    print_utf8_cell("单价", 8); putchar(' ');
    printf("| ");
    print_utf8_cell("科室", 10); putchar(' ');
    printf("|\n");
    printf("+------+--------------------+--------------------+--------+----------+------------+\n");
    for (d = db->drugs; d; d = d->next) {
        snprintf(idBuf, sizeof(idBuf), "%d", d->id);
        snprintf(stockBuf, sizeof(stockBuf), "%d", d->stock);
        snprintf(priceBuf, sizeof(priceBuf), "%.2f", d->price);
        printf("| ");
        print_utf8_cell_fit(idBuf, 4); putchar(' ');
        printf("| ");
        print_utf8_cell_fit(d->genericName, 18); putchar(' ');
        printf("| ");
        print_utf8_cell_fit(d->brandName, 18); putchar(' ');
        printf("| ");
        print_utf8_cell_fit(stockBuf, 6); putchar(' ');
        printf("| ");
        print_utf8_cell_fit(priceBuf, 8); putchar(' ');
        printf("| ");
        print_utf8_cell_fit(d->dept, 10); putchar(' ');
        printf("|\n");
    }
    printf("+------+--------------------+--------------------+--------+----------+------------+\n");
}

/*
 * 说明：系统主菜单 - 显示所有功能模块并处理用户选择
 * 参数：db 数据库指针，包含所有业务数据
 * 参数：dataDir 数据文件存储目录路径
 * 功能：
 *   1. 显示主菜单选项（患者管理、挂号管理、看诊管理、检查管理、住院管理、药品管理、各类报表）
 *   2. 支持导入外部数据文件功能（选项A）
 *   3. 根据用户选择调用对应的子菜单或功能函数
 *   4. 选择0时保存所有数据并退出程序
 *   5. 循环执行直到用户选择退出
 */
void main_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n==============================\n");
        printf("  医疗综合管理系统 HIS\n");
        printf("==============================\n");
        printf("1. 患者管理\n");
        printf("2. 挂号管理\n");
        printf("3. 看诊管理\n");
        printf("4. 检查管理\n");
        printf("5. 住院管理\n");
        printf("6. 药品管理\n");
        printf("7. 患者视角查询\n");
        printf("8. 医护视角查询\n");
        printf("9. 管理视角报表\n");
        printf("0. 保存并退出\n");
        printf("A. 导入数据文件\n");
        printf("请选择：");
        
        char input[32];
        read_line("", input, sizeof(input));
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "A") == 0 || strcmp(input, "a") == 0) {
            char importDir[256];
            printf("请输入要导入的数据文件所在目录：");
            read_line("", importDir, sizeof(importDir));
            
            if (strlen(importDir) > 0) {
                int count = import_all(db, importDir);
                if (count > 0) {
                    printf("成功从 %s 导入了 %d 个数据文件。\n", importDir, count);
                    save_all(db, dataDir);
                    printf("数据已合并保存到当前数据库。\n");
                } else {
                    printf("未找到任何数据文件，请检查目录路径。\n");
                }
            } else {
                printf("取消导入操作。\n");
            }
            pause_and_wait();
            continue;
        }
        
        choice = atoi(input);
        
        // 验证输入是否为有效数字选项 (0-9)
        if (choice < 0 || choice > 9) {
            printf("无效的选择，请输入 0-9 或 A。\n");
            pause_and_wait();
            continue;
        }
        
        switch (choice) {
            case 1: patient_management_menu(db, dataDir); break;
            case 2: registration_management_menu(db, dataDir); break;
            case 3: visit_management_menu(db, dataDir); break;
            case 4: exam_management_menu(db, dataDir); break;
            case 5: inpatient_management_menu(db, dataDir); break;
            case 6: drug_management_menu(db, dataDir); break;
            case 7: patient_report(db); pause_and_wait(); break;
            case 8: doctor_report(db); pause_and_wait(); break;
            case 9: management_report(db); pause_and_wait(); break;
            case 0: save_all(db, dataDir); printf("数据已保存。\n"); return;
        }
    }
}
