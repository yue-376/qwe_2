/*
 * 文件：models.h
 * 说明：数据模型定义头文件
 * 
 * 本文件定义了医院管理系统中的所有核心数据结构：
 * - 患者 (Patient)
 * - 医生 (Doctor)
 * - 挂号记录 (Registration)
 * - 看诊记录 (Visit)
 * - 检查记录 (Exam)
 * - 病房 (Ward)
 * - 住院记录 (Inpatient)
 * - 药品 (Drug)
 * - 药品出入库日志 (DrugLog)
 * - 数据库结构 (Database)
 * 
 * 所有链表结构都使用 next 指针连接，便于动态管理数据
 */

#ifndef MODELS_H
#define MODELS_H

#include "common.h"

/* ==================== 用户账号与角色定义 ==================== */

/*
 * 枚举：UserRole
 * 说明：用户角色枚举
 * 
 * - ROLE_PATIENT: 患者角色
 * - ROLE_DOCTOR: 医生角色
 * - ROLE_MANAGER: 管理员角色
 */
typedef enum { 
    ROLE_PATIENT = 0, 
    ROLE_DOCTOR = 1, 
    ROLE_MANAGER = 2 
} UserRole;

/*
 * 结构体：Account
 * 说明：用户账号结构体
 * 
 * 字段说明：
 * - username: 用户名（唯一标识）
 * - password: 密码（加密存储）
 * - role: 用户角色
 * - linkedId: 关联ID（患者病历号/医生工号/管理员填0）
 * - next: 指向下一个账号的指针（链表结构）
 */
typedef struct Account {
    char username[32];
    char password[64];
    UserRole role;
    int linkedId;  // 患者病历号/医生工号/管理员填0
    struct Account *next;
} Account;

/*
 * 结构体：UserSession
 * 说明：用户登录会话结构体
 * 
 * 字段说明：
 * - isLoggedIn: 是否已登录
 * - role: 当前登录用户的角色
 * - userId: 当前登录用户的关联ID
 * - username: 当前登录用户的用户名
 */
typedef struct {
    int isLoggedIn;
    UserRole role;
    int userId;    // 当前登录用户的关联ID
    char username[32];
} UserSession;

/* ==================== 数据模型定义 ==================== */

/*
 * 结构体：Patient
 * 说明：患者信息结构体
 * 
 * 字段说明：
 * - id: 病历号（唯一标识）
 * - name: 姓名
 * - gender: 性别（"男"或"女"）
 * - birth: 出生日期（YYYY-MM-DD 格式）
 * - phone: 联系电话（11 位手机号）
 * - insurance: 医保类型
 * - archived: 是否已归档（0-正常，1-归档）
 * - next: 指向下一个患者的指针（链表结构）
 */
typedef struct Patient
{
    int id;                     /* 病历号 */
    char name[NAME_LEN];        /* 姓名 */
    char gender[16];            /* 性别 */
    char birth[DATE_LEN];       /* 出生日期 */
    char phone[PHONE_LEN];      /* 联系方式 */
    char insurance[SMALL_LEN];  /* 医保类型 */
    int archived;               /* 是否归档 */
    struct Patient *next;       /* 下一节点指针 */
} Patient;

/*
 * 结构体：Doctor
 * 说明：医生信息结构体
 * 
 * - id 工号（唯一标识）
 * - name 姓名
 * - dept 所属科室
 * - title 职称
 * - archived 是否已归档
 * - next 指向下一个医生的指针
 */
typedef struct Doctor
{
    int id;                     /* 工号 */
    char name[NAME_LEN];        /* 姓名 */
    char dept[SMALL_LEN];       /* 科室 */
    char title[SMALL_LEN];      /* 职称 */
    int archived;               /* 是否归档 */
    struct Doctor *next;        /* 下一节点指针 */
} Doctor;

