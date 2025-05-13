from fastapi import WebSocket
from typing import Dict

# Dict[user_id, WebSocket]
active_connections: Dict[str, WebSocket] = {}

async def connect_ws(websocket: WebSocket, user_id: int):
    await websocket.accept()
    active_connections[user_id] = websocket

async def disconnect_ws(user_id: int):
    if user_id in active_connections:
        del active_connections[user_id]

async def send_ws_to_user(user_id: int, message: dict):
    if user_id in active_connections:
        await active_connections[user_id].send_json(message)