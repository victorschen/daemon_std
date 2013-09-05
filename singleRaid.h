/*		
**		singleRaid.h   the raid's describe file
**		This file defines some raid's attributes and operations.
**		Include the raid's mapping and unmapping... ...
*/
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include<sstream>
#include   <dirent.h>   
#include   <sys/types.h>   
#include   <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include<dirent.h>  
#include <fstream>
#include <iostream>




#define CHANNELNUM 2
using namespace std;

#include "disk.h"
#include "global.h"
#ifndef  _SINGLERAID_H_
#define  _SINGLERAID_H_

class SingleRaid {
public:
	Disk raidDisks[16];	
	Disk spareDisks[8];
	int raidDiskNum;
	int spareDiskNum;
	int bindingNum;
	int popedomNum;
	int raidstat;
	char raidcap[20];
	char time[20];
	char speed[20];
	char percent[10];
	char mappingNo[5];
	char level[10];	
	char chunk[5];
	char index[5];
	char devName[10];	
	char popedom[30][20];
	char binding[20][4];

	SingleRaid();
	void setAttribute(char * i,char * l,char * c,char * m,int r,char *cap);
	void addRaidDisk(Disk& d);
	void addSpareDisk(Disk& d);	
	void create();
	void setBinding();
	void getBindingInfo();
	void start();
	void stop();
	void hotAddDisk(Disk& d);
//	void hotRemove(Disk& d);
	void removeDisk(Disk& d);

	void removeDiskInSingleRaid(Disk& d);
	void hotRemoveDisk(Disk& d);
	void mapping();
	//void startmapping();
	void unMapping();
	void encrypt();
	void addPopedom(char * name);	
       void addBinding(char * value);
	bool operator==(SingleRaid& sr);
	int  getRaidDiskIndex(Disk& d);
	int  getSpareDiskIndex(Disk& d); 
	void setMappingNo();
	void startMappingNo();

	void dealWithSingleRaidSpareDisk();
};

SingleRaid::SingleRaid()
{
	raidDiskNum = 0;
	spareDiskNum = 0;
	popedomNum = 0;
	bindingNum = 0;
	raidstat=1;
	strcpy(mappingNo,"");
	strcpy(level,"");
	strcpy(chunk,"");
	strcpy(index,"");
	strcpy(devName,"");
	strcpy(raidcap,"");
	strcpy(time,"");
	strcpy(speed,"");
	strcpy(percent,"");
}


void SingleRaid::setAttribute(char * i,char * l,char * c,char * m,int r,char *cap)
{
	strcpy(index,i);
	strcpy(level,l);
	strcpy(chunk,c);
	strcpy(devName,"/dev/md");
	strcat(devName,index);
	strcpy(mappingNo,m);
	strcpy(time,"");
	strcpy(speed,"");
	strcpy(percent,"");
	strcpy(raidcap,cap);
	raidstat=r;
	raidDiskNum = 0;
	spareDiskNum = 0;
}

void SingleRaid::addRaidDisk(Disk& d)
{
	raidDisks[raidDiskNum] = d;
	raidDiskNum++;
}

void SingleRaid::addSpareDisk(Disk& d)
{
	spareDisks[spareDiskNum] = d;
	spareDiskNum++;
}

void SingleRaid::create()
{
	char cmdline[200],temp[5];
	strcpy(cmdline,"mdadm -C ");
	strcat(cmdline,devName);
	strcat(cmdline," --run");	
	/*
	 *	-R, --run
     *         Insist  that mdadm run the array, even if some of the components
     *         appear to be active in another array  or  filesystem.   Normally
     *         mdadm will ask for confirmation before including such components
     *         in an array.  This option causes that question to be suppressed.
	*/
	strcat(cmdline," -l");
	strcat(cmdline,level);
	strcat(cmdline," -n");
	gcvt(raidDiskNum,5,temp);
	strcat(cmdline,temp);
	if(strcmp(level,"0"))		
	{
		strcat(cmdline," -x");
		gcvt(spareDiskNum,5,temp);
		strcat(cmdline,temp);
	}
	if((level[0] !=1) ||(level[1]==0)) //raid1 no need to set chuck size
	{             
		strcat(cmdline," -c");
		strcat(cmdline,chunk);
	}
	for(int i = 0;i < raidDiskNum;i++)
	{
		strcat(cmdline," ");
		strcat(cmdline,raidDisks[i].devName);
		strcat(cmdline," ");
	}
	if(strcmp(level,"0"))			//raid0 no need to set spare disks
	{
		for(int i = 0;i < spareDiskNum;i++)
		{
			strcat(cmdline," ");
			strcat(cmdline,spareDisks[i].devName);
			strcat(cmdline," ");
		}
	}
	system(cmdline);	
	mapping();
	setBinding();
}

