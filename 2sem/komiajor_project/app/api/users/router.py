from fastapi import APIRouter, HTTPException, status, Response, Depends
from app.api.users.auth import get_password_hash, authenticate_user, create_access_token
from app.api.users.dao import UsersDAO
from app.api.users.dependencies import get_current_user, get_token
from app.schemas.schemas import SUserRegister, SUserAuth
from app.models.models import User

router = APIRouter(prefix='/auth', tags=['Auth'])


@router.post("/sign-up/")
async def register_user(user_data: SUserRegister) -> dict:
    user = await UsersDAO.find_one_or_none(email=user_data.email)
    if user:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail='Пользователь уже существует'
        )
    user_dict = user_data.model_dump()
    user_dict['password'] = get_password_hash(user_data.password)
    await UsersDAO.add(**user_dict)
    return {'message': 'Вы успешно зарегистрированы! Ваш email： ' + user_dict['email']}

@router.post("/login/")
async def auth_user(response: Response, user_data: SUserAuth):
    check = await authenticate_user(email=user_data.email, password=user_data.password)
    if check is None:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED,
                            detail='Неверная почта или пароль')
    access_token = create_access_token({"sub": str(check.id)})
    response.set_cookie(key="users_access_token", value=access_token, httponly=True)
    return {'access_token': access_token, 'refresh_token': None}

@router.post("/logout/")
async def logout_user(response: Response, token: str = Depends(get_token)):
    response.delete_cookie(key="users_access_token")
    return {'message': 'Пользователь успешно вышел из системы'}

@router.get("/users/me/")
async def get_me(user_data: User = Depends(get_current_user)):
    return {
        "id": user_data.id,
        "email": user_data.email,
        "created_at": user_data.created_at.isoformat() if user_data.created_at else None
    }