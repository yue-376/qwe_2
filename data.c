#include "models.h"

/* 将患者节点追加到患者链表末尾 */
/* 将患者节点追加到患者链表末尾 */
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
/* 将医生节点追加到医生链表末尾 */
/* 将医生节点追加到医生链表末尾 */
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
/* 将挂号记录节点追加到挂号链表末尾 */
/* 将挂号记录节点追加到挂号链表末尾 */
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
/* 将看诊记录节点追加到看诊链表末尾 */
/* 将看诊记录节点追加到看诊链表末尾 */
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
/* 将检查记录节点追加到检查链表末尾 */
/* 将检查记录节点追加到检查链表末尾 */
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
/* 将病房节点追加到病房链表末尾 */
/* 将病房节点追加到病房链表末尾 */
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
/* 将住院记录节点追加到住院链表末尾 */
/* 将住院记录节点追加到住院链表末尾 */
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
/* 将药品节点追加到药品链表末尾 */
/* 将药品节点追加到药品链表末尾 */
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
/* 将药品节点追加到药品链表末尾 */
/* 将药品出入库日志节点追加到日志链表末尾 */
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
NEXT_ID_FUNC(Patient, patients, next_patient_id)
NEXT_ID_FUNC(Doctor, doctors, next_doctor_id)
NEXT_ID_FUNC(Registration, registrations, next_registration_id)
NEXT_ID_FUNC(Visit, visits, next_visit_id)
NEXT_ID_FUNC(Exam, exams, next_exam_id)
NEXT_ID_FUNC(Ward, wards, next_ward_id)
NEXT_ID_FUNC(Inpatient, inpatients, next_inpatient_id)
NEXT_ID_FUNC(Drug, drugs, next_drug_id)
NEXT_ID_FUNC(DrugLog, drugLogs, next_druglog_id)

/*
 * 说明：检查文件是否存在
 * 参数：path 文件路径
 * 返回值：1 表示存在，0 表示不存在
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
 */
static void path_join(char *out, size_t size, const char *dir, const char *name)
{
    snprintf(out, size, "%s/%s", dir, name);
}

/* 从文件加载患者数据到数据库 */
/*
 * 说明：从文件加载患者数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
        if (sscanf(line, "%d|%63[^|]|%15[^|]|%15[^|]|%31[^|]|%31[^|]|%d", &p->id, p->name, p->gender, p->birth, p->phone, p->insurance, &p->archived) == 7)
            append_patient(db, p);
        else
            free(p);
    }
    fclose(fp);
}
/* 从文件加载医生数据到数据库 */
/*
 * 说明：从文件加载医生数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
        if (sscanf(line, "%d|%d|%d|%31[^|]|%15[^|]|%31[^|]|%31[^\n]", &p->id, &p->patientId, &p->doctorId, p->dept, p->date, p->type, p->status) == 7)
            append_reg(db, p);
        else
            free(p);
    }
    fclose(fp);
}
/* 从文件加载看诊记录数据到数据库 */
/*
 * 说明：从文件加载看诊记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
 */
static void load_visits(Database *db, const char *path)
{
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    if (!fp)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        Visit *p = (Visit *)malloc(sizeof(Visit));
        if (sscanf(line, "%d|%d|%255[^|]|%255[^|]|%255[^\n]", &p->id, &p->regId, p->diagnosis, p->examItems, p->prescription) == 5)
            append_visit(db, p);
        else
            free(p);
    }
    fclose(fp);
}
/* 从文件加载检查记录数据到数据库 */
/*
 * 说明：从文件加载检查记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
        if (sscanf(line, "%d|%d|%d|%31[^|]|%63[^|]|%15[^|]|%lf|%255[^\n]", &p->id, &p->patientId, &p->doctorId, p->code, p->itemName, p->execTime, &p->fee, p->result) == 8)
            append_exam(db, p);
        else
            free(p);
    }
    fclose(fp);
}
/* 从文件加载病房数据到数据库 */
/*
 * 说明：从文件加载病房数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 从文件加载住院记录数据到数据库 */
/*
 * 说明：从文件加载住院记录数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
        if (sscanf(line, "%d|%d|%d|%d|%15[^|]|%15[^|]|%lf", &p->id, &p->patientId, &p->wardId, &p->bedNo, p->admitDate, p->expectedDischarge, &p->totalCost) == 7)
            append_inpatient(db, p);
        else
            free(p);
    }
    fclose(fp);
}
/* 从文件加载药品数据到数据库 */
/*
 * 说明：从文件加载药品数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 从文件加载药品日志数据到数据库 */
/*
 * 说明：从文件加载药品出入库日志数据到数据库
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
        if (sscanf(line, "%d|%d|%31[^|]|%d|%63[^|]|%15[^\n]", &p->id, &p->drugId, p->operation, &p->quantity, p->operatorName, p->date) == 6)
            append_druglog(db, p);
        else
            free(p);
    }
    fclose(fp);
}

/* 保存患者数据到文件 */
/*
 * 说明：保存患者数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 */
static void save_patients(Database *db, const char *path)
{
    FILE *fp = fopen(path, "w");
    Patient *p;
    if (!fp)
        return;
    for (p = db->patients; p; p = p->next)
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%d\n", p->id, p->name, p->gender, p->birth, p->phone, p->insurance, p->archived);
    fclose(fp);
}
/* 保存医生数据到文件 */
/*
 * 说明：保存医生数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 保存看诊记录数据到文件 */
/*
 * 说明：保存看诊记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 保存检查记录数据到文件 */
/*
 * 说明：保存检查记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 保存病房数据到文件 */
/*
 * 说明：保存病房数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 保存住院记录数据到文件 */
/*
 * 说明：保存住院记录数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 保存药品数据到文件 */
/*
 * 说明：保存药品数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
/* 保存药品日志数据到文件 */
/*
 * 说明：保存药品出入库日志数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
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
    return 1;
}

/* 从指定的目录导入数据（合并到现有数据） */
/*
 * 说明：从指定目录导入数据（合并到现有数据）
 * 参数：db 数据库指针
 * 参数：dir 要导入的数据文件目录
 * 返回值：成功导入的文件数量
 */
int import_all(Database *db, const char *dir)
{
    char path[256];
    int count = 0;
    
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
    
    return count;
}
