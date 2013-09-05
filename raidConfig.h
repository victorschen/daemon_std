/*		
**		raidConfig.h   the file raidConfig.xml's structure.
**		Include the attributes and operations. Very important to generate the config file at last 
*/
#include<iostream>
#include<string.h>
#include<stdio.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "diskArray.h"
#include "disk.h"
#include "singleRaid.h"
#include "global.h"
#include "flash_scull.h"
using namespace std;

#ifndef READ_WRITE_PCI
#define READ_WRITE_PCI
#endif

#ifndef  _RAIDCONFIG_H_
#define  _RAIDCONFIG_H_

class RaidConfig {
public:
	Disk disks[32];
	SingleRaid singleRaids[16];
	int diskNum;
	int singleRaidNum;
	RaidConfig();
	void addDisk(Disk& d);
	void addSingleRaid(SingleRaid& sr);
	void removeDisk(Disk& d);
	void removeSingleRaid(SingleRaid& sr);
	void saveRcToXml(char * xmlFileName);
	void buildRcFromXml(char * xmlFileName);
	int  getDiskIndex(Disk& d);
	int  getSingleRaidIndex(SingleRaid& sr);
	//bool findDiskInRC(Disk& d);
};

RaidConfig::RaidConfig()
{
	diskNum = 0;
	singleRaidNum = 0;
}

void RaidConfig::addDisk(Disk& d)
{
	disks[diskNum] = d;
	diskNum++;
}

void RaidConfig::addSingleRaid(SingleRaid& sr)
{
	singleRaids[singleRaidNum] = sr;
	singleRaidNum++;
}

void RaidConfig::removeDisk(Disk& d)
{
	//cout<<"remove disk here---------------------------------------"<<endl;
	for(int i = 0;i < diskNum;i++)
	{
		 if(disks[i] == d)
		 {
		 	 while(i < diskNum - 1)
		 	 {
		 	 	 disks[i] = disks[i + 1];
		 	 	 i++;
		 	 }
		 	 diskNum--;
		 	 break;
		 }
	}
}

void RaidConfig::removeSingleRaid(SingleRaid& sr)
{
	for(int i = 0;i < singleRaidNum;i++)
	{
		 if(singleRaids[i] == sr)
		 {
		 	 while(i < singleRaidNum - 1)
		 	 {
		 	 	 singleRaids[i] = singleRaids[i + 1];
		 	 	 i++;
		 	 }
		 	 singleRaidNum--;
		 	 break;
		 }
	}
}

