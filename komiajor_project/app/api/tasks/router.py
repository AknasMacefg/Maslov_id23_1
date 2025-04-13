from fastapi import APIRouter, HTTPException, status
from app.api.tasks.dao import TasksDAO
from app.schemas.schemas import SUserRegister


router = APIRouter(prefix='/task', tags=['Task'])


@router.get("/{id}", summary="Получить задачу по id")
async def get_student_by_id(task_id: int) -> SUserRegister | dict:
    rez = await SUserRegister.find_one_or_none_by_id(task_id)
    if rez is None:
        return {'message': f'Задача с ID {task_id} не найдена!'}
    return rez