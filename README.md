# Projet_Geolocalisation_cite_de_science

# Guide Interactif - Cité des Sciences (M5Stack CoreS3)

Ce projet permet de transformer une tablette M5Stack en guide interactif basé sur la localisation WiFi.

## Fonctionnement
1. Collecte : Scan des bornes WiFi avec la tablette.
2. Apprentissage : Traitement des données via Google Colab.
3. Restitution : La tablette affiche une image PNG depuis la carte SD en fonction de la zone détectée via MQTT.
4. Node-Red    :  Utilisation de node-red pour le Faire le Map 

## Matériel
- M5Stack CoreS3 (Tab5)
- Carte Micro SD (FAT32)
- Broker MQTT HiveMQ
