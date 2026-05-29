const router = require('express').Router()
const fs = require('fs');
const multer = require('multer');
const path = require('path');
const { spawn } = require("child_process");
const PathPlan = require('../path');
const { exec } = require("child_process");

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, 'public/image');
  },
  filename: (req, file, cb) => {
    const name = Date.now() + '-' + file.originalname;
    cb(null, name);
  }
});

const upload = multer({ storage });

router.post('/tractor', upload.single('image'), (req, res) => {
  const filePath = path.join(__dirname, '../public/data/tractor.json');

  const newTractor = {
    id: Date.now(),
    tractor_name: req.body.name,
    turning_radius: req.body.turning_radius,
    tractor_width: req.body.width,
    tractor_wheel_base: req.body.wheel_base,
    tractor_other: req.body.other,
    tractor_image: req.file
    ? `/image/${req.file.filename}`
    : null
  };

  let data = [];
  if (fs.existsSync(filePath)) {
    data = JSON.parse(fs.readFileSync(filePath));
  }

  data.push(newTractor);

  fs.writeFileSync(filePath, JSON.stringify(data, null, 2));

  res.json({ message: "Saved successfully" });
});

router.post('/implement', upload.single('image'), (req, res) => {
  const filePath = path.join(__dirname, '../public/data/implement.json');

  const newImplement = {
    id: Date.now(),
    implement_name: req.body.name,
    implement_type: req.body.implement_type,
    implement_width: req.body.width,
    implement_other: req.body.other,
    implement_image: req.file
    ? `/image/${req.file.filename}`
    : null
  };

  let data = [];
  if (fs.existsSync(filePath)) {
    data = JSON.parse(fs.readFileSync(filePath));
  }

  data.push(newImplement);

  fs.writeFileSync(filePath, JSON.stringify(data, null, 2));

  res.json({ message: "Saved successfully" });
});

router.delete('/tractor/:id', (req, res) => {
  const id = Number(req.params.id);
  const filePath = path.join(__dirname, '../public/data/tractor.json');

  try {

    let data = [];
    if (fs.existsSync(filePath)) {
      data = JSON.parse(fs.readFileSync(filePath, 'utf-8'));
    }

    const itemToDelete = data.find(item => item.id === id);

    const updatedData = data.filter(item => item.id !== id);

    fs.writeFileSync(filePath, JSON.stringify(updatedData, null, 2));

    if (itemToDelete && itemToDelete.tractor_image) {
      const imgPath = path.join(__dirname, '../public', itemToDelete.tractor_image);
      if (fs.existsSync(imgPath)) {
        fs.unlinkSync(imgPath);
      }
    }

    res.json({ message: "Deleted successfully" });

  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "Failed to delete" });
  }
});

router.delete('/implement/:id', (req, res) => {
  const id = Number(req.params.id);
  const filePath = path.join(__dirname, '../public/data/implement.json');

  try {
    let data = [];
    if (fs.existsSync(filePath)) {
      data = JSON.parse(fs.readFileSync(filePath, 'utf-8'));
    }

    const itemToDelete = data.find(item => item.id === id);

    const updatedData = data.filter(item => item.id !== id);

    fs.writeFileSync(filePath, JSON.stringify(updatedData, null, 2));

    if (itemToDelete && itemToDelete.implement_image) {
      const imgPath = path.join(__dirname, '../public', itemToDelete.implement_image);
      if (fs.existsSync(imgPath)) {
        fs.unlinkSync(imgPath);
      }
    }

    res.json({ message: "Deleted successfully" });

  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "Failed to delete" });
  }
});

router.post('/plotFarm', (req, res) => {
  const filePath = path.join(__dirname, '../public/data/farm.json');

  const newPlot = {
    id: Date.now(),
    farmName: req.body.farmName,
    coordinates: req.body.points
  };

  let data = [];
  if (fs.existsSync(filePath)) {
    data = JSON.parse(fs.readFileSync(filePath));
  }

  const exists = data.some(
    plot => plot.farmName.toLowerCase() === newPlot.farmName.toLowerCase()
  );

  if (exists) {
    return res.status(400).json({
      message: "Farm name already exists"
    });
  }

  data.push(newPlot);
  fs.writeFileSync(filePath, JSON.stringify(data, null, 2));

  res.json({ message: "Plot saved successfully" });
});

router.delete('/farm/:id', (req, res) => {
  const id = Number(req.params.id);
  const filePath = path.join(__dirname, '../public/data/farm.json');

  try {

    let data = [];
    if (fs.existsSync(filePath)) {
      data = JSON.parse(fs.readFileSync(filePath, 'utf-8'));
    }

    const itemToDelete = data.find(item => item.id === id);

    const updatedData = data.filter(item => item.id !== id);

    fs.writeFileSync(filePath, JSON.stringify(updatedData, null, 2));

    res.json({ message: "Deleted successfully" });

  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "Failed to delete" });
  }
});

