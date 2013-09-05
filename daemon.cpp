/*  	
**	 HiStor Raid Manager Software on Linux 2.6.11 Platform!
**	 All CopyRight reserved by HiStor Comp.Ltd., 2007-2008 Wuhan China
*/

#include <sys/param.h>
#include <time.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unistd.h>   /* nice */

using namespace std;

#include "disk.h"
#include "diskArray.h"
#include "singleRaid.h"
#include "raidConfig.h"
#include "global.h"
//#include "flash_scull.h"


//绑定设备的宏:
#define bind_dev(index, channel_no)  int rtval;\
	string cmdbind;\
	cmdbind="echo \"binding HiStorRAID"+index+" "+channel_no+"\"> /proc/scsi_tgt/vdisk/vdisk";\
	rtval=system(cmdbind.c_str());\
	if(rtval<0){\
	cout<<"ERROR:execute"<<endl;\
	exit(-1);\
	}\
	sleep(1)//绑定设备

//解除绑定设备的宏:
#define unbind_dev(index, channel_no)  int rtval;\
	string cmdunbind;\
	cmdunbind="echo \"unbinding HiStorRAID"+index+" "+channel_no+"\"> /proc/scsi_tgt/vdisk/vdisk";\
	rtval=system(cmdunbind.c_str());\
	if(rtval<0){\
	cout<<"ERROR:execute"<<endl;\
	exit(-1);\
	}\
	sleep(1)//绑定设备



/***************************************************************
* 功能: 截取字符串
* 输入: IP-IP-或者channelno-channelno-
* 输出: string数组
* 作者: zjf
* 日期: 2007-12-6
****************************************************************/
void separat_string(string src, string *result){
	
	int rtval;
	int i=0,p=0;
	while(p!=src.size()){
		result[i]=src.substr(p,src.find("-",p)-p);
		p=src.find("-",p)+1;
		i++;
	}
	
}

void init_daemon(void)
{
	int pid;
	int i;
	if(pid = fork())
		exit(0);
	else if(pid < 0)
		exit(1);
	setsid();
	if(pid = fork())
		exit(0);
	else if(pid < 0) 
		exit(1);
	for(i = 0;i < NOFILE;++i)
		close(i);
	chdir("/");
	umask(0);
	return;
}



void initCheck(RaidConfig& rc, DiskArray& da)
{
	chdir(workingDir);
	//检测已创建阵列中的磁盘是否存在
	for(int j = 0;j < rc.singleRaidNum;j++)
	{
		//检测阵列中的数据盘
		for(int k = 0;k < rc.singleRaids[j].raidDiskNum; k++)
		{
			if(strcmp(rc.singleRaids[j].raidDisks[k].status, "2"))		//当前盘不为坏盘
			{
				int currentflag = 1;
				//磁盘存在，则置currentflag为0
				for(int i = 0;i < da.diskNum;i++)
				{
					if(rc.singleRaids[j].raidDisks[k] == da.array[i])
					{
						currentflag = 0;
						break;
					}
				}
				//阵列中的数据盘盘丢失，则热移除磁盘
				if(currentflag)
				{
					strcpy(rc.singleRaids[j].raidDisks[k].status, "2");
						//rc.singleRaids[j].removeDiskInSingleRaid(rc.singleRaids[j].raidDisks[k]);
						//k--;
				}
			}	//if(strcmp(rc.singleRaids[j].raidDisks[k].status, "2"))
		}	//for(int k = 0;k < rc.singleRaids[j].raidDiskNum; k++)
			
		//检测阵列的热备盘
		for(int k = 0;k < rc.singleRaids[j].spareDiskNum;k++)
		{
			if(strcmp(rc.singleRaids[j].spareDisks[k].status, "2"))		//当前盘不为坏盘
			{
				int currentflag = 1;
				for(int i = 0;i < da.diskNum;i++)
				{
					if(rc.singleRaids[j].spareDisks[k] == da.array[i]){
						currentflag = 0;
						break;
					}
				}
				//局部热备盘丢失，则移除磁盘
				if(currentflag)
				{
					strcpy(rc.singleRaids[j].spareDisks[k].status, "2");
					//rc.singleRaids[j].removeDiskInSingleRaid(rc.singleRaids[j].spareDisks[k]);
					//k--;
				}
			}	//if(strcmp(rc.singleRaids[j].spareDisks[k].status, "2"))
		}
	}


		
	//cout<<rc.diskNum<<endl;
	//cout<<da.diskNum<<endl;

	//检测xml中非阵列里面的磁盘是否仍然存在

	for(int j = 0;j < rc.diskNum; j++)
	{
		if(strcmp(rc.disks[j].status, "2"))	//当前磁盘不为坏盘
		{
			int currentflag = 1;
			for(int i = 0;i < da.diskNum;i++)
			{
				if(rc.disks[j] == da.array[i])
				{
					currentflag =0;
					break;
				}
			}
			//磁盘丢失，移除磁盘
			if(currentflag)
			{
				//cout<<"lost a disk"<<endl;
				strcpy(rc.disks[j].status, "2");
				//rc.removeDisk(rc.disks[j]);
				//j--;
			}
		}	//if(strcmp(rc.disks[j].status, "2"))
	}
		
	//检测磁盘漫游
	//cout<<"check start"<<endl;
	for(int i = 0;i < da.diskNum;i++)
	{
		//cout<<"diskarray    "<<da.array[i].devName<<"    "<<da.array[i].scsiID<<"    "<<da.array[i].sn<<endl;
		//检测阵列中磁盘是否发生漫游
		for(int j = 0;j < rc.singleRaidNum;j++)
		{
			//检测阵列中的数据盘是否发生漫游
			for(int k = 0;k < rc.singleRaids[j].raidDiskNum;k++)
			{
				//发生了磁盘漫游，则更新相应的磁盘信息
				if(da.array[i] == rc.singleRaids[j].raidDisks[k])	
				{
					if((strcmp(da.array[i].devName, rc.singleRaids[j].raidDisks[k].devName) != 0)
						|| (strcmp(da.array[i].scsiID, rc.singleRaids[j].raidDisks[k].scsiID) != 0))
					{
						strcpy(rc.singleRaids[j].raidDisks[k].devName, da.array[i].devName);
						strcpy(rc.singleRaids[j].raidDisks[k].scsiID, da.array[i].scsiID);
					}
				}
			}

			//检测阵列中的局部热备盘是否发生漫游
			for(int k = 0;k < rc.singleRaids[j].spareDiskNum;k++)
			{
				if(rc.singleRaids[j].spareDisks[k] == da.array[i])
				{
					if((strcmp(da.array[i].devName, rc.singleRaids[j].spareDisks[k].devName) != 0)
						|| (strcmp(da.array[i].scsiID, rc.singleRaids[j].spareDisks[k].scsiID) != 0))
					{
						strcpy(rc.singleRaids[j].spareDisks[k].devName, da.array[i].devName);
						strcpy(rc.singleRaids[j].spareDisks[k].scsiID, da.array[i].scsiID);
					}
				}
			}	//for(int k = 0;k < rc.singleRaids[j].spareDiskNum;k++)
		}	//for(int j = 0;j < rc.singleRaidNum;j++)		
			
		//检查非阵列中的磁盘是否发生漫游
		for(int j = 0;j < rc.diskNum;j++)
		{
			//cout<<"raidconfig    "<<da.array[j].devName<<"    "<<da.array[j].scsiID<<"    "<<da.array[j].sn<<endl;
			if(da.array[i] == rc.disks[j])
			{
				if((strcmp(da.array[i].devName, rc.disks[j].devName) != 0)
					|| (strcmp(da.array[i].scsiID, rc.disks[j].scsiID) != 0))
				{
					//cout<<"-------------information updated"<<endl;
					strcpy(rc.disks[j].devName, da.array[i].devName);
					strcpy(rc.disks[j].scsiID, da.array[i].scsiID);
				}
				else
				{
				//cout<<"--------------------------"<<endl;
				}
				break;
			}
		}
	}	//for(int i = 0;i < da.diskNum;i++)
	//cout<<"check end"<<endl;	
}

