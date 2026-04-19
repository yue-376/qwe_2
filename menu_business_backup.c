#include "menu_business.h"
#include "data.h"
#include "auth.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ==================== 前向声明 ==================== */
/* 以下是本文件中定义的各个功能函数的前置声明，让编译器在遇到函数调用时知道它们的存在 */
static void create_doctor_archive(Database *db, const char *dataDir);   /* 创建医生档案 */
static void link_archive_to_account(Database *db, const char *dataDir); /* 将档案（患者/医生）关联到用户账号 */
static void add_archive(Database *db, const char *dataDir);             /* 新增档案（患者或医生） */
static void delete_archive(Database *db, const char *dataDir);          /* 删除档案（患者或医生） */
static void edit_archive(Database *db, const char *dataDir);            /* 修改档案（患者或医生） */
static void delete_doctor(Database *db, const char *dataDir);           /* 删除医生档案 */
static void edit_patient(Database *db, const char *dataDir);            /* 修改患者档案 */
static void edit_doctor(Database *db, const char *dataDir);             /* 修改医生档案 */
static void exam_management_menu(Database *db, const char *dataDir);    /* 检查记录管理子菜单 */
static void visit_management_menu(Database *db, const char *dataDir);   /* 看诊记录管理子菜单 */
static void edit_visit(Database *db, const char *dataDir);              /* 修改看诊记录 */
static void edit_exam(Database *db, const char *dataDir);               /* 修改检查记录 */
static void edit_inpatient(Database *db, const char *dataDir);          /* 修改住院记录 */
static void inpatient_management_menu(Database *db, const char *dataDir); /* 住院记录管理子菜单 */

/* ==================== 全局登录会话 ==================== */
/* 全局变量 g_session 用于保存当前登录用户的信息，包括登录状态、角色、关联 ID 和用户名 */
UserSession g_session = {0, ROLE_PATIENT, 0, ""};

/* ==================== 登录与注册功能 ==================== */

/*
 * 函数：register_menu - 用户注册功能
 * 
 * 功能说明：
 *   引导新用户完成注册流程，包括输入用户名、密码、选择角色（患者/医生/管理员），
 *   并可选地关联已有的病历号或医生工号。注册成功后会保存到账户文件。
 * 
 * 参数：
 *   db - 数据库指针，用于查询和创建账户
 * 
 * 流程：
 *   1. 提示输入用户名，检查是否为空以及是否已存在
 *   2. 循环要求输入密码（至少 4 位）并确认两次一致
 *   3. 让用户选择角色（1-患者，2-医生，3-管理员）
 *   4. 根据角色不同，提示输入关联的病历号或医生工号
 *   5. 调用 create_account 创建账户并保存到文件
 *   6. 显示注册结果
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
 * 函数：login_menu - 用户登录功能
 * 
 * 功能说明：
 *   提供用户登录界面，支持登录、注册新账号、退出程序三个选项。
 *   登录成功后会更新全局会话信息 g_session，并返回相应状态码。
 * 
 * 参数：
 *   db - 数据库指针，用于验证用户身份
 * 
 * 返回值：
 *   1  - 登录成功
 *   0  - 用户选择退出程序
 *   -1 - 登录失败或注册成功（需要重新显示登录菜单）
 * 
 * 流程：
 *   1. 显示登录菜单（登录/注册/退出）
 *   2. 如果选择注册，调用 register_menu 并返回 -1
 *   3. 如果选择退出，返回 0
 *   4. 否则提示输入用户名和密码
 *   5. 调用 authenticate_user 验证身份
 *   6. 验证成功则更新全局会话并返回 1，失败则返回 -1
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
 * 函数：logout_menu - 用户登出功能
 * 
 * 功能说明：
 *   调用 logout_user 清除当前登录会话，并显示登出成功提示。
 *   这是一个简单的辅助函数，通常在各角色菜单的"退出"选项中被调用。
 */
static void logout_menu(void) {
    logout_user();
    printf("已成功登出。\n");
}

/* ==================== 患者角色菜单 ==================== */

/* 
 * 前向声明辅助函数 - 这些函数在后续定义，提前声明以便在其他函数中使用
 */
static int registration_has_visit(Database *db, int regId);                                          /* 检查挂号记录是否有关联的看诊记录 */
static int count_patient_regs_same_day_dept(Database *db, int patientId, const char *date, const char *dept); /* 统计患者某天某科室的挂号次数 */
static int count_patient_regs_same_day(Database *db, int patientId, const char *date);               /* 统计患者某天的挂号总次数 */
static int read_date_with_back(const char *prompt, char *buf, int size);                             /* 读取日期输入，支持输入"0"返回上一步 */

