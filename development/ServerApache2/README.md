**Note:** This code does not currently function correctly. We are aware of the issues and are working on a fix. Please use the FastAPI server implementation in the main directory for a functional server setup.


[//]: # (Image References)
[image1]: ./data/server_diagram.png "diagram"

# Raspberry Pi Server


## Objective
The objective of this setup is to establish a reliable and secure server environment on a Raspberry Pi that supports both front-end and back-end development, with the capability to share files over the network and serve applications securely using SSL certificates.

![diagram][image1]

## Introduction
This guide outlines the steps to set up a server on a Raspberry Pi. The goal is to create a fully functional server that uses Samba for file sharing, Node.js and Vue CLI for front-end development, Apache2 as the web server, and FastAPI with Uvicorn for back-end services. Additionally, it includes instructions for setting up SSL certificates using Let's Encrypt and OpenSSL to secure the server.


## Setup Instructions

### Install Raspberry Pi OS

Install Raspberry Pi OS using Raspberry Pi Imager: [Download](https://www.raspberrypi.com/software/)

### Raspberry Pi Server Setup

#### System Update
```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt-get autoremove
```

#### Install Samba
```bash
sudo apt-get install samba samba-common-bin
sudo mkdir /Raspberry
sudo chmod 777 /Raspberry
sudo nano /etc/samba/smb.conf

# Add the following lines at the end of the smb.conf file:
[Raspberry]
path = /Raspberry
writeable = yes
create mask = 0777
directory mask = 0777
public = yes
```

#### Reboot the System
```bash
sudo reboot
```

#### Install Node.js and Vue CLI
```bash
curl -fsSL https://fnm.vercel.app/install | bash
source /home/pi/.bashrc
fnm use --install-if-missing 20
node -v
npm -v

npm install -g @vue/cli
vue --version

cd /Raspberry/Bioreactor
vue create frontend
sudo chmod 777 /Raspberry/Bioreactor
sudo chmod 777 /Raspberry/Bioreactor/frontend
```

#### Install Apache2
```bash
sudo apt-get upgrade -y
sudo apt-get install apache2 -y
sudo systemctl status apache2
```

#### Install FastAPI and Uvicorn
```bash
sudo apt-get install python3-pip
python3 -m venv ~/env_bioreactor
source ~/env_bioreactor/bin/activate
pip install fastapi uvicorn
nano ~/.bashrc

# Add the following line at the end of the ~/.bashrc file:
source ~/env_bioreactor/bin/activate
```

#### Create Directories for Logs and Data
```bash
mkdir -p /Raspberry/Bioreactor/logs /Raspberry/Bioreactor/data
```

#### Install Certbot for SSL Certificates
```bash
sudo apt-get update
sudo apt-get install certbot
```

#### Obtain SSL Certificates with Let's Encrypt
```bash
sudo systemctl stop apache2
sudo certbot certonly --standalone -d bioreactor.ddns.net
# Saving debug log to /var/log/letsencrypt/letsencrypt.log
sudo systemctl start apache2

sudo usermod -aG ssl-cert pi
sudo chmod 644 /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem
sudo chmod 644 /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
sudo reboot
```

#### Configure Uvicorn with SSL
```bash
cd /Raspberry/Bioreactor
uvicorn backend:app --host 0.0.0.0 --port 8000 --ssl-keyfile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem --ssl-certfile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
```

#### Configure Apache for Reverse Proxy
```bash
sudo a2enmod proxy proxy_http ssl
sudo nano /etc/apache2/sites-available/default-ssl.conf

# Add the following lines to the default-ssl.conf file:
<VirtualHost *:443>
    ServerName bioreactor.ddns.net

    SSLEngine on
    SSLCertificateFile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
    SSLCertificateKeyFile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem

    ProxyPass / http://127.0.0.1:8000/
    ProxyPassReverse / http://127.0.0.1:8000/

    ErrorLog ${APACHE_LOG_DIR}/error.log
    CustomLog ${APACHE_LOG_DIR}/access.log combined
</VirtualHost>

sudo a2ensite default-ssl
sudo systemctl reload apache2
sudo systemctl restart apache2
```

#### Create Uvicorn Service
```bash
sudo nano /etc/systemd/system/uvicorn.service

# Add the following lines to the uvicorn.service file:
[Unit]
Description=Uvicorn instance to serve FastAPI
After=network.target

[Service]
User=pi
Group=www-data
WorkingDirectory=/Raspberry/Bioreactor
Environment="PATH=/home/pi/env_bioreactor/bin"
ExecStart=/home/pi/env_bioreactor/bin/uvicorn backend:app --host 0.0.0.0 --port 8000 --ssl-keyfile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem --ssl-certfile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem

[Install]
WantedBy=multi-user.target


sudo systemctl daemon-reload
sudo systemctl enable uvicorn
sudo systemctl start uvicorn
```

#### Install Unattended Upgrades
```bash
sudo apt-get install unattended-upgrades
sudo dpkg-reconfigure --priority=low unattended-upgrades
```

#### Set Permissions for SSL Certificates
```bash
sudo chown root:ssl-cert /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem
sudo chown root:ssl-cert /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
sudo chmod 640 /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem
sudo chmod 640 /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
sudo usermod -aG ssl-cert pi
newgrp ssl-cert
```

#### Start Uvicorn with SSL
```bash
cd /Raspberry/Bioreactor
sudo -E /home/pi/env_bioreactor/bin/uvicorn backend:app --host 0.0.0.0 --port 8000 --ssl-keyfile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem --ssl-certfile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
```

#### Set Permissions for Frontend Directory
```bash
sudo chown -R pi:pi /Raspberry/Bioreactor/frontend
sudo chmod -R 755 /Raspberry/Bioreactor/frontend
```

#### Build Frontend
```bash
cd /Raspberry/Bioreactor/frontend
npm run build
```

#### Note on OpenSSL (Temporary Solution)
```bash
sudo apt-get install openssl
sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem -out /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
sudo chown root:ssl-cert /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem
sudo chown root:ssl-cert /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
sudo chmod 640 /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem
sudo chmod 640 /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
sudo nano /etc/apache2/sites-available/default-ssl.conf

# Ensure the following lines are present in the default-ssl.conf file:
<VirtualHost *:443>
    ServerName bioreactor.ddns.net

    SSLEngine on
    SSLCertificateFile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
    SSLCertificateKeyFile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem

    ProxyPass / http://127.0.0.1:8000/
    ProxyPassReverse / http://127.0.0.1:8000/

    ErrorLog ${APACHE_LOG_DIR}/error.log
    CustomLog ${APACHE_LOG_DIR}/access.log combined
</VirtualHost>


sudo systemctl restart apache2
cd /Raspberry/Bioreactor/
sudo -E /home/pi/env_bioreactor/bin/uvicorn backend:app --host 0.0.0.0 --port 8000 --ssl-keyfile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem --ssl-certfile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
```


