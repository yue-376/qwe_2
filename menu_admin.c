#include "menu_admin.h"
#include "data.h"
#include "auth.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
