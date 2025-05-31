from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy import select
from sqlalchemy.exc import IntegrityError
from sqlalchemy.ext.asyncio import AsyncSession
from starlette import status

from app.models.models import User
from database import get_db

async def get_user_id(db: AsyncSession, id: int):
    result = await db.execute(select(User).filter_by(id=id))
    user = result.scalars().first()
    if user is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail=f"Пользователя не существует")
    return user

async def get_user_username(db: AsyncSession, username: str):
    result = await db.execute(select(User).filter_by(username=username))
    user = result.scalars().first()
    if user is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Пользователя не существует")
    return user


async def create_user(db: AsyncSession, username: str, password: str):

    existing_user = await db.execute(select(User).filter_by(username=username))
    if existing_user.scalars().first():
        return None  # Пользователь уже существует

    new_user = User(username=username, password=password)
    db.add(new_user)

    try:
        await db.commit()
        await db.refresh(new_user)
        return new_user
    except IntegrityError as e:
        await db.rollback()
        return None
    except Exception as e:
        await db.rollback()
        raise HTTPException(
            status_code=500,
            detail=f"Database error: {str(e)}"
        )