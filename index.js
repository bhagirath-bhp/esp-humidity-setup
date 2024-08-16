const express = require('express');
const bodyParser = require('body-parser');
const jwt = require('jsonwebtoken');
const bcrypt = require('bcrypt');

const app = express();
const port = 3000;
const SECRET_KEY = 'your_secret_key'; 

// Dummy user data for login authentication
const users = {
    'user1': 'password1',
};

app.use(bodyParser.json());

app.post('/login', (req, res) => {
    const { username, password } = req.body;
    console.log("login request received", req.body);

    if (!username || !password) return res.status(400).send('Username and password are required');

    const hashedPassword = users[username];

    if (!hashedPassword) return res.status(401).send('Invalid username');

    bcrypt.compare(password, hashedPassword, (err, result) => {
        if(password != hashedPassword){ //temporarily using this instead of bcrypt for dummy data
            if (err || !result) return res.status(401).send('Invalid password');
        }

        const token = jwt.sign({ username }, SECRET_KEY, { expiresIn: '1h' });
        console.log("token issued", token);
        res.json({ token });
    });
});

const authenticateToken = (req, res, next) => {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];

    if (token == null) return res.sendStatus(401);

    jwt.verify(token, SECRET_KEY, (err, user) => {
        if (err) return res.sendStatus(403);
        req.user = user;
        next();
    });
};

app.post('/data', authenticateToken, (req, res) => {
    const { humidity, temperature } = req.body;
    console.log(req.body);
    if (humidity == null || temperature == null) {
        return res.status(400).send('Humidity data is required');
    }

    console.log(`Received sensor data: ${humidity}% humidity, ${temperature}°C temperature`);
    res.send('Sensor data received successfully');
});

app.listen(port, "ip", () => {
    console.log(`Server is running on http://ip:${port}`);
});
