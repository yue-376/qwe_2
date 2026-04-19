/*
 * 文件：missing_funcs.c
 * 说明：补全 menu_business.c 和 menu_admin.c 中缺失的函数实现
 */

#include "models.h"
#include "data.h"
#include "common.h"
#include "auth.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ==================== 前向声明 ==================== */
void create_doctor_archive(Database *db, const char *dataDir);
void delete_doctor(Database *db, const char *dataDir);
void edit_doctor(Database *db, const char *dataDir);
void read_date_with_back(const char *prompt, char *buf, int size);
int registration_has_visit(Database *db, int regId);
int count_patient_regs_same_day_dept(Database *db, int patientId, const char *date, const char *dept);
int count_patient_regs_same_day(Database *db, int patientId, const char *date);
void edit_patient(Database *db, const char *dataDir);
void edit_visit(Database *db, const char *dataDir);
void edit_exam(Database *db, const char *dataDir);
void edit_inpatient(Database *db, const char *dataDir);
void add_archive(Database *db, const char *dataDir);
void edit_archive(Database *db, const char *dataDir);
void delete_archive(Database *db, const char *dataDir);
void link_archive_to_account(Database *db, const char *dataDir);

/* ==================== 辅助函数 ==================== */

/*
 * 读取日期输入，支持输入"0"返回上一步
 */
void read_date_with_back(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    read_line("", buf, size);
}

/*
 * 检查挂号记录是否有关联的看诊记录
 */
int registration_has_visit(Database *db, int regId) {
    Visit *v;
    for (v = db->visits; v; v = v->next) {
        if (v->regId == regId) {
            return 1;
        }
    }
    return 0;
}

/*
 * 统计患者某天某科室的挂号次数
 */
int count_patient_regs_same_day_dept(Database *db, int patientId, const char *date, const char *dept) {
    Registration *r;
    int count = 0;
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == patientId && strcmp(r->date, date) == 0 && strcmp(r->dept, dept) == 0) {
            count++;
        }
    }
    return count;
}

/*
 * 统计患者某天的挂号总次数
 */
int count_patient_regs_same_day(Database *db, int patientId, const char *date) {
    Registration *r;
    int count = 0;
    for (r = db->registrations; r; r = r->next) {
        if (r->patientId == patientId && strcmp(r->date, date) == 0) {
            count++;
        }
    }
    return count;
}

/*
 * 检查药品是否有出入库日志
 */
int drug_has_logs(Database *db, int drugId) {
    DrugLog *log;
    for (log = db->drugLogs; log; log = log->next) {
        if (log->drugId == drugId) {
            return 1;
        }
    }
    return 0;
}

/* ==================== 统计函数 ==================== */

/*
 * 统计患者总数
 */
int count_patients(Database *db) {
    Patient *p;
    int count = 0;
    for (p = db->patients; p; p = p->next) {
        count++;
    }
    return count;
}

/*
 * 统计医生总数
 */
int count_doctors(Database *db) {
    Doctor *d;
    int count = 0;
    for (d = db->doctors; d; d = d->next) {
        count++;
    }
    return count;
}

/*
 * 统计挂号记录总数
 */
int count_regs(Database *db) {
    Registration *r;
    int count = 0;
    for (r = db->registrations; r; r = r->next) {
        count++;
    }
    return count;
}

/*
 * 统计住院人数
 */
int count_inpatients(Database *db) {
    Inpatient *ip;
    int count = 0;
    for (ip = db->inpatients; ip; ip = ip->next) {
        count++;
    }
    return count;
}

/*
 * 统计药品种类
 */
int count_drugs(Database *db) {
    Drug *d;
    int count = 0;
    for (d = db->drugs; d; d = d->next) {
        count++;
    }
    return count;
}

/* ==================== 登出功能 ==================== */

/*
 * 登出菜单
 */
void logout_menu(void) {
    logout_user();
    printf("已成功登出。\n");
}

/* ==================== 患者管理函数 ==================== */

/*
 * 查看患者列表
 */
void list_patients(Database *db) {
    Patient *p;
    printf("\n--- 患者列表 ---\n");
    printf("+--------+----------+------+------------+-------------+--------------+\n");
    printf("| %-6s | %-8s | %-4s | %-10s | %-11s | %-12s |\n", "病历号", "姓名", "性别", "出生日期", "联系电话", "医保类型");
    printf("+--------+----------+------+------------+-------------+--------------+\n");
    for (p = db->patients; p; p = p->next) {
        printf("| %-6d | %-8s | %-4s | %-10s | %-11s | %-12s |\n", 
               p->id, p->name, p->gender, p->birth, p->phone, p->insurance);
    }
    printf("+--------+----------+------+------------+-------------+--------------+\n");
}

