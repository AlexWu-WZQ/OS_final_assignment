#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#define MaxDiskSize 512 //磁盘大小512KB

//磁盘块结构体
typedef struct DiskBlock
{
    char userName[10];
    char fileName[10];
    int blockSize;
    int startPos;
    int endPos;
    struct DiskBlock *next;
}diskNode;

//用户目录（某用户名下存放的文件）
typedef struct ufd
{
    char fileName[10];
    int flag;
    int fileProtectCode[3]; //r\w\e
    char fileCreateDate[20];
    int fileLength;
    int fileStartPos;
    int fileEndPos;
}UF[10];

//主目录(用户名和密码)
struct mfd
{
    char userName[10];
    char password[20];
    UF uDir;
}UFD[10];

//运行文件目录
struct afd
{
    char openFileName[10];
    int flag;
    char openFileProtectCode[3];
    int rwPoint;
}AFD[5];

diskNode *diskHead; //磁盘头指针

//在内存中建立一个虚拟磁盘空间作为文件存储器（模拟磁盘）分配磁盘块，返回分配的磁盘块的起始地址
//因为不涉及具体读写操作，所以默认所有文件只是根据大小在磁盘上分配空间，并不存储数据
void initDisk() {
    if (diskHead == NULL) {
        diskHead = (diskNode *) malloc(sizeof(diskNode));
    }
    //初始化磁盘块
    for(int i = 0; i < 10; i++) {
        diskHead->userName[i] = '\0';
        diskHead->fileName[i] = '\0';
    }

    diskHead->blockSize = MaxDiskSize;
    diskHead->startPos = 0;
    diskHead->endPos = 0;
    diskHead->next = NULL;
}

//分配磁盘块
void allocateDiskBlock(int currentUserID, int fileNum) {
    //如果需求的磁盘块大小大于磁盘大小，则报错
    if (UFD[currentUserID].uDir[fileNum].fileLength > MaxDiskSize) {
        printf("文件大小超出磁盘大小，请重新分配磁盘块\n");
    }
    //如果是第一个文件，则直接分配磁盘块
    if (currentUserID == 0 && fileNum == 0) {
        diskHead->startPos = 0;
        diskHead->endPos = UFD[currentUserID].uDir[fileNum].fileLength;
        diskHead->blockSize = diskHead->blockSize - UFD[currentUserID].uDir[fileNum].fileLength;
        for(int i = 0; i < 10; i++) {
            diskHead->userName[i] = '\0';
            diskHead->fileName[i] = '\0';
        }
        strcpy(diskHead->userName, UFD[currentUserID].userName);
        strcpy(diskHead->fileName, UFD[currentUserID].uDir[fileNum].fileName);
        diskHead->next = NULL;
        UFD[currentUserID].uDir[fileNum].fileStartPos = diskHead->startPos;
        UFD[currentUserID].uDir[fileNum].fileEndPos = diskHead->startPos + diskHead->endPos;

    } else {
        //如果不是第一个文件，则需要在磁盘上分配空间，并且将文件信息写入磁盘块
        //如果需求的磁盘块大小小于剩余磁盘大小，则分配磁盘块
        diskNode *temp = diskHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        if (UFD[currentUserID].uDir[fileNum].fileLength <= temp->blockSize) {
            temp->next = (diskNode *) malloc(sizeof(diskNode));
            temp->next->startPos = temp->endPos;
            temp->next->endPos = temp->endPos + UFD[currentUserID].uDir[fileNum].fileLength;
            temp->next->blockSize = temp->blockSize - UFD[currentUserID].uDir[fileNum].fileLength;
            temp = temp->next;
            for(int i = 0; i < 10; i++) {
                temp->userName[i] = '\0';
                temp->fileName[i] = '\0';
            }
            strcpy(temp->userName, UFD[currentUserID].userName);
            strcpy(temp->fileName, UFD[currentUserID].uDir[fileNum].fileName);
            temp->next = NULL;
            UFD[currentUserID].uDir[fileNum].fileStartPos = temp->startPos;
            UFD[currentUserID].uDir[fileNum].fileEndPos = temp->endPos;
        } else {
            printf("磁盘空间不足，请重新分配磁盘块\n");
        }
    }
}

