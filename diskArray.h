/*
**		diskArray.h  include all the disks(sd* device) in the system
**		This file defines some of it's attribute and opertions!
*/
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "disk.h"
#include "global.h"

using namespace std;

#ifndef  _DISKARRAY_H_
#define  _DISKARRAY_H_


class DiskArray {
public:
	Disk array[50];
	int diskNum;

	DiskArray();
	void fillArray();
	//void refreshDiskArray();
	//int getIndexBySn(char * sn);
	void removeDiskAt(int index);
};

DiskArray::DiskArray()
{
	diskNum = 0;
}

/***********************************************************************
 *���ߣ�Ivan.Su
 *���ܣ����������飬��õ�ǰ����������Ϣ
 *��������
 *����ֵ����
 ***********************************************************************/
void DiskArray::fillArray()
{
	chdir(workingDir);
	string bufferDiskName;
	int fd;
	
	diskNum =0;

	//��/proc/partitions�ж�ȡ���ڵĴ���
	system("awk \'{if($4 ~/sd.$/) print \"/dev/\"$4}\' /proc/partitions > disk");

	ifstream fp;
	fp.open("disk",ios::in);
	
	//�ҳ����ڵĴ��̵��豸��
	while(!fp.eof())
	{
		getline(fp, bufferDiskName);     
		if(bufferDiskName.find("/dev/sd", 0) == string::npos)
		{
			continue;
		}
		
 		if((fd = open((char *)bufferDiskName.c_str(),O_RDWR)) < 0)
		{
			//cout<<"Disk "<<bufferDiskName<<" is not exist"<<endl;
			close(fd);
    			continue;
  	    }
		close(fd);
		Disk d;
		d.setAttribute((char *)bufferDiskName.c_str());
		array[diskNum] = d;
		diskNum ++;
	}
	
	fp.close();
	system("rm -f disk");

	//��DiskArray��������
	Disk d;
	for(int i = 0;i < diskNum-1; i++)
		for(int j = 0; j < diskNum-i-1; j ++)
			if(atoi(array[j].scsiID) > atoi(array[j+1].scsiID))
			{
				d = array[j];
				array[j] = array[j+1];
				array[j+1] = d;
			}
//cout<<"diskNum"<<diskNum<<endl;
}

/*
int DiskArray::getIndexBySn(char * sn)
{
	for(int i = 0;i < diskNum;i++)
	{
		if(!strcmp(array[i].sn,sn))
			return i;
	}
	return -1;
}
*/

/***********************************************************************
 *���ߣ�Ivan.Su
 *���ܣ����´�������
 *��������
 *����ֵ����
 ***********************************************************************/
/*
void DiskArray::refreshDiskArray()
{
	chdir(workingDir);

	string bufferDiskName;
	string diskDevName[20];
	int curDiskNum = 0;
	int fd;
	
	//��/proc/partitions�ж�ȡ���ڵĴ���
	system("awk \'{if($4 ~/sd.$/) print \"/dev/\"$4}\' /proc/partitions > disk");

	ifstream fp;
	fp.open("disk",ios::in);
	
	//�ҳ����ڵĴ��̵��豸��
	//cout<<"refreshDiskArray"<<endl;
	while(!fp.eof())
	{
		getline(fp, bufferDiskName);

		if(bufferDiskName.find("/dev/sd", 0) == string::npos)
		{
			continue;
		}
		
 		if((fd = open((char *)bufferDiskName.c_str(),O_RDWR)) < 0)
		{
			//cout<<"Disk "<<bufferDiskName<<" is not exist"<<endl;
			close(fd);
    			continue;
  		}
		close(fd);
		//cout<<bufferDiskName<<endl;
		diskDevName[curDiskNum++] = bufferDiskName;
	}
	
	fp.close();
	system("rm -f disk");

	//
	for (int i = 0; i< diskNum; i++)
	{
		bool isLostDisk =1;
		
		//��⵱ǰ���Ƿ���̻���
		for (int j = 0; j < curDiskNum ; j++ )
		{
			Disk d;
			//��ô�����Ϣ
			strcpy(d.devName,(char *)diskDevName[j].c_str());
			if(d.getSnByDevName() == false)
				continue;
			d.getScsiIDByDevName();

			if (d == array[i])
			{
				isLostDisk = 0;
				if(strcmp(d.devName, array[i].devName))
				{
					strcpy(array[i].devName, d.devName);
				}
				if(strcmp(d.scsiID, array[i].scsiID))
				{
					strcpy(array[i].scsiID, d.scsiID);
				}
				break;
			}
		}
		
		//�����ǰ���̵��̻��𻵣���ɾ���ô���
		if(isLostDisk)
		{
			//cout<<array[i].devName<<" is bad"<<endl;
			removeDiskAt(i);
			 i--;
		}
	}
	
	for (int i = 0; i < curDiskNum; i++ )
	{
		bool isNewDisk = 1;

		Disk d;
		//��ô�����Ϣ
		strcpy(d.devName,(char *)diskDevName[i].c_str());
		if(d.getSnByDevName() == false)
			continue;
		d.getScsiIDByDevName();

		//������ԭ���Ѿ��������У�����´�����Ϣ
		for(int j = 0; j < diskNum; j++)
		{
			if(d == array[j])	//���̲�Ϊ�¼���
			{
				isNewDisk = 0;	
				if(strcmp(d.devName, array[j].devName))
				{
					strcpy(array[j].devName, d.devName);
				}
				if(strcmp(d.scsiID, array[j].scsiID))
				{
					strcpy(array[j].scsiID, d.scsiID);
				}
				break;
			}	//if(d == array[j])
		}

		//�������Ϊ�¼��̣�����ӵ�DiskArray��
		if(isNewDisk==1)
		{
			d.setAttribute((char *)diskDevName[i].c_str());
			array[diskNum] = d;
			diskNum++;
		}
	}	//for (int i = 0; i < curDiskNum; i++ )

}
*/
void DiskArray::removeDiskAt(int index)
{
	for(int i = index;i < diskNum - 1;i++)
	{
		array[i] = array[i + 1];
	}
	diskNum--;
}

#endif
