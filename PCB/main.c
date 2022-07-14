#include <stdio.h>
#include<stdlib.h>
#include<math.h>
#define T 1

//进程的状态：初始化赋值I(initialize)，就绪状态 W，运行状态 R，完成状态 F
typedef struct node
{
    int name;
    int create_time;
    int need_time;
    int used_time;
    int remain_time;
    char status;
    struct node *next;
} PCB;

//队列结构体
typedef struct Queues
{
    int name;
    int timeSlice;
    PCB *data; //队列中的第一个进程指针
    struct Queues *next;
} Queue;

int time = 0;
int tempTimeSlice = 0;
int minWaitTime = 0;
Queue *head = NULL;
Queue *tail = NULL;
Queue *finishQF = NULL;
PCB *processHead = NULL;
PCB *processTail = NULL;

/*
 * 进程的初始化并随机录入进程的具体信息，进程链表作为准备队列
*/

//初始化进程链表（准备队列）并录入信息
void initProcess() {
    processHead = (PCB *)malloc(sizeof(PCB));
    processHead->next = NULL;
    processTail = processHead;

//    //随机录入信息
//    for (int i = 0; i < 5; ++i) {
//        PCB *newPCB = (PCB *)malloc(sizeof(PCB));
//        newPCB->name = i+1;
//        newPCB->create_time = rand() % 12;
//        newPCB->need_time = rand() % 10 + 1;
//        newPCB->used_time = 0;
//        newPCB->remain_time = newPCB->need_time;
//        newPCB->status = 'I';
//        newPCB->next = NULL;
//        processTail->next = newPCB;
//        processTail = newPCB;
//    }

//建立数据集，并将信息录入
    PCB *newPCB = (PCB *)malloc(sizeof(PCB));
    newPCB->name = 1;
    newPCB->create_time = 1;
    newPCB->need_time = 9;
    newPCB->used_time = 0;
    newPCB->remain_time = newPCB->need_time;
    newPCB->status = 'I';
    newPCB->next = NULL;
    processTail->next = newPCB;
    processTail = newPCB;

    newPCB = (PCB *)malloc(sizeof(PCB));
    newPCB->name = 2;
    newPCB->create_time = 3;
    newPCB->need_time = 5;
    newPCB->used_time = 0;
    newPCB->remain_time = newPCB->need_time;
    newPCB->status = 'I';
    newPCB->next = NULL;
    processTail->next = newPCB;
    processTail = newPCB;

    newPCB = (PCB *)malloc(sizeof(PCB));
    newPCB->name = 3;
    newPCB->create_time = 5;
    newPCB->need_time = 3;
    newPCB->used_time = 0;
    newPCB->remain_time = newPCB->need_time;
    newPCB->status = 'I';
    newPCB->next = NULL;
    processTail->next = newPCB;
    processTail = newPCB;

    newPCB = (PCB *)malloc(sizeof(PCB));
    newPCB->name = 4;
    newPCB->create_time = 6;
    newPCB->need_time = 1;
    newPCB->used_time = 0;
    newPCB->remain_time = newPCB->need_time;
    newPCB->status = 'I';
    newPCB->next = NULL;
    processTail->next = newPCB;
    processTail = newPCB;

    newPCB = (PCB *)malloc(sizeof(PCB));
    newPCB->name = 5;
    newPCB->create_time = 10;
    newPCB->need_time = 3;
    newPCB->used_time = 0;
    newPCB->remain_time = newPCB->need_time;
    newPCB->status = 'I';
    newPCB->next = NULL;
    processTail->next = newPCB;
    processTail = newPCB;

}