void SingleRaid::setBinding()
{
	char cmdline[200];
	for(int i=0;i<20;i++){
		if(strcmp(binding[i],"")){
			strcpy(cmdline,"echo \"binding HiStorRAID");
			char tempc[10];	
			gcvt(atoi(index) + 1, 10, tempc);				
			strcat(cmdline,tempc);
			strcat(cmdline," ");
			strcat(cmdline,binding[i]);
			strcat(cmdline,"\"> /proc/scsi_tgt/vdisk/vdisk ");
			system(cmdline);	
		}
	       
	}	
}

void SingleRaid::getBindingInfo()
{
 chdir(workingDir);
       int rtval;
	ifstream fp;
	string buffer;
	int i;
	//读取后台vdisk文件;
	//填充raid结构体数组的channel属性;
	//rtval=system("tw_cli /c0 show unitstatus>raid_conf");	
	//fp.open("raid_conf.txt", "r+");
	system("cat /proc/scsi_tgt/vdisk/vdisk>vdisk");
	
	fp.open("vdisk", ios::in);
	if(fp.bad()){
#ifdef DEBUG
		rtval=system("pwd");if(rtval<0){cout<<"ERROR:execute"<<endl;exit(-1);}
#endif
		cout<<"WRRONG:get vdisk"<<endl;
		return ;
	}
	getline(fp, buffer);
	getline(fp,buffer);
	while(!fp.eof()){		
		getline(fp, buffer);
		if(buffer.size()<10)
			break;
		char raid[15]="";
		char nouse[10]="";
		char nouse2[10]="";
		char nouse3[10]="";
		char devname[15]="";
		char channel[10]="";

		//sscanf(buffer.c_str(),"%s %s %s %s %s %s %s",unit,type,status,cmpl,nouse,stripe,size);
		sscanf(buffer.c_str(),"%s %s %s %s %s %s",raid,nouse,nouse2,nouse3,devname,channel);

		if(atoi(raid+10)-1==atoi(index)){//如果阵列索引等于映射的索引
			//string temp=channel;
			if(!strcmp(channel,"ALL")){
                        	for(int k=0;k<CHANNELNUM;k++){
                        		char temp3[10];
					sprintf(temp3,"%d",k); 										
                             		strcpy(binding[k],temp3);
					bindingNum=2;									
				} 
			 }				
			else	{				    	
	                        strcpy(binding[0],channel);
				bindingNum=1;
			}
			break;
		}
	
#ifdef DEBUG		
		
		cout<<binding<<endl;
#endif


	}
	fp.close();	
	system("rm -f vdisk");
}

void SingleRaid::start()
{
	char cmdline[200];
	strcpy(cmdline,"mdadm -A -R ");
	strcat(cmdline,devName);
	for(int i = 0;i < raidDiskNum;i++)
	{
		strcat(cmdline," ");
		strcat(cmdline,raidDisks[i].devName);
		strcat(cmdline," ");
	}
	if(strcmp(level,"0"))
	{
		for(int i = 0;i < spareDiskNum;i++)
		{
			strcat(cmdline," ");
			strcat(cmdline,spareDisks[i].devName);
			strcat(cmdline," ");
		}
	}
	system(cmdline);
	//cout<<"mapping start"<<endl;
	mapping();
	setBinding();
	//cout<<"mapping end"<<endl;
}