/************************************************************************
 *作者：Ivan.Su
 *功能：将整数转换成为字符串
 *参数：i－待转换整数
 *返回值：整数i所对应的字符串
 ************************************************************************/
string intToString(int i)
{ 
	stringstream s; 
	s << i; 
	return s.str(); 
}


/***********************************************************************
 *作者：Ivan.Su
 *功能：获得写日志的日期和时间
 *参数：无
 *返回值：写日志的日期和时间字符串
 ***********************************************************************/
string getLogDateAndTime()
{
	time_t t;
	struct tm *p;
	string logDateAndTime;
	string logYear,logMon,logMDay;
	string logHour,logMin,logSec;

	time(&t);
	p = localtime(&t);

	//年
	logYear = intToString(1900 + p->tm_year);
	//月
	logMon = intToString(1 + p->tm_mon);
	//日
	logMDay = intToString(p->tm_mday);

	logDateAndTime = logYear + "-" + logMon + "-" + logMDay + "|";

	//时
	logHour = intToString(p->tm_hour);
	//分
	logMin = intToString(p->tm_min);
	//秒
	logSec = intToString(p->tm_sec);

	logDateAndTime = logDateAndTime + logHour + ":" + logMin + ":" + logSec + "|";
	return logDateAndTime;
}


/***********************************************************************
 *作者：Ivan.Su
 *功能：写阵列日志
 *参数：logInfor－待写入阵列日志文件的一条日志
 *返回值：无
 ***********************************************************************/
void writeLog(string logInfor)
{
	int logLines =0;
	ifstream fpLog1;
	string buffer;

	chdir(workingDir);

	if(access("log",0))	//如果log文件不存在,创建log
	{
		ofstream fp("log");
		fp.close();
	}

	system("mv -f log log1"); //将log重命名为log1	
	ofstream fpLog("log");	//生成新的log文件
	fpLog1.open("log1", ios::in);	

	fpLog<< logInfor;	//写日志
	logLines ++;

	while(!fpLog1.eof())	 //将旧的日志添加到log文件中
	{
		getline(fpLog1, buffer);
		fpLog<<endl<<buffer;
		logLines ++;
		if(logLines >= 200)
		{
			break;			
		}
	}	//while(!fpLog1.eof())
	fpLog.close();
	fpLog1.close();
	system("rm -f log1");

	//write the LOG to pci
#ifdef READ_WRITE_PCI
	write_to_pci(LOG_DIR_WRITE, RAID_MON_LOG);
#endif
}



/***********************************************************************************************
 *作者：Ivan.Su
 *功能：监控磁盘，检测是否有掉盘和加盘，并处理热备盘
 *参数：da—磁盘数组，保存当前磁盘信息；rc—保存当前整个阵列的信息
 *返回值：无
 ***********************************************************************************************/

