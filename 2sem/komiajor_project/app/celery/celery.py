from celery import Celery
from redislite import Redis

redis = Redis('/tmp/redis.db')  # создаёт файл-базу для Redis

celery = Celery(
    'tasks',
    broker=redis.connection_pool.connection_class().connection_class().connection_pool.connection_kwargs["path"],
    backend='redis+socket:///tmp/redis.db'
)