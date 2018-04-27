# hostapd创建WIFI热点
本文章用来说明如何使用hostapd开源软件给无线网卡创建可用的热点

+ 操作系统: 	Centos7
+ hostapd:	2.6

### 1 无线网卡驱动安装

查看无线网卡厂商信息,执行`lspci -vv`找到你的无线网卡,我的无线网卡厂商是`Atheros`,芯片是:`AR9227`,驱动程序:`ath9k`,如图:
![](https://i.imgur.com/ITmzSx3.png)
如果你的无线网卡还没有驱动,那么到如下地址[无线网卡驱动说明](https://wireless.wiki.kernel.org/welcome)点击`driver`,然后根据厂商查找对应的驱动程序,首先该驱动程序对应列表中`AP`为`yes`说明该驱动支持AP模式,如果一个厂商有多个驱动程序,那么就逐个点开查看,每个驱动程序支持的芯片型号是不一样的,如果最终没有找到支持你芯片的驱动程序,那么很抱歉,第一关就没有过. 接下来就换无线网卡吧.

### 2 驱动程序编译安装
我使用的Centos7系统自带`ath9k`驱动程序,无线网卡安装上开机就自动识别了,如果驱动程序没有安装的话,那么按照第一步的方法下载对应的驱动源码,根据源码中的`INSTALL`说明编译安装加载驱动就行了.

### 3 hostapd编译安装
源码下载地址:[hostapd源码](https://w1.fi/releases/hostapd-2.6.tar.gz),下载完成解压,然后`cd hostapd-2.6/hostapd/`,可以阅读一下`README`,

    cp defconfig .config	//生成编译时配置文件
	make					
	make install
编译过程中可能会出现如下编译错误:

    ../src/drivers/driver_nl80211.c:17:31: fatal error: netlink/genl/genl.h: No such file or directory
这是因为缺少`NETLINK`头文件,解决办法
`yum install -y yum install -y libnl-devel.i686 libnl-devel.x86_64 libnl.i686 libnl.x86_64`

安装成功,终端输入`host`然后`TAB键`可以直接补全`hostapd`

### 4 hostapd配置文件
创建配置文件`touch ~/hostapd.conf`

`root@centos:~# cat /etc/hostapd.conf `

	interface=wlp3s4	#这个是无限网卡接口名称
	driver=nl80211
	hw_mode=g
	channel=3
	ssid=peiyake
	wpa=2
	wpa_key_mgmt=WPA-PSK
	wpa_pairwise=CCMP
	wpa_passphrase=12345678

### 5 运行hostapd
`hostapd -B ~/hostapd.conf`

正常的话,打印信息如下:

	Using interface wlp3s4 with hwaddr 14:75:90:fc:fb:13 and ssid "peiyake"
	random: Only 15/20 bytes of strong random data available from /dev/random
	random: Not enough entropy pool available for secure operations
	WPA: Not enough entropy in random pool for secure operations - update keys later when the first station connects
	wlp3s4: interface state UNINITIALIZED->ENABLED
	wlp3s4: AP-ENABLED 

失败的话,打印信息如下:
	
	Configuration file: /etc/hostapd.conf
	nl80211: Could not configure driver mode
	nl80211: deinit ifname=wlp3s4 disabled_11b_rates=0
	nl80211 driver initialization failed.
	wlp3s4: interface state UNINITIALIZED->DISABLED
	wlp3s4: AP-DISABLED 
	hostapd_free_hapd_data: Interface wlp3s4 wasn't started

开启失败解决办法,
*注意:下面方法,关闭所有网络端口后,你需要先将你的有线网口up起来才能再次使用*

	nmcli networking off 	//关闭所有网络端口
再次运行`hostapd -B ~/hostapd.conf`,我的环境上这种方法是可以解决问题的,如果你的环境上不能解决问题,再自行百度吧.

**配置到这里,手机应该可以搜到SSID了,但是还无法关联,因为无法获取IP地址**

### 6 配置IP地址和DHCPD服务
安装dhcpd服务,修改配置`/etc/dhcp/dhcpd.conf`文件如下
	
	# dhcpd.conf
	#
	# Sample configuration file for ISC dhcpd
	#
	# option definitions common to all supported networks...
	option domain-name "example.org";
	option domain-name-servers 8.8.8.8,114.114.114.114 ;
	
	#默认租约时间,租约到期客户端重新向服务器端获取配置信息
	default-lease-time 600;
	#最大租约时间,该时间到期,客户端仍然不能从服务器端获取配置信息,那么客户端弃用之前获取的配置,转而使用本地的配置
	max-lease-time 7200;
	
	# Use this to enble / disable dynamic dns updates globally.
	ddns-update-style none;
	
	# If this DHCP server is the official DHCP server for the local
	# network, the authoritative directive should be uncommented.
	authoritative;
	
	# Use this to send dhcp log messages to a different log file (you also
	# have to hack syslog.conf to complete the redirection).
	subnet 192.168.200.0 netmask 255.255.255.0 {
	                option routers 192.168.200.1;
	                pool{
	                                range 192.168.200.100 192.168.200.200;
	                }
	}
配置完成,`systemctl start dhcpd`,此时服务应该无法启动,因为我们dhcpd.conf中配置的地址池没有对应的网络端口,下一步

	ifconfig wlp3s4 192.168.200.1/24 up		//配置无线网口的地址,此地址应该和地址池对应routers一致
再次执行`systemctl start dhcpd`,dhcpd服务应该就可以起来了

### 7 配置iptables规则使终端可以上网
先清除所有规则

	iptable -F
	iptable -t nat -F
配置nat转发规则

	iptables -t nat -A POSTROUTING -o ens33 -j MASQUERADE //将其它端口的报文都转到有线网口


### 8 总结
以上所有步骤完成,那么你的hostapd热点就创建好了,中间过程有点繁杂,但是思路大概如此.后期还有很多工作要做,想再研究源码,用户管理/认证等. 暂时告一段落