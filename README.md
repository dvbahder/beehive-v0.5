beehive-v0.5
Stand 04-2020
# LoRaWAN Bienenwaage (experimentell)

Das Projekt soll den Bau einer Waage ermöglichen, die unter einen Bienenstock gestellt wird.
Die gewonnen Daten (Gewicht, Temperatur...) geben Informationen, anhand derer die Bienen mit weniger und dafür gezielten Eingriffen bewirtschaftet werden können.

Das Vorgängerprojekt hatte einen NodeMCUV2.0 mit WLAN - Anbindung verbaut. Das aktuelle Projekt soll die Anbindung per LoRaWAN ermöglichen.
- es muss ein LoRaWAN-Gateway in Reichweite sein
- ein TTN Account ist erforderlich, eine App und die Schlüssel müssen nutzbar sein

Rückfragen dauern etwas länger, da das Projekt ausschließlich als Hobby in Freizeit entsteht.
Viel Freude beim Probieren!

Teile:
- WEMOS D1 (ESP8266)
- RFM95 (SX1276 868MHz) auf einem Hallard-Shield https://github.com/hallard/WeMos-Lora
- HX711 A/D für Loadcell
- H30A Loadcell Bosche alternativ alle Loadcell-Varianten die als Vollbrücke arbeiten z.B. 4x Human_Loadcell je 50kg
- 1-2 DS18B20 Temperatursensoren
- Solarzelle 5V 0,5W
- LiFePo Akku 18650 mit Halterung
- TP5000 Laderegler
- ggf. S7V8F3 step-up/step-down Spannungsregler
- Widerstände 4K7, 320K
- Lochrasterplatte 70x50mm, passende Steckleisten, Drähte, Lötzubehör
