#!/bin/bash
# /etc/init.d/coopmgr

### BEGIN INIT INFO

# Provides:coopmgr
# Required-Start:$remote_fs $syslog
# Required-Stop:$remote_fs $syslog
# Default-Start:2 3 4 5
# Default-Stop:0 1 6
# Short-Description: coopmgr
# Description: chicken coop mgr auto start after boot
### END INIT INFO

case "$1" in
    start)
        echo "Starting coopmgr"
        cd /home/vinthewrench/chickencoop/bin
        nohup /home/vinthewrench/chickencoop/bin/coopmgr &
         ;;
    stop)
        echo "Stopping coopmgr"
     	 killall -9 coopmgr
         ;;
    *)
        echo "Usage: service coopmgr start|stop"
        exit 1
        ;;
esac
exit 0

##### notes
# sudo cp coopmgr.startup.sh /etc/init.d/coopmgr
# sudo chmod +x /etc/init.d/coopmgr
# sudo update-rc.d coopmgr defaults
# sudo update-rc.d  -f coopmgr remove
# sudo service coopmgr start 
# sudo service coopmgr stop
# sudo service coopmgr status

