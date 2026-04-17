/*
 * 文件：report.c
 * 说明：报表与统计功能实现文件
 * 
 * 本文件实现了医院管理系统的各种统计报表功能。
 * - 患者视角：查看个人完整就诊记录（挂号、检查、住院）
 * - 医生视角：查看个人工作统计（接诊量、检查开单量）
 * - 管理视角：全院营收估算、药品库存盘点、床位使用率统计
 * 
 * 这些报表帮助不同角色的用户从各自角度了解系统数据。
 * 
 * 本文件实现报表统计功能的原因：
 * 数据本身是没有意义的，只有把它们整理成报表，才能帮助人们做决策：
 * - 患者需要知道自己的诊疗历史和费用
 * - 医生需要了解自己的工作量和效率
 * - 管理者需要掌握医院的运营状况，做出资源调配决策
 */

#include "report.h"
#include "data.h"
#include <stdio.h>

/*
 * 函数：inpatient_total_cost - 计算患者住院费用总和
 * 
 * 功能说明：
 *   遍历住院记录链表，累加指定患者的所有住院费用。
 *   用于在患者视角报表中显示总住院花费。
 * 
 * 参数：
 *   db - 数据库指针，用于访问住院记录
 *   patientId - 患者的病历号
 * 
 * 返回值：
 *   该患者的住院费用总和（double 类型）
 * 
 * 处理流程：
 *   1. 初始化累计金额为 0
 *   2. 遍历住院记录链表（从第一条记录开始逐条查看）
 *   3. 对于每条记录，如果患者 ID 匹配则累加其费用
 *   4. 返回累计总额
 * 
 * 使用示例：
 *   double total = inpatient_total_cost(&db, patientId);
 *   printf("您本次住院总费用：%.2f 元\n", total);
 */
double inpatient_total_cost(Database *db, int patientId) {
    double s = 0;  /* 累计金额变量，初始值为 0 */
    Inpatient *p = db->inpatients;  /* 从住院链表头指针开始遍历 */
    while (p) {  /* 当 p 不为 NULL 时继续循环，表示还有记录未处理 */
        if (p->patientId == patientId) {  /* 判断当前记录的 patientId 是否等于目标患者 ID */
            s += p->totalCost;  /* 匹配时将当前记录的总费用累加到 s */
        }
        p = p->next;  /* 移动到链表中的下一条记录 */
    }
    return s;  /* 返回计算得到的总费用 */
}

/*
 * 函数：print_patient_full_record - 打印患者完整就诊记录
 * 
 * 功能说明：
 *   从患者视角展示其在本系统中的所有医疗记录，包括：
 *   - 基本信息：姓名、性别、出生日期、电话、医保类型
 *   - 挂号记录：每次挂号的日期、科室、医生、类型和状态
 *   - 检查记录：每项检查的编码、名称、时间、费用和结果
 *   - 住院记录：病房号、床位号、入院日期、预计出院日期和费用
 *   - 住院费用合计：所有住院记录的总费用
 * 
 * 参数：
 *   db - 数据库指针
 *   patientId - 要查询的患者病历号
 * 
 * 处理流程：
 *   1. 根据 ID 查找患者基本信息，未找到则提示并返回
 *   2. 显示患者基本信息
 *   3. 遍历挂号记录，筛选出该患者的记录并显示
 *   4. 遍历检查记录，筛选出该患者的记录并显示
 *   5. 遍历住院记录，筛选出该患者的记录并显示
 *   6. 计算并显示住院费用总和
 */
void print_patient_full_record(Database *db, int patientId) {
    Patient *p = find_patient(db, patientId);
    if (!p) { 
        printf("未找到该患者。\n"); 
        return; 
    }
    
    printf("\n===== 患者视角 =====\n");
    printf("病历号:%d 姓名:%s 性别:%s 出生:%s 电话:%s 医保:%s\n", 
           p->id, p->name, p->gender, p->birth, p->phone, p->insurance);

    /* 显示挂号记录 */
    printf("\n[挂号记录]\n");
    for (Registration *r = db->registrations; r; r = r->next) {
        if (r->patientId == patientId) {
            Doctor *doc = find_doctor(db, r->doctorId);
            printf("挂号号:%d 日期:%s 科室:%s 医生:%s 类型:%s 状态:%s\n",
                   r->id, r->date, r->dept, doc ? doc->name : "未知", r->type, r->status);
        }
    }

    /* 显示检查记录 */
    printf("\n[检查记录]\n");
    for (Exam *e = db->exams; e; e = e->next) {
        if (e->patientId == patientId) {
            printf("检查号:%d 编码:%s 名称:%s 时间:%s 费用:%.2f 结果:%s\n",
                   e->id, e->code, e->itemName, e->execTime, e->fee, e->result);
        }
    }

    /* 显示住院记录 */
    printf("\n[住院记录]\n");
    for (Inpatient *in = db->inpatients; in; in = in->next) {
        if (in->patientId == patientId) {
            printf("住院号:%d 病房:%d 床位:%d 入院:%s 预计出院:%s 费用:%.2f\n",
                   in->id, in->wardId, in->bedNo, in->admitDate, in->expectedDischarge, in->totalCost);
        }
    }
    printf("住院费用合计：%.2f\n", inpatient_total_cost(db, patientId));
}

/*
 * 函数：print_doctor_stats - 打印医生统计数据
 * 
 * 功能说明：
 *   从医生视角展示其工作统计信息，包括：
 *   - 基本信息：工号、姓名、科室、职称
 *   - 挂号关联数量：有多少次挂号记录选择了该医生
 *   - 执行检查数量：该医生开具了多少项检查
 * 
 * 参数：
 *   db - 数据库指针
 *   doctorId - 要查询的医生工号
 * 
 * 处理流程：
 *   1. 根据 ID 查找医生基本信息，未找到则提示并返回
 *   2. 遍历挂号记录，统计该医生的挂号数量
 *   3. 遍历检查记录，统计该医生开具的检查数量
 *   4. 显示医生信息和统计数据
 */
