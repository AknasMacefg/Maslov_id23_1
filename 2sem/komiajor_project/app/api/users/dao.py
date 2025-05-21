from core.base import BaseDAO
from models.models import User


class UsersDAO(BaseDAO):
    model = User
    