void SingleRaid::stop()
{
	//cout<<"unMapping start"<<endl;
	unMapping();	//解除映射
	//cout<<"unMappint end";
	char cmdline[100];
	//删除阵列
	strcpy(cmdline,"mdadm -S ");
	strcat(cmdline,devName);
	system(cmdline);
}

void SingleRaid::hotAddDisk(Disk& d)
{
	char cmdline[100];
	strcpy(d.isSpareDisk,"1");
	addSpareDisk(d);
	strcpy(cmdline,"mdadm -a ");
	strcat(cmdline,devName);
	strcat(cmdline," ");
	strcat(cmdline,d.devName);
	system(cmdline);
}
/*
void SingleRaid::hotRemove(Disk& d)
{
	char cmdline[100];
	strcpy(d.isSpareDisk,"0");
	strcpy(cmdline,"mdadm -f ");
	strcat(cmdline,devName);
	strcat(cmdline," ");
	strcat(cmdline,d.devName);
	system(cmdline);  
}
*/

void SingleRaid::removeDiskInSingleRaid(Disk& d)
{
	char cmdline[100];
	//strcpy(d.isSpareDisk,"0");

	//如果所掉盘为数据盘，在RaidConfig中移除相应盘
	for(int i = 0;i < raidDiskNum;i++)
	{
		if(raidDisks[i] == d)
		{
			while(i < raidDiskNum - 1)
			{
				raidDisks[i] = raidDisks[i + 1];
				i++;
			}
			raidDiskNum--;
			break;
		}
	}

	//如果所掉盘为热备盘，在RaidConfig中移除相应盘
	for(int i = 0;i < spareDiskNum;i++)
	{
		if(spareDisks[i] == d)
		{
			while(i < spareDiskNum - 1)
			{
				spareDisks[i] = spareDisks[i + 1];
				i++;
			}
			spareDiskNum--;
			break;
		}
	}
}

/************************************************************************
 *作者：Ivan.Su
 *功能：热移除已经启动的阵列中的磁盘
 *参数：d—待删除磁盘
 *返回值：无
 ************************************************************************/
void SingleRaid::hotRemoveDisk(Disk& d)
{
	char cmdline[100];
	strcpy(d.isSpareDisk,"0");
/*
	//置为坏盘
	strcpy(cmdline,"mdadm -f ");
	strcat(cmdline,devName);
	strcat(cmdline," ");
	strcat(cmdline,d.devName);
	system(cmdline);  

	//移除磁盘
	strcpy(cmdline,"mdadm -r ");
	strcat(cmdline,devName);
	strcat(cmdline," ");
	strcat(cmdline,d.devName);
	system(cmdline);
*/
	//如果所掉盘为数据盘，在RaidConfig中移除相应盘
	for(int i = 0;i < raidDiskNum;i++)
	{
		if(raidDisks[i] == d)
		{
			while(i < raidDiskNum - 1)
			{
				raidDisks[i] = raidDisks[i + 1];
				i++;
			}
			raidDiskNum--;
			return;
		}
	}

	//如果所掉盘为热备盘，在RaidConfig中移除相应盘
	for(int i = 0;i < spareDiskNum;i++)
	{
		if(spareDisks[i] == d)
		{
			while(i < spareDiskNum - 1)
			{
				spareDisks[i] = spareDisks[i + 1];
				i++;
			}
			spareDiskNum--;
			return;
		}
	}
}

