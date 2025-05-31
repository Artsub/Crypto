# redis_manager.py
import redis.asyncio as redis
from fastapi import FastAPI

async def init_redis(app: FastAPI) -> None:
    app.state.redis = redis.Redis(
        host='localhost',
        port=6379,
        decode_responses=True
    )

async def get_redis() -> redis.Redis:
    return redis.Redis(host='localhost', port=6379, decode_responses=True)