router.delete('/path/:id', (req, res) => {
  const id = Number(req.params.id);
  const filePath = path.join(__dirname, '../public/data/path.json');

  try {

    let data = [];
    if (fs.existsSync(filePath)) {
      data = JSON.parse(fs.readFileSync(filePath, 'utf-8'));
    }

    const itemToDelete = data.find(item => item.id === id);

    const updatedData = data.filter(item => item.id !== id);

    fs.writeFileSync(filePath, JSON.stringify(updatedData, null, 2));

    res.json({ message: "Deleted successfully" });

  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "Failed to delete" });
  }
});

router.post('/savePath', (req, res) => {
  const filePath = path.join(__dirname, '../public/data/path.json');
  const pathPlanPath = path.join(__dirname, '../public/data/path_points.json');
  const pathPlan = JSON.parse(
      fs.readFileSync(pathPlanPath, 'utf8')
    );

  const newPath = {
    id: Date.now(),
    pathName: req.body.pathName,
    tractorName: req.body.tractorName,
    implementName: req.body.implementName,
    farmName: req.body.farmName,
    coordinates: req.body.coordinates,
    pathPlan: pathPlan
  };

  let data = [];
  if (fs.existsSync(filePath)) {
    data = JSON.parse(fs.readFileSync(filePath));
  }

  const exists = data.some(
    path => path.pathName.toLowerCase() === newPath.pathName.toLowerCase()
  );

  if (exists) {
    return res.status(400).json({
      message: "Path name already exists"
    });
  }

  data.push(newPath);
  fs.writeFileSync(filePath, JSON.stringify(data, null, 2));

  res.json({ message: "Path saved successfully" });
});

const save_path = path.join(__dirname, '../public/data/path_points.json');
router.post("/generatePath", (req, res) => {
  const { farm, tractor, implement } = req.body;
  console.log("Farm",farm)
  console.log("implement",implement)
  console.log("tractor",tractor)

  let width = implement.implement_width;
  let boundary = farm.coordinates;
  let radius = tractor.turning_radius;
  let wheelbase = tractor.tractor_wheel_base;

  console.log(typeof width, typeof boundary, typeof radius, typeof wheelbase)

  let gcp = [];

  for (let n = 0; n < boundary.length; n++) {
    const prev = (n - 1 + boundary.length) % boundary.length;

    gcp.push([
      [boundary[prev][0], boundary[prev][1]],
      [boundary[n][0], boundary[n][1]]
    ]);
  }
  const process = spawn("python3", ["generate_path.py", JSON.stringify(gcp),save_path, width, radius, wheelbase],{cwd: path.join(__dirname, '../src')});
  let output = "";
  let errorOutput = "";

  process.stdout.on("data", (data) => {
   // console.log("STDOUT CHUNK:", data.toString());
    output += data.toString();
  });

  process.stderr.on("data", (data) => {
    console.log("STDERR CHUNK:", data.toString());
    errorOutput += data.toString();
  });

  process.on("error", (err) => {
    console.error("PROCESS ERROR:", err);
  });

  process.on("close", (code) => {
    flag = code;
    console.log("PROCESS CLOSED with code:", code);
    console.log("FINAL OUTPUT:", output);
    console.log("FINAL ERROR:", errorOutput);
    if(output){
      const pathPoints = JSON.parse(output);
      console.log(pathPoints);
      res.json({ success: true, path: pathPoints });
    }
    else{
      res.status(500).json({ success: false, message: "Failed to generate path", error: errorOutput });
    }
  });
});

let cProcess = null;
let cProcessStatus = false;

router.get('/run-c-project', (req, res) => {
  const projectPath = path.join(__dirname, '../../gnss_tcp_server');

  if (cProcess) {
    return res.send('Already running');
  }

  cProcess = spawn('make', [], {   
    cwd: projectPath,
   // stdio: ['ignore', 'pipe', 'pipe']  // isolated - won't interfere with program 2
  });
  cProcessStatus = true;

  cProcess.stdout.on('data', (data) => {
    // console.log('[gnss_tcp_server STDOUT]:', data.toString());
  });

  cProcess.stderr.on('data', (data) => {
    console.error('[gnss_tcp_server STDERR]:', data.toString());
  });

  cProcess.on('close', (code) => {
    cProcess = null;
    cProcessStatus = false;
    console.log('[gnss_tcp_server] exited with code:', code);
  });

  cProcess.on('error', (err) => {
    console.error('[gnss_tcp_server] Error:', err);
    cProcess = null;
    cProcessStatus = false;
  });

  res.send('Started C program');
});

router.get('/stop-c-project', (req, res) => {
  if (!cProcess) {
    return res.send('Nothing running');
  }

  try {
    cProcess.kill('SIGTERM');  

    setTimeout(() => {
      if (cProcess) {
        cProcess.kill('SIGKILL');
      }
    }, 2000);

    cProcess = null;
    cProcessStatus = false;
    if(operationProcess){
        try {
          operationProcess.kill('SIGTERM');  

          setTimeout(() => {
            if (operationProcess) {
              operationProcess.kill('SIGKILL');
            }
          }, 2000);

          operationProcess = null;
          operationStatus = false;
          console.log("operation stopped")
        } catch (err) {
          console.log("failed to stop operations")
        }
    }
    res.send('Stopped');
  } catch (err) {
    res.send('Error stopping: ' + err.message);
  }
});



