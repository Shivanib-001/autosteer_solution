const express = require('express');
const path = require('path');
const fs = require('fs');
const WebSocket = require('ws');

const app = express();
const PORT = 5000;

app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));



app.use(express.json()); 
app.use(express.urlencoded({ extended: true }));

app.use(express.static(path.join(__dirname, 'public')));

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.get('/tractor', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'tractor.html'));
});

app.get('/implement', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'implement.html'));
});

app.get('/farmland', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'farmland.html'));
});

app.get('/path', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'path.html'));
});

app.get('/settings', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'settings.html'));
});

app.get('/add-tractor', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'addTractor.html'));
});

app.get('/add-implement', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'addImplement.html'));
});

app.get('/farm', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'farm.html'));
});

app.get('/add-farm', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'addFarm.html'));
});

app.get('/plot-farm', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'plotFarm.html'));
});

app.get('/trace-farm', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'traceFarm.html'));
});

app.get('/helper', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'helper.html'));
});

app.get('/dashboard', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'dashboard.html'));
});
/*
app.get('/path', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'path.html'));
});*/

app.get('/add-path', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'addPath.html'));
});

app.get('/dropdown', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'dropdown.html'));
});

app.get('/operation', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'operation.html'));
});

app.get('/selected-farm', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'selectFarm.html'));
});

app.get('/settings', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'setting.html'));
});

app.use('/api', require('./routes'))



// const server = app.listen(PORT, () => {
//   console.log(`Server running at http://localhost:${PORT}`);
// });


const server = app.listen(PORT, '0.0.0.0', () => {
  console.log(`Server running on port http://localhost:${PORT}`);
});



const net = require('net');


const wss = new WebSocket.Server({ server });

let tcpClient = null;
let interval = null;

function connectTCP() {
    if (tcpClient) return;

    tcpClient = new net.Socket();

    tcpClient.connect(8080, '127.0.0.1', () => {
        console.log('TCP Connected');
    });

    tcpClient.on('data', (data) => {
        const message = data.toString();

        // console.log('TCP DATA =>', message);

        wss.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(message);
            }
        });
    });

    tcpClient.on('close', () => {
        console.log('TCP Closed');
        tcpClient = null;
    });

    tcpClient.on('error', (err) => {
        console.log('TCP Error', err);
        tcpClient = null;
    });
}

let streem = false;


app.get('/start-stream', (req, res) => {
  streem = true

  try {

    connectTCP();

    if (interval) {
      return res.send('Already Started');
    }

    interval = setInterval(() => {

      try {
        if (tcpClient) {
          // console.log('Requesting TCP Data...');
          tcpClient.write('GET_DATA');
        }

      } catch (err) {
        streem=false
        console.log('TCP Write Error:', err);
      }

    }, 3000);

    res.send('Started');

  } catch (err) {
    streem = false
    console.log('Start Stream Error:', err);

    res.status(500).json({
      success: false,
      message: 'Failed to start stream',
      error: err.message
    });
  }
});

app.get('/stop-stream', (req, res) => {
    streem=false
    clearInterval(interval);
    interval = null;

    console.log('Stopped polling');

    res.send('Stopped');
});

app.get('/check-streem', (req, res) => {
 res.status(200).json({ success: true, streem });
})
