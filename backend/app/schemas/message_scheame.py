from pydantic import BaseModel, Field
from datetime import datetime

class MessageIn(BaseModel):
    text: str = Field(..., min_length=1, max_length=250)

class MessageOut(BaseModel):
    text: str
    sender_id: int
    chat_id: int
    timestamp: datetime