/*
 * 结构体：Registration
 * 说明：挂号记录结构体
 * 
 * - id 挂号编号（唯一标识）
 * - patientId 患者病历号
 * - doctorId 医生工号
 * - dept 科室
 * - date 挂号日期
 * - type 挂号类型（普通/专家）
 * - status 就诊状态（未就诊/已就诊）
 * - next 指向下一条挂号记录的指针
 */
typedef struct Registration
{
    int id;                     /* 挂号编号 */
    int patientId;              /* 患者病历号 */
    int doctorId;               /* 医生工号 */
    char dept[SMALL_LEN];       /* 科室 */
    char date[DATE_LEN];        /* 挂号日期 */
    char type[SMALL_LEN];       /* 挂号类型 */
    char status[SMALL_LEN];     /* 就诊状态 */
    struct Registration *next;  /* 下一节点指针 */
} Registration;

/*
 * 结构体：Visit
 * 说明：看诊记录结构体
 * 
 * - id 看诊编号（唯一标识）
 * - regId 关联的挂号编号
 * - diagnosis 诊断结果
 * - examItems 检查项目
 * - prescription 处方信息
 * - next 指向下一条看诊记录的指针
 */
typedef struct Visit
{
    int id;                     /* 看诊编号 */
    int regId;                  /* 关联挂号编号 */
    char diagnosis[LONG_LEN * 2];   /* 诊断结果 */
    char examItems[LONG_LEN * 2];   /* 检查项目 */
    char prescription[LONG_LEN * 2];/* 处方信息 */
    struct Visit *next;         /* 下一节点指针 */
} Visit;

/*
 * 结构体：Exam
 * 说明：检查记录结构体
 * 
 * - id 检查编号（唯一标识）
 * - patientId 患者病历号
 * - doctorId 开单医生工号
 * - code 检查编码
 * - itemName 检查项目名称
 * - execTime 执行时间
 * - fee 检查费用
 * - result 检查结果
 * - next 指向下一条检查记录的指针
 */
typedef struct Exam
{
    int id;                     /* 检查编号 */
    int patientId;              /* 患者病历号 */
    int doctorId;               /* 医生工号 */
    char code[SMALL_LEN];       /* 检查编码 */
    char itemName[NAME_LEN];    /* 项目名称 */
    char execTime[DATE_LEN];    /* 执行时间 */
    double fee;                 /* 费用 */
    char result[LONG_LEN * 2];  /* 检查结果 */
    struct Exam *next;          /* 下一节点指针 */
} Exam;

/*
 * 结构体：Ward
 * 说明：病房信息结构体
 * 
 * - id 病房编号（唯一标识）
 * - wardType 病房类型
 * - dept 所属科室
 * - bedCount 床位总数
 * - occupiedBeds 已占用床位数
 * - maintenanceBeds 维护中床位数
 * - next 指向下一个病房的指针
 */
typedef struct Ward
{
    int id;                     /* 病房编号 */
    char wardType[SMALL_LEN];   /* 病房类型 */
    char dept[SMALL_LEN];       /* 所属科室 */
    int bedCount;               /* 床位总数 */
    int occupiedBeds;           /* 已占用床位数 */
    int maintenanceBeds;        /* 维护中床位数 */
    struct Ward *next;          /* 下一节点指针 */
} Ward;

/*
 * 结构体：Inpatient
 * 说明：住院记录结构体
 * 
 * - id 住院编号（唯一标识）
 * - patientId 患者病历号
 * - wardId 病房编号
 * - bedNo 床位号
 * - admitDate 入院日期
 * - expectedDischarge 预计出院日期
 * - totalCost 预估住院总费用
 * - next 指向下一条住院记录的指针
 */
typedef struct Inpatient
{
    int id;                     /* 住院编号 */
    int patientId;              /* 患者病历号 */
    int wardId;                 /* 病房编号 */
    int bedNo;                  /* 床位号 */
    char admitDate[DATE_LEN];   /* 入院日期 */
    char expectedDischarge[DATE_LEN]; /* 预计出院日期 */
    double totalCost;           /* 预估总费用 */
    struct Inpatient *next;     /* 下一节点指针 */
} Inpatient;