void DiskMonitor(DiskArray& da,RaidConfig& rc)
{
	chdir(workingDir);
	bool confIsChanged =0;

	DiskArray da1;
	da1.fillArray();
	da = da1;
	//da.refreshDiskArray();	//更新磁盘数组

	//检测是否有掉盘或加盘

	//检测阵列是否有掉盘
	for(int i = 0; i < rc.singleRaidNum;i++)
	{
		for(int j = 0; j < rc.singleRaids[i].raidDiskNum; j++)	 //对阵列中的每个数据盘进行检测
		{
			if (strcmp(rc.singleRaids[i].raidDisks[j].status, "2"))	 //如果当前盘不为坏盘
			{
				bool bLostSign = 1;

				for(int k = 0; k < da.diskNum; k++)	//在da中查找磁盘是否存在
				{
					if(da.array[k] == rc.singleRaids[i].raidDisks[j])
					{
						bLostSign = 0;
						break;
					}
				}	//for(int k = 0; k < da.diskNum; k++)
					
				if(bLostSign)	//磁盘丢失
				{
					strcpy(rc.singleRaids[i].raidDisks[j].status,"2");

					confIsChanged = 1;

					string logInfor = getLogDateAndTime() + "DISKFAULTY|有磁盘损坏或拔除,序号为:  "+ rc.singleRaids[i].raidDisks[j].scsiID;
					writeLog(logInfor);

					//特殊情况
					if(rc.singleRaids[i].raidstat == 50 && !strcmp(rc.singleRaids[i].level, "5") )
					{
						rc.singleRaids[i].raidstat = 40;
						confIsChanged = 1;
						string logInfor;
						logInfor = getLogDateAndTime();
						logInfor += "DISKFAULTY|阵列";
						logInfor += rc.singleRaids[i].index; 
						logInfor += "已经损坏，不能正常使用，请删除该阵列";
						writeLog(logInfor);

					}
					
				}	//if(bLostSign)
			}	//if(strcmp(rc.singleRaids[i].raidDisks[j].status, "2"))
		}	//for(int j = 0; j < rc.singleRaids[i].raidDiskNum;j++)

		for(int j = 0; j < rc.singleRaids[i].spareDiskNum;j++)	 //对阵列中的每个热备盘进行检测
		{
			if (strcmp(rc.singleRaids[i].spareDisks[j].status, "2"))	 //如果当前盘不为坏盘
			{
				bool bLostSign = 1;

				for(int k = 0; k < da.diskNum; k++)	//在da中查找磁盘是否存在
				{
					if(da.array[k] == rc.singleRaids[i].spareDisks[j])
					{
						bLostSign = 0;
						break;
					}
				}	//for(int k = 0; k < da.diskNum; k++)
				
				if(bLostSign)	//磁盘丢失
				{
					strcpy(rc.singleRaids[i].spareDisks[j].status,"2");
					confIsChanged = 1;
					string logInfor = getLogDateAndTime() + "DISKFAULTY|有磁盘损坏或拔除,序号为:  "+rc.singleRaids[i].spareDisks[j].scsiID;
					writeLog(logInfor);
				}	//if(bLostSign)
			}	//if (strcmp(rc.singleRaids[i].spareDisks[j].status, "2"))
		}	//if (rc.singleRaids[i].spareDisks[j].status != 2)
	}	//for(int i = 0; i < rc.singleRaidNum;i++)

	//检测阵列外的磁盘是否掉盘
	for(int i = 0; i < rc.diskNum; i++)
	{
		if(strcmp(rc.disks[i].status, "2"))
		{
			bool bLostSign =1;
			for(int j = 0; j < da.diskNum;j++)
			{

				if(da.array[j] == rc.disks[i])
				{
					bLostSign = 0;
					break;
				}
			}	//for(int j = 0; j < da.diskNum;j++)

			if(bLostSign)
			{
				//cout<<"lost a disk"<<endl;

				strcpy(rc.disks[i].status, "2");
				confIsChanged = 1;
				string logInfor = getLogDateAndTime() + "DISKFAULTY|有磁盘损坏或拔除,序号为:  "+rc.disks[i].scsiID;
				writeLog(logInfor);
			}
		}	//if(strcmp(rc.disks[i].status, "2")
	}	//for(int i = 0; i < rc.diskNum; i++)

	//检测是否有新加盘
	for(int i = 0; i < da.diskNum; i++)
	{
		//cout<<da.array[i].devName<<"  "<<da.array[i].scsiID<<"   "<<da.array[i].sn<<endl;
		bool isNewDisk =1;

		//检查磁盘在阵列中是否存在
		for(int j = 0; j < rc.singleRaidNum; j++)
		{
			for(int k = 0;k < rc.singleRaids[j].raidDiskNum;k++)
			{
				if(!strcmp(da.array[i].sn, rc.singleRaids[j].raidDisks[k].sn))	//当前磁盘在singleRaid中存在
				{ 
					if(!strcmp(rc.singleRaids[j].raidDisks[k].status,"2"))	 //磁盘在阵列中已经标记为损坏
					{
						rc.singleRaids[j].hotRemoveDisk(rc.singleRaids[j].raidDisks[k]);
						k--;
					}
					else
					{
						isNewDisk = 0;
						break;
					}
				}	//if(da.array[i] == rc.singleRaids[j].raidDisks[k])
				else if(!strcmp(da.array[i].scsiID, rc.singleRaids[j].raidDisks[k].scsiID))	//如果新添加磁盘的scsiID原来存在
				{
					rc.singleRaids[j].hotRemoveDisk(rc.singleRaids[j].raidDisks[k]);
					k--;
				}
			}	//for(int k = 0;k < rc.singleRaids[j].raidDiskNum;k++)

			if(!isNewDisk)	 //如果不是新加盘，则退出循环
			{
				break;
			}

			for(int k = 0; k < rc.singleRaids[j].spareDiskNum; k++)
			{
				if(da.array[i] == rc.singleRaids[j].spareDisks[k])	//当前磁盘在singleRaid中存在
				{
					if(!strcmp(rc.singleRaids[j].spareDisks[k].status,"2"))	 //磁盘在阵列中已经标记为损坏
					{
						rc.singleRaids[j].hotRemoveDisk(rc.singleRaids[j].spareDisks[k]);
						k--;
					}
					else
					{
						isNewDisk = 0;	
						break;
					}
				}	//if(da.array[i] == rc.singleRaids[j].raidDisks[k])
				else if(!strcmp(da.array[i].scsiID, rc.singleRaids[j].spareDisks[k].scsiID))	//如果新添加磁盘的scsiID原来存在
				{
					rc.singleRaids[j].hotRemoveDisk(rc.singleRaids[j].spareDisks[k]);
					k--;
				}
			}

			if(!isNewDisk)
			{
				break;
			}

		}	//for(int j = 0;j < rc.singleRaidNum;j++)

		if (!isNewDisk)	
		{
			continue;
		}

		for(int j = 0 ; j < rc.diskNum; j++)
		{
			//cout<<"-----"<<rc.disks[j].devName<<"  "<<rc.disks[j].scsiID<<"   "<<rc.disks[j].sn<<"   "<<rc.disks[j].status<<endl;
			if(!strcmp(da.array[i].sn, rc.disks[j].sn))	//磁盘在阵列外存在
			{
				if(!strcmp(rc.disks[j].status,"2"))	//磁盘已经被置为坏盘
				{
					//cout<<"remove bad disk which has the same sn with new disk"<<endl;
					rc.removeDisk(rc.disks[j]);
					j--;
				}
				else
				{ 
					isNewDisk = 0;
					break;
				}
			}
			else if(!strcmp(da.array[i].scsiID, rc.disks[j].scsiID))	//新添加磁盘的scsiID在rc中被找到
			{
				//cout<<"remove bad disk which has the same scsiID with new disk"<<endl;
				rc.removeDisk(rc.disks[j]);
				j--;
			}
		}	//for(int j = 0 ; j < rc.diskNum; j++)
                           		

		if(isNewDisk)
		{ 
                	confIsChanged = 1;
			rc.addDisk(da.array[i]);
			string logInfor = getLogDateAndTime() + "DISKADD|有新的磁盘插入，序号为:  "+da.array[i].scsiID;
			writeLog(logInfor);
			//cout<<logInfor<<endl;
		}
	}	//for(int i = 0; i < da.diskNum; i++)


	//更新热备盘信息
	for(int i = 0; i<rc.singleRaidNum; i++)
	{
		if(rc.singleRaids[i].raidstat != 40)
		{
			rc.singleRaids[i].dealWithSingleRaidSpareDisk();
		}
	}

	if(!access("raidConfig.xml",0))
			system("rm -f raidConfig.xml");
	rc.saveRcToXml("raidConfig.xml");

	//write the raidConfig.xml to pci
	if(confIsChanged)
	{
	#ifdef READ_WRITE_PCI
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP);
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);
	#endif
	}
}


/***********************************************************************************************
 *作者：Ivan.Su
 *功能：阵列相关操作——添加/删除阵列
 *参数：rc—保存当前整个阵列的信息
 *返回值：无
 ***********************************************************************************************/
void raidConfig(RaidConfig& rc, DiskArray& da)
{
	chdir(workingDir);

	if((!access("raidConfigNew.xml",0)) && (!access("restore",0)))
	{
		system("rm -f restore");
		//int m=rc.singleRaidNum;

		//删除当前所有阵列
		for(int i =0; i < rc.singleRaidNum; i++)
		{
			rc.singleRaids[i].stop();		//停止阵列
			//usleep(100000);
			//rc.removeSingleRaid(rc.singleRaids[0]);
			
		}

		RaidConfig rc1;
		rc1.buildRcFromXml("raidConfigNew.xml");

		rc = rc1;
		initCheck(rc, da);

		for(int i = 0;i < rc.singleRaidNum; i++)
		{
			if(rc.singleRaids[i].raidstat != 40)
			{
				rc.singleRaids[i].create();
				//usleep(100000);
			}
		}

		system("rm -f raidConfigNew.xml");
		if(!access("raidConfig.xml",0))
			system("rm -f raidConfig.xml");
		rc.saveRcToXml("raidConfig.xml");

		//write the raidConfig.xml to pci
	#ifdef READ_WRITE_PCI
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP);
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);
	#endif
		
		return;
	}


	//cout<<"raidConfigNew.xml exist"<<endl;
	if(!access("raidConfigNew.xml",0))
	{
		RaidConfig rc1;
		rc1.buildRcFromXml("raidConfigNew.xml");

		//查找是否有新创建阵列
		for(int i = 0;i < rc1.singleRaidNum;i++)
		{
		    if(rc.getSingleRaidIndex(rc1.singleRaids[i]) == -1)
		    {
				rc1.singleRaids[i].create();			
				//usleep(100000);
				rc.addSingleRaid(rc1.singleRaids[i]);
		    }
		}
		//cout<<"search for  new raid end"<<endl;
		//查找是否有删除阵列
		for(int i = 0;i < rc.singleRaidNum; i++)
		{
			//cout<<"i = "<<i<<endl;
			if(rc1.getSingleRaidIndex(rc.singleRaids[i]) == -1)
			{
				rc.singleRaids[i].stop();
				//usleep(100000);
				string logInfor = getLogDateAndTime() + "DISKFAULTY|有阵列被删除，序号为："+ rc.singleRaids[i].index;		
				writeLog(logInfor);	//写日志
				rc.removeSingleRaid(rc.singleRaids[i]);
				i--;
				
			}
		}
		//cout<<"search for deleted raid end"<<endl;
		
		for(int i = 0;i < rc.singleRaidNum;i++)
		{
			int j = rc1.getSingleRaidIndex(rc.singleRaids[i]);
			if(j != -1)	//当前阵列没有被删除
			{
				//检测阵列中的数据盘是否被删除
				for(int k = 0;k < rc.singleRaids[i].raidDiskNum;k++)
				{
					if(rc1.singleRaids[j].getRaidDiskIndex(rc.singleRaids[i].raidDisks[k]) == -1)
					{
						rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].raidDisks[k]);
					/*

						if(!strcmp(rc.singleRaids[i].raidDisks[k].status, "2"))	 //如果删除的是已损坏的磁盘
						{
							rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].raidDisks[k]);
						}
						else	 //删除的是好盘
						{
							rc.singleRaids[i].hotRemoveDisk(rc.singleRaids[i].raidDisks[k]);
						}
					*/
						k--;
					}
				}

				//检测阵列中的热备盘是否被删除
				for(int k = 0;k < rc.singleRaids[i].spareDiskNum;k++)
				{
					if(rc1.singleRaids[j].getSpareDiskIndex(rc.singleRaids[i].spareDisks[k]) == -1)
					{
						rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].spareDisks[k]);
					/*
						if(!strcmp(rc.singleRaids[i].spareDisks[k].status, "2"))	//删除的是损坏的磁盘
						{
							rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].spareDisks[k]);
						}
						else	 //删除的是好盘
						{
							rc.singleRaids[i].hotRemoveDisk(rc.singleRaids[i].spareDisks[k]);
						}
					*/
						k--;  
					}
				}
