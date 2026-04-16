#include "menu.h"
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
}

/* ==================== 医生角色菜单 ==================== */

/*
 * 函数：doctor_view_patients - 医生查看患者列表
 * 
 * 功能说明：
 *   显示当前登录医生名下的所有挂号患者信息。
 *   遍历挂号记录，找出属于该医生的记录，并查找对应的患者信息显示。
 * 
 * 权限检查：
 *   仅允许已登录的医生角色调用此函数
 * 
 * 参数：
 *   db - 数据库指针
 */
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

/*
 * 函数：doctor_add_visit - 医生添加看诊记录
 * 
 * 功能说明：
 *   允许医生为属于自己名下的挂号记录添加看诊信息。
 *   采用分步输入方式，支持返回上一步重新修改。
 * 
 * 权限检查：
 *   仅允许已登录的医生角色调用此函数
 * 
 * 流程：
 *   1. 输入挂号编号并验证（必须存在且属于当前医生）
 *   2. 分步输入：诊断结果 → 检查项目 → 处方信息
 *   3. 创建看诊记录并更新挂号状态为"已就诊"
 *   4. 保存所有数据到文件
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 */
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
    printf("看诊记录已添加，编号：%d。\n", v->id);
}

/*
 * 函数：doctor_menu - 医生角色主菜单
 * 
 * 功能说明：
 *   医生登录后的主界面，提供以下功能选项：
 *   1. 查看我的患者 - 显示当前医生名下的所有挂号患者
 *   2. 看诊记录管理 - 进入看诊记录子菜单（新增/删除/修改）
 *   3. 检查记录管理 - 进入检查记录子菜单（新增/删除/修改）
 *   4. 住院记录管理 - 进入住院记录子菜单（新增/删除/修改）
 *   0. 登出并返回登录界面
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 * 
 * 流程：
 *   循环显示菜单，根据用户选择调用相应功能函数，直到用户选择登出
 */
void doctor_menu(Database *db, const char *dataDir) {
    int choice;
    
    while (1) {
        printf("\n========== 医生工作菜单 ==========\n");
        printf("欢迎，%s\n", g_session.username);
        printf("1. 查看我的患者\n");
        printf("2. 看诊记录管理\n");
        printf("3. 检查记录管理\n");
        printf("4. 住院记录管理\n");
        printf("0. 登出并返回登录界面\n");
        printf("请选择：");
        
        choice = read_int("", 0, 4);
        
        switch (choice) {
            case 1: doctor_view_patients(db); break;
            case 2: visit_management_menu(db, dataDir); break;
            case 3: exam_management_menu(db, dataDir); break;
            case 4: inpatient_management_menu(db, dataDir); break;
            case 0: 
                logout_menu();
                return;
        }
    }
}

/* ==================== 管理员角色菜单 ==================== */

/*
 * 函数：manager_menu - 管理员角色主菜单
 * 
 * 功能说明：
 *   管理员登录后的主界面，提供全院级别的管理功能：
 *   1. 患者管理 - 查看/新增/删除/修改患者信息
 *   2. 档案管理 - 管理患者和医生档案、关联账号
 *   3. 药品管理 - 药品出入库、新增/删除药品
 *   4. 全院统计报表 - 显示医院运营数据统计
 *   5. 用户账号管理 - 管理所有用户账号
 *   A. 导入数据文件 - 从外部目录导入数据
 *   0. 登出并返回登录界面
 * 
 * 参数：
 *   db - 数据库指针
 *   dataDir - 数据文件存储目录
 * 
 * 流程：
 *   循环显示菜单，支持数字选项和字母选项（A/a），根据用户选择调用相应功能
 */
void manager_menu(Database *db, const char *dataDir) {
    int choice;
    
    while (1) {
        printf("\n========== 管理员菜单 ==========\n");
        printf("欢迎，%s\n", g_session.username);
        printf("1. 患者管理\n");
        printf("2. 档案管理\n");
        printf("3. 药品管理\n");
        printf("4. 全院统计报表\n");
        printf("5. 用户账号管理\n");
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
        
        // 将输入字符串转换为整数
        choice = atoi(input);
        
        // 验证输入是否为有效数字选项 (0-5)
        if (choice < 0 || choice > 5) {
            printf("无效的选择，请输入 0-5 或 A。\n");
            pause_and_wait();
            continue;
        }
        
        switch (choice) {
            case 1: 
                patient_management_menu(db, dataDir); 
                break;
            case 2: 
                archive_management_menu(db, dataDir); 
                break;
            case 3: 
                drug_management_menu(db, dataDir); 
                break;
            case 4: 
                management_report(db); 
                pause_and_wait(); 
                break;
            case 5:
                user_account_management_menu(db, dataDir);
                break;
            case 0: 
                logout_menu();
                return;
        }
    }
}