/*
 * 函数：patient_view_medical_records - 患者查看个人医疗记录
 * 
 * 功能说明：
 *   合并显示当前登录患者的所有医疗相关记录，包括：
 *   - 挂号记录：显示每次挂号的时间、科室、医生和状态
 *   - 看诊记录：显示诊断结果、检查项目和处方信息
 *   - 检查记录：显示检查项目、费用和结果
 *   - 住院记录：显示病房、床位、入院时间和费用等信息
 * 
 * 权限检查：
 *   仅允许已登录的患者角色调用此函数
 */
static void patient_view_medical_records(Database *db) {
    Registration *r;
    Visit *v;
    Exam *e;
    Inpatient *ip;
    int found = 0;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    printf("\n========== 我的医疗记录 ==========\n");
    
    /* 显示挂号记录 */
    printf("\n--- 挂号记录 ---\n");
    found = 0;
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == g_session.userId) {
            printf("[%d] %s %s 医生%d %s %s\n", r->id, r->date, r->dept, r->doctorId, r->type, r->status);
            found = 1;
        }
    }
    if (!found) {
        printf("暂无挂号记录。\n");
    }
    
    /* 显示看诊记录 */
    printf("\n--- 看诊记录 ---\n");
    found = 0;
    for (v = db->visits; v; v = v->next) {
        /* 通过 regId 找到对应的挂号记录，再判断是否属于当前患者 */
        Registration *reg = find_registration(db, v->regId);
        if (reg && reg->patientId == g_session.userId) {
            printf("[挂号%d] 诊断：%s\n    检查项目：%s\n    处方：%s\n", 
                   v->regId, v->diagnosis, v->examItems, v->prescription);
            found = 1;
        }
    }
    if (!found) {
        printf("暂无看诊记录。\n");
    }
    
    /* 显示检查记录 */
    printf("\n--- 检查记录 ---\n");
    found = 0;
    for (e = db->exams; e; e = e->next) {
        if (e->patientId == g_session.userId) {
            printf("[%d] %s %s %.2f %s\n", e->id, e->code, e->itemName, e->fee, e->result);
            found = 1;
        }
    }
    if (!found) {
        printf("暂无检查记录。\n");
    }
    
    /* 显示住院记录 */
    printf("\n--- 住院记录 ---\n");
    found = 0;
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
 * 函数：patient_add_registration - 患者新增挂号
 * 
 * 功能说明：
 *   允许当前登录的患者为自己创建新的挂号记录。
 *   采用分步输入方式，每一步都支持返回上一步重新修改。
 * 
 * 权限检查：
 *   仅允许已登录的患者角色调用此函数
 * 
 * 流程：
 *   1. 检查患者档案是否存在
 *   2. 分步输入：医生工号 → 科室 → 日期 → 挂号类型
 *   3. 每步都可输入"0"返回上一步或取消
 *   4. 验证医生是否存在
 *   5. 检查挂号限制：
 *      - 同一患者同一天同一科室最多挂号 1 次
 *      - 同一患者同一天最多挂号 3 次
 *   6. 创建挂号记录并保存到文件
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 */
static void patient_add_registration(Database *db, const char *dataDir) {
    Registration *r;
    int doctorId = 0;
    int step = 0;
    char dept[SMALL_LEN], date[DATE_LEN], type[SMALL_LEN];
    int patientId = g_session.userId;  // 使用当前登录患者的 ID
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    /* 检查患者档案是否存在 */
    if (!find_patient(db, patientId)) {
        printf("未找到您的患者档案，请先联系管理员创建档案。\n");
        return;
    }
    
    printf("\n=== 患者挂号 ===\n");
    
    while (step < 4) {
        int ok = 0;
        if (step == 0) {
            ok = read_int_or_back("医生工号 (输入 0 返回上一步): ", 1, 1000000, &doctorId);
            if (ok && !find_doctor(db, doctorId)) { 
                printf("医生不存在。\n"); 
                ok = 0; 
            }
        } else if (step == 1) {
            ok = read_line_or_back("科室 (输入 0 返回上一步): ", dept, sizeof(dept));
        } else if (step == 2) {
            ok = read_date_with_back("挂号日期 (YYYY-MM-DD，输入 0 返回上一步): ", date, sizeof(date));
        } else {
            ok = read_line_or_back("挂号类型 (普通/专家，输入 0 返回上一步): ", type, sizeof(type));
        }

        if (ok) step++;
        else if (step == 0) { 
            printf("已返回上一步。\n"); 
            return; 
        }
        else { 
            printf("已返回上一项输入。\n"); 
            step--; 
        }
    }
    
    /* 检查挂号限制 */
    if (count_patient_regs_same_day_dept(db, patientId, date, dept) >= 1) { 
        printf("同一患者同一天同一科室最多挂号 1 次。\n"); 
        return; 
    }
    if (count_patient_regs_same_day(db, patientId, date) >= 3) { 
        printf("同一患者同一天最多挂号 3 次。\n"); 
        return; 
    }
    
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
 * 函数：patient_cancel_registration - 患者取消挂号
 * 
 * 功能说明：
 *   允许当前登录的患者查看并取消自己的挂号记录。
 *   取消前会进行多项检查以确保操作合法。
 * 
 * 权限检查：
 *   仅允许已登录的患者角色调用此函数
 * 
 * 流程：
 *   1. 显示当前患者的所有挂号记录
 *   2. 输入要取消的挂号编号
 *   3. 验证挂号记录是否存在且属于当前患者
 *   4. 检查是否已就诊（已就诊不能取消）
 *   5. 检查是否有关联的看诊记录（有则不能取消）
 *   6. 要求用户确认（y/n）
 *   7. 删除记录并保存
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 */
static void patient_cancel_registration(Database *db, const char *dataDir) {
    int id = 0;
    Registration *prev = NULL;
    Registration *cur = db->registrations;
    char confirm[16];
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    printf("\n=== 取消挂号 ===\n");
    
    /* 先显示患者的挂号记录 */
    printf("您的挂号记录：\n");
    int found = 0;
    for (cur = db->registrations; cur; cur = cur->next) {
        if (cur->patientId == g_session.userId) {
            printf("[%d] %s %s 医生%d %s %s\n", cur->id, cur->date, cur->dept, cur->doctorId, cur->type, cur->status);
            found = 1;
        }
    }
    if (!found) {
        printf("您暂无挂号记录。\n");
        return;
    }
    
    id = read_int("要取消的挂号编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }
    
    /* 查找挂号记录并验证属于当前患者 */
    prev = NULL;
    cur = db->registrations;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) { 
        printf("挂号记录不存在。\n"); 
        return; 
    }
    
    if (cur->patientId != g_session.userId) {
        printf("这不是您的挂号记录，无权取消。\n");
        return;
    }
    
    if (strcmp(cur->status, "已就诊") == 0) {
        printf("该挂号记录已就诊，无法取消。\n");
        return;
    }
    
    if (registration_has_visit(db, id)) {
        printf("删除失败：该挂号记录已有关联看诊记录，无法取消。\n");
        return;
    }
    
    printf("确认取消挂号 [%d] %s %s ? (y/n): ", cur->id, cur->date, cur->dept);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) { 
        printf("已取消操作。\n"); 
        return; 
    }
    
    if (prev) prev->next = cur->next; else db->registrations = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("取消成功。\n");
}