void RaidConfig::saveRcToXml(char * xmlFileName)
{
	char temp[300];
	FILE * fp = fopen("temp.xml","w");
	fputs("<?xml version=\"1.0\" ?>\n",fp);
	fputs("<Rack>\n",fp);
	fputs("</Rack>\n",fp);
	fclose(fp);
	xmlDocPtr doc = xmlParseFile("temp.xml");
	system("rm -f temp.xml");
	int rackDiskNum = 0;  
	int rackRaidNum = 0; 
	int lastRaidIndex = 0; 
	xmlNodePtr cur = xmlDocGetRootElement(doc);
	for(int i = 0;i < diskNum;i++)
	{	
		xmlNodePtr newNode = xmlNewTextChild(cur,NULL,(xmlChar *)"Disk",NULL);
		xmlNewProp(newNode,(xmlChar *)"type",(xmlChar *)disks[i].type);
		xmlNewProp(newNode,(xmlChar *)"status",(xmlChar *)disks[i].status);
		xmlNewProp(newNode,(xmlChar *)"sn",(xmlChar *)disks[i].sn);
		xmlNewProp(newNode,(xmlChar *)"vendor",(xmlChar *)disks[i].vendor);
		xmlNewProp(newNode,(xmlChar *)"capacity",(xmlChar *)disks[i].capacity);
		xmlNewProp(newNode,(xmlChar *)"devName",(xmlChar *)disks[i].devName);
		xmlNewProp(newNode,(xmlChar *)"isSpareDisk",(xmlChar *)disks[i].isSpareDisk);
		xmlNewProp(newNode,(xmlChar *)"host",(xmlChar *)disks[i].host);
		xmlNewProp(newNode,(xmlChar *)"scsiID",(xmlChar *)disks[i].scsiID);
		xmlNewProp(newNode,(xmlChar *)"lun",(xmlChar *)disks[i].lun);
		xmlNewProp(newNode,(xmlChar *)"channel",(xmlChar *)disks[i].channel);
	//	cout<<"-------------"<<endl;
	//	cout<<i<<endl;
	//	cout<<disks[i].devName<<endl;
		rackDiskNum++;
	}
	//将阵列添加到xml文件中
	for(int i = 0;i < singleRaidNum;i++)
	{
		xmlNodePtr newNode = xmlNewTextChild(cur,NULL,(xmlChar *)"singleRaid",NULL);
		xmlNewProp(newNode,(xmlChar *)"index",(xmlChar *)singleRaids[i].index);
		xmlNewProp(newNode,(xmlChar *)"level",(xmlChar *)singleRaids[i].level);
		xmlNewProp(newNode,(xmlChar *)"chunk",(xmlChar *)singleRaids[i].chunk);
		xmlNewProp(newNode,(xmlChar *)"devName",(xmlChar *)singleRaids[i].devName);
		gcvt(singleRaids[i].raidstat,300,temp);
		xmlNewProp(newNode,(xmlChar *)"raidstat",(xmlChar *)temp);
		xmlNewProp(newNode,(xmlChar *)"raidcap",(xmlChar *)singleRaids[i].raidcap);
		xmlNewProp(newNode,(xmlChar *)"time",(xmlChar *)singleRaids[i].time);
		xmlNewProp(newNode,(xmlChar *)"speed",(xmlChar *)singleRaids[i].speed);
		xmlNewProp(newNode,(xmlChar *)"percent",(xmlChar *)singleRaids[i].percent);
		xmlNewProp(newNode,(xmlChar *)"mappingNo",(xmlChar *)singleRaids[i].mappingNo);
		
		gcvt(singleRaids[i].raidDiskNum,300,temp);
		xmlNewProp(newNode,(xmlChar *)"raidDiskNum",(xmlChar *)temp);
		gcvt(singleRaids[i].spareDiskNum,300,temp);
		xmlNewProp(newNode,(xmlChar *)"spareDiskNum",(xmlChar *)temp);
		strcpy(temp,"");
		for(int j = 0;j < singleRaids[i].popedomNum;j++)
		{
			strcat(temp,singleRaids[i].popedom[j]);
			strcat(temp,"-"); 
		}
		xmlNewProp(newNode,(xmlChar *)"popedom",(xmlChar *)temp);
		
		strcpy(temp,"");
		for(int j = 0;j < singleRaids[i].bindingNum;j++)
		{
			strcat(temp,singleRaids[i].binding[j]);
			strcat(temp,"-"); 
		}
		xmlNewProp(newNode,(xmlChar *)"binding",(xmlChar *)temp);
		
		for(int j = 0;j < singleRaids[i].raidDiskNum;j++)
		{
			xmlNodePtr childNode = xmlNewTextChild(newNode,NULL,(xmlChar *)"Disk",NULL);
			xmlNewProp(childNode,(xmlChar *)"type",(xmlChar *)singleRaids[i].raidDisks[j].type);
			xmlNewProp(childNode,(xmlChar *)"status",(xmlChar *)singleRaids[i].raidDisks[j].status);
			xmlNewProp(childNode,(xmlChar *)"sn",(xmlChar *)singleRaids[i].raidDisks[j].sn);
			xmlNewProp(childNode,(xmlChar *)"vendor",(xmlChar *)singleRaids[i].raidDisks[j].vendor);
			xmlNewProp(childNode,(xmlChar *)"capacity",(xmlChar *)singleRaids[i].raidDisks[j].capacity);
			xmlNewProp(childNode,(xmlChar *)"devName",(xmlChar *)singleRaids[i].raidDisks[j].devName);
			xmlNewProp(childNode,(xmlChar *)"isSpareDisk",(xmlChar *)singleRaids[i].raidDisks[j].isSpareDisk);
			xmlNewProp(childNode,(xmlChar *)"host",(xmlChar *)singleRaids[i].raidDisks[j].host);
			xmlNewProp(childNode,(xmlChar *)"scsiID",(xmlChar *)singleRaids[i].raidDisks[j].scsiID);
			xmlNewProp(childNode,(xmlChar *)"lun",(xmlChar *)singleRaids[i].raidDisks[j].lun);
			xmlNewProp(childNode,(xmlChar *)"channel",(xmlChar *)singleRaids[i].raidDisks[j].channel);
			rackDiskNum++;
		}
		for(int j = 0;j < singleRaids[i].spareDiskNum;j++)
		{
			xmlNodePtr childNode = xmlNewTextChild(newNode,NULL,(xmlChar *)"Disk",NULL);
			xmlNewProp(childNode,(xmlChar *)"type",(xmlChar *)singleRaids[i].spareDisks[j].type);
			xmlNewProp(childNode,(xmlChar *)"status",(xmlChar *)singleRaids[i].spareDisks[j].status);
			xmlNewProp(childNode,(xmlChar *)"sn",(xmlChar *)singleRaids[i].spareDisks[j].sn);
			xmlNewProp(childNode,(xmlChar *)"vendor",(xmlChar *)singleRaids[i].spareDisks[j].vendor);
			xmlNewProp(childNode,(xmlChar *)"capacity",(xmlChar *)singleRaids[i].spareDisks[j].capacity);
			xmlNewProp(childNode,(xmlChar *)"devName",(xmlChar *)singleRaids[i].spareDisks[j].devName);
			xmlNewProp(childNode,(xmlChar *)"isSpareDisk",(xmlChar *)singleRaids[i].spareDisks[j].isSpareDisk);
			xmlNewProp(childNode,(xmlChar *)"host",(xmlChar *)singleRaids[i].spareDisks[j].host);
			xmlNewProp(childNode,(xmlChar *)"scsiID",(xmlChar *)singleRaids[i].spareDisks[j].scsiID);
			xmlNewProp(childNode,(xmlChar *)"lun",(xmlChar *)singleRaids[i].spareDisks[j].lun);
			xmlNewProp(childNode,(xmlChar *)"channel",(xmlChar *)singleRaids[i].spareDisks[j].channel);
			rackDiskNum++;
		}
		rackRaidNum++;
		int index = atoi(singleRaids[i].index);
		if(index + 1 > lastRaidIndex)
			lastRaidIndex = index + 1;
	}
	gcvt(rackDiskNum,10,temp);
	xmlNewProp(cur,(xmlChar *)"diskNum",(xmlChar *)temp);
	gcvt(rackRaidNum,10,temp);
	xmlNewProp(cur,(xmlChar *)"raidNum",(xmlChar *)temp);
	gcvt(lastRaidIndex,10,temp);
	xmlNewProp(cur,(xmlChar *)"lastRaidIndex",(xmlChar *)temp);
	xmlSaveFormatFile(xmlFileName,doc,1);
	xmlFreeDoc(doc);
}