/* ==================== 辅助统计函数 ==================== */

/* 统计患者总数 - 遍历患者链表计算节点数量 */
static int count_patients(Database *db) {
    int c = 0;
    Patient *p = db->patients;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}

/* 统计医生总数 - 遍历医生链表计算节点数量 */
static int count_doctors(Database *db) {
    int c = 0;
    Doctor *p = db->doctors;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}

/* 统计挂号记录总数 - 遍历挂号链表计算节点数量 */
static int count_regs(Database *db) {
    int c = 0;
    Registration *p = db->registrations;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}

/* 统计住院患者总数 - 遍历住院链表计算节点数量 */
static int count_inpatients(Database *db) {
    int c = 0;
    Inpatient *p = db->inpatients;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}

/* 统计药品总数 - 遍历药品链表计算节点数量 */
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
 * 函数：read_line_or_back - 读取字符串输入，支持返回上一步
 * 
 * 功能说明：
 *   封装了 read_line 函数，增加了对"0"输入的特殊处理。
 *   当用户输入"0"时，表示希望返回上一步操作，函数返回 0。
 *   否则返回 1 表示成功读取了有效输入。
 * 
 * 参数：
 *   prompt - 显示给用户的提示信息
 *   buf - 存储输入的缓冲区
 *   size - 缓冲区大小
 * 
 * 返回值：
 *   1 - 成功读取有效输入（非"0"）
 *   0 - 用户输入"0"选择返回上一步
 */
int read_line_or_back(const char *prompt, char *buf, int size) {
    read_line(prompt, buf, size);
    if (strcmp(buf, "0") == 0) {
        return 0;
    }
    return 1;
}

/* 
 * 函数：read_gender_with_back - 读取性别输入并验证，支持返回上一步
 * 
 * 功能说明：
 *   循环读取用户输入的性别，调用 validate_gender 验证是否为"男"或"女"。
 *   输入"0"可返回上一步。
 */
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
    printf("看诊记录已添加，编号：%d。\n", v->id);
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
 * 说明：修改看诊记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要修改的看诊编号
 * 2. 查找看诊记录
 * 3. 逐步修改诊断结果、检查项目、处方信息
 */