/*
 * 结构体：Drug
 * 说明：药品信息结构体
 * 
 * - id 药品编号（唯一标识）
 * - genericName 通用名
 * - brandName 商品名
 * - alias 别名
 * - type 药品类别
 * - dept 所属科室
 * - price 单价
 * - stock 库存数量
 * - next 指向下一个药品的指针
 */
typedef struct Drug
{
    int id;                     /* 药品编号 */
    char genericName[NAME_LEN]; /* 通用名 */
    char brandName[NAME_LEN];   /* 商品名 */
    char alias[NAME_LEN];       /* 别名 */
    char type[SMALL_LEN];       /* 类别 */
    char dept[SMALL_LEN];       /* 所属科室 */
    double price;               /* 单价 */
    int stock;                  /* 库存数量 */
    struct Drug *next;          /* 下一节点指针 */
} Drug;

/*
 * 结构体：DrugLog
 * 说明：药品出入库日志结构体
 * 
 * - id 日志编号（唯一标识）
 * - drugId 药品编号
 * - operation 操作类型（入库/出库）
 * - quantity 数量
 * - operatorName 操作人姓名
 * - date 操作日期
 * - next 指向下一条日志的指针
 */
typedef struct DrugLog
{
    int id;                     /* 日志编号 */
    int drugId;                 /* 药品编号 */
    char operation[SMALL_LEN];  /* 操作类型 */
    int quantity;               /* 数量 */
    char operatorName[NAME_LEN];/* 操作人 */
    char date[DATE_LEN];        /* 操作日期 */
    struct DrugLog *next;       /* 下一节点指针 */
} DrugLog;

/*
 * 结构体：Database
 * 说明：数据库结构体，包含所有数据链表的头指针
 * 
 * 该结构体是整个系统的数据核心，所有数据都通过链表形式存储
 * 
 * - patients 患者链表头指针
 * - doctors 医生链表头指针
 * - registrations 挂号记录链表头指针
 * - visits 看诊记录链表头指针
 * - exams 检查记录链表头指针
 * - wards 病房链表头指针
 * - inpatients 住院记录链表头指针
 * - drugs 药品链表头指针
 * - drugLogs 药品日志链表头指针
 * - accounts 账号链表头指针
 */
typedef struct Database
{
    Patient *patients;          /* 患者链表头 */
    Doctor *doctors;            /* 医生链表头 */
    Registration *registrations;/* 挂号记录链表头 */
    Visit *visits;              /* 看诊记录链表头 */
    Exam *exams;                /* 检查记录链表头 */
    Ward *wards;                /* 病房链表头 */
    Inpatient *inpatients;      /* 住院记录链表头 */
    Drug *drugs;                /* 药品链表头 */
    DrugLog *drugLogs;          /* 药品日志链表头 */
    Account *accounts;          /* 账号链表头 */
} Database;

/* ==================== 数据库管理函数 ==================== */

/*
 * 说明：初始化数据库
 * 参数：db 数据库指针
 * 将所有链表头指针设置为 NULL
 */
void init_database(Database *db);

/*
 * 说明：释放数据库所有内存
 * 参数：db 数据库指针
 * 遍历并释放所有链表节点
 */
void free_database(Database *db);

/* ==================== 内部辅助函数（用于导入数据时清空现有数据）==================== */

/* 释放患者链表所有节点 */
void free_patients(Patient *head);

/* 释放医生链表所有节点 */
void free_doctors(Doctor *head);

/* 释放挂号记录链表所有节点 */
void free_regs(Registration *head);

/* 释放看诊记录链表所有节点 */
void free_visits(Visit *head);

/* 释放检查记录链表所有节点 */
void free_exams(Exam *head);

/* 释放病房链表所有节点 */
void free_wards(Ward *head);

