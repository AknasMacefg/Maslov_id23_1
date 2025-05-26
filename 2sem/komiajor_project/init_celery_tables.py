# scripts/init_celery_tables.py
from celery.backends.database import Task, TaskSet
from sqlalchemy import create_engine, MetaData

def init_celery_tables():
    engine = create_engine("postgresql://postgres:postgres@localhost/FastAPIDB")  # Sync connection
    metadata = MetaData()
    
    # Create Celery tables if they don't exist
    for model in [Task, TaskSet]:
        model.__table__.create(bind=engine, checkfirst=True)

if __name__ == "__main__":
    init_celery_tables()