//释放块，将磁盘块释放到磁盘上
void freeDiskBlock(int currentUserID, int fileNum) {
    diskNode *temp = diskHead;
    //寻找文件
    while (strcmp(temp->userName, UFD[currentUserID].userName) != 0 || strcmp(temp->fileName, UFD[currentUserID].uDir[fileNum].fileName) != 0) {
        temp = temp->next;
    }
    if (temp->startPos == 0) {
        diskHead = diskHead->next;
        diskNode *temp2 = diskHead;
        int size = temp->endPos - temp->startPos;
        //空间增加
        while (temp2->next != NULL) {
            temp2->blockSize = temp2->blockSize + size;
            temp2->startPos = temp2->startPos - size;
            temp2->endPos = temp2->endPos - size;
            temp2 = temp2->next;
        }
        temp->next = NULL;
        free(temp);
    } //如果是最后一个
    else if (temp->next == NULL) {
        diskNode *temp2 = diskHead;
        int size = temp->endPos - temp->startPos;
        while (temp2->next != temp) {
            temp2->blockSize = temp2->blockSize + size;
            temp2 = temp2->next;
        }
        temp2->next = NULL;
        free(temp->next);
        temp->next = NULL;
    } else {
        diskNode *temp2 = temp->next;
        diskNode *temp3 = temp->next;
        diskNode *temp4 = diskHead;
        //寻找前一个磁盘块
        while (temp4->next != temp) {
            temp4 = temp4->next;
        }
        int size = temp->endPos - temp->startPos;
        //计算节点数量
        int count = 0;
        while (temp3 != NULL) {
            count++;
            temp3 = temp3->next;
        }
        //空间增加
        while (count > 0) {
            temp2->blockSize = temp2->blockSize + size;
            temp2->startPos = temp2->startPos - size;
            temp2->endPos = temp2->endPos - size;
            temp2 = temp2->next;
            count--;
        }
        //链接节点
        temp4->next = temp->next;
        temp->next = NULL;
        free(temp);
    }
}


//初始化主目录和用户目录的展示例子: 俩个账户root和alex，每个账户下面都有2个文件。
void initData()
{
    //root账户
    strcpy(UFD[0].userName, "root");
    strcpy(UFD[0].password, "1");

    //a和b2个txt文件
    strcpy(UFD[0].uDir[0].fileName, "a.txt");
    UFD[0].uDir[0].flag = 1;
    for(int i=0; i < 3; i++)
    {
        UFD[1].uDir[1].fileProtectCode[i] = 1;
    }
    strcpy(UFD[0].uDir[0].fileCreateDate, "2022/07/01");
    UFD[0].uDir[0].fileLength = 10;
    //将文件a.txt分配到磁盘上
    allocateDiskBlock(0, 0);

    strcpy(UFD[0].uDir[1].fileName, "b.txt");
    UFD[0].uDir[1].flag = 1;
    for(int i=0; i < 3; i++)
    {
        UFD[1].uDir[1].fileProtectCode[i] = 1;
    }
    strcpy(UFD[0].uDir[1].fileCreateDate, "2022/07/01");
    UFD[0].uDir[1].fileLength = 20;
    //将文件b.txt分配到磁盘上
    allocateDiskBlock(0, 1);


    //alex账户
    strcpy(UFD[1].userName, "alex");
    strcpy(UFD[1].password, "111");
    //c和d2个txt文件
    strcpy(UFD[1].uDir[0].fileName, "c.txt");
    UFD[1].uDir[0].flag = 1;
    for(int i=0; i < 3; i++)
    {
        UFD[1].uDir[1].fileProtectCode[i] = 0;
    }
    strcpy(UFD[1].uDir[0].fileCreateDate, "2022/07/02");
    UFD[1].uDir[0].fileLength = 30;
    //将文件c.txt分配到磁盘上
    allocateDiskBlock(1, 0);

    strcpy(UFD[1].uDir[1].fileName, "d.txt");
    UFD[1].uDir[1].flag = 1;
    for(int i=0; i < 3; i++)
    {
        //随机生成0-1的三个字符串，作为文件的保护码
        UFD[1].uDir[1].fileProtectCode[i] = 0;
    }
    strcpy(UFD[1].uDir[1].fileCreateDate, "2022/07/02");
    UFD[1].uDir[1].fileLength = 40;
    //将文件d.txt分配到磁盘上
    allocateDiskBlock(1, 1);
}

//初始化打开文件目录
void initAFD()
{
    for(int i=0;i<5;i++)
    {
        strcpy(AFD[i].openFileName, "XXX");
        AFD[i].flag=0;
        for(int k=0;k<3;k++)
        {
            AFD[k].openFileProtectCode[k]=0;
        }
        AFD[i].rwPoint=0;
    }
}

