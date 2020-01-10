# Kernel Driver Hack
### Designed for APEX Legends

## Components

- "StrunderSv" Kernel Driver with memory Read/Write commands by Sockets
- "LocalStrunder" CPP application to connect and send the commands and do the wallhack/aimbot
- "AmazingWindow" Simply dummy transparent application to show the wallhack in topmost

## Features

- External DirectX 9 Wallhack
- Aimbot (Writing View Angles directly in the game memory)

## How to run

- Load StrunderSv into System memory with KDMapper (Basic Vulnerable Driver Exploit)
- Run AmazingWindow
- Run LocalStrunder
- Open Apex Legends

## KDMapper

KDMapper is an application that uses a vulnerable drivers to load bad drivers into system memory as ring0

- Download: https://github.com/alxbrn/kdmapper-1803-1903


## Note: Apex Legends Have "Easy Anti Cheat" and will ban your account if you use this source without rewrite anything