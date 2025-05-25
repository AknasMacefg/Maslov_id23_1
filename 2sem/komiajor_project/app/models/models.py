from sqlalchemy import ForeignKey, JSON
from sqlalchemy.orm import relationship, Mapped, mapped_column
from app.db.database import Base, str_uniq, int_pk
from datetime import date


# создаем модель таблицы пользователей
class User(Base):
    id: Mapped[int_pk]
    email: Mapped[str_uniq]
    password: Mapped[str]


    def __str__(self):
        return (f"{self.__class__.__name__}(id={self.id}, "
                f"email={self.email!r}, password={self.password}")

    def __repr__(self):
        return str(self)

class UniTask(Base):
    task_id: Mapped[int_pk]
    client_id: Mapped[int] = mapped_column(ForeignKey('users.id', ondelete='CASCADE'), nullable=False)
    status: Mapped[str]
    progress: Mapped[int]
    task: Mapped[dict] = mapped_column(JSON)
    result: Mapped[dict] = mapped_column(JSON)
    client: Mapped["User"] = relationship("User", backref='tasks')


    def __str__(self):
        return (f"{self.__class__.__name__}(task_id={self.task_id}, "
                f"status={self.status}, progress={self.progress}, task={self.task}, result={self.result}")

    def __repr__(self):
        return str(self)
