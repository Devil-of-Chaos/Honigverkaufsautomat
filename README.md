# Honigverkaufsautomat
Honigverkaufsautomat Heltec wifikit

<strong>ACHTUNG NOCH IN ENTWICKLUNG</strong>


Benötigte Bauteile:

<ul>
  <li>Heltec ESP32 WiFi https://www.amazon.de/gp/product/B076P8GRWV/ref=ppx_yo_dt_b_asin_image_o06_s00?ie=UTF8&psc=1</li>
  <li>3 x Türöffner https://www.amazon.de/Elektroschloss-Elektromagnetschloss-Türzugriffskontrollsystem-Elektromagnet-Aktenschrank-Tür/dp/B07RKMHCTD/ref=sr_1_66</li>
  <li>Schloss https://www.amazon.de/gp/product/B008Q8CD80/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1</li>
  <li>Münzprüfer https://www.amazon.de/gp/product/B07DKBF1ZV/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1</li>
  <li>12V Netzteil https://www.amazon.de/gp/product/B085XY2H9D/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1</li>
  <li>5V Spannungswandler https://www.amazon.de/gp/product/B08H4J52XZ/ref=ppx_yo_dt_b_asin_title_o08_s00?ie=UTF8&psc=1</li>
  <li>2 x 4-Kanal Relais https://www.amazon.de/gp/product/B078Q8S9S9/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1</li>
  <li>3 x Taster Beleuchtet https://www.amazon.de/gp/product/B06XNMCH7W/ref=ppx_yo_dt_b_asin_title_o00_s03?ie=UTF8&psc=1</li>
  <li>RFID Sensor https://www.amazon.de/gp/product/B01M28JAAZ/ref=ppx_yo_dt_b_asin_title_o00_s04?ie=UTF8&psc=1</li>
  <li>Platinen https://www.amazon.de/gp/product/B0728HZHTR/ref=ppx_yo_dt_b_asin_title_o07_s00?ie=UTF8&psc=1</li>
</ul>

5V Spannungswandler versorgt Relais sowie den Heltec über den Heltec reicht der Strom nicht für 8 Relais.
2 x 4-Relais aus Platzgründen
Doppelseitige Platinen wenn man sich hier Brücken baut kann man auf der einen Seite den Heltec Stecken, hat eine Befestigung an der Türe und auf der Rückseite Zeitgleich Steckkontakte für die Verdrahtung


An die Kontakte zum noch Freien Relais geht es an die Türöffner.
Die Freien Kontakte auf der Platine gehen an die Türöffner als Lese Kontakte.


Einstellung Münzprüfer:
1 Impuls = 10 cent
2 Impulse = 20 cent
5 Impulse = 50 cent
10 Impulse = 1 euro
20 Impulse = 2 euro


Belegung:
PIN 34,35,36 jeweils zu den Schaltern mit einem Widerstand auf GND. Am Taster noch zusätzlich 3,3V anschließen
PIN 12,13,14 jeweils zu dem ersten Relais CH1-CH3 auf Taster für beleuchtung. Am Taster beleuchtung GND auf GND

PIN 25,26,27 jeweils zu dem zweiten Relais CH1-CH3 und von dort auf die Türöffner
PIN 37,38,39 jeweils mit einem Widerstand auf GND und mit den Schlosskontakten verbinden. Schlosskontakt zusätzlich an 3,3V anschließen

PIN 32 an 3,3V mit vorgeschalteten Widerstand an Münzzähler auf den PIN Counter oder Coin ist egal der andere PIN auf GND

PIN 33 wird mit einem Relais(CH4) und dem kleinen Freigabe Motor am Münzprüfer verbunden.
PIN 17 wird über einen Taster mit 3,3V verbunden SETUP Menü

Münzzähler mit 12V versorgen.
Der Heltec wird mit 5V über den 5V pin verbunden.
Bei den Relais VCC Brücke entfernen und am oberen PIN 5V anschließen, bei den CH eingängen VCC auf 3,3V anschließen (Optokoppler)

Um Masse probleme zu vermeiden 12V GND, 5V GND und Heltec GND alles miteinander verbinden.
