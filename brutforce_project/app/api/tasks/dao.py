from app.core.base import BaseDAO
from app.models.models import Task


class TasksDAO(BaseDAO):
    model = Task