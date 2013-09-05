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
 *作者：Ivan.Su
 *功能：填充磁盘数组，获得当前磁盘最新信息
 *参数：无
 *返回值：无
 ***********************************************************************/
void DiskArray::fillArray()
{
	chdir(workingDir);
	string bufferDiskName;
	int fd;
	
	diskNum =0;

	//从/proc/partitions中读取存在的磁盘
	system("awk \'{if($4 ~/sd.$/) print \"/dev/\"$4}\' /proc/partitions > disk");

	ifstream fp;
	fp.open("disk",ios::in);
	
	//找出存在的磁盘的设备名
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

	//对DiskArray进行排序
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
 *作者：Ivan.Su
 *功能：更新磁盘数组
 *参数：无
 *返回值：无
 ***********************************************************************/
/*
void DiskArray::refreshDiskArray()
{
	chdir(workingDir);

	string bufferDiskName;
	string diskDevName[20];
	int curDiskNum = 0;
	int fd;
	
	//从/proc/partitions中读取存在的磁盘
	system("awk \'{if($4 ~/sd.$/) print \"/dev/\"$4}\' /proc/partitions > disk");

	ifstream fp;
	fp.open("disk",ios::in);
	
	//找出存在的磁盘的设备名
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
		
		//检测当前盘是否掉盘或损坏
		for (int j = 0; j < curDiskNum ; j++ )
		{
			Disk d;
			//获得磁盘信息
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
		
		//如果当前磁盘掉盘或损坏，则删除该磁盘
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
		//获得磁盘信息
		strcpy(d.devName,(char *)diskDevName[i].c_str());
		if(d.getSnByDevName() == false)
			continue;
		d.getScsiIDByDevName();

		//若磁盘原来已经在阵列中，则更新磁盘信息
		for(int j = 0; j < diskNum; j++)
		{
			if(d == array[j])	//磁盘不为新加盘
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

		//如果磁盘为新加盘，则添加到DiskArray中
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
