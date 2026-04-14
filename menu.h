/*
  * 文件：menu.h
 * 说明：主菜单功能函数声明头文件
 * 
 * 本文件声明了系统主菜单相关的函数：
 * - main_menu: 显示主菜单并处理用户选择
 * 
 * 主菜单是系统的核心交互界面，提供以下功能入口：
 * - 患者管理（查看、新增、删除患者）
 * - 挂号管理（新增、删除挂号记录）
 * - 看诊管理（新增、删除看诊记录）
 * - 检查管理（新增、删除检查记录）
 * - 住院管理（新增、删除住院记录）
 * - 药品管理（出入库、新增、删除药品）
 * - 报表查询（患者视角、医生视角、管理视角）
 */

#ifndef MENU_H
#define MENU_H

#include "models.h"

/*
 * 说明：全局登录会话
 */
extern UserSession g_session;

/*
 * 说明：登录菜单函数
 * 参数：db 数据库指针
 * 返回值：登录成功返回 1，失败返回 0
 */
int login_menu(Database *db);

/*
 * 说明：注册菜单函数
 * 参数：db 数据库指针
 */
void register_menu(Database *db);

/*
 * 说明：患者角色菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录
 */
void patient_menu(Database *db, const char *dataDir);

/*
 * 说明：医生角色菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录
 */
void doctor_menu(Database *db, const char *dataDir);

/*
 * 说明：管理员角色菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录
 */
void manager_menu(Database *db, const char *dataDir);

/*
 * 说明：主菜单函数
 * 参数：db 数据库指针
 * 参数：dataDir 数据文件存储目录
 */
void main_menu(Database *db, const char *dataDir);

#endif