//显示磁盘块链表信息
void showDiskBlock()
{
    //计算链表长度
    int length = 1;
    diskNode *temp = diskHead;
    while (temp->next != NULL) {
        length++;
        temp = temp->next;
    }
    diskNode *temp1 = diskHead;
    int index = 1;
    printf("磁盘块链表信息:\n");
    printf("--------------------------------------------------------------------------\n");
    printf("序号      用户名      文件名      文件大小      文件开始位置      文件结束位置\n");
    while (index <= length) {
        printf(" %d\t %s\t    %s\t  %d\t       %d\t       %d\t\n", index, temp1->userName, temp1->fileName,temp1->endPos - temp1->startPos, temp1->startPos, temp1->endPos);
        temp1 = temp1->next;
        index++;
    }
    printf("--------------------------------------------------------------------------\n\n");
}

//展示用户拥有的所有文件
void showUserFile(int userNum)
{
    printf("--------------------------------------------------------------------------\n\n");
    printf("用户%s的文件列表:\n", UFD[userNum].userName);
    for(int i=0;i<10;i++)
    {
        if(UFD[userNum].uDir[i].flag == 1)
        {
            printf("%s\n", UFD[userNum].uDir[i].fileName);
        }
    }
    printf("--------------------------------------------------------------------------\n\n");
}

//显示所有文件的详细信息
void showAllFileInfo(int currentUserID)
{
    printf("该用户所有文件的详细信息:\n");
    printf("--------------------------------------------------------------------------\n");
    printf("文件名      创建时间     文件大小  读权限   写权限   执行权限 文件起始位置 文件结束位置\n");
    for(int i=0;i<10;i++)
    {
        if(UFD[currentUserID].uDir[i].flag == 1)
        {
            printf("%s\t %s\t %d\t %d\t %d\t %d\t   %d\t     %d\t\n", UFD[currentUserID].uDir[i].fileName, UFD[currentUserID].uDir[i].fileCreateDate, UFD[currentUserID].uDir[i].fileLength, UFD[currentUserID].uDir[i].fileProtectCode[0], UFD[currentUserID].uDir[i].fileProtectCode[1], UFD[currentUserID].uDir[i].fileProtectCode[2], UFD[currentUserID].uDir[i].fileStartPos, UFD[currentUserID].uDir[i].fileEndPos);
        }
    }
    printf("--------------------------------------------------------------------------\n\n");
}

//显示已经打开的文件的详细信息
void showOpenFileInfo()
{
    printf("该用户所有已经打开的文件的详细信息:\n");
    printf("--------------------------------------------------------------------------\n");
    printf("文件名   读权限    写权限   执行权限 \n");
    for(int i=0;i<5;i++)
    {
        if(AFD[i].flag == 1)
        {
            printf("%s\t %d\t %d\t %d\t \n",AFD[i].openFileName, AFD[i].openFileProtectCode[0], AFD[i].openFileProtectCode[1], AFD[i].openFileProtectCode[2]);
        }
    }
    printf("--------------------------------------------------------------------------\n\n");
}

//显示所有指令
void showAllCommand()
{
    printf("---------------------------------可用指令如下---------------------------------\n");
    printf("1. 新建文件 -- create\n");
    printf("2. 删除文件 -- delete\n");
    printf("3. 打开文件 -- open\n");
    printf("4. 关闭文件 -- close\n");
    printf("5. 文件读写 -- read/write\n");
    printf("6. 文件信息 -- info\n");
    printf("7. 打开文件信息 -- oinfo\n");
    printf("8. 文件列表 -- ls\n");
    printf("9. 磁盘信息 -- disk\n");
    printf("10. 退出系统 -- exit\n");
    printf("--------------------------------------------------------------------------\n\n");
}

//登录到系统
int login()
{   while(1)
    {
        int i;
        char userName[10];
        char password[20];
        printf("-----------------------------简单文件系统登录页面-----------------------------\n");
        printf("请输入用户名：");
        scanf("%s",userName);
        printf("请输入密码：");
        scanf("%s",password);
        for(i=0;i<10;i++)
        {
            if(strcmp(userName,UFD[i].userName)==0 && strcmp(password,UFD[i].password)==0)
            {
                printf("登录成功！\n");
                printf("--------------------------------------------------------------------------\n\n");
                return i;
            }
        }
        printf("用户名或密码错误，登录失败！\n");
        exit(0);
    }
}

