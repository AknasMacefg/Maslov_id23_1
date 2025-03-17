from fastapi import FastAPI, BackgroundTasks, HTTPException, status
from pydantic import BaseModel, Field
from typing import Dict, Optional, List
import itertools
import uuid
import hashlib

app = FastAPI()

# Хранилище задач в памяти
tasks: Dict[str, Dict] = {}

class BruteforceRequest(BaseModel):
    hash: str
    charset: List[str] = Field(..., example=["a", "b", "c"])
    max_length: int = Field(..., ge=1, le=8)

class TaskStatus(BaseModel):
    status: str
    result: Optional[str] = None

def generate_passwords(charset: List[str], max_length: int):
    for length in range(1, max_length + 1):
        for combo in itertools.product(charset, repeat=length):
            yield ''.join(combo)

def bruteforce_task(task_id: str, request: BruteforceRequest):
    tasks[task_id]["status"] = "running"
    try:
        for password in generate_passwords(request.charset, request.max_length):
            # Условная проверка: сравниваем хеш пароля с предоставленным (в реальности нужно использовать хеш RAR)
            hashed_password = hashlib.sha256(password.encode()).hexdigest()
            if hashed_password == request.hash:
                tasks[task_id]["result"] = password
                tasks[task_id]["status"] = "completed"
                return
        tasks[task_id]["status"] = "completed"
        tasks[task_id]["result"] = None  # Пароль не найден
    except Exception as e:
        tasks[task_id]["status"] = "failed"
        tasks[task_id]["result"] = str(e)

@app.post("/brut_hash", status_code=status.HTTP_202_ACCEPTED)
async def start_bruteforce(request: BruteforceRequest, background_tasks: BackgroundTasks):
    task_id = str(uuid.uuid4())
    tasks[task_id] = {"status": "pending", "result": None}
    background_tasks.add_task(bruteforce_task, task_id, request)
    return {"task_id": task_id}

@app.get("/get_status/{task_id}", response_model=TaskStatus)
async def get_status(task_id: str):
    task = tasks.get(task_id)
    if not task:
        raise HTTPException(status_code=404, detail="Task not found")
    return task