void RaidConfig::buildRcFromXml(char * xmlFileName)
{
	string cmdline="";
	chdir(workingDir);
	xmlDocPtr doc = xmlParseFile(xmlFileName);
	if(doc == NULL)
	{
		
		if(!strcmp(xmlFileName, "raidConfigNew.xml"))
		{
			system("rm -f raidConfigNew.xml");
		}	

	#ifdef READ_WRITE_PCI		
		read_from_pci(RAIDCONFIG_DIR_READ_BACKUP, RAID_CONFIG_BACKUP);
	#endif
	
		system("mv raidConfig_backup.xml raidConfig.xml");
		//system("chmod 777 raidConfig.xml");
		doc = xmlParseFile("raidConfig.xml");
		if(doc == NULL)
		{
			//system("rm -f restore");
			system("touch raidConfig_error");
			while(1)
			{
				if(!access("raidConfigNew.xml",0) && !access("restore", 0))
				{
					system("rm -f restore");
					system("mv raidConfigNew.xml raidConfig.xml");
					//system("chmod 777 raidConfig");
					doc = xmlParseFile("raidConfig.xml");
					if(doc == NULL)
					{
						RaidConfig rc1;
						DiskArray da1;
						
						system("rm -f raidConfig.xml");

						da1.fillArray(); 					
						for(int i = 0;i < da1.diskNum;i++)
						{
							rc1.addDisk(da1.array[i]);
						}

						rc1.saveRcToXml("raidConfig.xml"); 
						doc = xmlParseFile("raidConfig.xml");
					}
					break;
				}
			}	//while(1)
		}
	}
	
	xmlNodePtr cur = xmlDocGetRootElement(doc);
	cur = cur -> xmlChildrenNode;
	while(cur != NULL)
	{
		if(!xmlStrcmp(cur -> name,(const xmlChar *)"Disk"))
		{
			Disk d;
			strcpy(d.type,(char *)xmlGetProp(cur,(xmlChar *)"type"));
			strcpy(d.status,(char *)xmlGetProp(cur,(xmlChar *)"status"));
			strcpy(d.sn,(char *)xmlGetProp(cur,(xmlChar *)"sn"));
			strcpy(d.vendor,(char *)xmlGetProp(cur,(xmlChar *)"vendor"));
			strcpy(d.capacity,(char *)xmlGetProp(cur,(xmlChar *)"capacity"));
			strcpy(d.devName,(char *)xmlGetProp(cur,(xmlChar *)"devName"));
			strcpy(d.isSpareDisk,(char *)xmlGetProp(cur,(xmlChar *)"isSpareDisk"));
			strcpy(d.host,(char *)xmlGetProp(cur,(xmlChar *)"host"));
			strcpy(d.scsiID,(char *)xmlGetProp(cur,(xmlChar *)"scsiID"));
			strcpy(d.lun,(char *)xmlGetProp(cur,(xmlChar *)"lun"));
			strcpy(d.channel,(char *)xmlGetProp(cur,(xmlChar *)"channel"));
			addDisk(d);
		}
		else if(!xmlStrcmp(cur -> name,(const xmlChar *)"singleRaid"))
		{
			char i[5], l[10], c[5], m[5],temp[300],rs[3],cap[20];int r;
			strcpy(i,(char *)xmlGetProp(cur,(xmlChar *)"index"));
			strcpy(l,(char *)xmlGetProp(cur,(xmlChar *)"level"));
			strcpy(c,(char *)xmlGetProp(cur,(xmlChar *)"chunk"));
			strcpy(rs,(char *)xmlGetProp(cur,(xmlChar *)"raidstat"));
			r=atoi(rs);
			strcpy(cap,(char *)xmlGetProp(cur,(xmlChar *)"raidcap"));
			strcpy(m,(char *)xmlGetProp(cur,(xmlChar *)"mappingNo"));
			SingleRaid sr;
			sr.setAttribute(i,l,c,m,r,cap);
			
			strcpy(temp,(char *)xmlGetProp(cur,(xmlChar *)"popedom"));
			char * p1 = temp;
			char * p2 = p1;
			char temp1[20];
			if(strstr(p2,"all-") !=NULL)
			{
				sr.addPopedom("all");
			}
			else
			{
				while(strcmp(p2,""))
				{
					strcpy(temp1,"");
					p1 = strstr(p2,"-");
					strncat(temp1,p2,p1 - p2);
					sr.addPopedom(temp1);
					p2 = p1 + 1;  
				}
			}
			
                    strcpy(temp,(char *)xmlGetProp(cur,(xmlChar *)"binding"));
		      p1 = temp;
		      p2 = p1;
			char temp2[4];
			while(strcmp(p2,""))
				{
					strcpy(temp2,"");
					p1 = strstr(p2,"-");
					strncat(temp2,p2,p1 - p2);
					sr.addBinding(temp2);
					p2 = p1 + 1;  
				}
					
			xmlNodePtr childCur = cur -> xmlChildrenNode;
			while(childCur != NULL)
			{
				Disk d;
				strcpy(d.type,(char *)xmlGetProp(childCur,(xmlChar *)"type"));
				strcpy(d.status,(char *)xmlGetProp(childCur,(xmlChar *)"status"));
				strcpy(d.sn,(char *)xmlGetProp(childCur,(xmlChar *)"sn"));
				strcpy(d.vendor,(char *)xmlGetProp(childCur,(xmlChar *)"vendor"));
				strcpy(d.capacity,(char *)xmlGetProp(childCur,(xmlChar *)"capacity"));
				strcpy(d.devName,(char *)xmlGetProp(childCur,(xmlChar *)"devName"));
				strcpy(d.isSpareDisk,(char *)xmlGetProp(childCur,(xmlChar *)"isSpareDisk"));
				strcpy(d.host,(char *)xmlGetProp(childCur,(xmlChar *)"host"));
				strcpy(d.scsiID,(char *)xmlGetProp(childCur,(xmlChar *)"scsiID"));
				strcpy(d.lun,(char *)xmlGetProp(childCur,(xmlChar *)"lun"));
				strcpy(d.channel,(char *)xmlGetProp(childCur,(xmlChar *)"channel"));
				if(!strcmp(d.isSpareDisk,"0"))
					sr.addRaidDisk(d);
				else
					sr.addSpareDisk(d);
				childCur = childCur -> next;
			}
			addSingleRaid(sr);
		}
		cur = cur -> next;
	}
	xmlFreeDoc(doc);
}