void print_doctor_stats(Database *db, int doctorId) {
    Doctor *d = find_doctor(db, doctorId);
    int regCount = 0, examCount = 0;
    
    if (!d) { 
        printf("未找到该医生。\n"); 
        return; 
    }

    /* 统计挂号数量 */
    for (Registration *r = db->registrations; r; r = r->next) {
        if (r->doctorId == doctorId) {
            regCount++;
        }
    }
    
    /* 统计检查开单数量 */
    for (Exam *e = db->exams; e; e = e->next) {
        if (e->doctorId == doctorId) {
            examCount++;
        }
    }

    printf("\n===== 医护视角 =====\n");
    printf("工号:%d 姓名:%s 科室:%s 职称:%s\n", d->id, d->name, d->dept, d->title);
    printf("挂号关联数量：%d\n", regCount);
    printf("执行检查数量：%d\n", examCount);
}

/*
 * 函数：print_management_report - 打印管理视角统计报表
 * 
 * 功能说明：
 *   从医院管理者视角展示全院运营统计数据，包括：
 *   - 全院营收估算：检查费用 + 住院费用的总和
 *   - 药品库存盘点：列出所有药品及其库存，标记低库存药品
 *   - 床位使用率：每个病房的床位使用情况（百分比和实际数值）
 * 
 * 参数：
 *   db - 数据库指针
 * 
 * 处理流程：
 *   1. 遍历住院记录和检查记录，累加计算全院营收
 *   2. 显示营收估算
 *   3. 以表格形式显示药品库存，统计低库存药品数量（库存<30）
 *   4. 遍历病房列表，计算并显示每个病房的床位使用率
 */
void print_management_report(Database *db) {
    double revenue = 0;  /* 全院营收累计 */
    int lowStock = 0;    /* 低库存药品计数器 */

    /* 计算住院收入 */
    for (Inpatient *in = db->inpatients; in; in = in->next) 
        revenue += in->totalCost;
    
    /* 计算检查收入 */
    for (Exam *e = db->exams; e; e = e->next) 
        revenue += e->fee;

    printf("\n===== 管理视角 =====\n");
    printf("全院营收估算 (检查 + 住院): %.2f\n", revenue);

    /* 药品库存盘点表格 */
    printf("\n[药品库存盘点]\n");
    printf("+--------------------------+--------+\n");
    printf("| ");
    print_utf8_cell_fit("药品名称", 24);
    printf(" | ");
    print_utf8_cell_fit("库存", 8);
    printf(" |\n");
    printf("+--------------------------+--------+\n");

    for (Drug *drug = db->drugs; drug; drug = drug->next) {
        if (drug->stock < 30) 
            lowStock++;  /* 统计低库存药品 */

        printf("| ");
        print_utf8_cell_fit(drug->genericName, 24);
        printf(" | ");
        printf("%-8d", drug->stock);
        printf(" |\n");
    }

    printf("+------------------------------+----------+\n");
    printf("低库存药品数量：%d\n", lowStock);

    /* 床位使用率统计 */
    printf("\n[床位使用率]\n");
    for (Ward *w = db->wards; w; w = w->next) {
        printf("病房 ID:%d 类型:%s 使用率:%.2f%% (%d/%d)\n", 
               w->id, w->wardType,
               w->bedCount ? 100.0 * w->occupiedBeds / w->bedCount : 0.0,
               w->occupiedBeds, w->bedCount);
    }
}

/*
 * 函数：report_menu - 报表统计查询菜单
 * 
 * 功能说明：
 *   提供报表查询的主菜单界面，用户可以选择三种视角进行查询：
 *   1. 患者视角：输入病历号查看该患者的完整就诊记录
 *   2. 医护视角：输入医生工号查看该医生的工作统计
 *   3. 管理视角：查看全院运营统计报表
 *   0. 返回上级菜单
 * 
 * 参数：
 *   db - 数据库指针
 * 
 * 流程：
 *   循环显示菜单，根据用户选择调用相应的报表函数，
 *   每次查询后等待用户按回车键继续，直到用户选择返回
 */
void report_menu(Database *db) {
    int ch;   /* 菜单选择 */
    int id;   /* 查询的 ID（患者病历号或医生工号） */
    
    while (1) {
        printf("\n===== 报表与统计查询 =====\n");
        printf("1. 患者视角查询\n2. 医护视角查询\n3. 管理视角查询\n0. 返回上一级\n");
        ch = read_int("请选择：", 0, 3);
        
        switch (ch) {
            case 1:  /* 患者视角查询 */
                id = read_int("输入患者病历号 (输入 0 返回): ", 0, 9999999);
                if (id == 0) { 
                    printf("已返回上一步。\n"); 
                    break; 
                }
                print_patient_full_record(db, id);
                printf("\n按回车继续..."); 
                getchar();
                break;
                
            case 2:  /* 医生视角查询 */
                id = read_int("输入医生工号 (输入 0 返回): ", 0, 9999999);
                if (id == 0) { 
                    printf("已返回上一步。\n"); 
                    break; 
                }
                print_doctor_stats(db, id);
                printf("\n按回车继续..."); 
                getchar();
                break;
                
            case 3:  /* 管理视角查询 */
                print_management_report(db);
                printf("\n按回车继续..."); 
                getchar();
                break;
                
            case 0:  /* 返回上级菜单 */
                return;
        }
    }
}
