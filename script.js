let port;
let writer;
let reader;
let remainingAttempts = 3;
let alarmTriggered = false;
let isConnected = false;
const correctUsername = "admin";
const correctPassword = "1234";
const connectBtn = document.getElementById("connectBtn");
const loginBtn = document.getElementById("loginBtn");
const connectionStatus = document.getElementById("connectionStatus");
const statusMessage = document.getElementById("statusMessage");
const attemptsDisplay = document.getElementById("attempts");
connectBtn.addEventListener("click", connectArduino);
loginBtn.addEventListener("click", loginUser);
async function connectArduino() {
 try {
 if (!("serial" in navigator)) {
 alert("Use Google Chrome or Microsoft Edge with Web Serial support.");
 return;
 }
 port = await navigator.serial.requestPort();
 await port.open({ baudRate: 9600 });
const encoder = new TextEncoderStream();
 encoder.readable.pipeTo(port.writable);
 writer = encoder.writable.getWriter();
 const decoder = new TextDecoderStream();
 port.readable.pipeTo(decoder.writable);
 reader = decoder.readable.getReader();
 connectionStatus.textContent = "Connected";
 statusMessage.textContent = "Arduino connected";
 isConnected = true;
 // Important: wait for Arduino Uno reset after serial connection
 await delay(2000);
 await sendCommand("CONNECTED");
 readArduinoMessages();
 } catch (error) {
 console.error(error);
 connectionStatus.textContent = "Connection Failed";
 statusMessage.textContent = "Unable to connect Arduino";
 }
}
async function sendCommand(command) {
 if (!writer || !isConnected) {
 alert("Please connect the Arduino first.");
 return;
 }
 await writer.write(command + "\n");
}
async function loginUser() {
 if (!isConnected) {
 alert("Please connect the Arduino first.");
 return;
 }
 if (alarmTriggered) {
 statusMessage.textContent = "System locked: Intruder Alarm";
 return;
 }
 const username = document.getElementById("username").value.trim();
 const password = document.getElementById("password").value.trim();
 if (username === correctUsername && password === correctPassword) {
 statusMessage.textContent = "Authorized user";
 remainingAttempts = 3;
 attemptsDisplay.textContent = remainingAttempts;
 await sendCommand("AUTHORIZED");
 } else {
 remainingAttempts--;
 if (remainingAttempts < 0) remainingAttempts = 0;
 attemptsDisplay.textContent = remainingAttempts;
 statusMessage.textContent = "Unauthorized user";
 await sendCommand("UNAUTHORIZED");
 if (remainingAttempts === 0) {
 alarmTriggered = true;
 statusMessage.textContent = "Intruder Alarm";
 }
 }
}
async function readArduinoMessages() {
 let buffer = "";
 try {
 while (true) {
 const { value, done } = await reader.read();
 if (done) break;
 buffer += value;
 let lines = buffer.split("\n");
 buffer = lines.pop();
 for (let line of lines) {
 line = line.trim();
 if (!line) continue;
 console.log("Arduino:", line);
 if (line === "ARDUINO_CONNECTED") {
 statusMessage.textContent = "Arduino ready";
 } else if (line === "LOGIN_OK") {
 statusMessage.textContent = "Authorized user";
 } else if (line.startsWith("LOGIN_FAIL:")) {
 statusMessage.textContent = "Unauthorized user";
 } else if (line === "ALARM_ON" || line === "ALARM_LOCKED") {
 alarmTriggered = true;
 remainingAttempts = 0;
 attemptsDisplay.textContent = "0";
 statusMessage.textContent = "Intruder Alarm";
 }
 }
 }
 } catch (error) {
 console.error("Read error:", error);
 }
}
function delay(ms) {
 return new Promise(resolve => setTimeout(resolve, ms));
}
