#!/bin/bash
sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/g' /etc/ssh/sshd_config  
# sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/g' /etc/ssh/sshd_config  
# service ssh restart  
passwd root << EOF  
123456
123456
EOF
mkdir -p /root/.ssh
mv docker/id_rsa /root/.ssh/
mv docker/id_rsa.pub /root/.ssh/ 
mv docker/authorized_keys /root/.ssh/