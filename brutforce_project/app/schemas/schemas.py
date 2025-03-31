from pydantic import BaseModel, EmailStr, Field
import re


class SUserRegister(BaseModel):
    email: EmailStr = Field(..., description="Электронная почта")
    password: str = Field(..., min_length=5, max_length=50, description="Пароль, от 5 до 50 знаков")

class SUserAuth(BaseModel):
    email: EmailStr = Field(..., description="Электронная почта")
    password: str = Field(..., min_length=5, max_length=50, description="Пароль, от 5 до 50 знаков")

class STaskData(BaseModel):
    id: int = Field(..., description="Идентификатор")
    status: str = Field(..., min_length=1, max_length=50, description="Статус")
    result: str = Field(..., min_length=1, max_length=8, description="Результат")

class STaskDataAdd(BaseModel):
    status: str = 'Обработка...'
    result: str = 'Нет'