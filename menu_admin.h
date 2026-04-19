/*
 * 文件：menu_admin.h
 * 说明：管理功能菜单头文件（管理员/设置/辅助函数）
 */

#ifndef MENU_ADMIN_H
#define MENU_ADMIN_H

#include "models.h"

/* 全局登录会话 */
extern UserSession g_session;

/* 角色菜单 */
void manager_menu(Database *db, const char *dataDir);
void main_menu(Database *db, const char *dataDir);

/* 管理功能菜单 */
void patient_management_menu(Database *db, const char *dataDir);
void archive_management_menu(Database *db, const char *dataDir);
void drug_management_menu(Database *db, const char *dataDir);
void management_report(Database *db);
void user_account_management_menu(Database *db, const char *dataDir);

/* 输入辅助函数 */
int read_line_or_back(const char *prompt, char *buf, int size);
int read_int_or_back(const char *prompt, int min, int max, int *out);

#endif