/* 释放住院记录链表所有节点 */
void free_inpatients(Inpatient *head);

/* 释放药品链表所有节点 */
void free_drugs(Drug *head);

/* 释放药品日志链表所有节点 */
void free_druglogs(DrugLog *head);

/* 释放账号链表所有节点 */
void free_accounts(Account *head);

/* ==================== 查找函数 ==================== */

/*
 * 说明：根据病历号查找患者
 * 参数：db 数据库指针
 * 参数：id 病历号
 * 返回值：找到的患者指针，未找到返回 NULL
 */
Patient *find_patient(Database *db, int id);

/*
 * 说明：根据工号查找医生
 * 参数：db 数据库指针
 * 参数：id 工号
 * 返回值：找到的医生指针，未找到返回 NULL
 */
Doctor *find_doctor(Database *db, int id);

/*
 * 说明：根据挂号编号查找挂号记录
 * 参数：db 数据库指针
 * 参数：id 挂号编号
 * 返回值：找到的挂号记录指针，未找到返回 NULL
 */
Registration *find_registration(Database *db, int id);

/*
 * 说明：根据病房编号查找病房
 * 参数：db 数据库指针
 * 参数：id 病房编号
 * 返回值：找到的病房指针，未找到返回 NULL
 */
Ward *find_ward(Database *db, int id);

/*
 * 说明：根据药品编号查找药品
 * 参数：db 数据库指针
 * 参数：id 药品编号
 * 返回值：找到的药品指针，未找到返回 NULL
 */
Drug *find_drug(Database *db, int id);

/* ==================== ID 生成函数 ==================== */

/* 生成下一个患者病历号（当前最大 ID+1） */
int next_patient_id(Database *db);
/* 生成下一个医生工号 */
int next_doctor_id(Database *db);
/* 生成下一个挂号编号 */
int next_registration_id(Database *db);
/* 生成下一个看诊编号 */
int next_visit_id(Database *db);
/* 生成下一个检查编号 */
int next_exam_id(Database *db);
/* 生成下一个病房编号 */
int next_ward_id(Database *db);
/* 生成下一个住院编号 */
int next_inpatient_id(Database *db);
/* 生成下一个药品编号 */
int next_drug_id(Database *db);
/* 生成下一个药品日志编号 */
int next_druglog_id(Database *db);

/* ==================== 账号管理函数 ==================== */

/*
 * 说明：根据用户名查找账号
 * 参数：db 数据库指针
 * 参数：username 用户名
 * 返回值：找到的账号指针，未找到返回 NULL
 */
Account *find_account(Database *db, const char *username);

/*
 * 说明：验证用户登录
 * 参数：db 数据库指针
 * 参数：username 用户名
 * 参数：password 密码
 * 返回值：登录成功返回账号指针，失败返回 NULL
 */
Account *authenticate_user(Database *db, const char *username, const char *password);

/*
 * 说明：创建新账号
 * 参数：db 数据库指针
 * 参数：username 用户名
 * 参数：password 密码
 * 参数：role 角色
 * 参数：linkedId 关联 ID
 * 返回值：1 表示成功，0 表示失败
 */
int create_account(Database *db, const char *username, const char *password, UserRole role, int linkedId);

/*
 * 说明：获取角色名称字符串
 * 参数：role 角色枚举
 * 返回值：角色名称字符串
 */
const char *get_role_name(UserRole role);

/* ==================== 数据加载/保存函数 ==================== */

/*
 * 说明：从指定目录加载所有数据文件
 * 参数：db 数据库指针
 * 参数：dir 数据文件目录
 * 返回值：1 表示成功
 */
int load_all(Database *db, const char *dir);

/*
 * 说明：保存所有数据到指定目录
 * 参数：db 数据库指针
 * 参数：dir 数据文件目录
 * 返回值：1 表示成功
 */
int save_all(Database *db, const char *dir);

#endif
