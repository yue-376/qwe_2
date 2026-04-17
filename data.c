/*
 * 文件：data.c
 * 说明：数据加载与保存功能实现文件
 * 
 * 本文件实现了医院管理系统中所有数据的持久化操作，包括：
 * - 将各种数据节点追加到链表末尾的辅助函数
 * - 从文本文件加载数据到内存链表
 * - 将内存链表数据保存到文本文件
 * - 批量加载和保存所有数据
 * - 数据导入功能
 * 
 * 数据文件格式：使用 '|' 作为字段分隔符的文本文件
 * 支持的数据类型：患者、医生、挂号记录、看诊记录、检查记录、
 *                  病房、住院记录、药品、药品日志、账号
 */

#include "models.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== 链表节点追加函数 ==================== */

/*
 * 说明：将患者节点追加到患者链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的患者节点
 * 
 * 处理流程：
 * 1. 将新节点的 next 指针设为 NULL
 * 2. 如果链表为空，直接将新节点设为头节点
 * 3. 否则遍历到链表末尾，将新节点接在最后一个节点后面
 */
static void append_patient(Database *db, Patient *node)
{
    node->next = NULL;
    if (!db->patients)
        db->patients = node;
    else
    {
        Patient *p = db->patients;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将医生节点追加到医生链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的医生节点
 */
static void append_doctor(Database *db, Doctor *node)
{
    node->next = NULL;
    if (!db->doctors)
        db->doctors = node;
    else
    {
        Doctor *p = db->doctors;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将挂号记录节点追加到挂号链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的挂号记录节点
 */
static void append_reg(Database *db, Registration *node)
{
    node->next = NULL;
    if (!db->registrations)
        db->registrations = node;
    else
    {
        Registration *p = db->registrations;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将看诊记录节点追加到看诊链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的看诊记录节点
 */
static void append_visit(Database *db, Visit *node)
{
    node->next = NULL;
    if (!db->visits)
        db->visits = node;
    else
    {
        Visit *p = db->visits;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将检查记录节点追加到检查链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的检查记录节点
 */
static void append_exam(Database *db, Exam *node)
{
    node->next = NULL;
    if (!db->exams)
        db->exams = node;
    else
    {
        Exam *p = db->exams;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将病房节点追加到病房链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的病房节点
 */
static void append_ward(Database *db, Ward *node)
{
    node->next = NULL;
    if (!db->wards)
        db->wards = node;
    else
    {
        Ward *p = db->wards;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将住院记录节点追加到住院链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的住院记录节点
 */
static void append_inpatient(Database *db, Inpatient *node)
{
    node->next = NULL;
    if (!db->inpatients)
        db->inpatients = node;
    else
    {
        Inpatient *p = db->inpatients;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将药品节点追加到药品链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的药品节点
 */
static void append_drug(Database *db, Drug *node)
{
    node->next = NULL;
    if (!db->drugs)
        db->drugs = node;
    else
    {
        Drug *p = db->drugs;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将药品出入库日志节点追加到日志链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的药品日志节点
 */
static void append_druglog(Database *db, DrugLog *node)
{
    node->next = NULL;
    if (!db->drugLogs)
        db->drugLogs = node;
    else
    {
        DrugLog *p = db->drugLogs;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：将账号节点追加到账号链表末尾
 * 参数：db 数据库指针
 * 参数：node 要添加的账号节点
 */
static void append_account(Database *db, Account *node)
{
    node->next = NULL;
    if (!db->accounts)
        db->accounts = node;
    else
    {
        Account *p = db->accounts;
        while (p->next)
            p = p->next;
        p->next = node;
    }
}

/*
 * 说明：根据 ID 查找患者
 * 参数：db 数据库指针
 * 参数：id 要查找的患者 ID
 * 返回值：找到则返回患者节点指针，否则返回 NULL
 * 
 * 处理流程：
 * 1. 从患者链表头开始遍历
 * 2. 逐个比较节点 ID 与目标 ID
 * 3. 找到匹配则返回该节点，遍历结束未找到则返回 NULL
 */
Patient *find_patient(Database *db, int id)
{
    Patient *p = db->patients;
    while (p)
    {
        if (p->id == id)
            return p;
        p = p->next;
    }
    return NULL;
}

/*
 * 说明：根据 ID 查找医生
 * 参数：db 数据库指针
 * 参数：id 要查找的医生 ID
 * 返回值：找到则返回医生节点指针，否则返回 NULL
 */
Doctor *find_doctor(Database *db, int id)
{
    Doctor *p = db->doctors;
    while (p)
    {
        if (p->id == id)
            return p;
        p = p->next;
    }
    return NULL;
}

/*
 * 说明：根据 ID 查找挂号记录
 * 参数：db 数据库指针
 * 参数：id 要查找的挂号记录 ID
 * 返回值：找到则返回挂号记录节点指针，否则返回 NULL
 */
Registration *find_registration(Database *db, int id)
{
    Registration *p = db->registrations;
    while (p)
    {
        if (p->id == id)
            return p;
        p = p->next;
    }
    return NULL;
}

/*
 * 说明：根据 ID 查找病房
 * 参数：db 数据库指针
 * 参数：id 要查找的病房 ID
 * 返回值：找到则返回病房节点指针，否则返回 NULL
 */
Ward *find_ward(Database *db, int id)
{
    Ward *p = db->wards;
    while (p)
    {
        if (p->id == id)
            return p;
        p = p->next;
    }
    return NULL;
}

/*
 * 说明：根据 ID 查找药品
 * 参数：db 数据库指针
 * 参数：id 要查找的药品 ID
 * 返回值：找到则返回药品节点指针，否则返回 NULL
 */
Drug *find_drug(Database *db, int id)
{
    Drug *p = db->drugs;
    while (p)
    {
        if (p->id == id)
            return p;
        p = p->next;
    }
    return NULL;
}

/*
 * 说明：宏定义，用于生成获取下一个可用 ID 的函数
 * 参数：type 数据类型（如 Patient、Doctor 等）
 * 参数：field 数据库中对应的链表字段名
 * 参数：name 生成的函数名
 * 
 * 工作原理：
 * 1. 遍历指定类型的链表，找到最大的 ID 值
 * 2. 返回最大 ID + 1 作为下一个可用 ID
 * 3. 如果链表为空，返回 1
 */
#define NEXT_ID_FUNC(type, field, name) \
    int name(Database *db)              \
    {                                   \
        int max = 0;                    \
        type *p = db->field;            \
        while (p)                       \
        {                               \
            if (p->id > max)            \
                max = p->id;            \
            p = p->next;                \
        }                               \
        return max + 1;                 \
    }

/* 为各种数据类型生成 next_xxx_id 函数 */
NEXT_ID_FUNC(Patient, patients, next_patient_id)
NEXT_ID_FUNC(Doctor, doctors, next_doctor_id)
NEXT_ID_FUNC(Registration, registrations, next_registration_id)
NEXT_ID_FUNC(Visit, visits, next_visit_id)
NEXT_ID_FUNC(Exam, exams, next_exam_id)
NEXT_ID_FUNC(Ward, wards, next_ward_id)
NEXT_ID_FUNC(Inpatient, inpatients, next_inpatient_id)
NEXT_ID_FUNC(Drug, drugs, next_drug_id)
NEXT_ID_FUNC(DrugLog, drugLogs, next_druglog_id)

/* ==================== 文件操作辅助函数 ==================== */

/*
 * 说明：检查文件是否存在
 * 参数：path 文件路径
 * 返回值：1 表示存在，0 表示不存在
 * 
 * 处理流程：
 * 1. 尝试以只读模式打开文件
 * 2. 如果打开成功则关闭文件并返回 1
 * 3. 如果打开失败则返回 0
 */
static int file_exists(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp)
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

/*
 * 说明：拼接目录路径和文件名
 * 参数：out 输出缓冲区
 * 参数：size 缓冲区大小
 * 参数：dir 目录路径
 * 参数：name 文件名
 * 
 * 示例：path_join(out, sizeof(out), "/data", "patients.txt") 
 *       结果：out = "/data/patients.txt"
 */
void path_join(char *out, size_t size, const char *dir, const char *name)
{
    snprintf(out, size, "%s/%s", dir, name);
}

/* ==================== 数据加载函数 ==================== */

/*
 * 说明：从文件加载患者数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|name|gender|birth|phone|insurance|archived
 * 处理流程：
 * 1. 打开文件逐行读取
 * 2. 为每行数据分配患者节点内存
 * 3. 使用 sscanf 解析字段，成功则添加到链表，失败则释放内存
 */
static void load_patients(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Patient *p = (Patient *)malloc(sizeof(Patient));
        if (sscanf(line, "%d|%63[^|]|%15[^|]|%16[^|]|%31[^|]|%31[^|]|%d", &p->id, p->name, p->gender, p->birth, p->phone, p->insurance, &p->archived) == 7)
            append_patient(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载医生数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|name|dept|title|archived
 */
static void load_doctors(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Doctor *p = (Doctor *)malloc(sizeof(Doctor));
        if (sscanf(line, "%d|%63[^|]|%31[^|]|%31[^|]|%d", &p->id, p->name, p->dept, p->title, &p->archived) == 5)
            append_doctor(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载挂号记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|patientId|doctorId|dept|date|type|status
 */
static void load_regs(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Registration *p = (Registration *)malloc(sizeof(Registration));
        if (sscanf(line, "%d|%d|%d|%31[^|]|%16[^|]|%31[^|]|%31[^\n]", &p->id, &p->patientId, &p->doctorId, p->dept, p->date, p->type, p->status) == 7)
            append_reg(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载看诊记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|regId|diagnosis|examItems|prescription
 */
static void load_visits(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE * 2];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Visit *p = (Visit *)malloc(sizeof(Visit));
        if (sscanf(line, "%d|%d|%511[^|]|%511[^|]|%511[^\n]", &p->id, &p->regId, p->diagnosis, p->examItems, p->prescription) == 5)
            append_visit(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载检查记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|patientId|doctorId|code|itemName|execTime|fee|result
 */
static void load_exams(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Exam *p = (Exam *)malloc(sizeof(Exam));
        if (sscanf(line, "%d|%d|%d|%31[^|]|%63[^|]|%16[^|]|%lf|%511[^\n]", &p->id, &p->patientId, &p->doctorId, p->code, p->itemName, p->execTime, &p->fee, p->result) == 8)
            append_exam(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载病房数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|wardType|dept|bedCount|occupiedBeds|maintenanceBeds
 */
static void load_wards(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Ward *p = (Ward *)malloc(sizeof(Ward));
        if (sscanf(line, "%d|%31[^|]|%31[^|]|%d|%d|%d", &p->id, p->wardType, p->dept, &p->bedCount, &p->occupiedBeds, &p->maintenanceBeds) == 6)
            append_ward(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载住院记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|patientId|wardId|bedNo|admitDate|expectedDischarge|totalCost
 */
static void load_inpatients(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Inpatient *p = (Inpatient *)malloc(sizeof(Inpatient));
        if (sscanf(line, "%d|%d|%d|%d|%16[^|]|%16[^|]|%lf", &p->id, &p->patientId, &p->wardId, &p->bedNo, p->admitDate, p->expectedDischarge, &p->totalCost) == 7)
            append_inpatient(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载药品数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|genericName|brandName|alias|type|dept|price|stock
 */
static void load_drugs(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Drug *p = (Drug *)malloc(sizeof(Drug));
        if (sscanf(line, "%d|%63[^|]|%63[^|]|%63[^|]|%31[^|]|%31[^|]|%lf|%d", &p->id, p->genericName, p->brandName, p->alias, p->type, p->dept, &p->price, &p->stock) == 8)
            append_drug(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载药品出入库日志数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|drugId|operation|quantity|operatorName|date
 */
static void load_druglogs(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        DrugLog *p = (DrugLog *)malloc(sizeof(DrugLog));
        if (sscanf(line, "%d|%d|%31[^|]|%d|%63[^|]|%16[^\n]", &p->id, &p->drugId, p->operation, &p->quantity, p->operatorName, p->date) == 6)
            append_druglog(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/*
 * 说明：从文件加载账号数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：username|password|role|linkedId
 */
static void load_accounts(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Account *p = (Account *)malloc(sizeof(Account));
        int roleVal;
        /* 数据格式：用户名 | 密码 | 角色 | 关联 ID */
        if (sscanf(line, "%31[^|]|%63[^|]|%d|%d", p->username, p->password, &roleVal, &p->linkedId) == 4)
        {
            p->role = (UserRole)roleVal;
            append_account(db, p);
        }
        else
            free(p);
    }
    fclose(fp);
}

/* ==================== 数据保存函数 ==================== */

/*
 * 说明：保存患者数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|name|gender|birth|phone|insurance|archived
 * 处理流程：
 * 1. 以写模式打开文件
 * 2. 遍历患者链表，将每个节点格式化写入文件
 * 3. 关闭文件
 */
void save_patients(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Patient *p;
    if (!fp)
        return;
    for (p = db->patients; p; p = p->next)
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%d\n", p->id, p->name, p->gender, p->birth, p->phone, p->insurance, p->archived);
    fclose(fp);
}

/*
 * 说明：保存医生数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|name|dept|title|archived
 */
static void save_doctors(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Doctor *p;
    if (!fp)
        return;
    for (p = db->doctors; p; p = p->next)
        fprintf(fp, "%d|%s|%s|%s|%d\n", p->id, p->name, p->dept, p->title, p->archived);
    fclose(fp);
}

/*
 * 说明：保存挂号记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|patientId|doctorId|dept|date|type|status
 */
static void save_regs(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Registration *p;
    if (!fp)
        return;
    for (p = db->registrations; p; p = p->next)
        fprintf(fp, "%d|%d|%d|%s|%s|%s|%s\n", p->id, p->patientId, p->doctorId, p->dept, p->date, p->type, p->status);
    fclose(fp);
}

/*
 * 说明：保存看诊记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|regId|diagnosis|examItems|prescription
 */
static void save_visits(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Visit *p;
    if (!fp)
        return;
    for (p = db->visits; p; p = p->next)
        fprintf(fp, "%d|%d|%s|%s|%s\n", p->id, p->regId, p->diagnosis, p->examItems, p->prescription);
    fclose(fp);
}

/*
 * 说明：保存检查记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|patientId|doctorId|code|itemName|execTime|fee|result
 */
static void save_exams(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Exam *p;
    if (!fp)
        return;
    for (p = db->exams; p; p = p->next)
        fprintf(fp, "%d|%d|%d|%s|%s|%s|%.2f|%s\n", p->id, p->patientId, p->doctorId, p->code, p->itemName, p->execTime, p->fee, p->result);
    fclose(fp);
}

/*
 * 说明：保存病房数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|wardType|dept|bedCount|occupiedBeds|maintenanceBeds
 */
static void save_wards(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Ward *p;
    if (!fp)
        return;
    for (p = db->wards; p; p = p->next)
        fprintf(fp, "%d|%s|%s|%d|%d|%d\n", p->id, p->wardType, p->dept, p->bedCount, p->occupiedBeds, p->maintenanceBeds);
    fclose(fp);
}

/*
 * 说明：保存住院记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|patientId|wardId|bedNo|admitDate|expectedDischarge|totalCost
 */
static void save_inpatients(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Inpatient *p;
    if (!fp)
        return;
    for (p = db->inpatients; p; p = p->next)
        fprintf(fp, "%d|%d|%d|%d|%s|%s|%.2f\n", p->id, p->patientId, p->wardId, p->bedNo, p->admitDate, p->expectedDischarge, p->totalCost);
    fclose(fp);
}

/*
 * 说明：保存药品数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|genericName|brandName|alias|type|dept|price|stock
 */
static void save_drugs(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Drug *p;
    if (!fp)
        return;
    for (p = db->drugs; p; p = p->next)
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%.2f|%d\n", p->id, p->genericName, p->brandName, p->alias, p->type, p->dept, p->price, p->stock);
    fclose(fp);
}

/*
 * 说明：保存药品出入库日志数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：id|drugId|operation|quantity|operatorName|date
 */
static void save_druglogs(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    DrugLog *p;
    if (!fp)
        return;
    for (p = db->drugLogs; p; p = p->next)
        fprintf(fp, "%d|%d|%s|%d|%s|%s\n", p->id, p->drugId, p->operation, p->quantity, p->operatorName, p->date);
    fclose(fp);
}

/*
 * 说明：保存账号数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 * 
 * 文件格式：username|password|role|linkedId
 * 注意：账号数据包含敏感信息（密码），实际应用中应加密存储
 */
void save_accounts(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Account *p;
    if (!fp)
        return;
    for (p = db->accounts; p; p = p->next)
        fprintf(fp, "%s|%s|%d|%d\n", p->username, p->password, p->role, p->linkedId);
    fclose(fp);
}

/*
 * 说明：从指定目录加载所有数据文件到数据库
 * 参数：db 数据库指针
 * 参数：dir 数据文件目录
 * 返回值：1 表示成功
 */
int load_all(Database *db, const char *dir)
{
    char path[256];
    path_join(path, sizeof(path), dir, "patients.txt");
    if (file_exists(path))
        load_patients(db, path);
    path_join(path, sizeof(path), dir, "doctors.txt");
    if (file_exists(path))
        load_doctors(db, path);
    path_join(path, sizeof(path), dir, "registrations.txt");
    if (file_exists(path))
        load_regs(db, path);
    path_join(path, sizeof(path), dir, "visits.txt");
    if (file_exists(path))
        load_visits(db, path);
    path_join(path, sizeof(path), dir, "exams.txt");
    if (file_exists(path))
        load_exams(db, path);
    path_join(path, sizeof(path), dir, "wards.txt");
    if (file_exists(path))
        load_wards(db, path);
    path_join(path, sizeof(path), dir, "inpatients.txt");
    if (file_exists(path))
        load_inpatients(db, path);
    path_join(path, sizeof(path), dir, "drugs.txt");
    if (file_exists(path))
        load_drugs(db, path);
    path_join(path, sizeof(path), dir, "druglogs.txt");
    if (file_exists(path))
        load_druglogs(db, path);
    /* 注意：程序启动时加载 accounts.txt，保持账号数据 */
    path_join(path, sizeof(path), dir, "accounts.txt");
    if (file_exists(path))
        load_accounts(db, path);
    return 1;
}

/*
 * 说明：将数据库中所有数据保存到指定目录
 * 参数：db 数据库指针
 * 参数：dir 数据文件保存目录
 * 返回值：1 表示成功
 */
int save_all(Database *db, const char *dir)
{
    char path[256];
    path_join(path, sizeof(path), dir, "patients.txt");
    save_patients(db, path);
    path_join(path, sizeof(path), dir, "doctors.txt");
    save_doctors(db, path);
    path_join(path, sizeof(path), dir, "registrations.txt");
    save_regs(db, path);
    path_join(path, sizeof(path), dir, "visits.txt");
    save_visits(db, path);
    path_join(path, sizeof(path), dir, "exams.txt");
    save_exams(db, path);
    path_join(path, sizeof(path), dir, "wards.txt");
    save_wards(db, path);
    path_join(path, sizeof(path), dir, "inpatients.txt");
    save_inpatients(db, path);
    path_join(path, sizeof(path), dir, "drugs.txt");
    save_drugs(db, path);
    path_join(path, sizeof(path), dir, "druglogs.txt");
    save_druglogs(db, path);
    path_join(path, sizeof(path), dir, "accounts.txt");
    save_accounts(db, path);
    return 1;
}

/* 从指定的目录导入数据（替换现有数据） */
/*
 * 说明：从指定目录导入数据（替换现有数据）
 * 参数：db 数据库指针
 * 参数：dir 要导入的数据文件目录
 * 返回值：成功导入的文件数量
 */
int import_all(Database *db, const char *dir)
{
    char path[256];
    int count = 0;
    
    /* 先清空现有数据（但保留账号数据，避免覆盖） */
    free_patients(db->patients);
    db->patients = NULL;
    free_doctors(db->doctors);
    db->doctors = NULL;
    free_regs(db->registrations);
    db->registrations = NULL;
    free_visits(db->visits);
    db->visits = NULL;
    free_exams(db->exams);
    db->exams = NULL;
    free_wards(db->wards);
    db->wards = NULL;
    free_inpatients(db->inpatients);
    db->inpatients = NULL;
    free_drugs(db->drugs);
    db->drugs = NULL;
    free_druglogs(db->drugLogs);
    db->drugLogs = NULL;
    /* 注意：不清空账号数据，避免导入时覆盖现有账号 */
    
    path_join(path, sizeof(path), dir, "patients.txt");
    if (file_exists(path))
    {
        load_patients(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "doctors.txt");
    if (file_exists(path))
    {
        load_doctors(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "registrations.txt");
    if (file_exists(path))
    {
        load_regs(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "visits.txt");
    if (file_exists(path))
    {
        load_visits(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "exams.txt");
    if (file_exists(path))
    {
        load_exams(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "wards.txt");
    if (file_exists(path))
    {
        load_wards(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "inpatients.txt");
    if (file_exists(path))
    {
        load_inpatients(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "drugs.txt");
    if (file_exists(path))
    {
        load_drugs(db, path);
        count++;
    }
    path_join(path, sizeof(path), dir, "druglogs.txt");
    if (file_exists(path))
    {
        load_druglogs(db, path);
        count++;
    }
    /* 注意：导入时不再加载 accounts.txt，避免覆盖现有账号数据 */
    
    return count;
}
