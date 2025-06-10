Cílem tohoto projektu bylo vytvořit distribuovaný IoT systém pro monitorování a centrální 
správu vizuálních a zvukových upozornění v průmyslovém prostředí.
Vyrobený IoT systém 

byl vybaven jednotkou s mikrokontrolérem ESP32, je napájen baterií typu 18650 s cílem 
zajistit výdrž až 1 rok. K ESP32 byly připojeny relátka, která umožní detekci aktivity 
sledovaných zařízení. 

Jednotka je umístěna v 3D tištěné krabičce s magnety pro snadné 
připevnění k různým částem výrobních strojů, bez nutnosti zásahů do jejich konstrukce.  
Zařízení komunikuje přes WiFi pomocí MQTT protokolu s centrálním serverem, kde budou 
data zpracovávána v Node-Red. Data budou ukládána v časové databázi InfluxDB a 
zobrazována pomocí Grafany na monitoru ve výrobě, což poskytne vizuální přehled o stavu 
všech sledovaných zařízení.  

Systém slouží ke sledování stavu výroby pomocí signalizačních zařízení, jako jsou majáky, 
houkačky, a jiné upozorňovací systémy, s možností dálkového zobrazení jejich stavu. Tento 
systém bude umožňovat indikaci kritických stavů jako je porucha (červená), problém (žlutá) 
a normální stav (zelená) na dálkově připojených semaforech. 