//显示进程信息
void showProcess() {
    if (time == 0) {
        printf("-----------------------所有进程情况---------------------------\n");
    } else {
        printf("-------------------在运行队列中的进程情况-----------------------\n");
    }
    printf("进程名 创建时间 需要运行时间 已用CPU时间 还需时间 进程状态\n");
    //如果三级队列为空，单纯显示进程链表信息
    if (head == NULL) {
        PCB *p = processHead;
        while(p->next!=NULL) {
            p=p->next;
            printf(" P%d\t %d\t  %d\t     %d\t      %d\t    %c\t\n",p->name,p->create_time,p->need_time,p->used_time,p->remain_time,p->status);
        }
        printf("------------------------------------------------------------\n\n");
    } else {
        //如果三级队列不为空，显示三级队列信息
        Queue *q = head;
        while (q != NULL) {
            PCB *p = q->data;
            while (p != NULL) {
                printf(" P%d\t %d\t  %d\t     %d\t      %d\t    %c\t\n",p->name,p->create_time,p->need_time,p->used_time,p->remain_time,p->status);
                p = p->next;
            }
            q = q->next;
        }
        printf("------------------------------------------------------------\n\n");
    }
}

/*
 * 初始化三级队列，每一级可以存放的进程数量为4。
 * 初始化时间片为2，一级队列的时间片为1，二级队列的时间片为2，三级队列的时间片为4
*/

//初始化队列：三级运行队列和完成队列
void initQueues() {
    //初始化三级运行队列
    for(int i = 0; i < 3; i++) {
        Queue *newQueue = (Queue *)malloc(sizeof(Queue));
        newQueue->timeSlice = pow(2, i) * T;
        newQueue->name = i+1;
        newQueue->data = NULL;
        newQueue->next = NULL;
        if(head == NULL) {
            head = newQueue;
        } else {
            Queue *p = head;
            while(p->next != NULL) {
                p = p->next;
            }
            p->next = newQueue;
        }
        //tail
        if(i == 0) {
            tail = newQueue;
        } else {
            tail = tail->next;
        }
    }

    //初始化完成队列
    Queue *newQueue = (Queue *)malloc(sizeof(Queue));
    newQueue->timeSlice = 0;
    newQueue->name = 4;
    newQueue->data = NULL;
    newQueue->next = NULL;
    finishQF = newQueue;
}

//显示队列
void showQueues() {
    printf("----------------------三级运行队列情况-------------------------\n");
    //显示三级运行队列
    Queue *p = head;
    printf("队列名\t时间片\t   队列内进程\n");
    while(p != NULL) {
        printf(" Q%d\t %d\t", p->name, p->timeSlice);
        PCB *q = p->data;
        while(q != NULL) {
            printf("P%d ", q->name);
            q = q->next;
        }
        printf("\n");
        p = p->next;
    }
    printf("------------------------------------------------------------\n\n");

    //显示完成队列
    Queue *q = finishQF;
    printf("----------------------已完成进程情况---------------------------\n");
    printf("进程名 创建时间 需要运行时间 已用CPU时间 还需时间 进程状态\n");
    while (q != NULL) {
        PCB *temp = q->data;
        while (temp != NULL) {
            printf(" P%d\t %d\t  %d\t     %d\t      %d\t    %c\t\n", temp->name, temp->create_time, temp->need_time, temp->used_time, temp->remain_time, temp->status);
            temp = temp->next;
        }
        q = q->next;
    }
    printf("------------------------------------------------------------\n\n\n");
}


//将进程插入队列尾部
void insertQueue(Queue *queue, PCB *pcb) {
    if(queue->data == NULL) {
        pcb->next = NULL;
        queue->data = pcb;
    } else {
        PCB *p = queue->data;
        while(p->next != NULL) {
            p = p->next;
        }
        pcb->next = NULL;
        p->next = pcb;
    }
}

//打印每个时刻进程块和队列的情况
void printAllByTime() {
    printf("\n");
    printf("时间块：%d - %d\n", time-1, time);
    printf("进程块\n");
    showProcess();
    showQueues();
}

/*
 * 进程插入队列
*/

//检查进程链表节点的create_time，当time等于create_time时，将该节点插入队列尾部
void checkIfAdd() {
    PCB *p = processHead;
    while(p != NULL) {
        if(p->create_time <= time) {
            //将该节点从进程链表中删除,头结点后移
            PCB *q = processHead;
            if(q == p) {
                processHead = p->next;
            } else {
                while(q->next != p) {
                    q = q->next;
                }
                q->next = p->next;
            }
            insertQueue(head, p);
            p->status = 'W';
        }
        p = p->next;
    }
}

