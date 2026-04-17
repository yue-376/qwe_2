/*
 * 文件：main.c
 * 说明：医院管理系统 (HIS) 主程序入口
 * 
 * 这是整个医疗综合管理系统的主入口文件，
 * 它负责协调整个程序的运行流程：
 * - 设置控制台编码为 UTF-8，确保中文字符能正常显示（否则会出现乱码）
 * - 初始化数据库结构，准备存储所有医疗数据
 * - 从文件加载数据，将之前保存的患者、医生、挂号记录等信息读入内存
 * - 进入主菜单界面，让用户可以进行各种操作
 * - 程序退出时释放资源，把动态分配的内存归还给系统，避免内存泄漏
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
 * 返回值：0 表示程序正常退出，非 0 表示异常退出
 * 
 * 执行流程如下：
 * 1. 设置控制台输入输出编码为 UTF-8（仅 Windows 有效）
 *    - 设置 UTF-8 编码的原因：Windows 控制台默认使用 GBK 编码，
 *      而我们的程序使用 UTF-8，不转换会显示乱码
 * 2. 创建并初始化数据库结构体
 *    - Database 结构体包含所有链表头指针
 *    - 初始化就是把所有指针设为 NULL，表示空数据库
 * 3. 从当前目录加载所有数据文件
 *    - 把 patients.txt、doctors.txt 等文件中的数据读到内存链表中
 * 4. 进入登录流程，验证用户身份
 * 5. 根据用户角色（患者/医生/管理员）进入不同的菜单界面
 * 6. 用户选择退出后，释放所有动态分配的内存
 *    - 此步骤的必要性：不释放内存会导致内存泄漏
 */
int main(void)
{
    /* 设置控制台输入编码为 UTF-8，确保中文正常显示 */
    /* 设置控制台使用 UTF-8 编码 */
    SetConsoleCP(65001);
    /* 设置控制台输出编码为 UTF-8，确保中文正常显示 */
    /* 这样 printf 输出的中文字符才能正确显示 */
    SetConsoleOutputCP(65001);

    Database db;              /* 数据库结构体 */
                              /* 该结构体包含 10 个链表头指针，每种类型的数据使用一个链表存储 */
                              /* patients 字段存储患者链表，doctors 字段存储医生链表 */
    const char *dataDir = "."; /* 数据文件存储目录，"."表示当前工作目录 */
                               /* 修改此路径可指定其他数据目录，例如"/data"或"./data" */
    init_database(&db);       /* 初始化数据库，将所有链表指针设为 NULL */
                              /* 这步不做的话，指针会是随机值，可能导致程序崩溃 */
    load_all(&db, dataDir);   /* 从数据文件加载所有记录到内存 */
                              /* 加载完成后，db 结构体中包含所有从文件读取的数据记录 */
    
    /* 先执行登录流程，登录成功后才进入主菜单 */
    /* login_menu 返回值的含义：
     *   1  - 登录成功
     *   0  - 用户选择退出程序
     *  -1  - 注册成功或登录失败，需要重新显示登录菜单
     */
    int loginResult = login_menu(&db);
    while (loginResult == -1) {
        /* 注册成功后重新回到登录选择界面 */
        /* 这样设计是为了让用户注册完可以直接登录，不用重新启动程序 */
        loginResult = login_menu(&db);
    }
    
    if (loginResult) {
        /* 登录成功后，根据用户角色进入不同的菜单界面 */
        /* g_session 是全局变量，保存着当前登录用户的信息 */
        switch (g_session.role) {
            case ROLE_PATIENT:
                /* 患者只能看到与自己相关的功能：挂号、查看记录等 */
                patient_menu(&db, dataDir);
                break;
            case ROLE_DOCTOR:
                /* 医生可以看到：接诊患者、开具检查、写诊断等 */
                doctor_menu(&db, dataDir);
                break;
            case ROLE_MANAGER:
                /* 管理员拥有最高权限：管理档案、药品、查看统计报表等 */
                manager_menu(&db, dataDir);
                break;
            default:
                /* 防御性编程：万一遇到未知角色，回退到主菜单 */
                main_menu(&db, dataDir);
                break;
        }
        
        /* 用户从角色菜单返回（选择"返回登录选择"）后，重新进入登录流程 */
        /* 这样设计允许用户在不退出程序的情况下切换账号 */
        while (1) {
            loginResult = login_menu(&db);
            while (loginResult == -1) {
                loginResult = login_menu(&db);
            }
            
            if (loginResult == 0) {
                /* 用户选择退出程序，跳出循环 */
                break;
            }
            
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
                    main_menu(&db, dataDir);
                    break;
            }
        }
    } else {
        /* 用户在登录菜单选择退出程序，直接结束运行 */
        /* 此时不显示任何提示信息，保持界面简洁 */
    }
    
    free_database(&db);       /* 释放所有动态分配的内存 */
                              /* 释放所有通过 malloc 分配的内存 */
                              /* 防止内存泄漏 */
    return 0;                 /* 返回 0 表示程序正常结束 */
}
