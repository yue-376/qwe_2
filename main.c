/*
 * 文件：main.c
 * 说明：医院管理系统 (HIS) 主程序入口
 * 
 * 这是整个医疗综合管理系统的主入口文件，就像一本书的目录一样，
 * 它负责协调整个程序的运行流程：
 * - 设置控制台编码为 UTF-8，确保中文字符能正常显示（不然会看到乱码）
 * - 初始化数据库结构，准备好存放所有医疗数据的"容器"
 * - 从文件加载数据，把之前保存的患者、医生、挂号记录等信息读到内存中
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
 * 这是整个程序的起点，就像电影的开场一样。执行流程如下：
 * 1. 设置控制台输入输出编码为 UTF-8（仅 Windows 有效）
 *    - 为什么要这样做？因为中文在 Windows 控制台默认使用 GBK 编码，
 *      而我们的程序使用 UTF-8，不转换会显示乱码
 * 2. 创建并初始化数据库结构体
 *    - Database 是一个"大容器"，里面装着所有链表头指针
 *    - 初始化就是把所有指针设为 NULL，表示空数据库
 * 3. 从当前目录加载所有数据文件
 *    - 把 patients.txt、doctors.txt 等文件中的数据读到内存链表中
 * 4. 进入登录流程，验证用户身份
 * 5. 根据用户角色（患者/医生/管理员）进入不同的菜单界面
 * 6. 用户选择退出后，释放所有动态分配的内存
 *    - 这一步很重要！如果不释放，会造成内存泄漏
 */
int main(void)
{
    /* 设置控制台输入编码为 UTF-8，确保中文正常显示 */
    /* 想象成告诉控制台："接下来我要用 UTF-8 编码显示文字了" */
    SetConsoleCP(65001);
    /* 设置控制台输出编码为 UTF-8，确保中文正常显示 */
    /* 这样 printf 输出的中文字符才能正确显示 */
    SetConsoleOutputCP(65001);

    Database db;              /* 数据库结构体，相当于一个"大柜子" */
                              /* 这个柜子里有 10 个抽屉，每个抽屉存放一种类型的数据链表 */
                              /* 比如 patients 抽屉放患者链表，doctors 抽屉放医生链表 */
    const char *dataDir = "."; /* 数据文件存储目录，"."表示当前目录 */
                               /* 如果改成"/data"，程序就会从/data 目录读取文件 */
    init_database(&db);       /* 初始化数据库，将所有链表指针设为 NULL */
                              /* 这步不做的话，指针会是随机值，可能导致程序崩溃 */
    load_all(&db, dataDir);   /* 从数据文件加载所有记录到内存 */
                              /* 加载完成后，db 里面就装满了从文件读到的数据 */
    
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
        /* 用户一开始就选择退出程序，不显示任何信息 */
        /* 这是一种简洁的退出方式，避免不必要的提示 */
    }
    
    free_database(&db);       /* 释放所有动态分配的内存 */
                              /* 这步非常重要！程序运行中 malloc 的所有内存都要在这里 free */
                              /* 否则会造成内存泄漏，虽然程序结束后系统会回收，但这是坏习惯 */
    return 0;                 /* 返回 0 表示程序正常结束 */
}