//创建文件
void createFile(int currentUserID)
{
    int i;
    char fileName[10];
    char fileCreateDate[10];
    int fileLength = 0;

    //初始化char
    for(i=0;i<10;i++)
    {
        fileName[i] = '\0';
        fileCreateDate[i] = '\0';
    }

    LOOP:while(1)
    {
        printf("请输入文件名: ");
        scanf("%s", fileName);
        for (i = 0; i < 10; i++) {
            if (strcmp(fileName, UFD[currentUserID].uDir[i].fileName) == 0) {
                printf("文件已存在，请重新输入文件名！\n");
                goto LOOP;
            }
        }
        for (i = 0; i < 10; i++) {
            if (strcmp(UFD[currentUserID].uDir[i].fileName, "") == 0) {
                strcpy(UFD[currentUserID].uDir[i].fileName, fileName);
                UFD[currentUserID].uDir[i].flag = 1;
                for(int k=0; k < 3; k++)
                {
                    //默认权限为1
                    UFD[currentUserID].uDir[i].fileProtectCode[k] = 1;
                }

                UFD[currentUserID].uDir[i].fileLength = fileLength;
                printf("请输入文件创建时间（例如：2022/07/02）: ");
                scanf("%s", fileCreateDate);
                strcpy(UFD[currentUserID].uDir[i].fileCreateDate, fileCreateDate);
                printf("请输入文件大小（KB）:\n");
                scanf_s("%d", &fileLength);
                UFD[currentUserID].uDir[i].fileLength = fileLength;
                //分配到磁盘上
                allocateDiskBlock(currentUserID, i);
                printf("文件创建成功！\n");
                break;
            }
        }
        break;
    }
}

//删除文件
void deleteFile(int currentUserID)
{
    char fileName[10];
    //初始化char
    for(int i=0;i<10;i++)
    {
        fileName[i] = '\0';
    }
    LOOP:while(1)
    {
        printf("请输入文件名：");
        scanf("%s", fileName);
        //如果文件正在运行则不能删除
        for(int i=0;i<5;i++)
        {
            if(strcmp(fileName, AFD[i].openFileName)==0)
            {
                printf("文件正在运行，不能删除！\n");
                goto LOOP;
            }
        }
        for (int i = 0; i < 10; i++) {
            if (strcmp(fileName, UFD[currentUserID].uDir[i].fileName) == 0) {
                freeDiskBlock(currentUserID,i);
                //将该用户名下的文件从后面移到前面
                for (int j = i; j < 9; j++) {
                    for(int k = 0; k < 10; k++)
                    {
                        UFD[currentUserID].uDir[j].fileName[k] = UFD[currentUserID].uDir[j+1].fileName[k];
                    }
                    UFD[currentUserID].uDir[j].flag = UFD[currentUserID].uDir[j + 1].flag;
                    for(int k=0; k < 3; k++)
                    {
                        UFD[currentUserID].uDir[j].fileProtectCode[k] = UFD[currentUserID].uDir[j + 1].fileProtectCode[k];
                    }
                    UFD[currentUserID].uDir[j].fileLength = UFD[currentUserID].uDir[j + 1].fileLength;
                    for(int k=0; k < 20; k++)
                    {
                        UFD[currentUserID].uDir[j].fileCreateDate[k] = UFD[currentUserID].uDir[j + 1].fileCreateDate[k];
                    }
                    //起始位置不变，结束位置变化
                    UFD[currentUserID].uDir[j].fileEndPos = UFD[currentUserID].uDir[j].fileStartPos + UFD[currentUserID].uDir[j].fileLength;
                }
                printf("文件删除成功！\n");
                break;
            } else if(i == 9) {
                printf("文件不存在，删除失败！\n");
            }
        }
        break;
    }
}

//打开文件
void openFile(int currentUserID)
{
    int i;
    char fileName[10];
    LOOP:while(1)
    {
        printf("请输入文件名：");
        scanf("%s", fileName);
        //文件是否已经打开
        for(i=0;i<5;i++)
        {
            if(strcmp(fileName, AFD[i].openFileName)==0)
            {
                printf("文件已经打开！\n");
                goto LOOP;
            }
        }
        //检查文件是否存在
        for (i = 0; i < 10; i++) {
            //检查权限是否允许读取文件
            if (strcmp(fileName, UFD[currentUserID].uDir[i].fileName) == 0 && UFD[currentUserID].uDir[i].fileProtectCode[2] == 0) {
                printf("没有权限执行该文件！\n");
                break;
            } else if (strcmp(fileName, UFD[currentUserID].uDir[i].fileName) == 0 && UFD[currentUserID].uDir[i].fileProtectCode[2] == 1) {
                //将文件添加到运行文件列表中
                for (int j = 0; j < 5; j++) {
                    if (AFD[j].flag == 0) {
                        for (int k = 0; k < 10; k++) {
                            AFD[j].openFileName[k] = UFD[currentUserID].uDir[i].fileName[k];
                        }
                        AFD[j].flag = 1;
                        for(int k=0; k < 3; k++)
                        {
                            AFD[j].openFileProtectCode[k] = UFD[currentUserID].uDir[i].fileProtectCode[k];
                        }
                        break;
                    } else if (j == 4) {
                        printf("运行文件列表已满，请关闭一个文件后再打开新文件！\n");
                        break;
                    }
                }
                printf("文件打开成功！\n");
                break;
            } else if(i == 9)
            {
                printf("文件不存在！\n");
                break;
            }
        }
        break;
    }
}

