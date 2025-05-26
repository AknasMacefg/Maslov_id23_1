from app.core.base import BaseDAO
from app.models.models import User, UniTask


class UsersDAO(BaseDAO):
    model = User

class TasksDAO(BaseDAO):
    task = UniTask