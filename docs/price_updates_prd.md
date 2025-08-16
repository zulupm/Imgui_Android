# Real-time Price Updates PRD

## Summary
The demo currently skips cryptocurrency price fetching when libcurl is missing and lacks real-time updates. This document outlines requirements to ensure price data is retrieved and refreshed live from Binance.

## Goals
- Ensure libcurl is available so REST price fetches always run.
- Stream real-time price updates from Binance's WebSocket API.
- Display the latest price and update ImPlot charts in real time.

## Requirements
1. **Libcurl Dependency**
   - Build system must provide libcurl. If a system installation is unavailable, fetch and compile libcurl during the build (via CMake's `FetchContent`).
   - Compilation should fail if libcurl cannot be resolved.
2. **WebSocket Integration**
   - Connect to Binance WebSocket endpoints (e.g. `wss://stream.binance.com:9443/ws/btcusdt@miniTicker`).
   - Parse incoming price messages and update on-screen values and charts.
   - Handle reconnection on errors.
3. **UI Updates**
   - Show the latest BTC and ETH prices in ImGui widgets.
   - Append new data points to ImPlot charts as messages arrive.
4. **Fallback Behaviour**
   - If WebSocket connection fails, retry periodically and optionally fall back to REST polling.

## Non-Goals
- Supporting exchanges other than Binance.
- Historical data storage beyond the session.

## Acceptance Criteria
- Demo builds with libcurl included and fails early if unavailable.
- Running the demo shows current prices and chart lines that move in real time.
- Disconnecting network results in reconnection attempts without crashes.