//找出到达时间最大的片段
void findMaxCreateTime() {
    PCB *p = processHead;
    int maxCreateTime = 0;
    int maxNeedTime = 0;
    while(p != NULL) {
        if(p->create_time >= maxCreateTime) {
            maxCreateTime = p->create_time;
            maxNeedTime = p->need_time;
        }
        p = p->next;
    }
    minWaitTime = (maxCreateTime + maxNeedTime)*5;
}



//运行进程：在该队列时间片内运行进程，如果进程运行完成，则将其从队列中删除，运行下一个；如果进程没有完成，就添加到下一级队列里
void runProcess(Queue *queue, int timeSlice) {
    //按时间片循环运行进程
    if(tempTimeSlice == 0) {
        tempTimeSlice = timeSlice;
    }
    while(queue->data != NULL&&tempTimeSlice > 0) {
        PCB *p = queue->data;
        while(p != NULL) {
            time++;
            p->used_time++;
            p->remain_time--;
            p->status = 'R';
            tempTimeSlice--;

            //进程运行完成,但时间片有剩余，则将其从队列中删除，在剩余时间片运行下一个。
            if(p->remain_time == 0) {
                printAllByTime();
                p->status = 'F';
                //将进程从队列中删除
                if(p->next != NULL) {
                    queue->data = p->next;
                    //添加到完成队列
                    insertQueue(finishQF, p);
                    break;
                } else {
                    queue->data = NULL;
                    //添加到完成队列
                    insertQueue(finishQF, p);
                    break;
                }

            }
            //如果进程没有完成，时间片已经用完，则将其从队列中删除,添加到下一级队列里，如果是最后一个队列，则重新添加到队列尾部
            if(p->remain_time > 0 && tempTimeSlice == 0) {
                printAllByTime();
                if(p->next != NULL) {
                    queue->data = p->next;
                } else {
                    queue->data = NULL;
                }
                if(queue->next != NULL) {
                    p->status = 'W';
                    insertQueue(queue->next, p);
                } else {
                    p->status = 'W';
                    insertQueue(tail, p);
                }
                break;
            }

            //进程没有运行完成,时间片未运行完，则继续运行该进程。
            if(p->remain_time > 0) {
                printAllByTime();
                continue;
            }
        }
    }
}

/*
 * 正式循环运行三级队列，直到所有程序运行结束
*/

//运行队列：
void runQueue() {
    Queue *q1 = head;
    Queue *q2 = head->next;
    Queue *q3 = head->next->next;
    //循环运行三级队列，直到所有程序运行结束
    while(1) {
        checkIfAdd();
//        printf("time:%d minWaitTime:%d\n",time,minWaitTime);
        if(q1->data != NULL && time >= q1->data->create_time) {
            runProcess(q1, q1->timeSlice);
        } else if(q2->data != NULL && time >= q2->data->create_time) {
            runProcess(q2, q2->timeSlice);
        } else if(q3->data != NULL && time >= q3->data->create_time) {
            runProcess(q3, q3->timeSlice);
        } else {
            if(q1->data == NULL && q2->data == NULL && q3->data == NULL && time == minWaitTime) {
                printf("三级队列中的所有进程都运行完成啦！\n");
                break;
            } else {
                time++;
            }
        }
    }
}

//释放所有内存
void freeAll() {
    //释放队列进程
    PCB *p1 = processHead;
    while(p1 != NULL) {
        PCB *q1 = p1;
        p1 = p1->next;
        free(q1);
    }
    //释放三级队列
    Queue *p2 = head;
    while(p2 != NULL) {
        Queue *q2 = p2;
        p2 = p2->next;
        free(q2);
    }
    //释放完成队列
    Queue *p3 = finishQF;
    free(p3);
}

int main() {
    system("chcp 65001");
    initProcess();
    showProcess();

    initQueues();
    showQueues();
    processHead = processHead->next;
    findMaxCreateTime();

    runQueue();
    freeAll();
    return 0;
}