/*
				for(int k =0; k < rc1.singleRaids[j].raidDiskNum; k++)
				{
					if(rc.singleRaids[i].getRaidDiskIndex(rc1.singleRaids[j].raidDisks[k]) == -1)	//添加了一个数据盘
					{
						int spareDiskIndex = rc.getDiskIndex(rc1.singleRaids[j].raidDisks[k]);
						strcpy(rc.disks[spareDiskIndex].isSpareDisk, "1");

						//rc.singleRaids[i].addSpareDisk(rc.disks[spareDiskIndex]);
						rc.removeDisk(rc1.singleRaids[j].raidDisks[k]);

						rc.singleRaids[i].hotAddDisk(rc1.singleRaids[j].raidDisks[k]);
					}
				}
*/
				//检测是否添加了热备盘
				for(int k = 0;k < rc1.singleRaids[j].spareDiskNum;k++)
				{
					if(rc.singleRaids[i].getSpareDiskIndex(rc1.singleRaids[j].spareDisks[k]) == -1)	//添加了热备盘
					{						
						int spareDiskIndex = rc.getDiskIndex(rc1.singleRaids[j].spareDisks[k]);
						//rc.singleRaids[i].addSpareDisk(rc.disks[spareDiskIndex]);	//
						rc.removeDisk(rc1.singleRaids[j].spareDisks[k]);
						rc.singleRaids[i].hotAddDisk(rc1.singleRaids[j].spareDisks[k]);						
					}
				}	//for(int k = 0;k < rc1.singleRaids[j].spareDiskNum;k++)

			}	//if(j != -1)
		}	//for(int i = 0;i < rc.singleRaidNum;i++)

		for(int i = 0;i < rc.diskNum;i++)
		{
			if(rc1.getDiskIndex(rc.disks[i]) == -1)
			{
				rc.removeDisk(rc.disks[i]);
				i--;
			}
		}

		for(int i = 0;i < rc1.diskNum;i++)
		{
			if(rc.getDiskIndex(rc1.disks[i]) == -1)
			{
				rc.addDisk(rc1.disks[i]);
			}
		}
		
		system("rm -f raidConfigNew.xml");
		if(!access("raidConfig.xml",0))
			system("rm -f raidConfig.xml");
		rc.saveRcToXml("raidConfig.xml");
		
		//write the raidConfig.xml to pci
	#ifdef READ_WRITE_PCI
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP);
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);
	#endif

	}	
}


/*******************************************************************
 *作者：Ivan.Su
 *功能：检测Raid10是否损坏
 *参数：stateString－从raidstat中读出的Raid10的状态字符串
 *返回值：1－损坏，阵列不可用；0－阵列仍可用
 *******************************************************************/
bool isRaid10Bad(string stateString)
{

	string oneChar="";
	for(int i=0;i<stateString.size();i++)
	{
		if(i%2==0)
		{
			oneChar = stateString.substr(i,1);
		}
		else
		{
			if(stateString.substr(i,1)=="_")
			{
				if(oneChar=="_")
					return 1;
			}

		}

	}
	return 0;
}


/******************************************************************
 *作者：Ivan.Su
 *功能：根据从raidstat中读出的阵列的状态字符串，获得阵列的状态
 *参数：raidInfor－raidstat中阵列信息的第二行，raidLevel－阵列的级别
 *返回值：10－正常；40－损坏；50－降级
 ******************************************************************/
int getRaidStat(string raidInfor, string raidLevel)
{
	int stat = -1;	//当前阵列的状态
	int iRaidLevel = atoi(raidLevel.c_str());	//当前阵列的级别
	int lastCharacter = raidInfor.size();	//存放当前字符串的长度

	int U_pos = raidInfor.rfind("[",lastCharacter)+1; //存放U字符串的起始位置
	string raidStateString = raidInfor.substr(U_pos, lastCharacter - U_pos - 1);	//获得整个U字符串
	//cout<<raidStateString<<endl;

	int iUNum=0;					
	int i_Num=0;
						
	//获得字符串中U，_的个数，为获取raidstat做准备
	for(int i=0;i<raidStateString.size();i++)
	{	
		if(raidStateString.substr(i,1)=="U")
			iUNum++;
		else 
			i_Num++;
	}

	switch(iRaidLevel)
	{
	//阵列为RAID1,
	case 1:		
		if(iUNum==0)	//全部都为坏盘，则设置为损坏状态
		{
			stat = 40;
		}
		else
		{
			if(i_Num>=1)	//有盘损坏，则设置为降级状态
			{
				stat = 50;
			}
			else	//全部正常，则设置为正常状态
			{
				stat = 10;
			}
		}
		break;
	case 4:
	case 5:
		if(i_Num>=2)	//有两个以及两个以上的坏盘，则设置为损坏状态
		{
			stat = 40;
		}
		else
		{
			if(i_Num>=1) //只有一个坏盘，则设置为降级状态
			{
				stat = 50;
			}
			else	//没有坏盘，则设置为正常状态
			{
				stat = 10;
			}
		}
		break;
	//阵列为RAID6
	case 6:
		if (i_Num > 2)
		{
			stat = 40;
		}
		else
		{
			if(i_Num >= 1)	//有一个以上坏盘，则设置为降级状态
			{
				stat = 50;
			}
			else	//没有坏盘，设置为正常状态
			{
				stat = 10;
			}
		}
		break;
	//阵列为RAID10
	case 10:
		if(isRaid10Bad(raidStateString))	//如果有一组盘同时损坏，则设置为损坏状态
		{
			stat = 40;
		}
		else
		{
			if(i_Num >= 1)	//如果有盘损坏，则设置为降级状态
			{
				stat = 50;
			}
			else	//没有坏盘，设置为正常状态
			{
				stat = 10;
			}
		}
		break;
	}
	return stat;
}


/***********************************************************************
 *作者：Ivan.Su
 *功能：更新阵列的状态，监控阵列，生成阵列日志
 *参数：rc－阵列的配置信息
 *返回值：无
 ***********************************************************************/
