from celery import Celery
celery = Celery(
    "tasks",
    broker="redis://localhost:6379/0",  # Redis как брокер
    backend="redis://localhost:6379/0",  # PostgreSQL как бэкенд
)

celery.conf.update(
    task_track_started=True,
    result_extended=True,
)