import os
import time

from celery import Celery


app = Celery(
    "tasks",
    broker="redis://localhost:6379/0",
    backend="sqla+postgresql://user:password@database:5432/alpha",
)


@app.task(name="create_task")
def create_task(task_type):
    time.sleep(int(task_type) * 10)
    return True