import json
from datetime import datetime
import logging
import base64
import redis
from fastapi import APIRouter, Depends, HTTPException, WebSocket, WebSocketDisconnect, Body
from sqlalchemy.future import select
from sqlalchemy import or_
from sqlalchemy.ext.asyncio import AsyncSession
from app.auth.oauth import get_current_user
from app.models.models import Chat, User
from app.redis_config import get_redis
from app.schemas.chat_scheame import ChatCreate
from database import get_db
from aiokafka import AIOKafkaProducer
from app.schemas.message_scheame import MessageIn, MessageOut

router = APIRouter(prefix="/chats", tags=["chats"])

@router.get("/")
async def get_chats_by_user(db: AsyncSession = Depends(get_db), current_user: User = Depends(get_current_user)):
    result = await db.execute(select(Chat).where(or_(Chat.creator_id == current_user.id,
                                                   Chat.receiver_id == current_user.id)))

    chats = result.scalars().all()
    return chats

@router.post("/")
async def create_chat(chat: ChatCreate, db: AsyncSession = Depends(get_db), current_user: User = Depends(get_current_user)):
    new_chat = Chat(name=chat.name,
                cryptAlgorithm=chat.cryptAlgorithm,
                cryptPadding=chat.cryptPadding,
                cryptMode=chat.cryptMode, creator_id = current_user.id, receiver_id = None,
                    )

    try:
        db.add(new_chat)
        await db.flush()
        await db.commit()
        await db.refresh(new_chat)
    except Exception as e:
        await db.rollback()
        raise HTTPException(status_code=500, detail=f"DB error: {e}")

    return new_chat

@router.delete("/{chat_id}")
async def delete_chat(chat_id: int,  db: AsyncSession = Depends(get_db), current_user: User = Depends(get_current_user)):
    result = await db.execute(select(Chat).filter_by(id = chat_id))
    chat = result.scalar_one_or_none()

    if chat is None:
        raise HTTPException(status_code=404, detail="Чат не найден")

    if chat.creator_id != current_user.id:
        raise HTTPException(status_code=403, detail="Нет прав на удаление чата")

    try:
        await db.delete(chat)
        await db.commit()
    except Exception as e:
        await db.rollback()
        raise HTTPException(status_code=500, detail=f"DB error: {e}")

    return {"detail": "Chat deleted successfully"}

@router.patch("/{chat_id}/connect")
async def connect_to_chat(chat_id: int,  db: AsyncSession = Depends(get_db), current_user: User = Depends(get_current_user)):
    result = await db.execute(select(Chat).filter_by(id=chat_id))
    chat = result.scalar_one_or_none()
    if chat is None:
        raise HTTPException(status_code=404, detail="Чат не найден")

    if chat.creator_id == current_user.id:
        raise HTTPException(status_code=403, detail="Невозможно подключиться к своему чату")

    if chat.receiver_id is not None:
        raise HTTPException(status_code=403, detail="Чат недоступен")

    chat.receiver_id = current_user.id

    try:
        await db.commit()
        await db.refresh(chat)
    except Exception as e:
        await db.rollback()
        raise HTTPException(status_code=500, detail=f"DB error: {e}")


@router.patch("/{chat_id}/disconnect")
async def disconnect_from_chat(chat_id: int, db: AsyncSession = Depends(get_db), current_user: User = Depends(get_current_user)):
    result = await db.execute(select(Chat).filter_by(id=chat_id))
    chat = result.scalar_one_or_none()
    if chat is None:
        raise HTTPException(status_code=404, detail="Чат не найден")

    if chat.creator_id == current_user.id:
        raise HTTPException(status_code=403, detail="Невозможно подключиться к своему чату")

    chat.receiver_id = None

    try:
        await db.commit()
        await db.refresh(chat)
    except Exception as e:
        await db.rollback()
        raise HTTPException(status_code=500, detail=f"DB error: {e}")