/**************************************************************************
 *作者： Ivan.Su
 *功能：在创建阵列时，进行映射操作
 *参数：无
 *返回值：无
 **************************************************************************/
 void SingleRaid::mapping ()
{
	char cmdline[200];	//命令行
	string dir;	//目录
	string sSubString = "";	//子字符串
	string buffer;	
	char  mappingIndex[10];
	ifstream  fp;
	
	strcpy(cmdline, "echo \"open HiStorRAID");
	gcvt(atoi(index) + 1, 10, mappingIndex);
	strcpy(mappingNo, mappingIndex);
	strcat(cmdline, mappingIndex);
	strcat(cmdline, " ");
	strcat(cmdline, devName);
	strcat(cmdline, " BLOCKIO");
	strcat(cmdline," \"> /proc/scsi_tgt/vdisk/vdisk");
	//cout<<cmdline<<endl;
	try
	{
		system(cmdline);//打开设备
	}
	catch(int)
	{
		//cout<<"cmdline open device is wrong"<<endl;
		return;
	}
	
	//cout<<"peopedom"<<popedom[0]<<endl;
	if(!strcmp(popedom[0], "all"))//all的情况下，先将设备加到Default中，再逐一添加到其他group_index中
	{
		//cout<<"come to 1st part"<<endl;
		DIR   *director; 		//定义目录		
		struct   dirent   *dt;	//目录结构体
		
		dir = "/proc/scsi_tgt/groups";

		if((director=opendir(dir.c_str())) == NULL)//打开目录
		{   
			printf("opendir   %s/%s   error\n");   
			return;   
		}   
		
		chdir(dir.c_str());
		
		
		//将设备添加到各个组				
		while((dt=readdir(director))!=NULL)
		{
			string dirname;
			dirname = dt->d_name;
			if(dirname=="." || dirname=="..")// 目录为.和..是继续
			{
				continue;
			}

			dir="/proc/scsi_tgt/groups/"+dirname+"/devices";		
			
			strcpy(cmdline, "echo \"add HiStorRAID");
			//gcvt(atoi(index) + 1, 10, mappingIndex);
			strcat(cmdline, mappingIndex);
			strcat(cmdline, " ");
			strcat(cmdline, mappingIndex);
			strcat(cmdline, "\"> ");
			strcat(cmdline, dir.c_str());
			//cout<<cmdline<<endl;
			try
			{
				system(cmdline);//将设备映射到Default/devices下
			}
			catch(int)
			{
				//cout<<"cmdline add device is wrong"<<endl;
				return;
			}
			
		}
		closedir(director);
		//cout<<"end add mapping"<<endl;
		chdir(workingDir);		
	}
	else
	{
		for(int i = 0; i<popedomNum; i++)
		{
			//cout<<popedom[i]<<endl;
			
			char * temp = popedom[i];
			string IP = temp;
			
			dir = "/proc/scsi_tgt/groups/group" + IP +"/devices";

			if(!access(dir.c_str(), 0))	//gourpIP exist
			{
				strcpy(cmdline, "echo \"add HiStorRAID");

				//gcvt(atoi(index)+1, 10, mappingIndex);

				strcat(cmdline, mappingIndex);
				strcat(cmdline, " ");
				strcat(cmdline, mappingIndex);
				strcat(cmdline, "\">");
				strcat(cmdline, dir.c_str());
				try
				{
					system(cmdline);
				}
				catch (int)
				{
					//cout<<"cmdline add raid error"<<endl;
					return;
				}
			}
			else
			{
				//add group
				strcpy(cmdline, "echo \"add_group group");
				strcat(cmdline, popedom[i]);
				strcat(cmdline, "\" > /proc/scsi_tgt/scsi_tgt");
				//cout<<cmdline<<endl;
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					//cout<<"cmdline add_group error"<<endl;
					return;
				}

				//add IP to names
				strcpy(cmdline, "echo \"add ");
				strcat(cmdline, popedom[i]);
				strcat(cmdline, "\" >/proc/scsi_tgt/groups/group");
				strcat(cmdline, popedom[i]);
				strcat(cmdline, "/names");
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					//cout<<"cmd line add IP error"<<endl;
					return;
				}

				//add mapping No.0
				/*strcpy(cmdline, "echo \"add histor 0\">/proc/scsi_tgt/groups/group");
				strcat(cmdline, popedom[i]);
				strcat(cmdline, "/devices");
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					//cout<<"cmdline add mapping No.0 error"<<endl;
					return;
				}*/

				//add device
				strcpy(cmdline, "echo \"add HiStorRAID");
				gcvt(atoi(index) +1, 10, mappingIndex);
				strcat(cmdline, mappingIndex);
				strcat(cmdline, " ");
				strcat(cmdline, mappingIndex);
				strcat(cmdline, "\">");
				strcat(cmdline, dir.c_str());
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					//cout<<"add device error"<<endl;
					return;
				}

				//add devices in Default
				int iDeviceBegin = 0;
				int iDeviceEnd = 0;

				chdir(workingDir);
				system("cat /proc/scsi_tgt/groups/Default/devices > devices");
				fp.open("devices", ios::in);
				//fp.open("/proc/scsi_tgt/groups/Default/devices", ios::in);
				getline(fp,buffer);

				while(!fp.eof())
				{
					getline(fp,buffer);
					//cout<<buffer<<endl;
					iDeviceBegin = buffer.find("HiStorRAID",0);
					if(iDeviceBegin == string::npos)
					{
						continue;
					}
					iDeviceEnd = buffer.find(" ", iDeviceBegin);
					sSubString = buffer.substr(iDeviceBegin, iDeviceEnd - iDeviceBegin);

					strcpy(cmdline, "echo \"add ");
					strcat(cmdline, sSubString.c_str());
					strcat(cmdline, " ");
					//cout<<"sSubString="<<sSubString<<endl;
					strcat(cmdline, sSubString.substr(10,sSubString.size() -10).c_str());
					strcat(cmdline, "\">/proc/scsi_tgt/groups/group");
					strcat(cmdline, popedom[i]);
					strcat(cmdline, "/devices");
					try
					{
						system(cmdline);
					}
					catch(int)
					{
						//cout<<"cmdline add devices in Default error"<<endl;
						return;
					}					
				}
				fp.close();
				chdir(workingDir);
			}
		}
	}
}


