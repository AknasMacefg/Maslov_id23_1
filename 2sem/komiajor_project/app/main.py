from fastapi import FastAPI, Depends, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from app.celery.tasks import long_task
import uuid
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

connections = {}

@app.websocket("/ws/{client_id}")
async def websocket_endpoint(websocket: WebSocket, client_id: str):
    await websocket.accept()
    connections[client_id] = websocket
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        del connections[client_id]

@app.post("/start-task")
async def start_task(graph: PathIn, token: str = Depends(get_token)):
    task_id = str(uuid.uuid4())
    task = long_task.apply_async(args=[graph.graph.dict(), graph.start, graph.end], task_id=task_id)
    return {"task_id": task.id}

#{ "graph": { "nodes": [1, 2, 3, 4], "edges": [[1, 2, 1], [2, 3, 2], [1, 4, 5], [3, 4, 1]] }, "start": 1, "end": 4 }
def A_star(graph: Graph, start: int, end: int) -> Tuple[List[int], float]:
    adjacency_list: Dict[int, List[Tuple[int, float]]] = {node: [] for node in graph.nodes}
    for u, v, weight in graph.edges:
        adjacency_list[u].append((v, weight))
        adjacency_list[v].append((u, weight))

    open_set = [(0, start, [])]  # (cost, current_node, path)
    visited = set()

    while open_set:
        cost, node, path = heapq.heappop(open_set)
        if node in visited:
            continue
        path = path + [node]
        if node == end:
            return path, cost
        visited.add(node)
        for neighbor, weight in adjacency_list[node]:
            if neighbor not in visited:
                heapq.heappush(open_set, (cost + weight, neighbor, path))

    return [], float('inf')