from pydantic import BaseModel

class ChatCreate(BaseModel):
    name: str
    cryptAlgorithm: str
    cryptPadding: str
    cryptMode: str