/*
 * 函数：patient_registration_management - 患者挂号管理子菜单
 * 
 * 功能说明：
 *   提供患者挂号相关操作的子菜单，包括新增挂号和取消挂号两个功能。
 *   循环显示菜单直到用户选择返回上级。
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 */
static void patient_registration_management(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 挂号管理 ---\n");
        printf("1. 新增挂号\n");
        printf("2. 取消挂号\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择：", 0, 2);
        if (choice == 0) return;
        if (choice == 1) patient_add_registration(db, dataDir);
        else if (choice == 2) patient_cancel_registration(db, dataDir);
        pause_and_wait();
    }
}

/*
 * 函数：patient_edit_profile - 患者修改个人基本信息
 * 
 * 功能说明：
 *   允许当前登录的患者修改自己的基本信息，包括姓名、性别、出生日期、联系电话和医保类型。
 *   每个字段都支持直接回车保持原值，或输入"0"取消整个修改操作。
 *   对性别、日期格式、手机号格式进行验证。
 * 
 * 权限检查：
 *   仅允许已登录的患者角色调用此函数
 * 
 * 流程：
 *   1. 查找当前患者的档案信息
 *   2. 显示当前所有信息
 *   3. 逐项询问是否修改（姓名→性别→出生日期→联系电话→医保类型）
 *   4. 对输入进行格式验证
 *   5. 保存修改到文件并显示更新后的信息
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 */
static void patient_edit_profile(Database *db, const char *dataDir) {
    Patient *p;
    char newName[NAME_LEN], newGender[16], newBirth[DATE_LEN], newPhone[PHONE_LEN], newInsurance[SMALL_LEN];
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
    /* 查找当前登录患者的信息 */
    p = find_patient(db, g_session.userId);
    if (!p) {
        printf("未找到您的患者档案。\n");
        return;
    }
    
    printf("\n=== 修改个人信息 ===\n");
    printf("当前信息：\n");
    printf("  姓名：%s\n", p->name);
    printf("  性别：%s\n", p->gender);
    printf("  出生日期：%s\n", p->birth);
    printf("  联系电话：%s\n", p->phone);
    printf("  医保类型：%s\n", p->insurance);
    
    printf("\n请输入新信息（直接回车保持原值，输入 0 取消修改）：\n");
    
    /* 修改姓名 */
    printf("姓名 [%s]: ", p->name);
    read_line("", newName, sizeof(newName));
    if (strcmp(newName, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newName) > 0) {
        safe_copy(p->name, newName, sizeof(p->name));
    }
    
    /* 修改性别 */
    printf("性别 [%s]: ", p->gender);
    read_line("", newGender, sizeof(newGender));
    if (strcmp(newGender, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newGender) > 0) {
        if (strcmp(newGender, "男") != 0 && strcmp(newGender, "女") != 0) {
            printf("性别必须为\"男\"或\"女\"，已保持原值。\n");
        } else {
            safe_copy(p->gender, newGender, sizeof(p->gender));
        }
    }
    
    /* 修改出生日期 */
    printf("出生日期 [%s]: ", p->birth);
    read_line("", newBirth, sizeof(newBirth));
    if (strcmp(newBirth, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newBirth) > 0) {
        /* 简单验证日期格式 YYYY-MM-DD */
        if (strlen(newBirth) == 10 && newBirth[4] == '-' && newBirth[7] == '-') {
            safe_copy(p->birth, newBirth, sizeof(p->birth));
        } else {
            printf("日期格式应为 YYYY-MM-DD，已保持原值。\n");
        }
    }
    
    /* 修改联系电话 */
    printf("联系电话 [%s]: ", p->phone);
    read_line("", newPhone, sizeof(newPhone));
    if (strcmp(newPhone, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newPhone) > 0) {
        /* 验证手机号格式（11 位数字） */
        int valid = 1;
        if (strlen(newPhone) != 11) {
            valid = 0;
        } else {
            for (int i = 0; i < 11; i++) {
                if (!isdigit(newPhone[i])) {
                    valid = 0;
                    break;
                }
            }
        }
        if (valid) {
            safe_copy(p->phone, newPhone, sizeof(p->phone));
        } else {
            printf("手机号应为 11 位数字，已保持原值。\n");
        }
    }
    
    /* 修改医保类型 */
    printf("医保类型 [%s]: ", p->insurance);
    read_line("", newInsurance, sizeof(newInsurance));
    if (strcmp(newInsurance, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newInsurance) > 0) {
        safe_copy(p->insurance, newInsurance, sizeof(p->insurance));
    }
    
    /* 保存修改到文件 */
    char path[256];
    path_join(path, sizeof(path), dataDir, "patients.txt");
    save_patients(db, path);
    
    printf("\n个人信息修改成功！\n");
    printf("更新后的信息：\n");
    printf("  姓名：%s\n", p->name);
    printf("  性别：%s\n", p->gender);
    printf("  出生日期：%s\n", p->birth);
    printf("  联系电话：%s\n", p->phone);
    printf("  医保类型：%s\n", p->insurance);
}

/*
 * 函数：patient_menu - 患者角色主菜单
 * 
 * 功能说明：
 *   患者登录后的主界面，提供以下功能选项：
 *   1. 查看个人医疗记录（挂号、看诊、检查、住院）
 *   2. 挂号管理（新增/取消挂号）
 *   3. 修改个人信息
 *   0. 登出并返回登录界面
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录（保留参数以保持一致性）
 * 
 * 流程：
 *   循环显示菜单，根据用户选择调用相应功能函数，直到用户选择登出
 */
void patient_menu(Database *db, const char *dataDir) {
    int choice;
    
    while (1) {
        printf("\n========== 患者服务菜单 ==========\n");
        printf("欢迎，%s\n", g_session.username);
        printf("1. 查看我的医疗记录\n");
        printf("2. 挂号管理\n");
        printf("3. 修改个人信息\n");
        printf("0. 登出并返回登录界面\n");
        printf("请选择：");
        
        choice = read_int("", 0, 3);
        
        switch (choice) {
            case 1: patient_view_medical_records(db); break;
            case 2: patient_registration_management(db, dataDir); break;
            case 3: 
                patient_edit_profile(db, dataDir); 
                break;
            case 0: 
                logout_menu();
                return;
        }
    }
