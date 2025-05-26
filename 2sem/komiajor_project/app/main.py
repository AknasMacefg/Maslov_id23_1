from fastapi import FastAPI, Depends, Body
from app.schemas.schemas import PathIn, PathResult
from app.api.users.router import router as router_users
from app.api.users.dependencies import get_token
from app.schemas.schemas import PathResult, PathIn, Graph
from fastapi.responses import JSONResponse
from app.celery.tasks import A_star_task_add
from celery.result import AsyncResult
from app.core.funcs import A_star

import uuid
app = FastAPI()
@app.get("/")
def home_page():
    return {"message": "ИД23-1 МасловАН Коммивояжёр"}
app.include_router(router_users)

@app.post("/shortest-path", response_model=PathResult)
async def shortest_path(graph: PathIn, token: str = Depends(get_token)):
    path, total_distance = A_star(graph.graph, graph.start, graph.end)
    return PathResult(path=path, total_distance=total_distance)

@app.post("/add")
async def add(graph: PathIn, token: str = Depends(get_token)):
    task_id = str(uuid.uuid4())
    graph_dict = graph.graph.to_dict()
    A_star_task_add.apply_async(args=[graph_dict, graph.start, graph.end],task_id=task_id)
    return {"task_id": task_id}

@app.get("/status/{task_id}")
async def get_status(task_id: str):
    task_result = AsyncResult(task_id)
    return {
        "task_id": task_id,
        "status": task_result.status,
        "result": task_result.result if task_result.ready() else None
    }





