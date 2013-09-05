/*		
**		disk.h  describe the disk's attribute! Include the property and the opertions
**		You can change it (include add or remove some of the opertions to satisfy your need)
*/		

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

#include "global.h"
#ifndef  _DISK_H_
#define  _DISK_H_
double atof(const char *nptr);
class Disk {
public:
	char type[10];  
	char status[2]; 
	char sn[30];
	char vendor[30];
	char capacity[30];
	char devName[10];
	char host[10];
	char channel[5];
	char scsiID[5];
	char lun[5];
	char isSpareDisk[2];
	Disk();
	void setAttribute(char * device);  
	bool getSnByDevName();
	bool getVenByDevName();
	bool getCapByDevName();
	bool getScsiIDByDevName();
	//bool refreshStatus();
	bool operator==(Disk& d);
};

Disk::Disk()
{
	strcpy(type,"SATA");
	strcpy(status,"0");
	strcpy(sn,"");
	strcpy(capacity,"");
	strcpy(vendor,"");
	strcpy(devName,"");
	strcpy(host,"");
	strcpy(channel,"");
	strcpy(scsiID,"");
	strcpy(lun,"");
	strcpy(isSpareDisk,"0");
}

void Disk::setAttribute(char * device)
{
	strcpy(devName,device);
	getScsiIDByDevName();
	getSnByDevName();
	getVenByDevName();
	getCapByDevName();
}

bool Disk::getSnByDevName()
{

	chdir(workingDir);
/*	
	char buffer[200];
	
	strcpy(buffer,"hdparm -i ");
	strcat(buffer,devName);
	strcat(buffer," > sn");
	system(buffer);
	FILE *fpsn = fopen("sn","r");
	while(fgets(buffer,200,fpsn) != NULL)
	{
		char * p = strstr(buffer,"SerialNo");
		if(p == NULL)
			continue;
		strcpy(sn,p + 9);
		sn[strlen(sn) - 1] = '\0';
		fclose(fpsn);
		system("rm -f sn");
		return true;
	}
	fclose(fpsn);
	system("rm -f sn");
	return false;
*/
        char c='\0';
        int i=0;
        char buffer[200];
        strcpy(buffer,"sg_inq  ");
        strcat(buffer,devName);
        strcat(buffer,"  |grep \"Unit serial number\"  |awk \'{print $4}\' >sn");
        system(buffer);
        FILE *fpsn =fopen("sn","r");
        while((c=getc(fpsn))!=EOF)
        {
                sn[i]=c;
                i++;
        }
        sn[i-1]='\0';
		sn[i]='\0';
        //printf("%s",sn);
        system("rm -f sn");
		return true;

	
}

bool Disk::getVenByDevName()
{
	chdir(workingDir);
	char buffer[100];
	strcpy(buffer,"sg_inq ");
	strcat(buffer,devName);
	strcat(buffer," > vendor");
	system(buffer);
	FILE * fp = fopen("vendor","r");
	while(fgets(buffer,100,fp) != NULL)
	{
		if(strncmp(buffer," Vendor",7))
			continue;
		strcpy(vendor,buffer + 24);
		vendor[strlen(vendor) - 1] = '\0';	
		fclose(fp);
		system("rm -f vendor");
		return true;
	}
	fclose(fp);
	system("rm -f vendor");
	return false;
}

bool Disk::getCapByDevName()
{
	chdir(workingDir);
/*
	char buffer[100];
	strcpy(buffer,"sg_readcap ");
	strcat(buffer,devName);
	strcat(buffer," > capacity");
	system(buffer);
	FILE * fp = fopen("capacity","r");
	while(fgets(buffer,100,fp) != NULL)
	{
		char * p = strstr(buffer,"MiB");
		if(p == NULL)
			continue;
		strcpy(capacity,p + 5);
		capacity[strlen(capacity) - 1] = '\0';	
		fclose(fp);
		system("rm -f capacity");
		return true;
	}
	fclose(fp);
	system("rm -f capacity");
	return false;
*/
	char buffer[100];
	char i=0;
	char c='\0';
	double size=0;
	string sDiskSize="";
	strcpy(buffer,"sg_readcap ");
	strcat(buffer,devName);
	strcat(buffer," |grep \"Device size\" |awk \'{print $3}\' > capacity");
	system(buffer);
	FILE * fp = fopen("capacity","r");
	while((c=getc(fp)) !=EOF )
	{
		capacity[i]=c;
		i++;
	}
	capacity[i-1]='\0';
	capacity[i]='\0';
	fclose(fp);
	size=atof(capacity)/1024/1024/1024 ;
	gcvt(size,10,capacity);
	sDiskSize=capacity;
	if(sDiskSize.find(".",0)!=string::npos)
	{
		sDiskSize =sDiskSize.substr(0,sDiskSize.find(".",0)+2) +"GB";
	}
	strcpy(capacity,(char *)sDiskSize.c_str());
	system("rm -f capacity");
	return true;



}

