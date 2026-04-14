/*
  * 文件：data.h
 * 说明：数据加载和保存函数声明头文件
 * 
 * 本文件声明了与数据持久化相关的函数：
 * - 从文件加载数据到内存
 * - 将内存数据保存到文件
 * - 根据 ID 查找各类记录
 * - 生成下一个可用 ID
 */

#ifndef DATA_H
#define DATA_H

#include "models.h"

/*
 * 说明：从指定目录加载所有数据文件到数据库
 * 参数：db 数据库指针
 * 参数：dir 数据文件所在目录
 * 返回值：1 表示成功
 */
int load_all(Database *db, const char *dir);

/*
 * 说明：将数据库中所有数据保存到指定目录
 * 参数：db 数据库指针
 * 参数：dir 数据文件保存目录
 * 返回值：1 表示成功
 */
int save_all(Database *db, const char *dir);

/*
 * 说明：从指定目录导入数据（替换现有数据）
 * 参数：db 数据库指针
 * 参数：dir 要导入的数据文件目录
 * 返回值：成功导入的文件数量
 */
int import_all(Database *db, const char *dir);

/* ==================== 查找函数 ==================== */

/* 根据病历号查找患者 */
Patient *find_patient(Database *db, int id);

/* 根据工号查找医生 */
Doctor *find_doctor(Database *db, int id);

/* 根据挂号编号查找挂号记录 */
Registration *find_registration(Database *db, int id);

/* 根据病房编号查找病房 */
Ward *find_ward(Database *db, int id);

/* 根据药品编号查找药品 */
Drug *find_drug(Database *db, int id);

/* ==================== ID 生成函数 ==================== */

/* 生成下一个患者病历号 */
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

/* ==================== 账号数据保存函数 ==================== */

/*
 * 说明：保存账号数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 */
void save_accounts(Database *db, const char *path);

/* ==================== 工具函数 ==================== */

/*
 * 说明：拼接目录路径和文件名
 * 参数：out 输出缓冲区
 * 参数：size 缓冲区大小
 * 参数：dir 目录路径
 * 参数：name 文件名
 */
void path_join(char *out, size_t size, const char *dir, const char *name);

/*
 * 说明：保存患者数据到文件
 * 参数：db 数据库指针
 * 参数：path 文件路径
 */
void save_patients(Database *db, const char *path);

#endif
