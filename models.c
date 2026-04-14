/*
  * 文件：models.c
 * 说明：数据模型管理函数实现文件
 * 
 * 本文件实现了数据库的初始化和内存释放功能：
 * - init_database: 初始化数据库结构
 * - free_database: 释放所有链表节点占用的内存
 */

#include "models.h"

/*
 * 说明：释放患者链表所有节点
 * 参数：head 患者链表头指针
 */
static void free_patients(Patient *head)
{
    while (head)
    {
        Patient *t = head;      /* 保存当前节点 */
        head = head->next;      /* 移动到下一节点 */
        free(t);                /* 释放当前节点 */
    }
}

/*
 * 说明：释放医生链表所有节点
 * 参数：head 医生链表头指针
 */
static void free_doctors(Doctor *head)
{
    while (head)
    {
        Doctor *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放挂号记录链表所有节点
 * 参数：head 挂号记录链表头指针
 */
static void free_regs(Registration *head)
{
    while (head)
    {
        Registration *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放看诊记录链表所有节点
 * 参数：head 看诊记录链表头指针
 */
static void free_visits(Visit *head)
{
    while (head)
    {
        Visit *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放检查记录链表所有节点
 * 参数：head 检查记录链表头指针
 */
static void free_exams(Exam *head)
{
    while (head)
    {
        Exam *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放病房链表所有节点
 * 参数：head 病房链表头指针
 */
static void free_wards(Ward *head)
{
    while (head)
    {
        Ward *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放住院记录链表所有节点
 * 参数：head 住院记录链表头指针
 */
static void free_inpatients(Inpatient *head)
{
    while (head)
    {
        Inpatient *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放药品链表所有节点
 * 参数：head 药品链表头指针
 */
static void free_drugs(Drug *head)
{
    while (head)
    {
        Drug *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：释放药品日志链表所有节点
 * 参数：head 药品日志链表头指针
 */
static void free_druglogs(DrugLog *head)
{
    while (head)
    {
        DrugLog *t = head;
        head = head->next;
        free(t);
    }
}

/*
 * 说明：初始化数据库结构
 * 参数：db 数据库指针
 * 
 * 将所有链表头指针设置为 NULL，表示空数据库
 * 必须在加载数据或添加任何记录前调用此函数
 */
void init_database(Database *db)
{
    db->patients = NULL;
    db->doctors = NULL;
    db->registrations = NULL;
    db->visits = NULL;
    db->exams = NULL;
    db->wards = NULL;
    db->inpatients = NULL;
    db->drugs = NULL;
    db->drugLogs = NULL;
}

/*
 * 说明：释放数据库所有内存
 * 参数：db 数据库指针
 * 
 * 依次释放所有链表中的节点，防止内存泄漏
 * 释放后重新初始化数据库结构
 */
void free_database(Database *db)
{
    free_patients(db->patients);
    free_doctors(db->doctors);
    free_regs(db->registrations);
    free_visits(db->visits);
    free_exams(db->exams);
    free_wards(db->wards);
    free_inpatients(db->inpatients);
    free_drugs(db->drugs);
    free_druglogs(db->drugLogs);
    init_database(db);  /* 重置为初始状态 */
}
