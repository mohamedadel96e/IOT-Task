const express = require("express");

app = express();
const PORT = process.env.PORT || 3000;

app.get("/", (req, res) => {
    res.sendFile(__dirname + "/views/index.html");
});

server = app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});

const socketServer = require("ws").Server;
const wss = new socketServer({server});

let clients = [];
let brightness = 0;

wss.on("connection", (ws) => {
    clients.push(ws);
    console.log("New client connected");
    // Send the current brightness value to the newly connected client
    ws.send(JSON.stringify({ brightness }));
    ws.on("message", (message) => {
        let data = JSON.parse(message);
        brightness = data.brightness;
        clients.forEach((client) => {
            if (client.readyState === client.OPEN) {
                client.send(JSON.stringify({ brightness }));
            }
        });
    });

    ws.on("close", () => {
        clients = clients.filter((client) => client !== ws);
        console.log("Client disconnected");
    });
});

