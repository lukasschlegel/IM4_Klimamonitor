<?php
// ============================
// WETTERSTATION – DATENAUSGABE
// ============================
//
// Übersicht: In diesem Script
// - wird eine Verbindung zur Datenbank aufgebaut
// - werden alle Messwerte der letzten 7 Tage abgefragt
// - werden Temperatur, Feuchtigkeit, Licht und Zeit sortiert zurückgegeben
// - wird die Ausgabe als JSON formatiert (für JavaScript abrufbar)
//

require_once("../../private/config_wetterstation.php");

try {
    $pdo = new PDO($dsn, $db_user, $db_pass, $options);
} catch (PDOException $e) {
    echo json_encode(["status" => "error", "message" => "DB-Verbindung fehlgeschlagen"]);
    exit;
}

// Letzte 7 Tage (168 Stunden) abfragen
$sql = "SELECT temperatur, feuchtigkeit, licht, zeit 
        FROM wetterstation 
        WHERE zeit >= NOW() - INTERVAL 7 DAY 
        ORDER BY zeit ASC";

$stmt = $pdo->prepare($sql);
$stmt->execute();
$daten = $stmt->fetchAll(PDO::FETCH_ASSOC);

header('Content-Type: application/json');
echo json_encode($daten);
?>
