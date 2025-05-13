from fastapi import FastAPI, Depends, WebSocket, WebSocketDisconnect, BackgroundTasks, Query
from app.websockets.websocket import connect_ws, disconnect_ws, send_ws_to_user
from app.celery.tasks import long_task
from app.schemas.schemas import PathIn, PathResult
import uuid
from app.core.pathfinder import A_star
from app.api.users.router import router as router_users
from app.api.users.dependencies import get_token
from app.schemas.schemas import PathResult, PathIn, Graph
from typing import List, Tuple, Dict
import heapq

app = FastAPI()
@app.get("/")
def home_page():
    return {"message": "ИД23-1 МасловАН Коммивояжёр"}
  
app.include_router(router_users)

@app.post("/shortest-path", response_model=PathResult)
async def shortest_path(graph: PathIn, token: str = Depends(get_token)):
    path, total_distance = A_star(graph.graph, graph.start, graph.end)
    return PathResult(path=path, total_distance=total_distance)


@app.websocket("/ws/{user_id}")
async def websocket_endpoint(websocket: WebSocket, user_id: int):
    await connect_ws(websocket, user_id)
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        await disconnect_ws(user_id)

@app.post("/start-task")
async def start_task(
    graph: PathIn,
    user_id: str = Query(...),
    background_tasks: BackgroundTasks = None
):
    task_id = str(uuid.uuid4())
    
    # Уведомим о старте
    await send_ws_to_user(user_id, {
        "status": "STARTED",
        "task_id": task_id,
        "message": "Задача запущена"
    })

    long_task.apply_async(
        args=[graph.graph.model_dump_json(), graph.start, graph.end, user_id, task_id]
    )
    return {"task_id": task_id}


#{ "graph": { "nodes": [1, 2, 3, 4], "edges": [[1, 2, 1], [2, 3, 2], [1, 4, 5], [3, 4, 1]] }, "start": 1, "end": 4 }
