# forDual
## Raspberry Pi model 3 initial setting
<pre><code>
sudo apt-get update
sudo apt-get upgrade //something error happend but just reboot

sudo apt-get install vim
sudo apt-get install xrdp
sudo apt-get install git

netstat -nr //check the gateway address

sudo vim /etc/dhcpcd.conf

> #static IP configuration 
> interface wlan0 //wifi

> static ip_address=<static ip address>/24 
> static routers=<gateway address>
> static domain_name_servers=<gateway address> 8.8.8.8

reboot

sudo raspi-config //5 interfacing Options //P2 SSH --> YES

ifconfig  

reboot

</code></pre>
