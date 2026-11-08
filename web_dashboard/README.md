# Pathfinder II local dashboard

This is a standalone browser-based telemetry demo and does not connect to a live vehicle or serial port. It uses only static HTML, CSS, and JavaScript.

Use the phase buttons or time slider to inspect the simulated flight, or select
**Fly it** to play it back. The summary can export the simulated flight to CSV.
The serial port list and earlier flights are mockup data; arming and saved-flight
replay do not control or load a real vehicle.

From the repository root, start the local server with:

```sh
python3 -m http.server 8000 --directory web_dashboard
```

Then open <http://localhost:8000>. Stop the server with Ctrl+C.
