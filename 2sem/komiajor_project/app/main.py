from fastapi import FastAPI, Depends, WebSocket, WebSocketDisconnect, Request
from fastapi.middleware.cors import CORSMiddleware
from app.schemas.schemas import PathIn, PathResult
from app.api.users.router import router as router_users
from app.api.users.dependencies import get_token
from app.schemas.schemas import PathResult, PathIn
from app.celery.tasks import A_star_task_add
from celery.result import AsyncResult
from app.core.funcs import A_star
from fastapi.templating import Jinja2Templates
import uuid
import redis
import asyncio
import json


active_connections: dict[str, WebSocket] = {}
app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


templates = Jinja2Templates(directory="app/templates")

@app.get("/")
def home_page(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})
app.include_router(router_users)

@app.post("/shortest-path", response_model=PathResult)
async def shortest_path(graph: PathIn, token: str = Depends(get_token)):
    path, total_distance = A_star(graph.graph, graph.start, graph.end)
    return PathResult(path=path, total_distance=total_distance)

@app.post("/add")
async def add(graph: PathIn, token: str = Depends(get_token)):
    task_id = str(uuid.uuid4())
    graph_dict = graph.graph.to_dict()
    A_star_task_add.apply_async(args=[graph_dict, graph.start, graph.end, task_id], task_id=task_id)
    return {"task_id": task_id}

@app.get("/status/{task_id}")
async def get_status(task_id: str):
    task_result = AsyncResult(task_id)
    return {
        "task_id": task_id,
        "status": task_result.status,
        "result": task_result.result if task_result.ready() else None
    }


r = redis.Redis(host='redis', port=6379)

@app.websocket("/ws/{task_id}")
async def websocket_endpoint(websocket: WebSocket, task_id: str):
    await websocket.accept()
    pubsub = r.pubsub()
    pubsub.subscribe(f"task_updates:{task_id}")

    try:
        while True:
            message = pubsub.get_message(ignore_subscribe_messages=True, timeout=1)
            if message:
                data = message['data'].decode('utf-8')
                await websocket.send_text(data)
                try:
                    message_data = json.loads(data)
                    if message_data.get("status") == "COMPLETED":
                        await websocket.close(code=1000)
                        break
                except json.JSONDecodeError:
                    continue
            try:
                data = await asyncio.wait_for(websocket.receive_text(), timeout=0.1)
            except asyncio.TimeoutError:
                pass
                
    except WebSocketDisconnect:
        print(f"Client {task_id} disconnected")
    finally:
        pubsub.close()



