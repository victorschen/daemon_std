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


//���豸�ĺ�:
#define bind_dev(index, channel_no)  int rtval;\
	string cmdbind;\
	cmdbind="echo \"binding HiStorRAID"+index+" "+channel_no+"\"> /proc/scsi_tgt/vdisk/vdisk";\
	rtval=system(cmdbind.c_str());\
	if(rtval<0){\
	cout<<"ERROR:execute"<<endl;\
	exit(-1);\
	}\
	sleep(1)//���豸

//������豸�ĺ�:
#define unbind_dev(index, channel_no)  int rtval;\
	string cmdunbind;\
	cmdunbind="echo \"unbinding HiStorRAID"+index+" "+channel_no+"\"> /proc/scsi_tgt/vdisk/vdisk";\
	rtval=system(cmdunbind.c_str());\
	if(rtval<0){\
	cout<<"ERROR:execute"<<endl;\
	exit(-1);\
	}\
	sleep(1)//���豸



/***************************************************************
* ����: ��ȡ�ַ���
* ����: IP-IP-����channelno-channelno-
* ���: string����
* ����: zjf
* ����: 2007-12-6
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
	//����Ѵ��������еĴ����Ƿ����
	for(int j = 0;j < rc.singleRaidNum;j++)
	{
		//��������е�������
		for(int k = 0;k < rc.singleRaids[j].raidDiskNum; k++)
		{
			if(strcmp(rc.singleRaids[j].raidDisks[k].status, "2"))		//��ǰ�̲�Ϊ����
			{
				int currentflag = 1;
				//���̴��ڣ�����currentflagΪ0
				for(int i = 0;i < da.diskNum;i++)
				{
					if(rc.singleRaids[j].raidDisks[k] == da.array[i])
					{
						currentflag = 0;
						break;
					}
				}
				//�����е��������̶�ʧ�������Ƴ�����
				if(currentflag)
				{
					strcpy(rc.singleRaids[j].raidDisks[k].status, "2");
						//rc.singleRaids[j].removeDiskInSingleRaid(rc.singleRaids[j].raidDisks[k]);
						//k--;
				}
			}	//if(strcmp(rc.singleRaids[j].raidDisks[k].status, "2"))
		}	//for(int k = 0;k < rc.singleRaids[j].raidDiskNum; k++)
			
		//������е��ȱ���
		for(int k = 0;k < rc.singleRaids[j].spareDiskNum;k++)
		{
			if(strcmp(rc.singleRaids[j].spareDisks[k].status, "2"))		//��ǰ�̲�Ϊ����
			{
				int currentflag = 1;
				for(int i = 0;i < da.diskNum;i++)
				{
					if(rc.singleRaids[j].spareDisks[k] == da.array[i]){
						currentflag = 0;
						break;
					}
				}
				//�ֲ��ȱ��̶�ʧ�����Ƴ�����
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

	//���xml�з���������Ĵ����Ƿ���Ȼ����

	for(int j = 0;j < rc.diskNum; j++)
	{
		if(strcmp(rc.disks[j].status, "2"))	//��ǰ���̲�Ϊ����
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
			//���̶�ʧ���Ƴ�����
			if(currentflag)
			{
				//cout<<"lost a disk"<<endl;
				strcpy(rc.disks[j].status, "2");
				//rc.removeDisk(rc.disks[j]);
				//j--;
			}
		}	//if(strcmp(rc.disks[j].status, "2"))
	}
		
	//����������
	//cout<<"check start"<<endl;
	for(int i = 0;i < da.diskNum;i++)
	{
		//cout<<"diskarray    "<<da.array[i].devName<<"    "<<da.array[i].scsiID<<"    "<<da.array[i].sn<<endl;
		//��������д����Ƿ�������
		for(int j = 0;j < rc.singleRaidNum;j++)
		{
			//��������е��������Ƿ�������
			for(int k = 0;k < rc.singleRaids[j].raidDiskNum;k++)
			{
				//�����˴������Σ��������Ӧ�Ĵ�����Ϣ
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

			//��������еľֲ��ȱ����Ƿ�������
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
			
		//���������еĴ����Ƿ�������
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
 *���ߣ�Ivan.Su
 *���ܣ�������ת����Ϊ�ַ���
 *������i����ת������
 *����ֵ������i����Ӧ���ַ���
 ************************************************************************/
