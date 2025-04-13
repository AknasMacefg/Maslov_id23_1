from sqlalchemy import ForeignKey, text, Text
from sqlalchemy.orm import relationship, Mapped, mapped_column
from app.db.database import Base, str_uniq, int_pk, str_null_true
from datetime import date


# создаем модель таблицы пользователей
class User(Base):
    id: Mapped[int_pk]
    email: Mapped[str_uniq]
    password: Mapped[str]


    def __str__(self):
        return (f"{self.__class__.__name__}(id={self.id}, "
                f"email={self.email!r},")

    def __repr__(self):
        return str(self)
    
class Task(Base):
    id: Mapped[int_pk]
    status: Mapped[str]
    result: Mapped[str]


    def __str__(self):
        return (f"{self.__class__.__name__}(id={self.id}, "
                f"status={self.status!r},")

    def __repr__(self):
        return str(self)
