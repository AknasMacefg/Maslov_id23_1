import os
import time
from app.schemas.schemas import Graph
from app.core.funcs import A_star
from celery import Celery


app = Celery(
    "tasks",
    broker="redis://localhost:6379/0",
    backend="db+postgresql://postgres:postgres@localhost:5432/FastAPIDB",
)

@app.task
def A_star_task_add(graph_dict: dict, start: int, end: int) -> tuple[list[int], float]:
    time.sleep(5)  # Simulate processing time
    graph = Graph.from_dict(graph_dict)
    return A_star(graph, start, end)