static void edit_visit(Database *db, const char *dataDir) {
    int id = read_int("要修改的看诊编号 (输入 0 返回): ", 0, 1000000);
    Visit *cur;
    char diagnosis[TEXT_LEN], examItems[TEXT_LEN], prescription[TEXT_LEN];
    
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }
    
    cur = db->visits;
    while (cur && cur->id != id) {
        cur = cur->next;
    }
    if (!cur) { 
        printf("看诊记录不存在。\n"); 
        return; 
    }
    
    printf("\n=== 修改看诊记录 ===\n");
    printf("当前信息：\n");
    printf("  诊断结果：%s\n", cur->diagnosis);
    printf("  检查项目：%s\n", cur->examItems);
    printf("  处方信息：%s\n", cur->prescription);
    
    printf("\n请输入新信息（直接回车保持原值，输入 0 取消修改）：\n");
    
    /* 修改诊断结果 */
    printf("诊断结果 [%s]: ", cur->diagnosis);
    read_line("", diagnosis, sizeof(diagnosis));
    if (strcmp(diagnosis, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(diagnosis) > 0) {
        safe_copy(cur->diagnosis, diagnosis, sizeof(cur->diagnosis));
    }
    
    /* 修改检查项目 */
    printf("检查项目 [%s]: ", cur->examItems);
    read_line("", examItems, sizeof(examItems));
    if (strcmp(examItems, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(examItems) > 0) {
        safe_copy(cur->examItems, examItems, sizeof(cur->examItems));
    }
    
    /* 修改处方信息 */
    printf("处方信息 [%s]: ", cur->prescription);
    read_line("", prescription, sizeof(prescription));
    if (strcmp(prescription, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(prescription) > 0) {
        safe_copy(cur->prescription, prescription, sizeof(cur->prescription));
    }
    
    save_all(db, dataDir);
    printf("\n看诊记录修改成功！\n");
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
    printf("检查记录已添加，编号：%d。\n", e->id);
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
 * 说明：修改检查记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要修改的检查编号
 * 2. 查找检查记录
 * 3. 逐步修改患者病历号、医生工号、检查编码、项目名称、执行时间、费用、结果
 */
static void edit_exam(Database *db, const char *dataDir) {
    int id = read_int("要修改的检查编号 (输入 0 返回): ", 0, 1000000);
    Exam *cur;
    char buf[64];
    
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }
    
    cur = db->exams;
    while (cur && cur->id != id) {
        cur = cur->next;
    }
    if (!cur) { 
        printf("检查记录不存在。\n"); 
        return; 
    }
    
    printf("\n=== 修改检查记录 ===\n");
    printf("当前信息：\n");
    printf("  患者病历号：%d\n", cur->patientId);
    printf("  医生工号：%d\n", cur->doctorId);
    printf("  检查编码：%s\n", cur->code);
    printf("  项目名称：%s\n", cur->itemName);
    printf("  执行时间：%s\n", cur->execTime);
    printf("  检查费用：%.2f\n", cur->fee);
    printf("  检查结果：%s\n", cur->result);
    
    printf("\n请输入新信息（直接回车保持原值，输入 0 取消修改）：\n");
    
    /* 修改患者病历号 */
    printf("患者病历号 [%d]: ", cur->patientId);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        int newPid = atoi(buf);
        if (find_patient(db, newPid)) {
            cur->patientId = newPid;
        } else {
            printf("患者不存在，已保持原值。\n");
        }
    }
    
    /* 修改医生工号 */
    printf("医生工号 [%d]: ", cur->doctorId);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        int newDid = atoi(buf);
        if (find_doctor(db, newDid)) {
            cur->doctorId = newDid;
        } else {
            printf("医生不存在，已保持原值。\n");
        }
    }
    
    /* 修改检查编码 */
    printf("检查编码 [%s]: ", cur->code);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        safe_copy(cur->code, buf, sizeof(cur->code));
    }
    
    /* 修改项目名称 */
    printf("项目名称 [%s]: ", cur->itemName);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        safe_copy(cur->itemName, buf, sizeof(cur->itemName));
    }
    
    /* 修改执行时间 */
    printf("执行时间 [%s]: ", cur->execTime);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        if (strlen(buf) == 10 && buf[4] == '-' && buf[7] == '-') {
            safe_copy(cur->execTime, buf, sizeof(cur->execTime));
        } else {
            printf("日期格式应为 YYYY-MM-DD，已保持原值。\n");
        }
    }
    
    /* 修改检查费用 */
    printf("检查费用 [%.2f]: ", cur->fee);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        cur->fee = atof(buf);
    }
    
    /* 修改检查结果 */
    printf("检查结果 [%s]: ", cur->result);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        safe_copy(cur->result, buf, sizeof(cur->result));
    }
    
    save_all(db, dataDir);
    printf("\n检查记录修改成功！\n");
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
 * 说明：修改住院记录
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 * 
 * 流程：
 * 1. 读取要修改的住院编号
 * 2. 查找住院记录并显示当前信息
 * 3. 允许用户修改各项字段（直接回车保持原值，输入 0 取消）
 * 4. 保存修改
 */
