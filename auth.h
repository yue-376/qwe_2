/*
 * 文件：auth.h
 * 说明：认证与授权功能声明头文件
 */

#ifndef AUTH_H
#define AUTH_H

#include "models.h"

/*
 * 说明：检查用户是否有指定角色权限
 * 参数：requiredRole 需要的角色
 * 返回值：1 表示有权限，0 表示无权限
 */
int check_permission(UserRole requiredRole);

/*
 * 说明：登出当前用户
 */
void logout_user(void);

#endif