string intToString(int i)
{ 
	stringstream s; 
	s << i; 
	return s.str(); 
}


/***********************************************************************
 *���ߣ�Ivan.Su
 *���ܣ����д��־�����ں�ʱ��
 *��������
 *����ֵ��д��־�����ں�ʱ���ַ���
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

	//��
	logYear = intToString(1900 + p->tm_year);
	//��
	logMon = intToString(1 + p->tm_mon);
	//��
	logMDay = intToString(p->tm_mday);

	logDateAndTime = logYear + "-" + logMon + "-" + logMDay + "|";

	//ʱ
	logHour = intToString(p->tm_hour);
	//��
	logMin = intToString(p->tm_min);
	//��
	logSec = intToString(p->tm_sec);

	logDateAndTime = logDateAndTime + logHour + ":" + logMin + ":" + logSec + "|";
	return logDateAndTime;
}


/***********************************************************************
 *���ߣ�Ivan.Su
 *���ܣ�д������־
 *������logInfor����д��������־�ļ���һ����־
 *����ֵ����
 ***********************************************************************/
void writeLog(string logInfor)
{
	int logLines =0;
	ifstream fpLog1;
	string buffer;

	chdir(workingDir);

	if(access("log",0))	//���log�ļ�������,����log
	{
		ofstream fp("log");
		fp.close();
	}

	system("mv -f log log1"); //��log������Ϊlog1	
	ofstream fpLog("log");	//�����µ�log�ļ�
	fpLog1.open("log1", ios::in);	

	fpLog<< logInfor;	//д��־
	logLines ++;

	while(!fpLog1.eof())	 //���ɵ���־��ӵ�log�ļ���
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
 *���ߣ�Ivan.Su
 *���ܣ���ش��̣�����Ƿ��е��̺ͼ��̣��������ȱ���
 *������da���������飬���浱ǰ������Ϣ��rc�����浱ǰ�������е���Ϣ
 *����ֵ����
 ***********************************************************************************************/

void DiskMonitor(DiskArray& da,RaidConfig& rc)
{
	chdir(workingDir);
	bool confIsChanged =0;

	DiskArray da1;
	da1.fillArray();
	da = da1;
	//da.refreshDiskArray();	//���´�������

	//����Ƿ��е��̻����

	//��������Ƿ��е���
	for(int i = 0; i < rc.singleRaidNum;i++)
	{
		for(int j = 0; j < rc.singleRaids[i].raidDiskNum; j++)	 //�������е�ÿ�������̽��м��
		{
			if (strcmp(rc.singleRaids[i].raidDisks[j].status, "2"))	 //�����ǰ�̲�Ϊ����
			{
				bool bLostSign = 1;

				for(int k = 0; k < da.diskNum; k++)	//��da�в��Ҵ����Ƿ����
				{
					if(da.array[k] == rc.singleRaids[i].raidDisks[j])
					{
						bLostSign = 0;
						break;
					}
				}	//for(int k = 0; k < da.diskNum; k++)
					
				if(bLostSign)	//���̶�ʧ
				{
					strcpy(rc.singleRaids[i].raidDisks[j].status,"2");

					confIsChanged = 1;

					string logInfor = getLogDateAndTime() + "DISKFAULTY|�д����𻵻�γ�,���Ϊ:  "+ rc.singleRaids[i].raidDisks[j].scsiID;
					writeLog(logInfor);

					//�������
					if(rc.singleRaids[i].raidstat == 50 && !strcmp(rc.singleRaids[i].level, "5") )
					{
						rc.singleRaids[i].raidstat = 40;
						confIsChanged = 1;
						string logInfor;
						logInfor = getLogDateAndTime();
						logInfor += "DISKFAULTY|����";
						logInfor += rc.singleRaids[i].index; 
						logInfor += "�Ѿ��𻵣���������ʹ�ã���ɾ��������";
						writeLog(logInfor);

					}
					
				}	//if(bLostSign)
			}	//if(strcmp(rc.singleRaids[i].raidDisks[j].status, "2"))
		}	//for(int j = 0; j < rc.singleRaids[i].raidDiskNum;j++)

		for(int j = 0; j < rc.singleRaids[i].spareDiskNum;j++)	 //�������е�ÿ���ȱ��̽��м��
		{
			if (strcmp(rc.singleRaids[i].spareDisks[j].status, "2"))	 //�����ǰ�̲�Ϊ����
			{
				bool bLostSign = 1;

				for(int k = 0; k < da.diskNum; k++)	//��da�в��Ҵ����Ƿ����
				{
					if(da.array[k] == rc.singleRaids[i].spareDisks[j])
					{
						bLostSign = 0;
						break;
					}
				}	//for(int k = 0; k < da.diskNum; k++)
				
				if(bLostSign)	//���̶�ʧ
				{
					strcpy(rc.singleRaids[i].spareDisks[j].status,"2");
					confIsChanged = 1;
					string logInfor = getLogDateAndTime() + "DISKFAULTY|�д����𻵻�γ�,���Ϊ:  "+rc.singleRaids[i].spareDisks[j].scsiID;
					writeLog(logInfor);
				}	//if(bLostSign)
			}	//if (strcmp(rc.singleRaids[i].spareDisks[j].status, "2"))
		}	//if (rc.singleRaids[i].spareDisks[j].status != 2)
	}	//for(int i = 0; i < rc.singleRaidNum;i++)

	//���������Ĵ����Ƿ����
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
				string logInfor = getLogDateAndTime() + "DISKFAULTY|�д����𻵻�γ�,���Ϊ:  "+rc.disks[i].scsiID;
				writeLog(logInfor);
			}
		}	//if(strcmp(rc.disks[i].status, "2")
	}	//for(int i = 0; i < rc.diskNum; i++)

	//����Ƿ����¼���
	for(int i = 0; i < da.diskNum; i++)
	{
		//cout<<da.array[i].devName<<"  "<<da.array[i].scsiID<<"   "<<da.array[i].sn<<endl;
		bool isNewDisk =1;

		//���������������Ƿ����
		for(int j = 0; j < rc.singleRaidNum; j++)
		{
			for(int k = 0;k < rc.singleRaids[j].raidDiskNum;k++)
			{
				if(!strcmp(da.array[i].sn, rc.singleRaids[j].raidDisks[k].sn))	//��ǰ������singleRaid�д���
				{ 
					if(!strcmp(rc.singleRaids[j].raidDisks[k].status,"2"))	 //�������������Ѿ����Ϊ��
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
				else if(!strcmp(da.array[i].scsiID, rc.singleRaids[j].raidDisks[k].scsiID))	//�������Ӵ��̵�scsiIDԭ������
				{
					rc.singleRaids[j].hotRemoveDisk(rc.singleRaids[j].raidDisks[k]);
					k--;
				}
			}	//for(int k = 0;k < rc.singleRaids[j].raidDiskNum;k++)

			if(!isNewDisk)	 //��������¼��̣����˳�ѭ��
			{
				break;
			}

			for(int k = 0; k < rc.singleRaids[j].spareDiskNum; k++)
			{
				if(da.array[i] == rc.singleRaids[j].spareDisks[k])	//��ǰ������singleRaid�д���
				{
					if(!strcmp(rc.singleRaids[j].spareDisks[k].status,"2"))	 //�������������Ѿ����Ϊ��
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
				else if(!strcmp(da.array[i].scsiID, rc.singleRaids[j].spareDisks[k].scsiID))	//�������Ӵ��̵�scsiIDԭ������
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
			if(!strcmp(da.array[i].sn, rc.disks[j].sn))	//���������������
			{
				if(!strcmp(rc.disks[j].status,"2"))	//�����Ѿ�����Ϊ����
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
			else if(!strcmp(da.array[i].scsiID, rc.disks[j].scsiID))	//����Ӵ��̵�scsiID��rc�б��ҵ�
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
			string logInfor = getLogDateAndTime() + "DISKADD|���µĴ��̲��룬���Ϊ:  "+da.array[i].scsiID;
			writeLog(logInfor);
			//cout<<logInfor<<endl;
		}
	}	//for(int i = 0; i < da.diskNum; i++)


	//�����ȱ�����Ϣ
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
 *���ߣ�Ivan.Su
 *���ܣ�������ز����������/ɾ������
 *������rc�����浱ǰ�������е���Ϣ
 *����ֵ����
 ***********************************************************************************************/
void raidConfig(RaidConfig& rc, DiskArray& da)
{
	chdir(workingDir);

	if((!access("raidConfigNew.xml",0)) && (!access("restore",0)))
	{
		system("rm -f restore");
		//int m=rc.singleRaidNum;

		//ɾ����ǰ��������
		for(int i =0; i < rc.singleRaidNum; i++)
		{
			rc.singleRaids[i].stop();		//ֹͣ����
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

		//�����Ƿ����´�������
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
		//�����Ƿ���ɾ������
		for(int i = 0;i < rc.singleRaidNum; i++)
		{
			//cout<<"i = "<<i<<endl;
			if(rc1.getSingleRaidIndex(rc.singleRaids[i]) == -1)
			{
				rc.singleRaids[i].stop();
				//usleep(100000);
				string logInfor = getLogDateAndTime() + "DISKFAULTY|�����б�ɾ�������Ϊ��"+ rc.singleRaids[i].index;		
				writeLog(logInfor);	//д��־
				rc.removeSingleRaid(rc.singleRaids[i]);
				i--;
				
			}
		}
		//cout<<"search for deleted raid end"<<endl;
		
		for(int i = 0;i < rc.singleRaidNum;i++)
		{
			int j = rc1.getSingleRaidIndex(rc.singleRaids[i]);
			if(j != -1)	//��ǰ����û�б�ɾ��
			{
				//��������е��������Ƿ�ɾ��
				for(int k = 0;k < rc.singleRaids[i].raidDiskNum;k++)
				{
					if(rc1.singleRaids[j].getRaidDiskIndex(rc.singleRaids[i].raidDisks[k]) == -1)
					{
						rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].raidDisks[k]);
					/*

						if(!strcmp(rc.singleRaids[i].raidDisks[k].status, "2"))	 //���ɾ���������𻵵Ĵ���
						{
							rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].raidDisks[k]);
						}
						else	 //ɾ�����Ǻ���
						{
							rc.singleRaids[i].hotRemoveDisk(rc.singleRaids[i].raidDisks[k]);
						}
					*/
						k--;
					}
				}

				//��������е��ȱ����Ƿ�ɾ��
				for(int k = 0;k < rc.singleRaids[i].spareDiskNum;k++)
				{
					if(rc1.singleRaids[j].getSpareDiskIndex(rc.singleRaids[i].spareDisks[k]) == -1)
					{
						rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].spareDisks[k]);
					/*
						if(!strcmp(rc.singleRaids[i].spareDisks[k].status, "2"))	//ɾ�������𻵵Ĵ���
						{
							rc.singleRaids[i].removeDiskInSingleRaid(rc.singleRaids[i].spareDisks[k]);
						}
						else	 //ɾ�����Ǻ���
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
					if(rc.singleRaids[i].getRaidDiskIndex(rc1.singleRaids[j].raidDisks[k]) == -1)	//�����һ��������
					{
						int spareDiskIndex = rc.getDiskIndex(rc1.singleRaids[j].raidDisks[k]);
						strcpy(rc.disks[spareDiskIndex].isSpareDisk, "1");

						//rc.singleRaids[i].addSpareDisk(rc.disks[spareDiskIndex]);
						rc.removeDisk(rc1.singleRaids[j].raidDisks[k]);

						rc.singleRaids[i].hotAddDisk(rc1.singleRaids[j].raidDisks[k]);
					}
				}
*/
				//����Ƿ�������ȱ���
				for(int k = 0;k < rc1.singleRaids[j].spareDiskNum;k++)
				{
					if(rc.singleRaids[i].getSpareDiskIndex(rc1.singleRaids[j].spareDisks[k]) == -1)	//������ȱ���
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
 *���ߣ�Ivan.Su
 *���ܣ����Raid10�Ƿ���
 *������stateString����raidstat�ж�����Raid10��״̬�ַ���
 *����ֵ��1���𻵣����в����ã�0�������Կ���
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
 *���ߣ�Ivan.Su
 *���ܣ����ݴ�raidstat�ж��������е�״̬�ַ�����������е�״̬
 *������raidInfor��raidstat��������Ϣ�ĵڶ��У�raidLevel�����еļ���
 *����ֵ��10��������40���𻵣�50������
 ******************************************************************/
int getRaidStat(string raidInfor, string raidLevel)
{
	int stat = -1;	//��ǰ���е�״̬
	int iRaidLevel = atoi(raidLevel.c_str());	//��ǰ���еļ���
	int lastCharacter = raidInfor.size();	//��ŵ�ǰ�ַ����ĳ���

	int U_pos = raidInfor.rfind("[",lastCharacter)+1; //���U�ַ�������ʼλ��
	string raidStateString = raidInfor.substr(U_pos, lastCharacter - U_pos - 1);	//�������U�ַ���
	//cout<<raidStateString<<endl;

	int iUNum=0;					
	int i_Num=0;
						
	//����ַ�����U��_�ĸ�����Ϊ��ȡraidstat��׼��
	for(int i=0;i<raidStateString.size();i++)
	{	
		if(raidStateString.substr(i,1)=="U")
			iUNum++;
		else 
			i_Num++;
	}

	switch(iRaidLevel)
	{
	//����ΪRAID1,
	case 1:		
		if(iUNum==0)	//ȫ����Ϊ���̣�������Ϊ��״̬
		{
			stat = 40;
		}
		else
		{
			if(i_Num>=1)	//�����𻵣�������Ϊ����״̬
			{
				stat = 50;
			}
			else	//ȫ��������������Ϊ����״̬
			{
				stat = 10;
			}
		}
		break;
	case 4:
	case 5:
		if(i_Num>=2)	//�������Լ��������ϵĻ��̣�������Ϊ��״̬
		{
			stat = 40;
		}
		else
		{
			if(i_Num>=1) //ֻ��һ�����̣�������Ϊ����״̬
			{
				stat = 50;
			}
			else	//û�л��̣�������Ϊ����״̬
			{
				stat = 10;
			}
		}
		break;
	//����ΪRAID6
	case 6:
		if (i_Num > 2)
		{
			stat = 40;
		}
		else
		{
			if(i_Num >= 1)	//��һ�����ϻ��̣�������Ϊ����״̬
			{
				stat = 50;
			}
			else	//û�л��̣�����Ϊ����״̬
			{
				stat = 10;
			}
		}
		break;
	//����ΪRAID10
	case 10:
		if(isRaid10Bad(raidStateString))	//�����һ����ͬʱ�𻵣�������Ϊ��״̬
		{
			stat = 40;
		}
		else
		{
			if(i_Num >= 1)	//��������𻵣�������Ϊ����״̬
			{
				stat = 50;
			}
			else	//û�л��̣�����Ϊ����״̬
			{
				stat = 10;
			}
		}
		break;
	}
	return stat;
}


/***********************************************************************
 *���ߣ�Ivan.Su
 *���ܣ��������е�״̬��������У�����������־
 *������rc�����е�������Ϣ
 *����ֵ����
 ***********************************************************************/
void RaidMonitor(RaidConfig& rc)
{
	ifstream  fpRaidStat;
	string buffer[3];	//���һ�����е���Ϣ
	bool confIsChanged = 0;

	chdir(workingDir);

	system("cat /proc/mdstat > mdstat");	//�õ�����״̬��Ϣ�������raidstat��
	fpRaidStat.open("mdstat",ios::in);
	if(!fpRaidStat)	//���ļ����󣬻��ļ�������
	{
		//cout<<" mdstat file io wrong"<<endl;
		return;
	}
	//cout <<"RaidMonitor"<<endl;
	getline(fpRaidStat,buffer[0]);	 //���ļ�ָ���ƶ����ڶ���

	while(!fpRaidStat.eof())
	{
		getline(fpRaidStat,buffer[0]); 

		if(buffer[0].find("unused devices:",0) != string::npos) //����״̬�����ɣ����˳�
		{
			break;
		}

		if(buffer[0].find("md", 0) == string::npos)
		{
			continue;
		}

		if(string::npos != buffer[0].find("inactive",0))//�������Ϊ��״̬
		{
			string raidIndex = buffer[0].substr(buffer[0].find("md", 0)+2, buffer[0].find(" ",2)-2);	//��õ�ǰ��������

			//��rc���ҵ���ǰ����
			for(int i = 0; i < rc.singleRaidNum; i++)	
			{
				if(!strcmp(rc.singleRaids[i].index,(char *)raidIndex.c_str()))	 
				{
					if (rc.singleRaids[i].raidstat != 40)	//���л�û�б����Ϊ��
					{
						
						rc.singleRaids[i].raidstat = 40;
						confIsChanged =1;
						//д��־
						string logInfor;
						logInfor = getLogDateAndTime();
						logInfor += "DISKFAULTY|����";
						logInfor += raidIndex; 
						logInfor += "�Ѿ��𻵣���������ʹ�ã���ɾ��������";
						
						writeLog(logInfor);
					}	//if(rc.singleRaids[i].raidstat != "40")
					break;
				}	//if(rc.singleRaids[i].index == raidIndex)
			}	//for(int i = 0; i < rc.singleRaidNum; i++)

			getline(fpRaidStat,buffer[1]);
		}	//if(string::npos != buffer.find("inactive",0))
		else
		{
			getline(fpRaidStat,buffer[1]);	 //������ǰ���е�2����Ϣ
			getline(fpRaidStat,buffer[2]);	 //������ǰ���е�3����Ϣ

			int raidPos = -1;	//����ȷ�����е�λ��
			
			//����ǰ���е�1����Ϣ
			string raidIndex= buffer[0].substr(buffer[0].find("md", 0)+2,buffer[0].find(" ",2)-2);	//��õ�ǰ��������

			for(int i = 0; i < rc.singleRaidNum; i++)	//��rc���ҵ���ǰ���У������λ��
			{
				if(!strcmp(rc.singleRaids[i].index, (char *)raidIndex.c_str()))
				{
					raidPos = i;
					break;
				}	//if(rc.singleRaids[i].index == raidIndex)
			}	//for

			if (raidPos == -1)	//��ǰ������rc��û�б��ҵ�
			{
				continue;
			}

			if(rc.singleRaids[raidPos].raidstat == 40)
			{
				continue;
			}

			if(!strcmp(rc.singleRaids[raidPos].level, "0"))		//��ǰ����Ϊraid0
			{
				bool isRaid0Bad = 0;
				for(int j = 0; j < rc.singleRaids[raidPos].raidDiskNum; j++)	//�鿴raid0���Ƿ��д�����
				{
					if(!strcmp(rc.singleRaids[raidPos].raidDisks[j].status,"2"))	//raid0���л��̴���
					{
						if (rc.singleRaids[raidPos].raidstat != 40)	 //raid0��û�б�����Ϊ��
						{
							rc.singleRaids[raidPos].raidstat = 40;
							isRaid0Bad = 1;
						}
						break;
					}	//if(!strcmp(rc.singleRaids[raidPos].raidDisks[j].status,"2"))
				}	//for(int j=0;j<rc.singleRaids[raidPos].raidDiskNum;j++)

				if(buffer[0].find("(F)", 0) != string::npos)	//�д��̱���Ϊ����
				{
					if (rc.singleRaids[raidPos].raidstat != 40)	 //raid0��û�б�����Ϊ��
					{
						rc.singleRaids[raidPos].raidstat = 40;
						isRaid0Bad = 1;
					}
				}

				if(isRaid0Bad)	//��һ�μ�⵽������
				{
					confIsChanged =1;
					string logInfor;
					logInfor = getLogDateAndTime() + "DISKFAULTY|����  " + raidIndex + "�Ѿ��𻵣���������ʹ�ã���ɾ��������";
					writeLog(logInfor);
				}
				else if (rc.singleRaids[raidPos].raidstat != 40)	 //���д�������״̬
				{
					if(rc.singleRaids[raidPos].raidstat !=10)	//��һ�μ�⵽���д�������״̬
					{
						rc.singleRaids[raidPos].raidstat =10;
						confIsChanged =1;
						string logInfor;
						logInfor =getLogDateAndTime() +"RAIDSTATUS|����  " + raidIndex + "״̬����";
						writeLog(logInfor);
					}
				}
			}	//if(rc.singleRaids[raidPos].level == "0")

			//����ǰ���е�2����Ϣ
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
			//strcpy(rc.singleRaids[raidPos].raidcap, (char *)buffer[1].substr(6,GB_position-6).c_str());	//��õ�ǰ���е�����

			//�����в�Ϊraid0ʱ������ǰ���е�3���Լ���2�������Ϣ
			if(strcmp(rc.singleRaids[raidPos].level, "0"))	//�����ǰ����ΪRAID0���������Ϊ�գ�������Ҫ������������Ϣ
			{
				if(buffer[2].find("recovery =",0) != string::npos)	//��ǰ���д����ؽ�״̬
				{
					if(rc.singleRaids[raidPos].raidstat != 20)	// ��һ�μ�⵽���д����ؽ�״̬
					{
						rc.singleRaids[raidPos].raidstat = 20;	//��������״̬Ϊ�ؽ�״̬
						confIsChanged =1;
						//д��־
						string logInfor;
						logInfor = getLogDateAndTime() + "RAIDSTATUS|���� " + raidIndex + "���ڽ����ؽ� !";
						writeLog(logInfor);
					}
					strcpy(rc.singleRaids[raidPos].percent, (char *)buffer[2].substr(buffer[2].find("recovery =",0)+10, buffer[2].find("%",0)-buffer[2].find("recovery =",0)-10).c_str());	 	//�õ������ؽ��İٷֱ�
				}
				else if(buffer[2].find("resync =",0) != string::npos)	//��ǰ���д���ͬ��״̬
				{
					if(rc.singleRaids[raidPos].raidstat != 30)	// ��һ�μ�⵽���д����ؽ�״̬
					{
						rc.singleRaids[raidPos].raidstat = 30;	//��������״̬Ϊ�ؽ�״̬
						confIsChanged =1;
						//д��־
						string logInfor;
						logInfor = getLogDateAndTime() + "RAIDSTATUS|���� " + raidIndex + "���ڽ���ͬ�� !";
						writeLog(logInfor);
					}
					strcpy(rc.singleRaids[raidPos].percent, (char *)buffer[2].substr(buffer[2].find("resync =",0)+8, buffer[2].find("%",0)-buffer[2].find("resync =",0)-8).c_str());	//�õ�����ͬ���İٷֱ�
				}
				else	//���д��ڳ��ؽ���ͬ���������������״̬
				{
					int iRaidStat = getRaidStat(buffer[1], rc.singleRaids[raidPos].level ); //�õ���������״̬

					if(rc.singleRaids[raidPos].raidstat != iRaidStat)	// ����״̬�����˸ı�
					{
						confIsChanged =1;

						string logInfor;
						rc.singleRaids[raidPos].raidstat = iRaidStat;
						switch (iRaidStat)
						{
						case 10:
							logInfor = getLogDateAndTime() + "RAIDSTATUS|���� " + rc.singleRaids[raidPos].index + "��������״̬ !";
							break;
						case 40:
							logInfor = getLogDateAndTime() +"DISKFAULTY|���� " + rc.singleRaids[raidPos].index + "�Ѿ��𻵣���������ʹ�ã���ɾ�������� !";
							break;
						case 50:
							logInfor = getLogDateAndTime() + "RAIDSTATUS|���� " + rc.singleRaids[raidPos].index + "���ڽ���ģʽ !";
							break;				
						}
						writeLog(logInfor);
					}
				}
				//�õ������ؽ�/ͬ������ʱ��
				strcpy(rc.singleRaids[raidPos].time, (char *)buffer[2].substr( buffer[2].find("finish=",0)+7,buffer[2].find("min",0)-buffer[2].find("finish=",0)-7).c_str());
				//�õ������ؽ�/ͬ�����ٶ�
				strcpy(rc.singleRaids[raidPos].speed, (char *)buffer[2].substr(buffer[2].find("speed=",0)+6,buffer[2].find("K/sec",0)-buffer[2].find("speed=",0)-6).c_str());
			}	//if(raidInfo[iRaidNum].raidLevel!="0")
		}	//else		
	}	//while(!fpRaidStat.eof())
	fpRaidStat.close();

	//���raidConfig.xml�е������Ƿ���û�������ɹ���
	for(int i =0; i<rc.singleRaidNum; i++)
	{		
		string currentSingleRaid = rc.singleRaids[i].index;
		currentSingleRaid = "md"+currentSingleRaid;
		bool isRaidExist = 0;
		fpRaidStat.open("mdstat", ios::in);
		while(!fpRaidStat.eof())
		{
			getline(fpRaidStat,buffer[0]);
			
			if(buffer[0].find("unused devices:",0) != string::npos) //����״̬�����ɣ����˳�
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
				logInfor = getLogDateAndTime() +"DISKFAULTY|���� " + rc.singleRaids[i].index + "�Ѿ��𻵣���������ʹ�ã���ɾ�������� !";
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
			
		//ɾ����ǰ��������
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

		//�����־��Ϣ
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
* ����:��������
* ����: ��
* ���: ��
* ����: zjf
* ����: 2007-12-6
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


   /* if(buffer.find("mapping")!=string::npos){//�޸�ӳ��
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
		
    if(buffer.find("binding")!=string::npos){//�޸İ�
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
	// ��Ҫ�ع�XML
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
	
	//����pci��������daemon����ʱ��ȡflash�е�config��log����Ϣ
	char buffer[10];
	FILE *fp1;


	string cmdline="";
	//����ӳ���ͨ���ű����0��ʼ���������Ƚ�һ���յ��豸ӳ��Ϊ0
	system("touch /usr/local/apache/htdocs/RaidManager/Monitor/lun");
	cmdline="echo \"open histor /usr/local/apache/htdocs/RaidManager/Monitor/lun\"> /proc/scsi_tgt/vdisk/vdisk";//���豸	
	try
	{
		system(cmdline.c_str());//���豸
	}
	catch(int)
	{
		cout<<"cmdline open device is wrong"<<endl;
		return 0;
	}
	
	cmdline="echo \"add histor 0\">/proc/scsi_tgt/groups/Default/devices";
	try
	{
		system(cmdline.c_str());//����豸��Default ��
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
			//�����¿�����һ�ζ�������users.txt�ļ�Ϊ�գ�ɾ��֮
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
			//�����¿�����һ�ζ�������operation_log�ļ�Ϊ�գ�ɾ��֮
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
			//�����¿�����һ�ζ�������blacklist.txt�ļ�Ϊ�գ�ɾ��֮
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
			//�����¿�����һ�ζ�������xml�ļ�Ϊ�գ�ɾ��֮
			system("rm -f raidConfig.xml");
		}
		fclose(fp1);
	}
	//end of pci
	chdir(workingDir);
	//raidConfig.xml�ļ�������
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
		//������Ϣ���
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