/*
 * 新增患者
 */
void add_patient(Database *db, const char *dataDir) {
    Patient *p = (Patient *)malloc(sizeof(Patient));
    if (!p) { printf("内存分配失败。\n"); return; }
    
    p->id = next_patient_id(db);
    printf("新增患者 (病历号自动分配：%d)\n", p->id);
    
    printf("姓名: ");
    read_line("", p->name, NAME_LEN);
    
    printf("性别 (男/女): ");
    read_line("", p->gender, 16);
    
    printf("出生日期 (YYYY-MM-DD): ");
    read_line("", p->birth, DATE_LEN);
    
    printf("联系电话: ");
    read_line("", p->phone, PHONE_LEN);
    
    printf("医保类型: ");
    read_line("", p->insurance, SMALL_LEN);
    
    p->archived = 0;
    p->next = NULL;
    
    /* 添加到链表头部 */
    p->next = db->patients;
    db->patients = p;
    
    save_all(db, dataDir);
    printf("患者添加成功。\n");
}

/*
 * 删除患者
 */
void delete_patient(Database *db, const char *dataDir) {
    int id = read_int("要删除的患者病历号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Patient *prev = NULL;
    Patient *cur = db->patients;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) {
        printf("患者不存在。\n");
        return;
    }
    
    char confirm[16];
    printf("确认删除患者 [%d] %s? (y/n): ", cur->id, cur->name);
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
 * 修改患者档案
 */
void edit_patient(Database *db, const char *dataDir) {
    int id = read_int("要修改的患者病历号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Patient *p = find_patient(db, id);
    if (!p) {
        printf("患者不存在。\n");
        return;
    }
    
    printf("当前信息：姓名=%s, 性别=%s, 出生日期=%s, 电话=%s, 医保=%s\n",
           p->name, p->gender, p->birth, p->phone, p->insurance);
    
    printf("新姓名 (回车保持不变): ");
    char buf[256];
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->name, buf, NAME_LEN - 1);
    
    printf("新性别 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->gender, buf, 16);
    
    printf("新出生日期 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->birth, buf, DATE_LEN - 1);
    
    printf("新联系电话 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->phone, buf, PHONE_LEN - 1);
    
    printf("新医保类型 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->insurance, buf, SMALL_LEN - 1);
    
    save_all(db, dataDir);
    printf("修改成功。\n");
}

/* ==================== 档案管理函数 ==================== */

/*
 * 新增档案（患者或医生）
 */
void add_archive(Database *db, const char *dataDir) {
    int type = read_int("选择档案类型 (1-患者，2-医生): ", 1, 2);
    if (type == 1) {
        add_patient(db, dataDir);
    } else if (type == 2) {
        create_doctor_archive(db, dataDir);
    }
}

/*
 * 修改档案（患者或医生）
 */
void edit_archive(Database *db, const char *dataDir) {
    int type = read_int("选择档案类型 (1-患者，2-医生): ", 1, 2);
    if (type == 1) {
        edit_patient(db, dataDir);
    } else if (type == 2) {
        edit_doctor(db, dataDir);
    }
}

/*
 * 删除档案（患者或医生）
 */
void delete_archive(Database *db, const char *dataDir) {
    int type = read_int("选择档案类型 (1-患者，2-医生): ", 1, 2);
    if (type == 1) {
        delete_patient(db, dataDir);
    } else if (type == 2) {
        delete_doctor(db, dataDir);
    }
}

/*
 * 将档案关联到用户账号
 */
void link_archive_to_account(Database *db, const char *dataDir) {
    char username[32];
    int linkedId;
    
    printf("用户名: ");
    read_line("", username, sizeof(username));
    
    Account *acc = find_account(db, username);
    if (!acc) {
        printf("账号不存在。\n");
        return;
    }
    
    linkedId = read_int("关联 ID(患者病历号/医生工号): ", 0, 1000000);
    
    acc->linkedId = linkedId;
    save_all(db, dataDir);
    printf("关联成功。\n");
}

/*
 * 创建医生档案
 */
void create_doctor_archive(Database *db, const char *dataDir) {
    Doctor *d = (Doctor *)malloc(sizeof(Doctor));
    if (!d) { printf("内存分配失败。\n"); return; }
    
    d->id = next_doctor_id(db);
    printf("新增医生 (工号自动分配：%d)\n", d->id);
    
    printf("姓名: ");
    read_line("", d->name, NAME_LEN);
    
    printf("科室: ");
    read_line("", d->dept, SMALL_LEN);
    
    printf("职称: ");
    read_line("", d->title, SMALL_LEN);
    
    d->archived = 0;
    d->next = NULL;
    
    d->next = db->doctors;
    db->doctors = d;
    
    save_all(db, dataDir);
    printf("医生添加成功。\n");
}

/*
 * 删除医生档案
 */
void delete_doctor(Database *db, const char *dataDir) {
    int id = read_int("要删除的医生工号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Doctor *prev = NULL;
    Doctor *cur = db->doctors;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) {
        printf("医生不存在。\n");
        return;
    }
    
    char confirm[16];
    printf("确认删除医生 [%d] %s? (y/n): ", cur->id, cur->name);
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
 * 修改医生档案
 */
void edit_doctor(Database *db, const char *dataDir) {
    int id = read_int("要修改的医生工号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Doctor *d = find_doctor(db, id);
    if (!d) {
        printf("医生不存在。\n");
        return;
    }
    
    printf("当前信息：姓名=%s, 科室=%s, 职称=%s\n", d->name, d->dept, d->title);
    
    printf("新姓名 (回车保持不变): ");
    char buf[256];
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(d->name, buf, NAME_LEN - 1);
    
    printf("新科室 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(d->dept, buf, SMALL_LEN - 1);
    
    printf("新职称 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(d->title, buf, SMALL_LEN - 1);
    
    save_all(db, dataDir);
    printf("修改成功。\n");
}

/* ==================== 挂号管理函数 ==================== */

/*
 * 新增挂号记录
 */
void add_registration(Database *db, const char *dataDir) {
    Registration *r = (Registration *)malloc(sizeof(Registration));
    if (!r) { printf("内存分配失败。\n"); return; }
    
    r->id = next_registration_id(db);
    printf("新增挂号记录 (编号自动分配：%d)\n", r->id);
    
    r->patientId = read_int("患者病历号: ", 1, 1000000);
    r->doctorId = read_int("医生工号: ", 1, 1000000);
    
    printf("科室: ");
    read_line("", r->dept, SMALL_LEN);
    
    printf("日期 (YYYY-MM-DD): ");
    read_line("", r->date, DATE_LEN);
    
    printf("挂号类型 (普通/专家): ");
    read_line("", r->type, SMALL_LEN);
    
    strcpy(r->status, "未就诊");
    r->next = NULL;
    
    r->next = db->registrations;
    db->registrations = r;
    
    save_all(db, dataDir);
    printf("挂号记录添加成功。\n");
}

/*
 * 删除挂号记录
 */
void delete_registration(Database *db, const char *dataDir) {
    int id = read_int("要删除的挂号编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Registration *prev = NULL;
    Registration *cur = db->registrations;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) {
        printf("挂号记录不存在。\n");
        return;
    }
    
    if (registration_has_visit(db, id)) {
        printf("删除失败：该挂号记录已有关联的看诊记录。\n");
        return;
    }
    
    char confirm[16];
    printf("确认删除挂号记录 [%d]? (y/n): ", cur->id);
    read_line(NULL, confirm, sizeof(confirm));
    
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("已取消删除。\n");
        return;
    }
    
    if (prev) prev->next = cur->next;
    else db->registrations = cur->next;
    
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/* ==================== 看诊管理函数 ==================== */

/*
 * 新增看诊记录
 */
void add_visit(Database *db, const char *dataDir) {
    Visit *v = (Visit *)malloc(sizeof(Visit));
    if (!v) { printf("内存分配失败。\n"); return; }
    
    v->id = next_visit_id(db);
    printf("新增看诊记录 (编号自动分配：%d)\n", v->id);
    
    v->regId = read_int("关联挂号编号: ", 1, 1000000);
    
    printf("诊断结果: ");
    read_line("", v->diagnosis, sizeof(v->diagnosis));
    
    printf("检查项目: ");
    read_line("", v->examItems, sizeof(v->examItems));
    
    printf("处方信息: ");
    read_line("", v->prescription, sizeof(v->prescription));
    
    v->next = NULL;
    
    v->next = db->visits;
    db->visits = v;
    
    /* 更新挂号状态 */
    Registration *r = find_registration(db, v->regId);
    if (r) {
        strcpy(r->status, "已就诊");
    }
    
    save_all(db, dataDir);
    printf("看诊记录添加成功。\n");
}

/*
 * 删除看诊记录
 */
void delete_visit(Database *db, const char *dataDir) {
    int id = read_int("要删除的看诊编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Visit *prev = NULL;
    Visit *cur = db->visits;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) {
        printf("看诊记录不存在。\n");
        return;
    }
    
    char confirm[16];
    printf("确认删除看诊记录 [%d]? (y/n): ", cur->id);
    read_line(NULL, confirm, sizeof(confirm));
    
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("已取消删除。\n");
        return;
    }
    
    if (prev) prev->next = cur->next;
    else db->visits = cur->next;
    
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 修改看诊记录
 */
void edit_visit(Database *db, const char *dataDir) {
    int id = read_int("要修改的看诊编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Visit *v = NULL;
    Visit *cur;
    for (cur = db->visits; cur; cur = cur->next) {
        if (cur->id == id) {
            v = cur;
            break;
        }
    }
    
    if (!v) {
        printf("看诊记录不存在。\n");
        return;
    }
    
    printf("当前诊断：%s\n", v->diagnosis);
    printf("新诊断 (回车保持不变): ");
    char buf[512];
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(v->diagnosis, buf, sizeof(v->diagnosis) - 1);
    
    printf("当前检查项目：%s\n", v->examItems);
    printf("新检查项目 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(v->examItems, buf, sizeof(v->examItems) - 1);
    
    printf("当前处方：%s\n", v->prescription);
    printf("新处方 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(v->prescription, buf, sizeof(v->prescription) - 1);
    
    save_all(db, dataDir);
    printf("修改成功。\n");
}

/* ==================== 检查管理函数 ==================== */

/*
 * 新增检查记录
 */
void add_exam(Database *db, const char *dataDir) {
    Exam *e = (Exam *)malloc(sizeof(Exam));
    if (!e) { printf("内存分配失败。\n"); return; }
    
    e->id = next_exam_id(db);
    printf("新增检查记录 (编号自动分配：%d)\n", e->id);
    
    e->patientId = read_int("患者病历号: ", 1, 1000000);
    e->doctorId = read_int("开单医生工号: ", 1, 1000000);
    
    printf("检查编码: ");
    read_line("", e->code, SMALL_LEN);
    
    printf("项目名称: ");
    read_line("", e->itemName, NAME_LEN);
    
    printf("执行时间 (YYYY-MM-DD): ");
    read_line("", e->execTime, DATE_LEN);
    
    e->fee = read_double("检查费用: ", 0, 100000);
    
    printf("检查结果: ");
    read_line("", e->result, sizeof(e->result));
    
    e->next = NULL;
    
    e->next = db->exams;
    db->exams = e;
    
    save_all(db, dataDir);
    printf("检查记录添加成功。\n");
}

/*
 * 删除检查记录
 */
void delete_exam(Database *db, const char *dataDir) {
    int id = read_int("要删除的检查编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Exam *prev = NULL;
    Exam *cur = db->exams;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) {
        printf("检查记录不存在。\n");
        return;
    }
    
    char confirm[16];
    printf("确认删除检查记录 [%d]? (y/n): ", cur->id);
    read_line(NULL, confirm, sizeof(confirm));
    
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("已取消删除。\n");
        return;
    }
    
    if (prev) prev->next = cur->next;
    else db->exams = cur->next;
    
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 修改检查记录
 */
void edit_exam(Database *db, const char *dataDir) {
    int id = read_int("要修改的检查编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Exam *e = NULL;
    Exam *cur;
    for (cur = db->exams; cur; cur = cur->next) {
        if (cur->id == id) {
            e = cur;
            break;
        }
    }
    
    if (!e) {
        printf("检查记录不存在。\n");
        return;
    }
    
    printf("当前结果：%s\n", e->result);
    printf("新结果 (回车保持不变): ");
    char buf[512];
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(e->result, buf, sizeof(e->result) - 1);
    
    printf("新费用 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) e->fee = atof(buf);
    
    save_all(db, dataDir);
    printf("修改成功。\n");
}

/* ==================== 住院管理函数 ==================== */

/*
 * 新增住院记录
 */
void add_inpatient(Database *db, const char *dataDir) {
    Inpatient *ip = (Inpatient *)malloc(sizeof(Inpatient));
    if (!ip) { printf("内存分配失败。\n"); return; }
    
    ip->id = next_inpatient_id(db);
    printf("新增住院记录 (编号自动分配：%d)\n", ip->id);
    
    ip->patientId = read_int("患者病历号: ", 1, 1000000);
    ip->wardId = read_int("病房编号: ", 1, 1000000);
    ip->bedNo = read_int("床位号: ", 1, 100);
    
    printf("入院日期 (YYYY-MM-DD): ");
    read_line("", ip->admitDate, DATE_LEN);
    
    printf("预计出院日期 (YYYY-MM-DD): ");
    read_line("", ip->expectedDischarge, DATE_LEN);
    
    ip->totalCost = read_double("预估总费用: ", 0, 1000000);
    
    ip->next = NULL;
    
    ip->next = db->inpatients;
    db->inpatients = ip;
    
    save_all(db, dataDir);
    printf("住院记录添加成功。\n");
}

/*
 * 删除住院记录
 */
void delete_inpatient(Database *db, const char *dataDir) {
    int id = read_int("要删除的住院编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Inpatient *prev = NULL;
    Inpatient *cur = db->inpatients;
    while (cur && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    
    if (!cur) {
        printf("住院记录不存在。\n");
        return;
    }
    
    char confirm[16];
    printf("确认删除住院记录 [%d]? (y/n): ", cur->id);
    read_line(NULL, confirm, sizeof(confirm));
    
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("已取消删除。\n");
        return;
    }
    
    if (prev) prev->next = cur->next;
    else db->inpatients = cur->next;
    
    free(cur);
    save_all(db, dataDir);
    printf("删除成功。\n");
}

/*
 * 修改住院记录
 */
void edit_inpatient(Database *db, const char *dataDir) {
    int id = read_int("要修改的住院编号 (输入 0 返回): ", 0, 1000000);
    if (id == 0) { printf("已返回。\n"); return; }
    
    Inpatient *ip = NULL;
    Inpatient *cur;
    for (cur = db->inpatients; cur; cur = cur->next) {
        if (cur->id == id) {
            ip = cur;
            break;
        }
    }
    
    if (!ip) {
        printf("住院记录不存在。\n");
        return;
    }
    
    printf("当前预计出院日期：%s, 费用：%.2f\n", ip->expectedDischarge, ip->totalCost);
    
    printf("新预计出院日期 (回车保持不变): ");
    char buf[64];
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(ip->expectedDischarge, buf, DATE_LEN - 1);
    
    printf("新预估总费用 (回车保持不变): ");
    read_line("", buf, sizeof(buf));
    if (strlen(buf) > 0) ip->totalCost = atof(buf);
    
    save_all(db, dataDir);
    printf("修改成功。\n");
}

/* ==================== 管理员菜单相关 ==================== */

/*
 * 用户账号管理子菜单
 */
void user_account_management_menu(Database *db, const char *dataDir) {
    int choice;
    while (1) {
        printf("\n--- 用户账号管理 ---\n");
        printf("1. 创建账号\n");
        printf("2. 删除账号\n");
        printf("3. 修改密码\n");
        printf("0. 返回上级菜单\n");
        choice = read_int("请选择: ", 0, 3);
        if (choice == 0) return;
        
        if (choice == 1) {
            char username[32], password[64], confirm[64];
            int role, linkedId;
            
            printf("用户名: ");
            read_line("", username, sizeof(username));
            
            if (find_account(db, username)) {
                printf("用户名已存在。\n");
                continue;
            }
            
            printf("密码: ");
            read_line("", password, sizeof(password));
            
            printf("确认密码: ");
            read_line("", confirm, sizeof(confirm));
            
            if (strcmp(password, confirm) != 0) {
                printf("两次密码不一致。\n");
                continue;
            }
            
            role = read_int("角色 (0-患者，1-医生，2-管理员): ", 0, 2);
            linkedId = read_int("关联 ID: ", 0, 1000000);
            
            if (create_account(db, username, password, role, linkedId)) {
                save_all(db, dataDir);
                printf("账号创建成功。\n");
            } else {
                printf("账号创建失败。\n");
            }
        } else if (choice == 2) {
            char username[32];
            printf("要删除的用户名: ");
            read_line("", username, sizeof(username));
            
            Account *acc = find_account(db, username);
            if (!acc) {
                printf("账号不存在。\n");
                continue;
            }
            
            Account *prev = NULL;
            Account *cur = db->accounts;
            while (cur && cur != acc) {
                prev = cur;
                cur = cur->next;
            }
            
            if (prev) prev->next = cur->next;
            else db->accounts = cur->next;
            
            free(cur);
            save_all(db, dataDir);
            printf("账号删除成功。\n");
        } else if (choice == 3) {
            char username[32], newpass[64];
            printf("用户名: ");
            read_line("", username, sizeof(username));
            
            Account *acc = find_account(db, username);
            if (!acc) {
                printf("账号不存在。\n");
                continue;
            }
            
            printf("新密码: ");
            read_line("", newpass, sizeof(newpass));
            
            strncpy(acc->password, newpass, sizeof(acc->password) - 1);
            save_all(db, dataDir);
            printf("密码修改成功。\n");
        }
        pause_and_wait();
    }
}
