#include "report.h"

/* 计算指定患者的住院费用总和 */
double inpatient_total_cost(Database *db, int patientId) {
    double s = 0;
    Inpatient *p = db->inpatients;
    while (p) {
        if (p->patientId == patientId) {
            s += p->totalCost;
        }
        p = p->next;
    }
    return s;
}

/* 打印患者完整就诊记录（含挂号、检查、住院） */
void print_patient_full_record(Database *db, int patientId) {
    Patient *p = find_patient(db, patientId);
    if (!p) { printf("未找到该患者。\n"); return; }
    printf("\n===== 患者视角 =====\n");
    printf("病历号:%d 姓名:%s 性别:%s 出生:%s 电话:%s 医保:%s\n", p->id, p->name, p->gender, p->birth, p->phone, p->insurance);

    printf("\n[挂号记录]\n");
    for (Registration *r = db->registrations; r; r = r->next) {
        if (r->patientId == patientId) {
            Doctor *doc = find_doctor(db, r->doctorId);
            printf("挂号号:%d 日期:%s 科室:%s 医生:%s 类型:%s 状态:%s\n",
                   r->id, r->date, r->dept, doc ? doc->name : "未知", r->type, r->status);
        }
    }

    printf("\n[检查记录]\n");
    for (Exam *e = db->exams; e; e = e->next) {
        if (e->patientId == patientId) {
            printf("检查号:%d 编码:%s 名称:%s 时间:%s 费用:%.2f 结果:%s\n",
                   e->id, e->code, e->itemName, e->execTime, e->fee, e->result);
        }
    }

    printf("\n[住院记录]\n");
    for (Inpatient *in = db->inpatients; in; in = in->next) {
        if (in->patientId == patientId) {
            printf("住院号:%d 病房:%d 床位:%d 入院:%s 预计出院:%s 费用:%.2f\n",
                   in->id, in->wardId, in->bedNo, in->admitDate, in->expectedDischarge, in->totalCost);
        }
    }
    printf("住院费用合计: %.2f\n", inpatient_total_cost(db, patientId));
}

/* 打印医生统计数据（接诊量、检查开单量） */
void print_doctor_stats(Database *db, int doctorId) {
    Doctor *d = find_doctor(db, doctorId);
    int regCount = 0, examCount = 0;
    if (!d) { printf("未找到该医生。\n"); return; }

    for (Registration *r = db->registrations; r; r = r->next) {
        if (r->doctorId == doctorId) {
            regCount++;
        }
    }
    for (Exam *e = db->exams; e; e = e->next) {
        if (e->doctorId == doctorId) {
            examCount++;
        }
    }

    printf("\n===== 医护视角 =====\n");
    printf("工号:%d 姓名:%s 科室:%s 职称:%s\n", d->id, d->name, d->dept, d->title);
    printf("挂号关联数量: %d\n", regCount);
    printf("执行检查数量: %d\n", examCount);
}

void print_management_report(Database *db) {
    double revenue = 0;
    int lowStock = 0;

    for (Inpatient *in = db->inpatients; in; in = in->next) revenue += in->totalCost;
    for (Exam *e = db->exams; e; e = e->next) revenue += e->fee;

    printf("\n===== 管理视角 =====\n");
    printf("全院营收估算(检查+住院): %.2f\n", revenue);

    printf("\n[药品库存盘点]\n");
printf("+--------------------------+--------+\n");

printf("| ");
print_utf8_cell_fit("药品名称", 24);
printf(" | ");
print_utf8_cell_fit("库存", 8);
printf(" |\n");

printf("+--------------------------+--------+\n");

for (Drug *drug = db->drugs; drug; drug = drug->next) {
    if (drug->stock < 30) lowStock++;

    printf("| ");
    print_utf8_cell_fit(drug->genericName, 24);
    printf(" | ");
    printf("%-8d", drug->stock);
    printf(" |\n");
}


    printf("+------------------------------+----------+\n");
    printf("低库存药品数量: %d\n", lowStock);

    printf("\n[床位使用率]\n");
    for (Ward *w = db->wards; w; w = w->next) {
        printf("病房ID:%d 类型:%s 使用率:%.2f%% (%d/%d)\n", w->id, w->wardType,
               w->bedCount ? 100.0 * w->occupiedBeds / w->bedCount : 0.0,
               w->occupiedBeds, w->bedCount);
    }
}

void report_menu(Database *db) {
    int ch;
    int id;
    while (1) {
        printf("\n===== 报表与统计查询 =====\n");
        printf("1. 患者视角查询\n2. 医护视角查询\n3. 管理视角查询\n0. 返回上一级\n");
        ch = read_int("请选择: ", 0, 3);
        switch (ch) {
            case 1:
                id = read_int("输入患者病历号(输入0返回): ", 0, 9999999);
                if (id == 0) { printf("已返回上一步。\n"); break; }
                print_patient_full_record(db, id);
                printf("\n按回车继续..."); getchar();
                break;
            case 2:
                id = read_int("输入医生工号(输入0返回): ", 0, 9999999);
                if (id == 0) { printf("已返回上一步。\n"); break; }
                print_doctor_stats(db, id);
                printf("\n按回车继续..."); getchar();
                break;
            case 3:
                print_management_report(db);
                printf("\n按回车继续..."); getchar();
                break;
            case 0:
                return;
        }
    }
}
