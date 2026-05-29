const fs = require("fs");

class PathPlan {
    constructor(gcp, savePath, applicationWidth, turningRadius, tractorWheelbase) {
        this.gcp = gcp;
        this.savePath = savePath;
        this.applicationWidth = applicationWidth;
        this.turningRadius = turningRadius;
        this.tractorWheelbase = tractorWheelbase;
    }

    path() {
        const { tp, headland } = this.pathPlanning(
            this.gcp,
            this.applicationWidth,
            this.turningRadius,
            this.tractorWheelbase
        );

        this.savePathToFile(tp, headland);
        return { tp, headland };
    }

    savePathTxt(data) {
        let content = "";
        for (let item of data) {
            content += `${item[0]},${item[1]}\r\n`;
        }

        fs.writeFileSync(this.savePath, content);
        console.log("path data stored to file in data/path_points.txt");
    }

    savePathToFile(pathPoints, headland) {
        const data = {
            farm_boundary: this.gcp,
            Application_width: this.applicationWidth,
            Turning_radius: this.turningRadius,
            Tractor_wheelbase: this.tractorWheelbase,
            path_points: pathPoints,
            headland: headland
        };

        fs.writeFileSync(this.savePath, JSON.stringify(data, null, 4));
        console.log("Data saved to", this.savePath);
    }

    pathPlanning(gcp, applicationWidth, turningRadius, tractorWheelbase) {
        // define variables
        let finalTrack = [];
        let filtSide = [];
        let sides = [];
        let sidesK = [];
        let trak = [];
        let green = [];
        let infData = {};
        let blue = [];
        let trakk = [];
        let defa = [];
        let turnss = 3;
        let boundary = [];

        for (let i of gcp) {
            boundary.push(i[0]);
        }

        return {
            tp: boundary,
            headland: boundary
        };
    }
}

module.exports = PathPlan;