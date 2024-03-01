#ifndef HTML_TEMPLATE_H
#define HTML_TEMPLATE_H

const char* htmlTemplate = R"(
<html>
  <head>
    <style>
      body {
        font-family: Arial, sans-serif;
      }
      .button-group {
        display: flex;
        gap: 10px; /* Abstand zwischen den Gruppen */
      }
      .button {
        display: inline-block;
        padding: 10px 20px;
        margin-right: 10px; /* Abstand zwischen den Buttons innerhalb einer Gruppe */
        font-size: 16px;
        cursor: pointer;
        text-align: center;
        text-decoration: none;
        outline: none;
        color: #fff;
        background-color: #4CAF50;
        border: none;
        border-radius: 5px;
        box-shadow: 0 4px #357a38;
        background-image: url('hintergrund.jpg'); /* Pfad zur JPG-Datei */
        background-size: cover; /* Das Bild so anpassen, dass es den gesamten Bildschirm abdeckt */
        background-position: center; /* Das Bild in der Mitte positionieren */
      }
      .button:hover {
        background-color: #45a049;
      }
      .button2 {
        background-color: #f44336;
      }
      .button2:hover {
        background-color: #d32f2f;
      }
    </style>
  </head>
  <body>
    <h1>Fahrmodus Websteuerung</h1>
<div id="uhrzeitAnzeige"></div>

<script>
  function aktualisiereUhrzeit() {
    // Erstelle ein neues Date-Objekt, um die aktuelle Uhrzeit zu erhalten
    var jetzt = new Date();

    // Extrahiere Stunden, Minuten und Sekunden
    var stunden = jetzt.getHours();
    var minuten = jetzt.getMinutes();
    var sekunden = jetzt.getSeconds();

    // Formatiere die Uhrzeit als HH:MM:SS
    var uhrzeitText = stunden + ":" + (minuten < 10 ? "0" : "") + minuten + ":" + (sekunden < 10 ? "0" : "") + sekunden;

    // Aktualisiere den Inhalt des HTML-Elements mit der ID 'uhrzeitAnzeige' mit der aktuellen Uhrzeit
    document.getElementById("uhrzeitAnzeige").innerText = "Aktuelle Uhrzeit: " + uhrzeitText;
  }

  // Aktualisiere die Uhrzeit alle Sekunde (1000 Millisekunden)
  setInterval(aktualisiereUhrzeit, 1000);
</script>
    <p>Standardroute:</p>
    <p><a href="/Standardroute"><button class="button" %s>ON</button></a></p>

    <p>Zurückfahren:</p>
    <p><a href="/Zurückfahren"><button class="button button2" %s>OFF</button></a></p>

    <p>Kalibrierung:</p>
    <p><a href="/Kalibrierung"><button class="button" %s>ON</button></a></p>

    <div class="button-group">
      <div>
        <p>oben links:</p>
        <p><a href="/oben links"><button class="button" %s>ON</button></a></p>
      </div>

      <div>
        <p>oben rechts:</p>
        <p><a href="/oben rechts"><button class="button" %s>ON</button></a></p>
      </div>
    </div>

    <div class="button-group">
      <div>
        <p>unten links:</p>
        <p><a href="/unten links"><button class="button" %s>ON</button></a></p>
      </div>

      <div>
        <p>unten rechts:</p>
        <p><a href="/unten rechts"><button class="button" %s>ON</button></a></p>
      </div>
    </div>
  </body>
</html>
)";

#endif // HTML_TEMPLATE_H
