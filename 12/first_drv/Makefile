#KERN_DIR = /work/system/linux-2.6.22.6
KERN_DIR = /work/share/wds_linux/system/linux-2.6.22.6_jz2440/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= first_drv.o