static void edit_inpatient(Database *db, const char *dataDir) {
    int id = read_int("要修改的住院编号 (输入 0 返回): ", 0, 1000000);
    Inpatient *cur;
    char buf[64];
    
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }
    
    cur = db->inpatients;
    while (cur && cur->id != id) {
        cur = cur->next;
    }
    if (!cur) { 
        printf("住院记录不存在。\n"); 
        return; 
    }
    
    printf("\n=== 修改住院记录 ===\n");
    printf("当前信息：\n");
    printf("  患者病历号：%d\n", cur->patientId);
    printf("  病房编号：%d\n", cur->wardId);
    printf("  床位号：%d\n", cur->bedNo);
    printf("  入院日期：%s\n", cur->admitDate);
    printf("  预计出院日期：%s\n", cur->expectedDischarge);
    printf("  预估住院费用：%.2f\n", cur->totalCost);
    
    printf("\n请输入新信息（直接回车保持原值，输入 0 取消修改）：\n");
    
    /* 修改患者病历号 */
    printf("患者病历号 [%d]: ", cur->patientId);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        int newPid = atoi(buf);
        if (find_patient(db, newPid)) {
            cur->patientId = newPid;
        } else {
            printf("患者不存在，已保持原值。\n");
        }
    }
    
    /* 修改病房编号 */
    printf("病房编号 [%d]: ", cur->wardId);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        int newWardId = atoi(buf);
        Ward *w = find_ward(db, newWardId);
        if (w) {
            cur->wardId = newWardId;
        } else {
            printf("病房不存在，已保持原值。\n");
        }
    }
    
    /* 修改床位号 */
    printf("床位号 [%d]: ", cur->bedNo);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        cur->bedNo = atoi(buf);
    }
    
    /* 修改入院日期 */
    printf("入院日期 [%s]: ", cur->admitDate);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        if (validate_date(buf)) {
            safe_copy(cur->admitDate, buf, sizeof(cur->admitDate));
        } else {
            printf("日期格式无效，已保持原值。\n");
        }
    }
    
    /* 修改预计出院日期 */
    printf("预计出院日期 [%s]: ", cur->expectedDischarge);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        if (validate_date(buf)) {
            safe_copy(cur->expectedDischarge, buf, sizeof(cur->expectedDischarge));
        } else {
            printf("日期格式无效，已保持原值。\n");
        }
    }
    
    /* 修改预估住院费用 */
    printf("预估住院费用 [%.2f]: ", cur->totalCost);
    read_line("", buf, sizeof(buf));
    if (strcmp(buf, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(buf) > 0) {
        cur->totalCost = atof(buf);
    }
    
    save_all(db, dataDir);
    printf("\n住院记录修改成功！\n");
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
        printf("\n--- 看诊记录管理 ---\n");
        printf("1. 新增看诊记录\n");
        printf("2. 删除看诊记录\n");
        printf("3. 修改看诊记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 3);
        if (choice == 0) return;
        if (choice == 1) add_visit(db, dataDir);
        else if (choice == 2) delete_visit(db, dataDir);
        else if (choice == 3) edit_visit(db, dataDir);
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
        printf("\n--- 检查记录管理 ---\n");
        printf("1. 新增检查记录\n");
        printf("2. 删除检查记录\n");
        printf("3. 修改检查记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 3);
        if (choice == 0) return;
        if (choice == 1) add_exam(db, dataDir);
        else if (choice == 2) delete_exam(db, dataDir);
        else if (choice == 3) edit_exam(db, dataDir);
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
        printf("\n--- 住院记录管理 ---\n");
        printf("1. 新增住院记录\n");
        printf("2. 删除住院记录\n");
        printf("3. 修改住院记录\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 3);
        if (choice == 0) return;
        if (choice == 1) add_inpatient(db, dataDir);
        else if (choice == 2) delete_inpatient(db, dataDir);
        else if (choice == 3) edit_inpatient(db, dataDir);
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
        printf("4. 修改患者\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 4);
        if (choice == 0) return;
        if (choice == 1) list_patients(db);
        else if (choice == 2) add_patient(db, dataDir);
        else if (choice == 3) delete_patient(db, dataDir);
        else if (choice == 4) edit_patient(db, dataDir);
        pause_and_wait();
    }
}

/* ==================== 档案管理菜单 ==================== */
/*
 * 说明：档案管理子菜单
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
void archive_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 档案管理 ---\n");
        printf("1. 新增档案（患者/医生）\n");
        printf("2. 修改档案（患者/医生）\n");
        printf("3. 删除档案（患者/医生）\n");
        printf("4. 关联账号\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择：", 0, 4);
        if (choice == 0) return;
        else if (choice == 1) add_archive(db, dataDir);
        else if (choice == 2) edit_archive(db, dataDir);
        else if (choice == 3) delete_archive(db, dataDir);
        else if (choice == 4) link_archive_to_account(db, dataDir);
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

/* ==================== 用户账号管理菜单 ==================== */
void user_account_management_menu(Database *db, const char *dataDir)
{
    (void)dataDir;
    if (!check_permission(ROLE_MANAGER))
    {
        printf("权限不足！\n");
        pause_and_wait();
        return;
    }

    while (1)
    {
        printf("\n\n============================================\n");
        printf("         用户账号管理\n");
        printf("============================================\n");
        printf("1. 查看所有账号\n");
        printf("2. 新增账号\n");
        printf("3. 删除账号\n");
        printf("4. 修改密码\n");
        printf("0. 返回上级菜单\n");
        printf("============================================\n");
        printf("请选择：");

        int choice;
        if (scanf("%d", &choice) != 1)
        {
            while (getchar() != '\n');
            continue;
        }

        switch (choice)
        {
            case 1:
            {
                printf("\n--- 所有账号列表 ---\n");
                Account *acc = db->accounts;
                if (!acc)
                {
                    printf("暂无账号记录。\n");
                }
                else
                {
                    printf("%-20s %-15s %-10s\n", "用户名", "角色", "关联ID");
                    printf("--------------------------------------------\n");
                    while (acc)
                    {
                        printf("%-20s %-15s %-10d\n", acc->username, get_role_name(acc->role), acc->linkedId);
                        acc = acc->next;
                    }
                }
                pause_and_wait();
                break;
            }
            case 2:
            {
                char username[32], password[64];
                int roleVal, linkedId;

                printf("\n--- 新增账号 ---\n");
                printf("用户名：");
                scanf("%31s", username);

                if (find_account(db, username))
                {
                    printf("错误：用户名已存在！\n");
                    pause_and_wait();
                    break;
                }

                printf("密码：");
                scanf("%63s", password);

                printf("角色（0-患者，1-医生，2-管理员）：");
                if (scanf("%d", &roleVal) != 1 || roleVal < 0 || roleVal > 2)
                {
                    printf("无效的角色选择！\n");
                    pause_and_wait();
                    break;
                }

                printf("关联ID（患者病历号/医生工号/管理员填0）：");
                if (scanf("%d", &linkedId) != 1)
                {
                    printf("无效的关联ID！\n");
                    pause_and_wait();
                    break;
                }

                if (create_account(db, username, password, (UserRole)roleVal, linkedId))
                {
                    printf("账号创建成功！\n");
                    save_all(db, dataDir);
                }
                else
                {
                    printf("账号创建失败！\n");
                }
                pause_and_wait();
                break;
            }
            case 3:
            {
                char username[32];
                printf("\n--- 删除账号 ---\n");
                printf("请输入要删除的用户名：");
                scanf("%31s", username);

                Account *acc = find_account(db, username);
                if (!acc)
                {
                    printf("未找到该用户！\n");
                    pause_and_wait();
                    break;
                }

                // 不允许删除自己
                if (strcmp(acc->username, g_session.username) == 0)
                {
                    printf("不能删除当前登录的账号！\n");
                    pause_and_wait();
                    break;
                }

                // 从链表中移除
                if (db->accounts == acc)
                {
                    db->accounts = acc->next;
                }
                else
                {
                    Account *prev = db->accounts;
                    while (prev && prev->next != acc)
                        prev = prev->next;
                    if (prev)
                        prev->next = acc->next;
                }
                // 只释放当前节点，不要调用 free_accounts 因为它会释放整个后续链表
                free(acc);
                printf("账号 %s 已删除！\n", username);
                save_all(db, dataDir);
                pause_and_wait();
                break;
            }
            case 4:
            {
                char username[32], newPassword[64];
                printf("\n--- 修改密码 ---\n");
                printf("请输入用户名：");
                scanf("%31s", username);

                Account *acc = find_account(db, username);
                if (!acc)
                {
                    printf("未找到该用户！\n");
                    pause_and_wait();
                    break;
                }

                printf("请输入新密码：");
                scanf("%63s", newPassword);

                strncpy(acc->password, newPassword, sizeof(acc->password) - 1);
                acc->password[sizeof(acc->password) - 1] = '\0';

                printf("密码修改成功！\n");
                save_all(db, dataDir);
                pause_and_wait();
                break;
            }
            case 0:
                return;
            default:
                printf("无效选择！\n");
                pause_and_wait();
                break;
        }
    }
}

/* ==================== 档案管理辅助函数 ==================== */

/*
 * 说明：创建医生档案
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void create_doctor_archive(Database *db, const char *dataDir) {
    Doctor *d = (Doctor*)malloc(sizeof(Doctor));
    if (!d) {
        printf("内存分配失败！\n");
        pause_and_wait();
        return;
    }
    d->id = next_doctor_id(db);
    printf("自动生成的工号：%d\n", d->id);
    
    read_line("姓名：", d->name, sizeof(d->name));
    read_line("科室：", d->dept, sizeof(d->dept));
    read_line("职称：", d->title, sizeof(d->title));
    
    d->archived = 0;
    d->next = NULL;
    
    // 追加到链表
    if (!db->doctors)
        db->doctors = d;
    else {
        Doctor *cur = db->doctors;
        while (cur->next) cur = cur->next;
        cur->next = d;
    }
    
    printf("医生档案创建成功！工号：%d\n", d->id);
    save_all(db, dataDir);
}

/*
 * 说明：关联档案到账号
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void link_archive_to_account(Database *db, const char *dataDir) {
    char username[32];
    int targetId;
    printf("\n--- 关联档案到账号 ---\n");
    printf("请输入要关联档案的用户名：");
    scanf("%31s", username);

    Account *acc = find_account(db, username);
    if (!acc) {
        printf("未找到该用户！\n");
        pause_and_wait();
        return;
    }

    if (acc->role == ROLE_MANAGER) {
        printf("管理员账号不需要关联档案。\n");
        pause_and_wait();
        return;
    }

    printf("当前关联 ID：%d\n", acc->linkedId);
    
    if (acc->role == ROLE_PATIENT) {
        printf("请输入要关联的患者病历号：");
        if (scanf("%d", &targetId) != 1) {
            printf("无效输入！\n");
            pause_and_wait();
            return;
        }
        
        Patient *p = find_patient(db, targetId);
        if (!p) {
            printf("未找到该病历号对应的患者档案！\n");
            pause_and_wait();
            return;
        }
        
        if (acc->linkedId != 0 && acc->linkedId != targetId) {
            printf("警告：该账号已关联其他患者档案（原病历号：%d）。\n", acc->linkedId);
            printf("确认要重新关联吗？(y/n): ");
            char confirm[16];
            scanf("%15s", confirm);
            if (confirm[0] != 'y' && confirm[0] != 'Y') {
                printf("已取消操作。\n");
                pause_and_wait();
                return;
            }
        }
        
        acc->linkedId = targetId;
        printf("已将患者账号 %s 关联到病历号 %d。\n", username, targetId);
    } else if (acc->role == ROLE_DOCTOR) {
        printf("请输入要关联的医生工号：");
        if (scanf("%d", &targetId) != 1) {
            printf("无效输入！\n");
            pause_and_wait();
            return;
        }
        
        Doctor *d = find_doctor(db, targetId);
        if (!d) {
            printf("未找到该工号对应的医生档案！\n");
            pause_and_wait();
            return;
        }
        
        if (acc->linkedId != 0 && acc->linkedId != targetId) {
            printf("警告：该账号已关联其他医生档案（原工号：%d）。\n", acc->linkedId);
            printf("确认要重新关联吗？(y/n): ");
            char confirm[16];
            scanf("%15s", confirm);
            if (confirm[0] != 'y' && confirm[0] != 'Y') {
                printf("已取消操作。\n");
                pause_and_wait();
                return;
            }
        }
        
        acc->linkedId = targetId;
        printf("已将医生账号 %s 关联到工号 %d。\n", username, targetId);
    }
    
    save_all(db, dataDir);
}

/*
 * 说明：新增医生或患者档案
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void add_archive(Database *db, const char *dataDir) {
    int choice;
    printf("\n--- 新增档案 ---\n");
    printf("1. 新增患者档案\n");
    printf("2. 新增医生档案\n");
    printf("0. 返回上一步\n");
    choice = read_int("请选择：", 0, 2);
    
    if (choice == 0) return;
    else if (choice == 1) add_patient(db, dataDir);
    else if (choice == 2) create_doctor_archive(db, dataDir);
}

/*
 * 说明：删除档案（患者或医生）
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void delete_archive(Database *db, const char *dataDir) {
    int choice;
    printf("\n--- 删除档案 ---\n");
    printf("1. 删除患者档案\n");
    printf("2. 删除医生档案\n");
    printf("0. 返回上一步\n");
    choice = read_int("请选择：", 0, 2);
    
    if (choice == 0) return;
    else if (choice == 1) delete_patient(db, dataDir);
    else if (choice == 2) delete_doctor(db, dataDir);
}

/*
 * 说明：修改档案（患者或医生）
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void edit_archive(Database *db, const char *dataDir) {
    int choice;
    printf("\n--- 修改档案 ---\n");
    printf("1. 修改患者档案\n");
    printf("2. 修改医生档案\n");
    printf("0. 返回上一步\n");
    choice = read_int("请选择：", 0, 2);
    
    if (choice == 0) return;
    else if (choice == 1) edit_patient(db, dataDir);
    else if (choice == 2) edit_doctor(db, dataDir);
}

/*
 * 说明：删除医生档案
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void delete_doctor(Database *db, const char *dataDir) {
    int id = read_int("要删除的医生工号 (输入 0 返回): ", 0, 1000000);
    Doctor *prev = NULL;
    Doctor *cur = db->doctors;
    char confirm[16];
    
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }

    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }

    if (!cur) {
        printf("医生不存在。\n");
        return;
    }

    /* 检查医生是否有关联的挂号记录 */
    Registration *r;
    for (r = db->registrations; r; r = r->next) {
        if (r->doctorId == id) {
            printf("删除失败：该医生存在关联的挂号记录，请先处理关联数据。\n");
            return;
        }
    }

    printf("确认删除医生 [%d] %s ? (y/n): ", cur->id, cur->name);
    read_line(NULL, confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("已取消删除。\n");
        return;
    }

    if (prev) prev->next = cur->next;
    else db->doctors = cur->next;
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 说明：修改患者档案
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void edit_patient(Database *db, const char *dataDir) {
    int id = read_int("请输入要修改的患者病历号 (输入 0 返回): ", 0, 1000000);
    Patient *p;
    char newName[NAME_LEN], newGender[16], newBirth[DATE_LEN], newPhone[PHONE_LEN], newInsurance[SMALL_LEN];
    
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }
    
    p = find_patient(db, id);
    if (!p) {
        printf("未找到该病历号对应的患者档案。\n");
        return;
    }
    
    printf("\n=== 修改患者档案 ===\n");
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
    save_all(db, dataDir);
    
    printf("\n患者档案修改成功！\n");
    printf("更新后的信息：\n");
    printf("  姓名：%s\n", p->name);
    printf("  性别：%s\n", p->gender);
    printf("  出生日期：%s\n", p->birth);
    printf("  联系电话：%s\n", p->phone);
    printf("  医保类型：%s\n", p->insurance);
}

/*
 * 说明：修改医生档案
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件目录
 */
static void edit_doctor(Database *db, const char *dataDir) {
    int id = read_int("请输入要修改的医生工号 (输入 0 返回): ", 0, 1000000);
    Doctor *d;
    char newName[NAME_LEN], newDept[SMALL_LEN], newTitle[SMALL_LEN];
    
    if (id == 0) { 
        printf("已返回上一步。\n"); 
        return; 
    }
    
    d = find_doctor(db, id);
    if (!d) {
        printf("未找到该工号对应的医生档案。\n");
        return;
    }
    
    printf("\n=== 修改医生档案 ===\n");
    printf("当前信息：\n");
    printf("  工号：%d\n", d->id);
    printf("  姓名：%s\n", d->name);
    printf("  科室：%s\n", d->dept);
    printf("  职称：%s\n", d->title);
    
    printf("\n请输入新信息（直接回车保持原值，输入 0 取消修改）：\n");
    
    /* 修改姓名 */
    printf("姓名 [%s]: ", d->name);
    read_line("", newName, sizeof(newName));
    if (strcmp(newName, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newName) > 0) {
        safe_copy(d->name, newName, sizeof(d->name));
    }
    
    /* 修改科室 */
    printf("科室 [%s]: ", d->dept);
    read_line("", newDept, sizeof(newDept));
    if (strcmp(newDept, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newDept) > 0) {
        safe_copy(d->dept, newDept, sizeof(d->dept));
    }
    
    /* 修改职称 */
    printf("职称 [%s]: ", d->title);
    read_line("", newTitle, sizeof(newTitle));
    if (strcmp(newTitle, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newTitle) > 0) {
        safe_copy(d->title, newTitle, sizeof(d->title));
    }
    
    /* 保存修改到文件 */
    save_all(db, dataDir);
    
    printf("\n医生档案修改成功！\n");
    printf("更新后的信息：\n");
    printf("  工号：%d\n", d->id);
    printf("  姓名：%s\n", d->name);
    printf("  科室：%s\n", d->dept);
    printf("  职称：%s\n", d->title);
}