void RaidMonitor(RaidConfig& rc)
{
	ifstream  fpRaidStat;
	string buffer[3];	//存放一个阵列的信息
	bool confIsChanged = 0;

	chdir(workingDir);

	system("cat /proc/mdstat > mdstat");	//得到阵列状态信息，存放在raidstat中
	fpRaidStat.open("mdstat",ios::in);
	if(!fpRaidStat)	//打开文件错误，或文件不存在
	{
		//cout<<" mdstat file io wrong"<<endl;
		return;
	}
	//cout <<"RaidMonitor"<<endl;
	getline(fpRaidStat,buffer[0]);	 //将文件指针移动到第二行

	while(!fpRaidStat.eof())
	{
		getline(fpRaidStat,buffer[0]); 

		if(buffer[0].find("unused devices:",0) != string::npos) //阵列状态检测完成，则退出
		{
			break;
		}

		if(buffer[0].find("md", 0) == string::npos)
		{
			continue;
		}

		if(string::npos != buffer[0].find("inactive",0))//如果阵列为损坏状态
		{
			string raidIndex = buffer[0].substr(buffer[0].find("md", 0)+2, buffer[0].find(" ",2)-2);	//获得当前阵列索引

			//在rc中找到当前阵列
			for(int i = 0; i < rc.singleRaidNum; i++)	
			{
				if(!strcmp(rc.singleRaids[i].index,(char *)raidIndex.c_str()))	 
				{
					if (rc.singleRaids[i].raidstat != 40)	//阵列还没有被标记为损坏
					{
						
						rc.singleRaids[i].raidstat = 40;
						confIsChanged =1;
						//写日志
						string logInfor;
						logInfor = getLogDateAndTime();
						logInfor += "DISKFAULTY|阵列";
						logInfor += raidIndex; 
						logInfor += "已经损坏，不能正常使用，请删除该阵列";
						
						writeLog(logInfor);
					}	//if(rc.singleRaids[i].raidstat != "40")
					break;
				}	//if(rc.singleRaids[i].index == raidIndex)
			}	//for(int i = 0; i < rc.singleRaidNum; i++)

			getline(fpRaidStat,buffer[1]);
		}	//if(string::npos != buffer.find("inactive",0))
		else
		{
			getline(fpRaidStat,buffer[1]);	 //读出当前阵列第2行信息
			getline(fpRaidStat,buffer[2]);	 //读出当前阵列第3行信息

			int raidPos = -1;	//用于确定阵列的位置
			
			//处理当前阵列第1行信息
			string raidIndex= buffer[0].substr(buffer[0].find("md", 0)+2,buffer[0].find(" ",2)-2);	//获得当前阵列索引

			for(int i = 0; i < rc.singleRaidNum; i++)	//在rc中找到当前阵列，获得其位置
			{
				if(!strcmp(rc.singleRaids[i].index, (char *)raidIndex.c_str()))
				{
					raidPos = i;
					break;
				}	//if(rc.singleRaids[i].index == raidIndex)
			}	//for

			if (raidPos == -1)	//当前阵列在rc中没有被找到
			{
				continue;
			}

			if(rc.singleRaids[raidPos].raidstat == 40)
			{
				continue;
			}

			if(!strcmp(rc.singleRaids[raidPos].level, "0"))		//当前阵列为raid0
			{
				bool isRaid0Bad = 0;
				for(int j = 0; j < rc.singleRaids[raidPos].raidDiskNum; j++)	//查看raid0中是否有磁盘损坏
				{
					if(!strcmp(rc.singleRaids[raidPos].raidDisks[j].status,"2"))	//raid0中有坏盘存在
					{
						if (rc.singleRaids[raidPos].raidstat != 40)	 //raid0还没有被设置为损坏
						{
							rc.singleRaids[raidPos].raidstat = 40;
							isRaid0Bad = 1;
						}
						break;
					}	//if(!strcmp(rc.singleRaids[raidPos].raidDisks[j].status,"2"))
				}	//for(int j=0;j<rc.singleRaids[raidPos].raidDiskNum;j++)

				if(buffer[0].find("(F)", 0) != string::npos)	//有磁盘被置为坏盘
				{
					if (rc.singleRaids[raidPos].raidstat != 40)	 //raid0还没有被设置为损坏
					{
						rc.singleRaids[raidPos].raidstat = 40;
						isRaid0Bad = 1;
					}
				}

				if(isRaid0Bad)	//第一次检测到阵列损坏
				{
					confIsChanged =1;
					string logInfor;
					logInfor = getLogDateAndTime() + "DISKFAULTY|阵列  " + raidIndex + "已经损坏，不能正常使用，请删除该阵列";
					writeLog(logInfor);
				}
				else if (rc.singleRaids[raidPos].raidstat != 40)	 //阵列处于正常状态
				{
					if(rc.singleRaids[raidPos].raidstat !=10)	//第一次检测到阵列处于正常状态
					{
						rc.singleRaids[raidPos].raidstat =10;
						confIsChanged =1;
						string logInfor;
						logInfor =getLogDateAndTime() +"RAIDSTATUS|阵列  " + raidIndex + "状态正常";
						writeLog(logInfor);
					}
				}
			}	//if(rc.singleRaids[raidPos].level == "0")

			//处理当前阵列第2行信息
			double arraySize = atof((char *)buffer[1].c_str())/1024/1024;
			char cArraySize[20];
			string sArraySize="";
			gcvt(arraySize, 10, cArraySize);
			sArraySize = cArraySize;
			//cout<<"sArraySize ="<<sArraySize<<endl;
			if(sArraySize.find(".", 0) != string::npos)
			{
				sArraySize = sArraySize.substr(0, sArraySize.find(".", 0)+2) + " GB";
			}
			//cout<<" Now sArraySize ="<<sArraySize<<endl;
			strcpy(rc.singleRaids[raidPos].raidcap, (char *)sArraySize.c_str());
			//strcpy(rc.singleRaids[raidPos].raidcap, (char *)buffer[1].substr(6,GB_position-6).c_str());	//获得当前阵列的容量

			//当阵列不为raid0时，处理当前阵列第3行以及第2行相关信息
			if(strcmp(rc.singleRaids[raidPos].level, "0"))	//如果当前阵列为RAID0，则第三行为空；否则需要解析第三行信息
			{
				if(buffer[2].find("recovery =",0) != string::npos)	//当前阵列处于重建状态
				{
					if(rc.singleRaids[raidPos].raidstat != 20)	// 第一次检测到阵列处于重建状态
					{
						rc.singleRaids[raidPos].raidstat = 20;	//设置阵列状态为重建状态
						confIsChanged =1;
						//写日志
						string logInfor;
						logInfor = getLogDateAndTime() + "RAIDSTATUS|阵列 " + raidIndex + "正在进行重建 !";
						writeLog(logInfor);
					}
					strcpy(rc.singleRaids[raidPos].percent, (char *)buffer[2].substr(buffer[2].find("recovery =",0)+10, buffer[2].find("%",0)-buffer[2].find("recovery =",0)-10).c_str());	 	//得到正在重建的百分比
				}
				else if(buffer[2].find("resync =",0) != string::npos)	//当前阵列处于同步状态
				{
					if(rc.singleRaids[raidPos].raidstat != 30)	// 第一次检测到阵列处于重建状态
					{
						rc.singleRaids[raidPos].raidstat = 30;	//设置阵列状态为重建状态
						confIsChanged =1;
						//写日志
						string logInfor;
						logInfor = getLogDateAndTime() + "RAIDSTATUS|阵列 " + raidIndex + "正在进行同步 !";
						writeLog(logInfor);
					}
					strcpy(rc.singleRaids[raidPos].percent, (char *)buffer[2].substr(buffer[2].find("resync =",0)+8, buffer[2].find("%",0)-buffer[2].find("resync =",0)-8).c_str());	//得到正在同步的百分比
				}
				else	//阵列处于除重建和同步以外的其他几种状态
				{
					int iRaidStat = getRaidStat(buffer[1], rc.singleRaids[raidPos].level ); //得到阵列所处状态

					if(rc.singleRaids[raidPos].raidstat != iRaidStat)	// 阵列状态发生了改变
					{
						confIsChanged =1;

						string logInfor;
						rc.singleRaids[raidPos].raidstat = iRaidStat;
						switch (iRaidStat)
						{
						case 10:
							logInfor = getLogDateAndTime() + "RAIDSTATUS|阵列 " + rc.singleRaids[raidPos].index + "处于正常状态 !";
							break;
						case 40:
							logInfor = getLogDateAndTime() +"DISKFAULTY|阵列 " + rc.singleRaids[raidPos].index + "已经损坏，不能正常使用，请删除该阵列 !";
							break;
						case 50:
							logInfor = getLogDateAndTime() + "RAIDSTATUS|阵列 " + rc.singleRaids[raidPos].index + "处于降级模式 !";
							break;				
						}
						writeLog(logInfor);
					}
				}
				//得到阵列重建/同步所需时间
				strcpy(rc.singleRaids[raidPos].time, (char *)buffer[2].substr( buffer[2].find("finish=",0)+7,buffer[2].find("min",0)-buffer[2].find("finish=",0)-7).c_str());
				//得到阵列重建/同步的速度
				strcpy(rc.singleRaids[raidPos].speed, (char *)buffer[2].substr(buffer[2].find("speed=",0)+6,buffer[2].find("K/sec",0)-buffer[2].find("speed=",0)-6).c_str());
			}	//if(raidInfo[iRaidNum].raidLevel!="0")
		}	//else		
	}	//while(!fpRaidStat.eof())
	fpRaidStat.close();

	//检测raidConfig.xml中的阵列是否有没有启动成功的
	for(int i =0; i<rc.singleRaidNum; i++)
	{		
		string currentSingleRaid = rc.singleRaids[i].index;
		currentSingleRaid = "md"+currentSingleRaid;
		bool isRaidExist = 0;
		fpRaidStat.open("mdstat", ios::in);
		while(!fpRaidStat.eof())
		{
			getline(fpRaidStat,buffer[0]);
			
			if(buffer[0].find("unused devices:",0) != string::npos) //阵列状态检测完成，则退出
			{
				break;
			}
			if(buffer[0].find(currentSingleRaid, 0) != string::npos)
			{
				isRaidExist =1;
				break;
			}
		}

		if(!isRaidExist)
		{
			if(rc.singleRaids[i].raidstat != 40)
			{
				confIsChanged =1;
				string logInfor ="";
				rc.singleRaids[i].raidstat = 40;
				logInfor = getLogDateAndTime() +"DISKFAULTY|阵列 " + rc.singleRaids[i].index + "已经损坏，不能正常使用，请删除该阵列 !";
				writeLog(logInfor);
			}
		}

		fpRaidStat.close();
	}
	
	system("rm -f mdstat");

	if(!access("raidConfig.xml",0))
	{
		system("rm -f raidConfig.xml");
	}
	rc.saveRcToXml("raidConfig.xml");

	//write the raidConfig.xml to pci
	if(confIsChanged)
	{
	#ifdef READ_WRITE_PCI
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP);
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);
	#endif
	}

	return;


}


