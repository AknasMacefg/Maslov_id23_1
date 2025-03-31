from fastapi import FastAPI, BackgroundTasks, HTTPException, UploadFile, File
from app.api.users.router import router as router_users
from app.api.tasks.dao import TasksDAO
from app.schemas.schemas import STaskData, STaskDataAdd
from celery import Celery
import itertools
import hashlib
import uuid
import subprocess
import os
import tempfile

app = FastAPI()


celery = Celery('tasks', broker='redis://localhost:6379/0', backend='redis://localhost:6379/0')

@app.get("/")
def home_page():
    return {"message": "ИД23-1 МасловАН Брутфорс"}
  
app.include_router(router_users)


def generate_passwords(dictionary, max_length, filename):
    with open(filename, "w") as f:
        for length in range(1, max_length + 1):
            for pwd in itertools.product(dictionary, repeat=length):
                f.write("".join(pwd) + "\n")


@celery.task(bind=True)
def brute_force(self, task_id, hash_value, dictionary, max_length):
    db = SessionLocal()
    task = db.query(BruteforceTask).filter(BruteforceTask.id == task_id).first()
    
    if not task:
        return
    
    task.status = "in_progress"
    db.commit()
    
    password_file = f"passwords_{task_id}.txt"
    hash_file = f"hash_{task_id}.txt"
    
    # Генерация списка паролей
    generate_passwords(dictionary, max_length, password_file)
    
    with open(hash_file, "w") as f:
        f.write(hash_value)
    
    try:
        result = subprocess.run(["john", "--wordlist=" + password_file, hash_file], capture_output=True, text=True)
        output = result.stdout
        
        if "password" in output:
            found_password = output.splitlines()[-1]
            task.status = "completed"
            task.result = found_password
        else:
            task.status = "failed"
    except Exception as e:
        task.status = "error"
        task.result = str(e)
    
    db.commit()
    db.close()
    
    os.remove(password_file)
    os.remove(hash_file)


@app.post("/brut_hash")
async def register_user(task_data: STaskDataAdd) -> dict:
    task_dict = task_data.model_dump()
    await TasksDAO.add(**task_data)
    brute_force.apply_async(args=[task_dict[id], hash_value, dictionary, max_length])
    return {'message': 'Задача успешно создана её id =' + task_data[id]}

@app.get("/get_status")
async def get_student_by_id(task_id: int) -> STaskData | None:
    return await TasksDAO.find_one_or_none_by_id(task_id)