/************************************************************************
 *作者：Ivan.Su
 *功能：停止映射
 *参数：无
 *返回值：无
 ************************************************************************/
void SingleRaid :: unMapping ()	//设备的名字 
{
	char cmdline[200];
	char  mappingIndex[10];
	char dir[100];
	string buffer;
	ifstream fp;

	strcpy(cmdline,"echo \"close HiStorRAID");
	gcvt(atoi(index) + 1, 10, mappingIndex);
	strcat(cmdline,mappingIndex);
	strcat(cmdline," ");	
	strcat(cmdline,devName);
	strcat(cmdline,"\" > /proc/scsi_tgt/vdisk/vdisk");
	try
	{
		system(cmdline);	//解除映射
	}
	catch(int)
	{
		//cout<<"cmdline delete devcie in default is wrong"<<endl;
		return;
	}
/*	
	if(!strcmp(popedom[0], "all"))
	{
		DIR   *director; //定义目录		
		struct   dirent   *dt;//目录结构体
		
		strcpy(dir, "/proc/scsi_tgt/groups/");

		if((director=opendir(dir)) == NULL)//打开目录
		{   
			printf("opendir   %s/%s   error\n");   
			return;   
		}   
		
		chdir(dir);
	
		while((dt=readdir(director))!=NULL)
		{		
			if(!strcmp(dt->d_name , ".") || !strcmp(dt->d_name, "..")||!strcmp(dt->d_name, "Default"))	// 目录为.和..以及Deault时退出本次循环
			{
				continue;
			}
			strcpy(dir, "/proc/scsi_tgt/groups/");
			strcat(dir, dt->d_name);
			strcat(dir, "/devices");
			
			fp.open(dir, ios::in);
			
			getline(fp, buffer);
			getline(fp, buffer);
			getline(fp, buffer);

			if(buffer == "")
			{
				//删除映射号为0的设备
				strcpy(cmdline, "echo \"del histor\">/proc/scsi_tgt/groups/");
				strcat(cmdline, dt->d_name);
				strcat(cmdline, "/devices");
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					//cout<<"del histor error!"<<endl;
					return;
				}

				//删除组
				strcpy(cmdline, "echo \"del_group ");
				strcat(cmdline, dt->d_name);
				strcat(cmdline, "\" > /proc/scsi_tgt/scsi_tgt");
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					//cout<<"del_group error!"<<endl;
					return;
				}			
			}				
		}
	}
	else
	{
		for(int i = 0;i < popedomNum; i++)	
		{
			strcpy(dir, "/proc/scsi_tgt/groups/group");
			strcat(dir, popedom[i]);
			strcat(dir, "/devices");

			fp.open(dir, ios::in);
			getline(fp, buffer);
			getline(fp, buffer);
			getline(fp, buffer);

			if(buffer == "")
			{
				strcpy(cmdline, "echo \"del_group group");
				strcat(cmdline, popedom[i]);
				strcat(cmdline, "\" > /proc/scsi_tgt/scsi_tgt");
				try
				{
					system(cmdline);
				}
				catch(int)
				{
					return;
					//cout<<"del_group error!"<<endl;
				}			
			}	
		}	
	}
*/
}


