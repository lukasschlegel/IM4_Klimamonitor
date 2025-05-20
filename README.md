# Klimamonitor

## Projektbeschreibung

### 4 Ablauf

#### 1. Projektidee und Konzeption
Zunächst hatten wir die Idee, eine Wetterstation zu bauen. Durch zwei Interviews im näheren Umfeld konnten wir zusätzliche Bedürfnisse identifizieren und in die Planung aufnehmen. Anschliessend erstellten wir ein Flussdiagramm, um den Ablauf – von der Datenerfassung über Sensoren bis hin zur Visualisierung auf einer Website – zu veranschaulichen.
Darauf basierend entwickelten wir in Figma ein UX-Design, das den Aufbau und die Funktionsweise der Weboberfläche grafisch darstellt.

![Alternativtext](img/Klimamonitor_Screenflow.png)

#### 2. Sensoranbindung und Datenerfassung
Zuerst wurde der ESP32-C6 mit den Sensoren DHT11 (Temperatur und Luftfeuchtigkeit) und einem Lichtsensor LM393-Modul (Lichtmessung) auf einem Breadboard verbunden. Die Werte wurden dann mithilfe des seriellen Monitors in der Arduino IDE ausgelesen, um die Funktion der Sensoren zu verifizieren.

#### 3. Datenbankeinrichtung und Anbindung
Anschliessend wurde auf dem Webserver eine Datenbank über phpMyAdmin erstellt. Die Tabelle enthielt Felder für Temperatur, Luftfeuchtigkeit, Lichtwert und Zeitstempel. Mit einem selbst geschriebenen PHP-Skript (load.php) wurde eine Schnittstelle entwickelt, über die die Sensordaten via HTTP POST an die Datenbank übertragen werden konnten.

#### 4. Energiesparmodus (Deep Sleep)
Um den Energieverbrauch zu minimieren, wurde der Deep-Sleep-Modus des ESP32 implementiert. Der Mikrocontroller schaltet sich automatisch stündlich ein, führt eine Messung durch, sendet die Daten an den Server und geht danach wieder in den Tiefschlaf. Ziel war es, die Station später draussen mit Akku betreiben zu können.

#### 5.	Wetterschutz und Gehäuse
Um die Elektronik vor Witterungseinflüssen zu schützen, wurde ein fertiges Thermometergehäuse beschafft, das sich aufgrund der Bauform ideal für Umgebungs-Messgeräte eignet, ohne die Messwerte zu verfälschen.

#### 6.	Anzeigeeinheit mit Buttonsteuerung
Zusätzlich wurde ein OLED-Display integriert, das auf Knopfdruck (über GPIO2) eine aktuelle Einzelmessung anzeigt. Diese Funktion unterbricht temporär den Deep Sleep. Zur Montage des Displays am Gehäuse wurde zunächst ein Prototyp aus Karton gefertigt, anschliessend das finale Displaygehäuse im 3D-Druck erstellt und direkt an die Thermometerhülle geklebt. So konnten auch der Button und das Display wetterfest untergebracht werden.

#### 7.	Verkabelung und Lötarbeiten
Einige Verbindungen, insbesondere für Stromversorgung und Signalverbindungen zum Display, wurden fest verlötet, da sich Steckverbindungen beim Einbau als instabil erwiesen hatten.

![Alternativtext](img/Klimamonitor_Steckplan.png)

#### 8.	Stromversorgung im Aussenbetrieb
Es wurde ein passender LiPo-Akku mit “Always-On”-Funktion beschafft, um den autonomen Betrieb im Freien sicherzustellen. Der Akku kann per USB-C geladen werden und versorgt die Wetterstation konstant mit Energie.

#### 9. Frontend & Datenvisualisierung
Abschliessend wurde eine Webseite mit HTML, CSS und JavaScript umgesetzt, auf der die Messdaten grafisch dargestellt werden. Die Ansicht lässt sich per Button zwischen Tages- und Wochenansicht umschalten. Zusätzlich kann die jeweils letzte Messung per Klick eingeblendet werden.
Die Seite enthält ausserdem ein manuell und automatisch schaltbares Darkmode-Feature: per Button (Mondsymbol oben rechts) oder automatisch bei Dunkelheit, sobald der Lichtwert unter eine definierte Schwelle fällt und dies in der Datenbank entsprechend vermerkt wird.
