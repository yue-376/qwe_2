/*
 * 文件：auth.c
 * 说明：认证与授权功能实现文件
 * 
 * 本文件实现了用户登录、注册和权限验证功能
 */

#include "models.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 全局登录会话 */
extern UserSession g_session;

/*
 * 说明：检查用户是否有指定角色权限
 * 参数：requiredRole 需要的角色
 * 返回值：1 表示有权限，0 表示无权限
 */
int check_permission(UserRole requiredRole)
{
    if (!g_session.isLoggedIn)
    {
        return 0;
    }
    
    // 管理员拥有所有权限
    if (g_session.role == ROLE_MANAGER)
    {
        return 1;
    }
    
    return g_session.role == requiredRole;
}

/*
 * 说明：登出当前用户
 */
void logout_user(void)
{
    g_session.isLoggedIn = 0;
    g_session.role = ROLE_PATIENT;
    g_session.userId = 0;
    g_session.username[0] = '\0';
}