void NetworkSet(RaidConfig& rc)
{
	chdir(workingDir);
	if(!access("eth0",0))
	{
		system("cp -a ../ifcfg-eth ./ifcfg-eth0");
		FILE *net0=fopen("ifcfg-eth0","r");
		char buffer0[100];
		while(fgets(buffer0,100,net0) != NULL)
		{
			if(strstr(buffer0,"IPADDR=") != NULL)
			{
				if(strstr(buffer0,".") == NULL)
				{
					fclose(net0);
					system("rm -f  ifcfg-eth0");
					return;
				}
				else break;
			}
		}
		fclose(net0);
		system("chown root:root ifcfg-eth0");
		system("mv -f ifcfg-eth0 /etc/sysconfig/network-scripts/ifcfg-eth0");
		system("/etc/init.d/network restart");
		system("rm -f eth0");
		
		//write ifcfg-eth0 to pci
	#ifdef READ_WRITE_PCI
		write_to_pci(IFCFG_ETH0_DIR_WRITE, RAID_ETH_ZERO);
	#endif
	}
	else 	if(!access("eth1",0))
	{
		system("cp -a ../ifcfg-eth ./ifcfg-eth1");
		FILE *net1=fopen("ifcfg-eth1","r");
		char buffer1[100];
		while(fgets(buffer1,100,net1) != NULL)
		{
			if(strstr(buffer1,"IPADDR=") != NULL)
			{
				if(strstr(buffer1,".") == NULL)
				{
					fclose(net1);
					system("rm -f  ifcfg-eth1");
					return;
				}
				else break;
			}
		}
		fclose(net1);
		system("chown root:root ifcfg-eth1");
		system("mv -f ifcfg-eth1 /etc/sysconfig/network-scripts/ifcfg-eth1");
		system("/etc/init.d/network restart");
		system("rm -f eth1");
		
		//write ifcfg-eth1 to pci	
	#ifdef READ_WRITE_PCI
		write_to_pci(IFCFG_ETH1_DIR_WRITE, RAID_ETH_ONE);
	#endif
	}
	return;
}
//add for users.txt 0403
void UserSet(RaidConfig&rc)
{
	chdir(workingDir);
	if(!access("user", 0))
	{
		system("rm -f user");
		//write users.txt to pci
	#ifdef READ_WRITE_PCI
		write_to_pci(USERS_DIR_WRITE, RAID_USERLIST);
	#endif
	}
}

void RebootHost(RaidConfig& rc)
{
	chdir(workingDir);
	if(!access("reboot_event",0))
	{
		for(int i = 0;i < rc.singleRaidNum;i++)
		{
			rc.singleRaids[i].stop();
		}
		system("rm -f reboot_event");

	#ifdef READ_WRITE_PCI
		write_to_pci(LOG_DIR_WRITE, RAID_MON_LOG);//write the LOG to pci
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP); //write raidConfig_backup xml to pci
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);//write the raidConfig.xml to pci
		write_to_pci(OPERATION_LOG_DIR_WRITE, RAID_OPERA_LOG);//write the operation_log to pci
		write_to_pci(BLACKLIST_DIR_WRITE, RAID_BLACKLIST);//write the blacklist to pci
	#endif
	
		
		system("reboot");
	}
}

