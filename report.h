/*
  * 文件：report.h
 * 说明：报表统计功能函数声明头文件
 * 
 * 本文件声明了用于生成各类统计报表的函数：
 * - 患者视角查询（查看患者的所有就诊记录）
 * - 医生视角查询（查看医生的工作统计）
 * - 管理视角查询（全院运营数据统计）
 */

#ifndef REPORT_H
#define REPORT_H

#include "models.h"

/*
 * 说明：报表统计菜单
 * 参数：db 数据库指针
 * 
 * 显示报表菜单，允许用户选择：
 * 1. 患者视角查询
 * 2. 医护视角查询
 * 3. 管理视角查询
 */
void report_menu(Database *db);

/*
 * 说明：打印患者完整就诊记录
 * 参数：db 数据库指针
 * 参数：patientId 患者病历号
 * 
 * 显示指定患者的：
 * - 基本信息
 * - 所有挂号记录
 * - 所有检查记录
 * - 所有住院记录及费用汇总
 */
void print_patient_full_record(Database *db, int patientId);

/*
 * 说明：打印医生工作统计
 * 参数：db 数据库指针
 * 参数：doctorId 医生工号
 * 
 * 显示指定医生的：
 * - 基本信息
 * - 挂号关联数量
 * - 执行检查数量
 */
void print_doctor_stats(Database *db, int doctorId);

/*
 * 说明：打印管理报表
 * 参数：db 数据库指针
 * 
 * 显示全院运营数据：
 * - 总收入估算（检查费 + 住院费）
 * - 药品库存盘点
 * - 床位使用率统计
 */
void print_management_report(Database *db);

#endif
