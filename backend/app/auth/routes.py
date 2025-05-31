from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordRequestForm
from sqlalchemy.ext.asyncio import AsyncSession
from database import get_db
from app.auth.oauth import get_current_user, create_access_token
from app.routers.user_router import create_user, get_user_username
from app.schemas.user_scheame import UserCreate, UserResponse

router = APIRouter(tags=['authentication'])

@router.post('/token')
async def get_token(request: OAuth2PasswordRequestForm = Depends(), db: AsyncSession = Depends(get_db)):
    user = await get_user_username(db, username=request.username)

    if not user:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail='Неверные данные')
    if not user.password == request.password:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail='Неправильный пароль')

    access_token = create_access_token(data={'username': user.username})

    return {
        'access_token': access_token,
        'token_type': 'bearer',
        'user_id': user.id,
        'username': user.username,
    }

@router.post("/register", response_model=UserResponse)
async def register_user(user_data: UserCreate, db: AsyncSession = Depends(get_db)):
    user = await create_user(db, user_data.username, user_data.password)
    if not user:
        raise HTTPException(status_code=400, detail="Пользователь уже существует")
    return user

@router.post("/logout")
async def logout(token: str, db: AsyncSession = Depends(get_db)):
    return {"message": "Successfully logged out"}