void HaltHost(RaidConfig& rc)
{
	chdir(workingDir);
	if(!access("halt_event",0))
	{
		for(int i = 0;i < rc.singleRaidNum;i++)
		{
			rc.singleRaids[i].stop();
		}
		system("rm -f halt_event");

	#ifdef READ_WRITE_PCI
		write_to_pci(LOG_DIR_WRITE, RAID_MON_LOG);//write the LOG to pci
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP);
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);//write the raidConfig.xml to pci
		write_to_pci(OPERATION_LOG_DIR_WRITE, RAID_OPERA_LOG);//write the operation_log to pci
		write_to_pci(BLACKLIST_DIR_WRITE, RAID_BLACKLIST);//write the blacklist to pci
	#endif
	
		system("poweroff");
	}
}
void Default(RaidConfig& rc)
{
	chdir(workingDir);
	if(!access("default", 0))
	{
		system("rm -f default");
		
		RaidConfig rc1;
		DiskArray da;
			
		//删除当前所有阵列
		for(int i =0;i < rc.singleRaidNum; i++)
		{
			rc.singleRaids[i].stop();
			//rc.removeSingleRaid(rc.singleRaids[0]);
		}
/*		
		for(int i = 0;i < rc.diskNum;)
		{
			rc.removeDisk(rc.disks[0]);
		}
*/
		da.fillArray();
		for(int i = 0;i < da.diskNum;i++)
		{
			rc1.addDisk(da.array[i]);
		}

		rc =rc1;

		chdir(workingDir);
		if(!access("raidConfig.xml",0))
		{
			system("rm -f raidConfig.xml");
		}
		rc.saveRcToXml("raidConfig.xml");

		//清除日志信息
		system("rm -f log");
		FILE *fp=fopen("log", "w");
		fclose(fp);
		
		system("cat /home/users.txt >/usr/local/apache/htdocs/RaidManager/users.txt");//users.txt->default
		
		system("rm -f /usr/local/apache/htdocs/RaidManager/logon/blacklist.txt");//blacklist.txt->empty
		fp = fopen("/usr/local/apache/htdocs/RaidManager/logon/blacklist.txt", "w");
		fclose(fp);
		
		system("cat /home/ifcfg-eth0 >/etc/sysconfig/network-scripts/ifcfg-eth0");//ifcfg-eth0->default
		system("cat /home/ifcfg-eth1 >/etc/sysconfig/network-scripts/ifcfg-eth1");//ifcfg-eth1->default
		system("/etc/init.d/network restart");

		system("rm -f default");
	#ifdef READ_WRITE_PCI
		write_to_pci(LOG_DIR_WRITE, RAID_MON_LOG);//write the LOG to pci
		write_to_pci(RAIDCONFIG_DIR_WRITE_BACKUP, RAID_CONFIG_BACKUP);
		write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);//write the raidConfig.xml to pci
		write_to_pci(OPERATION_LOG_DIR_WRITE, RAID_OPERA_LOG);//write the operation_log to pci
		write_to_pci(BLACKLIST_DIR_WRITE, RAID_BLACKLIST);//write the blacklist to pci
		write_to_pci(USERS_DIR_WRITE, RAID_USERLIST);//write users.txt to pci
		write_to_pci(IFCFG_ETH0_DIR_WRITE, RAID_ETH_ZERO);//write ifcfg-eth0 to pci
		write_to_pci(IFCFG_ETH1_DIR_WRITE, RAID_ETH_ONE);//write ifcfg-eth1 to pci	
	#endif
	}
}


/***************************************************************
* 功能:处理命令
* 输入: 无
* 输出: 无
* 作者: zjf
* 日期: 2007-12-6
****************************************************************/
int process_cmd(RaidConfig& rc){

	int rtval;
	chdir(workingDir);
	ifstream fp;
	string buffer;
	int i;
	fp.open("cmd",ios::in);
	if(fp.bad()){
#ifdef DEBUG
		rtval=system("pwd");if(rtval<0){cout<<"ERROR:execute"<<endl;exit(-1);}
#endif
		cout<<"WRONG:proccess_cmd"<<endl;
		return -1;
	}
	getline(fp,buffer);		


   /* if(buffer.find("mapping")!=string::npos){//修改映射
		getline(fp,buffer);
		string index=buffer;
		string IP[30];
		getline(fp,buffer);
		if(buffer!="null"){		
			separat_string(buffer, IP);
			for(int j=0;j<30;j++){
				if(IP[j]=="")
					break;
				del_dev(index, IP[j]);
			}
		}
		getline(fp,buffer);
		if(buffer!="null"){
			raid raids[RAIDNUM];
			getraidinfo(raids);
			get_dev_chnl(raids);
			if(raids[atoi(index.c_str())].dev_name==""){
				open_dev(index, get_dev_name(atoi(index.c_str())));
#ifdef DEBUG
				cout<<cmdopen<<endl;
#endif
			}

			separat_string(buffer, IP);
			for(int j=0;j<30;j++){
				if(IP[j]=="")
					break;
				add_dev(index, IP[j]);
			}
		}
		//xmlconf();
	}*/
		
    if(buffer.find("binding")!=string::npos){//修改绑定
		getline(fp,buffer);
		string index;
		char tempc[10];
		gcvt(atoi((char *)buffer.c_str()) + 1, 10, tempc);	
		index =tempc;
		
		string channel[CHANNELNUM];
		getline(fp,buffer);
#ifdef DEBUG
		cout<<index<<endl;
		cout<<buffer<<endl;
#endif
		if(buffer!="null"){				
			separat_string(buffer, channel);
			for(int j=0;j<CHANNELNUM;j++){
				if(channel[j]=="")
					break;
				unbind_dev(index, channel[j]);
			}
		}
		getline(fp,buffer);
		if(buffer!="null"){
			separat_string(buffer, channel);
			for(int j=0;j<CHANNELNUM;j++){
				if(channel[j]=="")
					break;
				bind_dev(index, channel[j]);
			}
		}
		//xmlconf();
	}
	fp.close();
	// 需要重构XML
	for(i=0;i<rc.singleRaidNum;i++)
	 {
        	 rc.singleRaids[i].getBindingInfo();
	 }
	
	rc.saveRcToXml("raidConfig.xml");
#ifdef READ_WRITE_PCI
	write_to_pci(RAIDCONFIG_DIR_WRITE, RAID_CONFIG);//write the raidConfig.xml to pci
#endif	
	rtval=system("rm -f cmd");if(rtval<0){cout<<"ERROR:execute"<<endl;exit(-1);}	
	return 0;
}

