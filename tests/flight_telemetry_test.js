const assert = require("node:assert/strict");
const fs = require("node:fs");
const vm = require("node:vm");
const {execFileSync} = require("node:child_process");
const {parseFlightPacket, LineBuffer} = require("../web_dashboard/telemetry.js");

assert.deepEqual(parseFlightPacket("SIM_FLIGHT,20,8000,2651,239,1464,46"),
  {seq:20,t:8,altitude:2651,distance:239,velocity:146.4,acceleration:0.46});
for (const line of [
  "", "IMU,1,2,3", "SIM_FLIGHT,1,0,0,0,0", "SIM_FLIGHT,1,0,0,0,0,100,extra",
  "SIM_FLIGHT,1,0,nan,0,0,100", "SIM_FLIGHT,1,0,0,-1,0,100",
  "SIM_FLIGHT,4294967296,0,0,0,0,100", "SIM_FLIGHT,1,133000,0,0,0,100"
]) assert.equal(parseFlightPacket(line), null, line);

const lines = [];
let overflows = 0;
const buffer = new LineBuffer(line => lines.push(line), () => overflows++);
buffer.push("noise\r\nSIM_FLI");
buffer.push("GHT,0,-12000,0,0,0,100\r\n");
buffer.push("x".repeat(1025) + "discard\nnext\n");
assert.deepEqual(lines, ["noise", "SIM_FLIGHT,0,-12000,0,0,0,100", "next"]);
assert.equal(overflows, 1);

const app = fs.readFileSync("web_dashboard/app.js", "utf8");
const modelSource = app.slice(0, app.indexOf("const clockStr="));
const model = vm.runInNewContext(modelSource + "\n({alt,down,vel,gee})");
const samples = execFileSync(process.argv[2], ["--samples"], {encoding:"utf8"}).trim().split("\n");
assert.equal(samples.length, 145);
for (const line of samples) {
  const [ms, altitude, distance, velocity, acceleration] = line.split(",").map(Number);
  const t = ms / 1000;
  assert.ok(Math.abs(altitude-model.alt(t)) <= 0.51, `altitude at ${t}`);
  assert.ok(Math.abs(distance-model.down(t)) <= 0.51, `downrange at ${t}`);
  assert.ok(Math.abs(velocity/10-model.vel(t)) <= 0.051, `velocity at ${t}`);
  assert.ok(Math.abs(acceleration/100-model.gee(t)) <= 0.0051, `acceleration at ${t}`);
}
