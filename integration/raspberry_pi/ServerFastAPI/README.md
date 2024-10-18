# Serveur Bioreactor - Guide d'Installation et de Vérification

Ce guide explique comment installer, configurer et vérifier le serveur Bioreactor sur un Raspberry Pi.

## Prérequis

- Raspberry Pi avec Raspbian OS installé
- Connexion Internet
- Accès SSH au Raspberry Pi

## Installation

### 1. Mise à jour du système

```bash
sudo apt update
sudo apt upgrade -y
```

### 2. Installation de Node.js et npm
Nous utiliserons fnm (Fast Node Manager) pour gérer les versions de Node.js.

```bash
curl -fsSL https://fnm.vercel.app/install | bash
source ~/.bashrc
fnm install --lts
```
### 3. Installation des dépendances du projet
```bash
cd /Raspberry/Bioreactor/ServerFastAPI/frontend
npm install
```

### 4. Installation des dépendances du projet
Créez le fichier de service :

```bash
sudo nano /etc/systemd/system/fastapi.service
```

Ajoutez le contenu suivant :
```bash
[Unit]
Description=FastAPI server
After=network.target

[Service]
User=pi
WorkingDirectory=/Raspberry/Bioreactor/ServerFastAPI
Environment="PATH=/home/pi/env_bioreactor/bin"
ExecStart=/home/pi/env_bioreactor/bin/uvicorn backend:app --host 0.0.0.0 --port 8000

[Install]
WantedBy=multi-user.target
```

Activez et démarrez le service :
```bash
sudo systemctl enable fastapi.service
sudo systemctl start fastapi.service
```

### 5. Configuration du service Frontend
Créez un script wrapper pour le frontend :

```bash
sudo nano /home/pi/start-frontend.sh
```

Ajoutez le contenu suivant :
```bash
#!/bin/bash

# Charger les variables d'environnement de l'utilisateur
source /home/pi/.profile
source /home/pi/.bashrc

# Configurer fnm
export PATH="/home/pi/.local/share/fnm:$PATH"
eval "$(fnm env --use-on-cd)"

# Aller dans le répertoire du projet
cd /Raspberry/Bioreactor/ServerFastAPI/frontend

# Lancer npm run serve
exec npm run serve
```

Rendez le script exécutable :
```bash
sudo chmod +x /home/pi/start-frontend.sh
```

Créez le fichier de service pour le frontend :
```bash
sudo nano /etc/systemd/system/bioreactor-frontend.service
```

Ajoutez le contenu suivant :
```bash
[Unit]
Description=Bioreactor Frontend Service
After=network.target

[Service]
Type=simple
User=pi
ExecStart=/home/pi/start-frontend.sh
Restart=on-failure
StandardOutput=append:/home/pi/frontend.log
StandardError=append:/home/pi/frontend.log

[Install]
WantedBy=multi-user.target
```

Activez et démarrez le service :
```bash
sudo systemctl enable bioreactor-frontend.service
sudo systemctl start bioreactor-frontend.service
```

### 6. Configuration du script de surveillance
Créez un script de surveillance pour vérifier et redémarrer les services si nécessaire :

```bash
sudo nano /home/pi/bioreactor_monitor.sh
```

Ajoutez le contenu suivant :
```bash
#!/bin/bash

# Chemins
LOG_FILE="/home/pi/bioreactor.log"

# Charger l'environnement de l'utilisateur
source /home/pi/.profile
source /home/pi/.bashrc

# Configurer fnm si nécessaire
export PATH="/home/pi/.local/share/fnm:$PATH"
eval "$(fnm env --use-on-cd)"

# Fonction de logging
log() {
    echo "$(date): $1" | tee -a "$LOG_FILE"
}

# Fonction pour vérifier et appliquer les mises à jour de sécurité
check_security_updates() {
    log "Vérification des mises à jour de sécurité..."
    sudo apt-get update 2>&1 | tee -a "$LOG_FILE"
    sudo apt-get upgrade -y 2>&1 | tee -a "$LOG_FILE"
}

# Fonction pour vérifier la connexion internet
check_connection() {
    log "Vérification de la connexion internet..."
    if ! ping -c 4 8.8.8.8 > /dev/null 2>&1; then
        log "La connexion internet est down. Redémarrage de l'interface réseau..."
        sudo ifconfig wlan0 down
        sleep 5
        sudo ifconfig wlan0 up
        sleep 10
    else
        log "La connexion internet fonctionne."
    fi
}

# Fonction pour vérifier et redémarrer les services
check_and_restart_service() {
    local service_name=$1
    if ! systemctl is-active --quiet "$service_name"; then
        log "Le service $service_name n'est pas en cours d'exécution. Redémarrage..."
        sudo systemctl restart "$service_name"
        sleep 10
        if systemctl is-active --quiet "$service_name"; then
            log "Le service $service_name a été redémarré avec succès."
        else
            log "Échec du redémarrage du service $service_name."
            log "Statut du service $service_name :"
            sudo systemctl status "$service_name" >> "$LOG_FILE" 2>&1
        fi
    else
        log "Le service $service_name est en cours d'exécution."
    fi
}

log "Script de surveillance démarré"

# Boucle principale
while true; do
    check_security_updates
    check_connection
    check_and_restart_service "fastapi"
    check_and_restart_service "bioreactor-frontend"
    log "Cycle de vérification terminé. Attente de 5 minutes avant le prochain cycle."
    sleep 300 # Attendre 5 minutes avant la prochaine vérification
done
```

Rendez le script exécutable :
```bash
sudo chmod +x /home/pi/bioreactor_monitor.sh
```

### 7. Configuration du démarrage automatique
Configurez le script de surveillance pour qu'il s'exécute au démarrage :

```bash
sudo crontab -e
```

Ajoutez la ligne suivante à la fin du fichier :
```bash
@reboot /home/pi/bioreactor_monitor.sh &
```

### Vérification
Après avoir effectué toutes ces étapes, redémarrez votre Raspberry Pi :
```bash
sudo reboot
```

Après le redémarrage, vérifiez que tout fonctionne correctement :

Vérifiez l'état des services :
```bash
sudo systemctl status fastapi
sudo systemctl status bioreactor-frontend
```

Vérifiez les logs :
```bash
cat /home/pi/bioreactor.log
cat /home/pi/frontend.log
```

Testez l'accès au frontend en ouvrant un navigateur et en accédant à :
```bash
http://192.168.1.25:8080
```
(remplacez l'IP par celle de votre Raspberry Pi si elle est différente)

Testez l'accès au backend en ouvrant un navigateur et en accédant à :
```bash
http://192.168.1.25:8000
```
(remplacez l'IP par celle de votre Raspberry Pi si elle est différente)

Si tout fonctionne correctement, vous devriez voir l'interface utilisateur de votre application sur le port 8080 et une page indiquant que le serveur FastAPI est en cours d'exécution sur le port 8000.