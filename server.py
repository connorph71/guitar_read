import asyncio
import websockets
import sys

async def handler(websocket):
    print("Client connected")
    for line in sys.stdin:
        await websocket.send(line.strip())

async def main():
    print("Starting server...")
    async with websockets.serve(handler, "0.0.0.0", 8766):
        await asyncio.Future()

asyncio.run(main())