//关闭文件
void closeFile()
{
    char fileName[10];
    while(1)
    {
        printf("请输入文件名：");
        scanf("%s", fileName);
        //将文件从运行文件列表中删除
        for (int i = 0; i < 5; i++) {
            if (strcmp(AFD[i].openFileName, fileName) == 0) {
                strcpy(AFD[i].openFileName, "XXX");
                AFD[i].flag = 0;
                for(int k=0;k<3;k++)
                {
                    AFD[i].openFileProtectCode[k] = 0;
                }
                AFD[i].rwPoint = 0;
                printf("文件关闭成功！\n");
                break;
            } else if (i == 4) {
                printf("文件不在运行文件列表中，请检查！\n");
                break;
            }
        }
        break;
    }
}

//读文件
void readFile()
{
    int i;
    char fileName[10];
    while(1)
    {
        printf("请输入文件名：");
        scanf("%s", fileName);
        //从运行文件列表中查找文件
        for (i = 0; i < 5; i++) {
            if (strcmp(fileName, AFD[i].openFileName) == 0) {
                //检查权限是否允许读取文件
                if (AFD[i].openFileProtectCode[2] == 0) {
                    printf("没有权限执行该文件！\n");
                    break;
                } else if (AFD[i].openFileProtectCode[2] == 1) {
                    AFD[i].rwPoint = 1;
                    printf("文件的读写标志位已置%d！\n", AFD[i].rwPoint);
                    printf("文件读出成功！\n");
                    AFD[i].rwPoint = 0;
                    printf("文件的读写标志位已置%d！\n", AFD[i].rwPoint);
                    break;
                }
            } else if (i == 4) {
                printf("文件不在运行文件列表中，请检查！\n");
                break;
            }
        }
        break;
    }
}

//写文件
void writeFile()
{
    int i;
    char fileName[10];
    while(1)
    {
        printf("请输入文件名：");
        scanf("%s", fileName);
        //从运行文件列表中查找文件
        for (i = 0; i < 5; i++) {
            if (strcmp(fileName, AFD[i].openFileName) == 0) {
                //检查权限是否允许读取文件
                if (AFD[i].openFileProtectCode[1] == 0) {
                    printf("没有权限执行该文件！\n");
                    break;
                } else if (AFD[i].openFileProtectCode[1] == 1) {
                    AFD[i].rwPoint = 1;
                    printf("文件的读写标志位已置%d！\n", AFD[i].rwPoint);
                    printf("文件写入成功！\n");
                    AFD[i].rwPoint = 0;
                    printf("文件的读写标志位已置%d！\n", AFD[i].rwPoint);
                    break;
                }
            } else if (i == 4) {
                printf("文件不在运行文件列表中，请检查！\n");
                break;
            }
        }
        break;
    }
}

//主流程循环
int main() {
    system("chcp 65001");
    initDisk();
    initAFD();
    initData();
    int currentUserID = login();
    printf("------------------------------%s用户的文件系统-----------------------------\n", UFD[currentUserID].userName);
    printf("友情提示：输入help可以查看可用指令哦!\n");
    while (1) {
        char command[10];
        printf("%s@simpleFileSys:~$", UFD[currentUserID].userName);
        scanf(" %s", command);
        if (strcmp(command, "create") == 0) {
            createFile(currentUserID);
        } else if (strcmp(command, "delete") == 0) {
            deleteFile(currentUserID);
        } else if (strcmp(command, "open") == 0) {
            openFile(currentUserID);
        } else if (strcmp(command, "close") == 0) {
            closeFile();
        } else if (strcmp(command, "read") == 0) {
            readFile();
        } else if (strcmp(command, "write") == 0) {
            writeFile();
        } else if(strcmp(command, "ls") == 0) {
            showUserFile(currentUserID);
        } else if(strcmp(command,"help") == 0) {
            showAllCommand();
        } else if(strcmp(command, "info") == 0) {
            showAllFileInfo(currentUserID);
        } else if(strcmp(command, "oinfo") == 0) {
            showOpenFileInfo();
        } else if(strcmp(command, "disk") == 0) {
            showDiskBlock();
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("\n命令错误,输入help可以查看命令提示！\n\n");
        }
    }
}









