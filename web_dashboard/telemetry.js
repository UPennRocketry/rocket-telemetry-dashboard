(function(root) {
  "use strict";

  function parseFlightPacket(line) {
    const fields = line.trim().split(",");
    if (fields.length !== 7 || fields[0] !== "SIM_FLIGHT" ||
        fields.slice(1).some(value => !/^-?\d+$/.test(value))) return null;
    const [seq, timeMs, altitude, distance, velocity10, acceleration100] =
      fields.slice(1).map(Number);
    if (![seq, timeMs, altitude, distance, velocity10, acceleration100].every(Number.isSafeInteger) ||
        seq < 0 || seq > 4294967295 || timeMs < -12000 || timeMs > 132000 ||
        altitude < 0 || altitude > 3600 || distance < 0 || distance > 2400 ||
        velocity10 < 0 || velocity10 > 2840 || acceleration100 < 0 || acceleration100 > 1220) return null;
    return {seq, t: timeMs / 1000, altitude, distance,
            velocity: velocity10 / 10, acceleration: acceleration100 / 100};
  }

  class LineBuffer {
    constructor(onLine, onOverflow) {
      this.buffer = "";
      this.discarding = false;
      this.onLine = onLine;
      this.onOverflow = onOverflow;
    }
    push(text) {
      for (const char of text) {
        if (char === "\n") {
          if (!this.discarding) this.onLine(this.buffer.replace(/\r$/, ""));
          this.buffer = "";
          this.discarding = false;
        } else if (!this.discarding) {
          this.buffer += char;
          if (this.buffer.length > 1024) {
            this.buffer = "";
            this.discarding = true;
            this.onOverflow();
          }
        }
      }
    }
  }

  const api = {parseFlightPacket, LineBuffer};
  if (typeof module !== "undefined" && module.exports) module.exports = api;
  else root.FlightTelemetry = api;
})(globalThis);
