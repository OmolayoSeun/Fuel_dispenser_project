static const char PROGMEM INDEX_HTML[] = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
    <title>Fuel Dispenser Setup page by Nodexihub</title>
</head>
<body>
    <h1>Form Example</h1>
    <form action="http://192.168.4.1/on" method="post">
        <label for="brandName">Brand Name:</label>
        <input type="text" id="brandName" name="brandName" required><br><br>

        <label for="location">Location:</label>
        <input type="text" id="location" name="location" required><br><br>

        <label for="ssid">SSID:</label>
        <input type="text" id="ssid" name="ssid" required><br><br>

        <label for="password">Password:</label>
        <input type="password" id="password" name="password" required><br><br>

        <input type="submit" value="Submit">
    </form>
</body>
</html>

)rawliteral";