import asyncio
from contextlib import asynccontextmanager
import redis.asyncio as redis
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.routers import chat_router
from app.auth import routes as auth_routes
from app.redis_config import init_redis
import uvicorn
import aiokafka
import os

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup logic
    app.state.redis = redis.Redis(
        host="localhost",
        port=6379,
        decode_responses=True
    )
    yield
    # Shutdown logic
    await app.state.redis.close()

app = FastAPI(lifespan=lifespan)

app.include_router(chat_router.router)
app.include_router(auth_routes.router)

origins = ['http://localhost:3000', 'http://192.168.56.1:3000']

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

if __name__ == "__main__":
    uvicorn.run(app, host="localhost", port=8000)