main(int argc, char *argv[])
{

	int back=1;
	if(argc !=1)
	{
		if(argc == 2 && !strcmp(argv[1], "-v"))
		{
			cout<<"***************************************************************"<<endl;
			cout<<"*                HiStor Daemon version 1.0"<<endl;
			cout<<"***************************************************************"<<endl;
			return 0;
		}
		else if(argc == 2 &&!strcmp(argv[1], "-a"))
		{
			cout<<"*************************************************************************"<<endl;
			cout<<"*Author::\tIvan.Su                                                                                    *"<<endl;
			cout<<"*E-mail::\tivansudun@163.com                                                                *"<<endl;
			cout<<"*************************************************************************"<<endl;
			return 0;
		}
		else if(argc == 2 &&!strcmp(argv[1], "-o"))
		{
			cout<<"*************************************************************************"<<endl;
			cout<<"*Thanks for the workgroup who goes with me forever. "<<endl;
			cout<<"*Mark.Yan, Zero.Zhou, Huibo Ren, Bo Gao, Guoling Rao, Jianfeng Zhao."<<endl;
			cout<<"*HiStor Co.Ltd., 2007-2008 Wuhan China"<<endl;
			cout<<"*************************************************************************"<<endl;
			return 0;
		}
		else if(argc == 2 &&!strcmp(argv[1], "-b"))
		{
			cout<<"*************************************************************************"<<endl;
			cout<<"*Thanks for the workgroup who goes with me forever. "<<endl;
			cout<<"*Mark.Yan, Zero.Zhou, Huibo Ren, Bo Gao, Guoling Rao, Jianfeng Zhao."<<endl;
			cout<<"*HiStor Co.Ltd., 2007-2008 Wuhan China"<<endl;
			cout<<"*************************************************************************"<<endl;
			back=0;
		}	
		else
		{
			cout<<"command line format error"<<endl;
			return 0;
		}
	}
	
	nice(0);
	if(back)
	init_daemon(); 

	chdir(workingDir);
	DiskArray da;
	RaidConfig rc;
	bool isUserAccess = 0;
	int immediateResponseTimes = 40;
	int curImmediateResponseTimes = 0;

	//sleep(3);
	
	//用于pci卡启动，daemon启动时读取flash中的config、log等信息
	char buffer[10];
	FILE *fp1;


	string cmdline="";
	//由于映射的通道号必须从0开始，所以首先将一个空的设备映射为0
	system("touch /usr/local/apache/htdocs/RaidManager/Monitor/lun");
	cmdline="echo \"open histor /usr/local/apache/htdocs/RaidManager/Monitor/lun\"> /proc/scsi_tgt/vdisk/vdisk";//打开设备	
	try
	{
		system(cmdline.c_str());//打开设备
	}
	catch(int)
	{
		cout<<"cmdline open device is wrong"<<endl;
		return 0;
	}
	
	cmdline="echo \"add histor 0\">/proc/scsi_tgt/groups/Default/devices";
	try
	{
		system(cmdline.c_str());//添加设备到Default 下
	}
	catch(int)
	{
		cout<<"cmdline open device is wrong"<<endl;
		return 0;
	}

#ifdef READ_WRITE_PCI
	//read log from pci
	read_from_pci(LOG_DIR_READ, RAID_MON_LOG);
	//read users.txt from pci	
	read_from_pci(USERS_DIR_READ, RAID_USERLIST);
#endif

	chdir(workingDir);
	if(!access("users.txt",0))
	{
		fp1 = fopen("users.txt", "r");

		if(fgets(buffer,10,fp1) == NULL)
		{
			//对于新卡，第一次读出来的users.txt文件为空，删除之
			system("rm -f users.txt");
		}
		else{
			system("chmod 666 users.txt");
			system("mv -f users.txt /usr/local/apache/htdocs/RaidManager/users.txt");
		}
		fclose(fp1);
	}

	//read operation_log from pci	
#ifdef READ_WRITE_PCI
	read_from_pci(OPERATION_LOG_DIR_READ, RAID_OPERA_LOG);
#endif

	chdir(workingDir);
	if(!access("operation_log",0))
	{
		fp1 = fopen("operation_log", "r");
		if(fgets(buffer,10,fp1) == NULL)
		{
			//对于新卡，第一次读出来的operation_log文件为空，删除之
			system("rm -f operation_log");
		}
		else
		{
			system("chmod 666 operation_log");
			system("mv -f operation_log /usr/local/apache/htdocs/RaidManager/log/log");
		}
		fclose(fp1);
	}
	
	//read blacklist.txt from pci	
#ifdef READ_WRITE_PCI
	read_from_pci(BLACKLIST_DIR_READ, RAID_BLACKLIST);
#endif

	chdir(workingDir);
	if(!access("blacklist.txt",0))
	{
		fp1 = fopen("blacklist.txt", "r");
		if(fgets(buffer,10,fp1) == NULL)
		{
			//对于新卡，第一次读出来的blacklist.txt文件为空，删除之
			system("rm -f blacklist.txt");
		}
		else
		{
			system("chmod 666 blacklist.txt");
			system("mv -f blacklist.txt /usr/local/apache/htdocs/RaidManager/logon/blacklist.txt");
		}
		fclose(fp1);
	}

	//read config from pci	
#ifdef READ_WRITE_PCI
	read_from_pci(RAIDCONFIG_DIR_READ, RAID_CONFIG);
#endif

	chdir(workingDir);
	if(!access("raidConfig.xml",0))
	{
		if((fp1 = fopen("raidConfig.xml", "r")) == NULL)
		{
		#ifdef READ_WRITE_PCI
			read_from_pci(RAIDCONFIG_DIR_READ_BACKUP, RAID_CONFIG_BACKUP);
		#endif
		
			system("mv raidConfig_backup.xml raidConfig.xml");
			//system("chmod 777 raidConfig");
			if((fp1 =fopen("raidConfig.xml", "r")) == 	NULL)
			{
				system("touch raidConfig_error");
				while(1)
				{
					if(!access("raidConfigNew.xml",0) && !access("restore", 0))
					{
						system("rm -f restore");
						system("mv raidConfigNew.xml raidConfig.xml");
						//system("chmod 777 raidConfig");
						if((fp1 =fopen("raidConfig.xml", "r")) == 	NULL)
						{
							system("rm -f raidConfig.xml");
							system("touch raidConfig.xml");
							fp1 = fopen("raidConfig.xml", "r");
						}

						break;
					}
				}	//while(1)
			}
			
		}
		if(fgets(buffer,10,fp1) == NULL)
		{
			//对于新卡，第一次读出来的xml文件为空，删除之
			system("rm -f raidConfig.xml");
		}
		fclose(fp1);
	}
	//end of pci
	chdir(workingDir);
	//raidConfig.xml文件不存在
//	cout<<"hh"<<endl;
	da.fillArray();
#ifdef DEBUG
	for(int i = 0;i < da.diskNum; i++)
		cout<<da.array[i].devName<<"     "<<da.array[i].scsiID<<"    "<<da.array[i].sn<<endl;
#endif
	if(access("raidConfig.xml",0))	//not exist  
	{
		for(int i = 0;i < da.diskNum;i++)
		{
			rc.addDisk(da.array[i]);
		}
	//	cout<<"save"<<endl;
		rc.saveRcToXml("raidConfig.xml"); 
	}
	else 
	{
		rc.buildRcFromXml("raidConfig.xml");
		//cout<<rc.diskNum<<"   "<<rc.singleRaidNum<<endl;
		initCheck(rc, da);
		if(!access("raidConfig.xml",0))
			system("rm -f raidConfig.xml");
		
		rc.saveRcToXml("raidConfig.xml");
		
		for(int p = 0;p < rc.singleRaidNum;p++)
		{
			if(rc.singleRaids[p].raidstat != 40)
			{
				rc.singleRaids[p].start();
				//usleep(100000);
			}
		}
	}
/*----daemon's circulate and report the raid system status and update the user's operations----*/
	//isUserAccess =0;
	while(1)
	{
	//	cout<<"----------------------------------------------------------------"<<endl;
		chdir(workingDir);
		if(!access("logon_flag",0))	
		{
			isUserAccess =1;
			system("rm -f logon_flag");
		}
		
		DiskMonitor(da, rc);
		//cout<<"DiskMonitor end!!!!!!!!!!"<<endl;
		//阵列信息监控
		if(!access("raidConfigNew.xml",0))
		{
		//	cout<<"raidConfig here"<<endl;
			isUserAccess = 1;
			raidConfig(rc, da);
		}
		//cout<<"raidConfig end!!!!!!!!!"<<endl;
		if(rc.singleRaidNum > 0)
		{
		//	cout<<rc.singleRaidNum<<endl;
			RaidMonitor(rc);
		}

		//cout<<"raidMonitor end!!!!!!!"<<endl;
		if((!access("eth0",0))||(!access("eth1",0)))
		{
			isUserAccess = 1;
			NetworkSet(rc);
		}
		if(!access("reboot_event",0))
		{
			isUserAccess = 1;
			RebootHost(rc);
		}
		if(!access("halt_event",0))
		{
			isUserAccess = 1;
			HaltHost(rc);
		}
		if(!access("default", 0))
		{
			isUserAccess = 1;
			Default(rc);
		}
		if(!access("user", 0))//for pci
		{
			isUserAccess = 1;
			UserSet(rc);	
		}
		if(!access("cmd",0))
		{
			isUserAccess = 1;
			if(process_cmd(rc)!=0){
			//	cout<<"can't process cmd"<<endl;
				int rtval=system("rm -f cmd");
			}
		}
	
		//cout<<"sleep begin!!!!!!!!!"<<endl;
		if(isUserAccess ==1)
		{
			curImmediateResponseTimes++;
			if(curImmediateResponseTimes < immediateResponseTimes)
			{
				//cout<<"sleep2"<<endl;
				sleep(2);
			}
			else
			{
				curImmediateResponseTimes =0;
				isUserAccess =0;
				//cout<<"sleep10"<<endl;
				sleep(10);
			}
		}
		else
		{
			//cout<<"sleep10"<<endl;
			sleep(10);
		}
		//cout<<"sleep end!!!!!!!!!!"<<endl;
	}	
}
