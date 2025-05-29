<?php
// ============================
// WETTERSTATION – DATENEMPFANG
// ============================
//
// Übersicht: In diesem Script
// - wird eine Datenbankverbindung hergestellt
// - werden JSON-Daten vom ESP32 empfangen
// - wird geprüft, ob Temperatur, Feuchtigkeit und Licht übermittelt wurden
// - werden diese Werte in die Datenbank gespeichert
// - wird eine JSON-Antwort zurückgegeben
//

require_once("../../private/config_wetterstation.php");

try {
    $pdo = new PDO($dsn, $db_user, $db_pass, $options);
} catch (PDOException $e) {
    echo json_encode(["status" => "error", "message" => "DB-Verbindung fehlgeschlagen"]);
    exit;
}

// JSON-Daten empfangen
$inputJSON = file_get_contents('php://input');
$input = json_decode($inputJSON, true);

// Prüfen, ob alle Werte vorhanden sind
if (isset($input["temperatur"]) && isset($input["feuchtigkeit"]) && isset($input["licht"])) {
    $temperatur = $input["temperatur"];
    $feuchtigkeit = $input["feuchtigkeit"];
    $licht = $input["licht"];

    // In Datenbank speichern
    $sql = "INSERT INTO wetterstation (temperatur, feuchtigkeit, licht) VALUES (?, ?, ?)";
    $stmt = $pdo->prepare($sql);
    $stmt->execute([$temperatur, $feuchtigkeit, $licht]);

    echo json_encode(["status" => "success", "message" => "Werte gespeichert"]);
} else {
    echo json_encode(["status" => "error", "message" => "Unvollständige Daten"]);
}
?>
