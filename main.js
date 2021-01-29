const { app, BrowserWindow } = require('electron')
const { exec } = require("child_process")
const fetch = require('node-fetch')

const http = require('http'); // or 'https' for https:// URLs
const fs = require('fs');

const file = fs.createWriteStream("src/cpp/offsets.hpp");
console.log("Downloading offsets.hpp from github...")
fetch('https://raw.githubusercontent.com/LizzyThePone/lizacjs/master/src/cpp/Offsets.hpp')
    .then(res => res.body.pipe(file).on('finish', function() {
        console.log("Done!")
        console.log("Rebuilding dependancies...")
        exec("npm rebuild", (error, stdout, stderr) => {
            if (error) {
                console.log(`error: ${error.message}`);
                return;
            }
            if (stderr) {
                console.log(`stderr: ${stderr}`);
                return;
            }
            console.log(`${stdout}`);
        }).on("close", code => {
            if (code == 0) {
                function createWindow() {
                    const win = new BrowserWindow({
                        width: 400,
                        height: 550,
                        backgroundColor: "#222",
                        titleBarStyle: 'hiddenInset',
                        webPreferences: {
                            nodeIntegration: true,
                        }
                    })

                    win.setMenuBarVisibility(false)
                    win.loadFile('src/html/index.html')
                }

                app.whenReady().then(createWindow)

                app.on('window-all-closed', () => {
                    if (process.platform !== 'darwin') {
                        app.quit()
                    }
                })

                app.on('activate', () => {
                    if (BrowserWindow.getAllWindows().length === 0) {
                        createWindow()
                    }
                })
            } else {
                console.log("Unable to rebuild. Send this log to Lizzu#3486")
                setTimeout(process.exit, 20000)
            }

        })

    }))