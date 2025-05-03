# GPS Tracker for Particle Electron

This project collects GPS data from [Particle Electron](https://docs.particle.io/electron/) devices and transmits it via cellular connection to a server endpoint.

## Features

- Periodic GPS acquisition using `TinyGPS` and `AssetTracker`
- Sends data to a configurable HTTP server
- Designed for low-power usage in the field

## Requirements

- Particle Electron with AssetTracker shield
- SIM card with data plan
- External battery for field deployment
- Server endpoint to send the data to

## Configuration

The HTTP endpoint is defined via (lines 43-45):

```cpp
#define HOST "127.0.0.1"
#define PATH "/server/electron"
#define PORT 80
```

Update these values to match your server setup.

## Deployment
1. Set up your server endpoint.
2. Flash the firmware via Particle Workbench or Particle CLI on your Particle Electron device.
3. Wait for incoming data.
