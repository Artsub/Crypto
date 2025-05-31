from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import relationship

from database import Base


class User(Base):
    __tablename__ = "users"

    id = Column(Integer, primary_key=True, index=True)
    username = Column(String, unique=True, nullable=False)
    password = Column(String, nullable=False)

    created_chats = relationship("Chat", back_populates="creator", foreign_keys="Chat.creator_id")
    received_chats = relationship("Chat", back_populates="receiver", foreign_keys="Chat.receiver_id")


class Chat(Base):
    __tablename__ = "chats"

    id = Column(Integer, primary_key=True, index=True)
    name = Column(String, nullable=False)

    cryptAlgorithm = Column(String, nullable=False)
    cryptPadding = Column(String, nullable=False)
    cryptMode = Column(String, nullable=False)

    creator_id = Column(Integer, ForeignKey("users.id"))
    receiver_id = Column(Integer, ForeignKey("users.id"))

    creator = relationship("User", back_populates="created_chats", foreign_keys=[creator_id])
    receiver = relationship("User", back_populates="received_chats", foreign_keys=[receiver_id])