@router.post("/{chat_id}/send_message")
async def send_message(
    chat_id: int,
    message: MessageIn,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
    redis_client: redis.Redis = Depends(get_redis)
):
    # Проверка доступа к чату
    chat = await db.execute(select(Chat).filter_by(id=chat_id))
    chat = chat.scalar_one_or_none()

    if current_user.id != chat.creator_id and current_user.id != chat.receiver_id:
        raise HTTPException(status_code=403, detail="Нет доступа к чату")

    # Формируем сообщение с преобразованием datetime в строку
    message_data = {
        "text": message.text,
        "sender_id": current_user.id,
        "chat_id": chat_id,
        "timestamp": datetime.utcnow().isoformat()  # Преобразуем в строку
    }

    # Отправляем в Redis Stream
    stream_name = f"chat:{chat_id}"
    try:
        await redis_client.xadd(
            name=stream_name,
            fields={
                "data": json.dumps(message_data),  # Теперь сериализуется без ошибок
                "sender_id": str(current_user.id),
                "timestamp": message_data["timestamp"]  # Уже строка
            }
        )
        return {"status": "sent"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка отправки: {str(e)}")


@router.get("/{chat_id}/messages")
async def get_messages(
        chat_id: int,
        db: AsyncSession = Depends(get_db),
        current_user: User = Depends(get_current_user),
        redis_client: redis.Redis = Depends(get_redis)
):
    # Проверка доступа к чату
    chat = await db.execute(select(Chat).filter_by(id=chat_id))
    chat = chat.scalar_one_or_none()

    if not chat:
        raise HTTPException(status_code=404, detail="Chat not found")

    if current_user.id not in {chat.creator_id, chat.receiver_id}:
        raise HTTPException(status_code=403, detail="Access denied")

    try:
        stream_name = f"chat:{chat_id}"
        messages = []

        # Получаем последние 100 сообщений
        records = await redis_client.xrevrange(
            stream_name, max='+', min='-', count=100
        )

        for record_id, fields in records:
            # Безопасное извлечение данных
            raw_data = fields.get(b'data') or fields.get('data')
            if not raw_data:
                continue

            try:
                # Декодируем данные (для bytes и str)
                if isinstance(raw_data, bytes):
                    message_data = json.loads(raw_data.decode('utf-8'))
                else:
                    message_data = json.loads(raw_data)

                messages.append({
                    "id": record_id.decode() if isinstance(record_id, bytes) else record_id,
                    "text": message_data.get("text", ""),
                    "sender_id": message_data.get("sender_id"),
                    "timestamp": message_data.get("timestamp")
                })
            except (json.JSONDecodeError, AttributeError) as e:
                continue  # Пропускаем некорректные сообщения

        return {"messages": messages[::-1]}  # Новые сообщения в конце

    except Exception as e:
        raise HTTPException(
            status_code=500,
            detail=f"Error fetching messages: {str(e)}"
        )

@router.post("/{chat_id}/dh/send_public_key")
async def send_public_key(
    chat_id: int,
    public_key_b64: str = Body(..., embed=True),
    redis_client: redis.Redis = Depends(get_redis),
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    chat = await db.execute(select(Chat).filter_by(id=chat_id))
    chat = chat.scalar_one_or_none()

    if not chat or current_user.id not in {chat.creator_id, chat.receiver_id}:
        raise HTTPException(status_code=403, detail="Access denied")

    redis_key = f"dh:{chat_id}:{current_user.id}:public_key"
    try:
        await redis_client.set(redis_key, public_key_b64)
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to store key: {str(e)}")

    return {"status": "public key stored"}


# Получаем публичный ключ собеседника
@router.get("/{chat_id}/dh/get_public_key")
async def get_public_key(
    chat_id: int,
    redis_client: redis.Redis = Depends(get_redis),
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    chat = await db.execute(select(Chat).filter_by(id=chat_id))
    chat = chat.scalar_one_or_none()

    if not chat or current_user.id not in {chat.creator_id, chat.receiver_id}:
        raise HTTPException(status_code=403, detail="Access denied")

    # Определим ID собеседника
    other_user_id = chat.receiver_id if current_user.id == chat.creator_id else chat.creator_id

    if other_user_id is None:
        raise HTTPException(status_code=404, detail="No partner connected")

    redis_key = f"dh:{chat_id}:{other_user_id}:public_key"
    try:
        public_key_b64 = await redis_client.get(redis_key)
        if not public_key_b64:
            raise HTTPException(status_code=404, detail="Public key not found")
        # Приводим к str
        if isinstance(public_key_b64, bytes):
            public_key_b64 = public_key_b64.decode('utf-8')
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to fetch key: {str(e)}")

    return {"public_key_b64": public_key_b64}