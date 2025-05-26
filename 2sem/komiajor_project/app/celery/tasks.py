import time
from app.schemas.schemas import Graph
from app.core.funcs import A_star
from celery import Celery
import redis
import json

app = Celery(
    "tasks",
    broker="redis://redis:6379/0",
    backend="db+postgresql://postgres:postgres@localhost:5432/FastAPIDB",
)



import redis
import json

r = redis.Redis(host='redis', port=6379)

def notify_websocket(task_id: str, message: dict):
    r.publish(f"task_updates:{task_id}", json.dumps(message))

@app.task
def A_star_task_add(graph_dict: dict, start: int, end: int, task_id: str):
    try:
        notify_websocket(task_id, {"status": "started"})
        for i in range(1, 6):
            notify_websocket(task_id, {"status": "progress", "percent": i*20})
            time.sleep(5)
        
        graph = Graph.from_dict(graph_dict)
        path, distance = A_star(graph, start, end)
        result = {"path": path, "distance": distance}
        notify_websocket(task_id, {
            "status": "completed",
            "result": result
        })
        
        return result
    except Exception as e:
        notify_websocket(task_id, {
            "status": "error",
            "message": str(e)
        })
        raise
            