let operationProcess = null;
let operationStatus = false;

router.get('/run-operation', (req, res) => {
  const projectPath = path.join(__dirname, '../../navigation-client');

  if (operationProcess) {
    return res.send('Already running');
  }

  operationProcess = spawn('make', [], {   
    cwd: projectPath,
   // stdio: ['ignore', 'pipe', 'pipe']  // isolated - won't steal stdio from program 1
  });
  operationStatus = true;

  operationProcess.stdout.on('data', (data) => {
    //console.log('[navigation-client STDOUT]:', data.toString());
  });

  operationProcess.stderr.on('data', (data) => {
    console.error('[navigation-client STDERR]:', data.toString());
  });

  operationProcess.on('close', (code) => {
    console.log('[navigation-client] exited with code:', code);
    operationProcess = null;
    operationStatus = false;
  });

  operationProcess.on('error', (err) => {
    console.error('[navigation-client] Error:', err);
    operationProcess = null;
    operationStatus = false;
  });

  res.send('Started operation');
});


// router.get('/run-operation', (req, res) => {
//   const projectPath = path.join(__dirname, '../operation');
//   const jsonPath = path.join(__dirname, '../public/data/flag.json'); 
//   if (operationProcess) {
//     return res.send('Already running');
//   }

//   //  Read and update JSON
//   try {
//     console.log(jsonPath)
//     const data = JSON.parse(fs.readFileSync(jsonPath, 'utf-8'));
//     data.operation = true;

//     fs.writeFileSync(jsonPath, JSON.stringify(data, null, 2));
//     console.log('Operation flag set to TRUE');
//   } catch (err) {
//     console.error('Error updating JSON:', err);
//   }

//   //  Start C program
//   operationProcess = spawn('./main', [], {   
//     cwd: projectPath,
//     stdio: 'inherit'
//   });
//   operationStatus = true;
//   operationProcess.on('close', (code) => {
//     console.log('Process exited with code:', code);
//     operationProcess = null;
//     operationStatus = false;

//     //  Set flag back to false when stopped
//     try {
//       const data = JSON.parse(fs.readFileSync(jsonPath, 'utf-8'));
//       data.operation = false;

//       fs.writeFileSync(jsonPath, JSON.stringify(data, null, 2));
//       console.log('Operation flag set to FALSE');
//     } catch (err) {
//       console.error('Error updating JSON:', err);
//     }
//   });

//   res.send('Started C program');
// });



router.get('/stop-operation', (req, res) => {
  console.log("Stop operation called")
  if (!operationProcess) {
    return res.send('Nothing running');
  }

  try {
    operationProcess.kill('SIGTERM');  

    setTimeout(() => {
      if (operationProcess) {
        operationProcess.kill('SIGKILL');
      }
    }, 2000);

    operationProcess = null;
    operationStatus = false;
    res.send('Stopped');
  } catch (err) {
    res.send('Error stopping: ' + err.message);
  }
});

router.get('/check-status', (req, res) => {
   res.status(200).json({ success: true, cProcessStatus, operationStatus });
});

router.post('/updatePathPlan/:id', (req, res) => {
  const id = Number(req.params.id);
  const filePath = path.join(__dirname, '../public/data/path.json');
  const pathPlanPath = path.join(__dirname, '../public/data/path_points.json');
  try {

    let data = [];
    if (fs.existsSync(filePath)) {
      data = JSON.parse(fs.readFileSync(filePath, 'utf-8'));
    }

    const path = data.find(item => item.id === id);
    console.log(path.pathPlan)

    fs.writeFileSync(pathPlanPath, JSON.stringify(path.pathPlan, null, 2));

    res.json({ message: "Updated successfully" });

  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "Failed to update" });
  }
})


router.get("/shutdown", (req, res) => {

    const scriptPath = path.join(__dirname, "../src/shutdown_pi.sh");

    exec(scriptPath, (error, stdout, stderr) => {

        if (error) {
            console.error("Error:", error.message);

            return res.status(500).json({
                success: false,
                error: error.message
            });
        }

        if (stderr) {
            console.error("stderr:", stderr);
        }

        console.log("stdout:", stdout);

        res.json({
            success: true,
            message: "Shutdown scheduled successfully",
            output: stdout
        });
    });
});

router.get("/cancel-shutdown", (req, res) => {

    exec("sudo shutdown -c", (error, stdout, stderr) => {

        if (error) {
            return res.status(500).json({
                success: false,
                error: error.message
            });
        }

        res.json({
            success: true,
            message: "Shutdown cancelled"
        });
    });
});

router.use((req, res) => {
    res.status(404).json({ success: false, message: "Route not found." });
});

module.exports = router