void SingleRaid::encrypt()
{
/*----reserved for later when we want to do the encrypt jobs to satisfy the user's need!---*/
}


void SingleRaid::addBinding(char * value)
{
	strcpy(binding[bindingNum],value);
	bindingNum++;
}

void SingleRaid::addPopedom(char * name)
{
	strcpy(popedom[popedomNum],name);
	popedomNum++;
}

bool SingleRaid::operator==(SingleRaid& sr)
{
	if((!strcmp(index,sr.index)) && (!strcmp(level,sr.level)))
		return true;
	else
		return false;
}

int SingleRaid::getRaidDiskIndex(Disk& d)
{
	for(int i = 0;i < raidDiskNum;i++)
	{
		if(d == raidDisks[i])
			return i;
	}
	return -1;
}

int SingleRaid::getSpareDiskIndex(Disk& d)
{
	for(int i = 0;i < spareDiskNum;i++)
	{
		if(d == spareDisks[i])
			return i;
	}
	return -1;
}

/************************************************************************
 *作者：Ivan.Su
 *功能：处理热备盘，检测热备盘是否转换为数据盘
 *参数：无
 *返回值：无
 ************************************************************************/
void SingleRaid::dealWithSingleRaidSpareDisk()
{
	chdir(workingDir);

	ifstream fp;
	system("cat /proc/mdstat > mdstat");
	fp.open("mdstat",ios::in);
	
	string buffer, raidName;
	while(!fp.eof())
	{
		getline(fp,buffer);
		
		if(buffer.find("unused devices:",0) !=string::npos) //阵列状态检测完成，则退出
		{
			break;
		}

		if(buffer.find("md",0) == string::npos)
		{
			continue;
		}
		
		raidName ="/dev/md" + buffer.substr(buffer.find("md", 0)+2, buffer.find(" ",buffer.find("md", 0))-2);

		if(raidName == devName)	 //当前字符串中存储的是当前设备的信息
		{
			if(buffer.find("inactive",0) != string::npos)	 //阵列已损坏
			{
				raidstat = 40;
				return;
			}
			
			if(strcmp(level,"0"))	//当前阵列不为RAID0
			{
				for(int i = 0; i < spareDiskNum; i++)	//检测每个热备盘
				{
					if(spareDisks[i].status != "2" &&buffer.find(spareDisks[i].devName,0) != string::npos)
					{
						int index1 = buffer.find("(S)",buffer.find(spareDisks[i].devName,0));	//以设备名为起始地址的第一个(S)的起始地址
						int index2 = buffer.find("]",buffer.find(spareDisks[i].devName, 0))+1;//如果当前设备为热备盘，理论上(S)的起始地址
						if(index1 != index2)
						{
							strcpy(spareDisks[i].isSpareDisk, "0");
							addRaidDisk(spareDisks[i]);
							removeDiskInSingleRaid(spareDisks[i]);
						}
					}
				}	//for(int i = 0; i < spareDiskNum; i++)
			}	//if(strcmp(level,"0"))
		}	//if(raidName == devName)
	}
}

#endif
