from pydantic import BaseModel, EmailStr, Field
import re
from typing import List, Tuple


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
    id: int = Field(..., description="Идентификатор")
    max_length: int = Field(..., description="Длина пароля, от 1 до 8")

class Graph(BaseModel):
    nodes: List[int] = Field(...)
    edges: List[Tuple[int, int, float]] = Field(...)

class PathIn(BaseModel):
    graph: Graph
    start: int = Field(...)
    end: int = Field(...)

class PathResult(BaseModel):
    path: List [int] = Field(..., description="Кратчайший путь")
    total_distance: float = Field(..., description="Длина пути")