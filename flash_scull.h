#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <fcntl.h>

#include "flashcfg.h"

#define SCULL_IOC_MAGIC  'k'
#define SCULL_IOCTFLSHZONE  _IO(SCULL_IOC_MAGIC,   1) 

#define   FLAG_ERR  -1
#define   OPEN_ERR  -2 
#define   IOCTL_ERR -3

int  open_scull(int flag, int flash_zone)
{
	int  fd ;
	
	if(flag != O_RDONLY && flag != O_WRONLY){
		printf("flag illegal.\n");
		return FLAG_ERR ;
	} // open mode not supported
		 
	fd = open("/dev/scull", flag) ;
	
	if(fd < 0) {
		printf("open err.\n");
		return OPEN_ERR ;
	}
	
	if( ioctl(fd, SCULL_IOCTFLSHZONE, flash_zone) ) {
		printf("ioctl err.\n");
		close(fd);
		return IOCTL_ERR ;
	}
	
	return fd ;
	
}


FILE * fopen_scull(char *modes, unsigned long flash_zone)
{
	FILE * fp = NULL ;
	int fd;
	
	if(strcmp(modes, "r") && strcmp(modes, "w")) {
		printf("modes illegal.\n");
		return NULL ;
	}
		
	fp = fopen("dev/scull", modes) ;
	if(fp == NULL) {
		printf("open err.\n");
		return NULL ;
	}
	
	fd = fileno(fp) ;
	if(! ioctl(fd, SCULL_IOCTFLSHZONE, flash_zone)) {
		printf("ioctl err.\n");
		return NULL ;
	}
	
	return fp ;
}



#define READ_FORM_PCI_OK   0
#define WRITE_TO_PCI_OK     0
#define READ_SCULL_ERR      -1
#define WRITE_SCULL_ERR    -2
#define READ_LOCAL_ERR      -3
#define WRITE_LOCAL_ERR    -4

int  read_from_pci(const char * filename, int flash_zone)
{
	int  fd_read, fd_write ;
	unsigned char buf[4096];
	int rlen, wlen, offset ;
	int retval ;
	
	fd_read = open_scull(O_RDONLY, flash_zone) ;
	if(fd_read < 0) {
		retval = READ_SCULL_ERR;
		goto out_none ;
	}
	
	fd_write = open(filename, O_CREAT | O_WRONLY, 0666) ;
	if(fd_write < 0){
		retval = WRITE_LOCAL_ERR ;
		goto out_r ;
	}

	while((rlen = read(fd_read, buf, 4096)) > 0) {
		
		wlen = write(fd_write, buf, rlen) ;
		if(wlen < 0) {
			retval = WRITE_LOCAL_ERR ;
			goto out ;
		}
		offset = wlen ;
		while((rlen = rlen - wlen) ) {
			wlen = write(fd_write, (void *)((char *)buf + offset), rlen) ;
			if(wlen <= 0) {
				retval = WRITE_LOCAL_ERR ;
				goto out ;
			}
			offset += wlen ;
		}
			
	}

	if(rlen < 0) {
		retval = READ_SCULL_ERR ;
		goto out ;
	}
	
	close(fd_read) ;
	close(fd_write) ;
	return READ_FORM_PCI_OK;

out:
	close(fd_write) ;
out_r:
	close(fd_read) ;
out_none:
	printf("read_from_pci return val=%d\n", retval);
	return retval ;

}


int write_to_pci(const char *filename, int flash_zone)
{
	int  fd_read, fd_write ;
	unsigned char buf[4096];
	int rlen, wlen, offset ;
	int retval ;
	
	fd_read = open(filename, O_RDONLY) ;
	if(fd_read< 0){
		retval = READ_LOCAL_ERR ;
		goto out_none ;
	}
	
	fd_write = open_scull(O_WRONLY, flash_zone) ;
	if(fd_write < 0) {
		retval = WRITE_SCULL_ERR ;
		goto out_r ;
	}

	while((rlen = read(fd_read, buf, 4096)) > 0) {
		
		wlen = write(fd_write, buf, rlen) ;
		if(wlen < 0) {
			retval = WRITE_SCULL_ERR ;
			goto out ;
		}
		offset = wlen ;
		while((rlen = rlen - wlen) ) {
			wlen = write(fd_write, (void *)((char *)buf + offset), rlen) ;
			if(wlen <= 0) {
				retval = WRITE_SCULL_ERR ;
				goto out ;
			}
			offset += wlen ;
		}
			
	}

	if(rlen < 0) {
		retval = READ_LOCAL_ERR ;
		goto out ;
	}
	
	close(fd_read) ;
	close(fd_write) ;
	return READ_FORM_PCI_OK;

out:
	close(fd_write) ;
out_r:
	close(fd_read) ;
out_none:
	printf("write_to_pci return val=%d\n", retval);
	return retval ;
}