int RaidConfig::getDiskIndex(Disk& d)
{
	for(int i = 0;i < diskNum;i++)
	{
		if(disks[i] == d)
			return i;
	}
    return -1;
}

int RaidConfig::getSingleRaidIndex(SingleRaid& sr)
{
	for(int i = 0;i < singleRaidNum;i++)
	{
		if(singleRaids[i] == sr)
			return i;
	}
	return -1;
}

/**************************************************************************
 *作者：Ivan.Su
 *功能：在RaidConfig中查找某一个磁盘，看是否存在
 *参数：待查找磁盘
 *返回值：true－找到磁盘；faulse－未找到磁盘
 **************************************************************************/
/*bool RaidConfig::findDiskInRC(Disk& d)
{
	//在阵列中查找磁盘
	for (int i = 0;i < singleRaidNum; i++ )
	{
		//在阵列的数据盘中查找磁盘
		for(int j =0; j<singleRaids[i].raidDiskNum; j++)
		{
			if(singleRaids[i].raidDisks[j] == d)
			{
				return 1;
			}
		}
		//在阵列的热备盘中查找磁盘
		for (int j = 0; j<singleRaids[i].spareDiskNum; j++ )
		{
			if(singleRaids[i].spareDisks[j] == d)
			{
				return 1;
			}
		}	
	}	//for (int i = 0;i < singleRaidNum; i++ )

	//在阵列外查找磁盘
	for (int i = 0; i < diskNum; i++ )
	{
		if(disks[i] == d)
		{
			return 1;
		}
	}

	return 0;
}
*/

#endif

