#include "menu_business.h"
#include "data.h"
#include "auth.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ==================== 前向声明 ==================== */
static void logout_menu(void);
static int registration_has_visit(Database *db, int regId);
static int count_patient_regs_same_day_dept(Database *db, int patientId, const char *date, const char *dept);
static int count_patient_regs_same_day(Database *db, int patientId, const char *date);
static int read_date_with_back(const char *prompt, char *buf, int size);

/* ==================== 全局登录会话 ==================== */
UserSession g_session = {0, ROLE_PATIENT, 0, ""};

/* ==================== 登录与注册功能 ==================== */

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
    
    if (find_account(db, username) != NULL) {
        printf("用户名已存在。\n");
        return;
    }
    
    while (1) {
        printf("请输入密码（至少 4 位）：");
        read_line("", password, sizeof(password));
        if (strlen(password) < 4) {
            printf("密码长度至少为 4 位，请重新输入。\n");
            continue;
        }
        
        printf("请确认密码：");
        read_line("", confirm, sizeof(confirm));
        if (strcmp(password, confirm) != 0) {
            printf("两次输入的密码不一致，请重新输入密码。\n");
            continue;
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
            printf("请输入关联的医生工号：");
            linkedId = read_int("", 1, 1000000);
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
        return -1;
    }
}

static void logout_menu(void) {
    logout_user();
    printf("已成功登出。\n");
}

/* ==================== 患者角色菜单 ==================== */

static int registration_has_visit(Database *db, int regId) {
    Visit *v;
    for (v = db->visits; v; v = v->next) {
        if (v->regId == regId) return 1;
    }
    return 0;
}

static int count_patient_regs_same_day_dept(Database *db, int patientId, const char *date, const char *dept) {
    Registration *r;
    int count = 0;
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == patientId && strcmp(r->date, date) == 0 && strcmp(r->dept, dept) == 0) {
            count++;
        }
    }
    return count;
}

static int count_patient_regs_same_day(Database *db, int patientId, const char *date) {
    Registration *r;
    int count = 0;
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == patientId && strcmp(r->date, date) == 0) {
            count++;
        }
    }
    return count;
}

static int read_date_with_back(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    read_line("", buf, size);
    if (strcmp(buf, "0") == 0) return 0;
    if (!validate_date(buf)) {
        printf("日期格式不正确，应为 YYYY-MM-DD。\n");
        return 0;
    }
    return 1;
}

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
    
    printf("\n--- 挂号记录 ---\n");
    found = 0;
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == g_session.userId) {
            printf("[%d] %s %s 医生%d %s %s\n", r->id, r->date, r->dept, r->doctorId, r->type, r->status);
            found = 1;
        }
    }
    if (!found) printf("暂无挂号记录。\n");
    
    printf("\n--- 看诊记录 ---\n");
    found = 0;
    for (v = db->visits; v; v = v->next) {
        Registration *reg = find_registration(db, v->regId);
        if (reg && reg->patientId == g_session.userId) {
            printf("[挂号%d] 诊断：%s\n    检查项目：%s\n    处方：%s\n", 
                   v->regId, v->diagnosis, v->examItems, v->prescription);
            found = 1;
        }
    }
    if (!found) printf("暂无看诊记录。\n");
    
    printf("\n--- 检查记录 ---\n");
    found = 0;
    for (e = db->exams; e; e = e->next) {
        if (e->patientId == g_session.userId) {
            printf("[%d] %s %s %.2f %s\n", e->id, e->code, e->itemName, e->fee, e->result);
            found = 1;
        }
    }
    if (!found) printf("暂无检查记录。\n");
    
    printf("\n--- 住院记录 ---\n");
    found = 0;
    for (ip = db->inpatients; ip; ip = ip->next) {
        if (ip->patientId == g_session.userId) {
            printf("[%d] 病房%d 床位%d %s ~ %s 费用%.2f\n", 
                   ip->id, ip->wardId, ip->bedNo, ip->admitDate, ip->expectedDischarge, ip->totalCost);
            found = 1;
        }
    }
    if (!found) printf("暂无住院记录。\n");
}

static void patient_add_registration(Database *db, const char *dataDir) {
    Registration *r;
    int doctorId = 0;
    int step = 0;
    char dept[SMALL_LEN], date[DATE_LEN], type[SMALL_LEN];
    int patientId = g_session.userId;
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
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

static void patient_edit_profile(Database *db, const char *dataDir) {
    Patient *p;
    char newName[NAME_LEN], newGender[16], newBirth[DATE_LEN], newPhone[PHONE_LEN], newInsurance[SMALL_LEN];
    
    if (!g_session.isLoggedIn || g_session.role != ROLE_PATIENT) {
        printf("权限不足。\n");
        return;
    }
    
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
    
    printf("姓名 [%s]: ", p->name);
    read_line("", newName, sizeof(newName));
    if (strcmp(newName, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newName) > 0) {
        safe_copy(p->name, newName, sizeof(p->name));
    }
    
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
    
    printf("出生日期 [%s]: ", p->birth);
    read_line("", newBirth, sizeof(newBirth));
    if (strcmp(newBirth, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newBirth) > 0) {
        if (!validate_date(newBirth)) {
            printf("日期格式不正确，应为 YYYY-MM-DD，已保持原值。\n");
        } else {
            safe_copy(p->birth, newBirth, sizeof(p->birth));
        }
    }
    
    printf("联系电话 [%s]: ", p->phone);
    read_line("", newPhone, sizeof(newPhone));
    if (strcmp(newPhone, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newPhone) > 0) {
        if (!validate_phone(newPhone)) {
            printf("手机号应为 11 位数字，已保持原值。\n");
        } else {
            safe_copy(p->phone, newPhone, sizeof(p->phone));
        }
    }
    
    printf("医保类型 [%s]: ", p->insurance);
    read_line("", newInsurance, sizeof(newInsurance));
    if (strcmp(newInsurance, "0") == 0) {
        printf("已取消修改。\n");
        return;
    }
    if (strlen(newInsurance) > 0) {
        safe_copy(p->insurance, newInsurance, sizeof(p->insurance));
    }
    
    save_all(db, dataDir);
    
    printf("\n患者档案修改成功！\n");
    printf("更新后的信息：\n");
    printf("  姓名：%s\n", p->name);
    printf("  性别：%s\n", p->gender);
    printf("  出生日期：%s\n", p->birth);
    printf("  联系电话：%s\n", p->phone);
    printf("  医保类型：%s\n", p->insurance);
}

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
            case 3: patient_edit_profile(db, dataDir); break;
            case 0: logout_menu(); return;
        }
    }
}

/* ==================== 医生角色菜单 ==================== */

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
            case 0: logout_menu(); return;
        }
    }
}

/* 以下函数需要从 menu_admin.c 引用，在此声明 */
void visit_management_menu(Database *db, const char *dataDir);
void exam_management_menu(Database *db, const char *dataDir);
void inpatient_management_menu(Database *db, const char *dataDir);
