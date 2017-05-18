sudo iptables -t nat -A POSTROUTING -o wlan1 -j MASQUERADE && sudo iptables -A FORWARD -i wlan1 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT && sudo iptables -A FORWARD -i wlan0 -o wlan1 -j ACCEPT;sudo hostapd /etc/hostapd/hostapd.conf

