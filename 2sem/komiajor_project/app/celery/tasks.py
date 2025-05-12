from celery.celery import celery
from app.main import connections
import time


@celery.task(bind=True)
def long_task(self, graph, start, end):
    import time
    from app.main import A_star
    time.sleep(3)
    path, cost = A_star(graph, start, end)
    
    client_id = self.request.id
    websocket = connections.get(client_id)
    if websocket:
        import asyncio
        asyncio.run(websocket.send_json({"path": path, "total_distance": cost}))
    
    return {"path": path, "total_distance": cost}

