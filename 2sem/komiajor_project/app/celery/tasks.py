from app.celery.celery_conf import celery
from app.core.pathfinder import A_star
from app.websockets.websocket import send_ws_to_user
import asyncio

@celery.task(name="app.celery.tasks.long_task", bind=True)
def long_task(self, graph_data, start, end, user_id, task_id):
    # Отправка прогресса (фиктивный пример: 3 этапа)
    for i in range(1, 4):
        progress = i * 33
        asyncio.run(send_ws_to_user(user_id, {
            "status": "PROGRESS",
            "task_id": task_id,
            "progress": progress
        }))
        import time; time.sleep(1)  # Имитация долгой работы

    path, total_distance = A_star(graph_data, start, end)

    asyncio.run(send_ws_to_user(user_id, {
        "status": "COMPLETED",
        "task_id": task_id,
        "path": path,
        "total_distance": total_distance
    }))
    return {"path": path, "total_distance": total_distance}