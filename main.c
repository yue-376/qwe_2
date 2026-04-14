/*
  * 文件：main.c
 * 说明：医院管理系统 (HIS) 主程序入口
 * 
 * 这是整个医疗综合管理系统的主入口文件，负责：
 * - 设置控制台编码为 UTF-8，支持中文显示
 * - 初始化数据库结构
 * - 加载数据文件
 * - 进入主菜单界面
 * - 程序退出时释放资源
 */

#include "menu.h"
#include <stdio.h>

#ifndef _WIN32
#define SetConsoleCP(x)
#define SetConsoleOutputCP(x)
#else
#include <windows.h>
#endif

/*
 * 说明：程序主函数
 * 返回值：0 表示程序正常退出
 * 
 * 执行流程：
 * 1. 设置控制台输入输出编码为 UTF-8（仅 Windows 有效）
 * 2. 创建并初始化数据库结构体
 * 3. 从当前目录加载所有数据文件
 * 4. 进入主菜单，开始用户交互
 * 5. 用户退出后，释放所有内存资源
 */
int main(void)
{
    // 设置控制台输入编码为 UTF-8，确保中文正常显示
    SetConsoleCP(65001);
    // 设置控制台输出编码为 UTF-8，确保中文正常显示
    SetConsoleOutputCP(65001);

    Database db;              // 数据库结构体，存储所有医疗数据的链表头指针
    const char *dataDir = "."; // 数据文件存储目录，"."表示当前目录
    init_database(&db);       // 初始化数据库，将所有链表指针设为 NULL
    load_all(&db, dataDir);   // 从数据文件加载所有记录到内存
    
    /* 先执行登录流程，登录成功后才进入主菜单 */
    if (login_menu(&db)) {
        /* 根据用户角色进入不同的菜单界面 */
        switch (g_session.role) {
            case ROLE_PATIENT:
                patient_menu(&db, dataDir);
                break;
            case ROLE_DOCTOR:
                doctor_menu(&db, dataDir);
                break;
            case ROLE_MANAGER:
                manager_menu(&db, dataDir);
                break;
            default:
                main_menu(&db, dataDir);  //  fallback to main menu for unknown roles
                break;
        }
    } else {
        printf("登录失败，程序退出。\n");
    }
    
    free_database(&db);       // 释放所有动态分配的内存
    return 0;
}