/*
bool Disk::refreshStatus()
{
	chdir(workingDir);
	char buffer[100],buffer1[200],*buffer2,buffer3[12],name[5];
	strcpy(name,devName+5);
	FILE *fpraid=fopen("/proc/mdstat","r");

	//�鿴�����Ƿ�Ϊ����
	while(fgets(buffer1,200,fpraid) != NULL)
	{	
		//��ǰ�ַ����в����ڡ�raid���Ӵ������˳�����ѭ��
		if(strncmp(buffer1,"raid",4)) 
			continue;
		//�ڵ�ǰ�ַ������ҵ����豸��
		if(strstr(buffer1,name) != NULL)
		{
			buffer2 = strstr(buffer1,name);
			strncpy(buffer3,buffer2,11);
			
			//��ǰ�̱����Ϊ����
			if(strstr(buffer3,"(F)") !=NULL)
			{
				strcpy(status,"2");
				fclose(fpraid);
				return false;
			}
		}
	}
	fclose(fpraid);

	//��ǰ���̲���������
	
	strcpy(buffer,"hdparm -i ");
	strcat(buffer,devName);
	strcat(buffer," > status");
	system(buffer);	//�����豸��������к�
	FILE *fpstatus = fopen("status","r");
	while(fgets(buffer,100,fpstatus) != NULL)
	{
		char * p = strstr(buffer,"SerialNo");
		//��ǰ�ַ����в��������к�
		if(p == NULL)
			continue;

		if(!strncmp(sn,p + 9,strlen(sn)))	 //�ַ������кź�ԭ����ȣ�������ǰ״̬��
		{
			strcpy(status,"0");
		}
		else if((strncmp(sn,p + 9,strlen(sn)))  && (status==0))	//���кŲ���ȣ���״̬Ϊ����
		{
			strcpy(sn,p+9);
			sn[strlen(sn) - 1] = '\0';
		}
		else	 //���кŲ���ȣ���״̬Ϊ������
		{
			strcpy(status,"1");
		}
		system("rm -f status");
		fclose(fpstatus);
		return true;
	}
	fclose(fpstatus);
	//û�в鵽������Ϣ
	system("rm -f status");
	strcpy(status,"2");
	return false;
}

*/
/***********************************************************************
 *���ߣ�Ivan.Su
 *���ܣ������豸����ô���ID��
 *��������
 *����ֵ��1���ɹ����ID��0����ȡIDʧ��
 ***********************************************************************/
bool Disk::getScsiIDByDevName ()
{
	chdir(workingDir);

	string cmdline;
	string buffer;
	ifstream  fp;

	string virtualID;
	string fileName;
	int iScsiID;

	//�����豸����ø��豸�����ID��
	cmdline="sg_scan ";
	cmdline+=devName;
	cmdline+=" > virtualID";	

	system((char *)cmdline.c_str());

	fp.open("virtualID",ios::in);
	if(!fp)	//���ļ����󣬻��ļ�������
	{
		return 0;
	}

	getline(fp,buffer);
	
	if(buffer.find(devName,0) != string::npos)	//��ǰ�豸ͨ��sg_scan�ܹ��ҵ� 
	{
		virtualID = "[DISK ";
		//virtualID += buffer.substr(buffer. find("scsi",buffer.find("scsi")+5)+4,1);
		virtualID += buffer.substr(buffer.find("scsi",18)+4,1);
		int scsi=atoi(buffer.substr(buffer.find("scsi",18)+4,1).c_str());
		virtualID += "_";
		virtualID += buffer.substr(buffer.find("scsiID=",0)+7,1);
		int id= atoi(buffer.substr(buffer.find("scsiID=",0)+7,1).c_str());
		virtualID += "]";
		fp.close();
		system("rm -f virtualID");

		if(scsi==0)
			iScsiID=id;
		else if(scsi==1)
			iScsiID=id+8;

	}
	else
	{
		//cout<<"this devName is not exist"<<endl;

		fp.close();
		system("rm -f virtualID");
		return 0;
	}

	
	gcvt(iScsiID,10,scsiID);
/*
	cmdline=" ls /proc/scsi/hp1108/ > fileList";
	system((char *)cmdline.c_str());

	fp.open("fileList",ios::in);
	if(!fp)	//���ļ����󣬻��ļ�������
	{
		//cout<<"file fileList wrong"<<endl;
		return 0;
	}

	getline(fp,buffer);
	fileName = buffer;	//��õ�ǰ�� /proc/scsi/hp1108/���ڵĵ�һ���ļ���
	//cout<<"filenam="<<fileName<<endl;

	fp.close();
	system("rm -f fileList");

	cmdline = "cat /proc/scsi/hp1108/" + fileName + ">scsiID";
	//cout<<cmdline<<endl;
	system((char *)cmdline.c_str());

	fp.open("scsiID",ios::in);
	if(!fp)	//���ļ����󣬻��ļ�������
	{
		//cout<<"file scsiID wrong"<<endl;
		return 0;
	}

	while(!fp.eof())
	{
		getline(fp,buffer);
		if(buffer.find(virtualID,0) != string::npos)
		{
			strcpy(host,(char *)buffer.substr(buffer.find("/",0)-1,1).c_str());
			iScsiID = (atoi(host)-1)*7+atoi((char *)buffer.substr(buffer.find("/",0)+1,1).c_str());
                  	gcvt(iScsiID,10,scsiID);
			
			fp.close();
//			system("rm -f scsiID");
			return 1;
		}
	}	//while

	fp.close();
//	system("rm -f scsiID");
*/	return 0;
}


bool Disk::operator==(Disk& d)
{
	if(!strcmp(sn,d.sn))
		return true;
	else
		return false;
}

#endif
