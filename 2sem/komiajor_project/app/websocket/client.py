import asyncio
import websockets
import httpx
import json

async def main():
    async with websockets.connect("ws://localhost:8000/ws/test-client") as websocket:
        # Отправка задачи
        payload = {
            "graph": {
                "nodes": [1, 2, 3, 4],
                "edges": [[1, 2, 1], [2, 3, 2], [1, 4, 5], [3, 4, 1]]
            },
            "start": 1,
            "end": 4
        }

        async with httpx.AsyncClient() as client:
            response = await client.post("http://localhost:8000/start-task", json=payload)
            print("Task started:", response.json())

        # Ожидаем уведомление
        result = await websocket.recv()
        print("Task finished:", result)

asyncio.run(main())