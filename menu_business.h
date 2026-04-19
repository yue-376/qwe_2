/*
 * 文件：menu_business.h
 * 说明：业务功能菜单头文件（患者/医生/挂号相关）
 */

#ifndef MENU_BUSINESS_H
#define MENU_BUSINESS_H

#include "models.h"

/* 全局登录会话 */
extern UserSession g_session;

/* 登录与注册 */
int login_menu(Database *db);
void register_menu(Database *db);

/* 角色菜单 */
void patient_menu(Database *db, const char *dataDir);
void doctor_menu(Database *db, const char *dataDir);

/* 管理员菜单和主菜单 */
void manager_menu(Database *db, const char *dataDir);
void main_menu(Database *db, const char *dataDir);

/* 管理功能菜单 */
void patient_management_menu(Database *db, const char *dataDir);
void archive_management_menu(Database *db, const char *dataDir);
void drug_management_menu(Database *db, const char *dataDir);
void management_report(Database *db);
void user_account_management_menu(Database *db, const char *dataDir);

/* 业务子菜单（医生视角需要） */
void visit_management_menu(Database *db, const char *dataDir);
void exam_management_menu(Database *db, const char *dataDir);
void inpatient_management_menu(Database *db, const char *dataDir);

/* 输入辅助函数 */
int read_line_or_back(const char *prompt, char *buf, int size);
int read_int_or_back(const char *prompt, int min, int max, int *out);

#endif
