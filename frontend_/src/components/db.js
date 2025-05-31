// src/db.js
import { openDB } from 'idb';

export const initDB = () =>
  openDB('chat-db', 1, {
    upgrade(db) {
      if (!db.objectStoreNames.contains('messages')) {
        const store = db.createObjectStore('messages', { keyPath: 'id' });
        store.createIndex('chatId', 'chatId');
        store.createIndex('sender_id', 'sender_id');
        store.createIndex('timestamp', 'timestamp');
      }
    }
  });

/**
 * Добавляет сообщение в базу данных
 * @param {string} chatId - ID чата
 * @param {Object} messageData - Данные сообщения
 */
export const addMessage = async (chatId, messageData) => {
  const db = await initDB();
  await db.put('messages', {
    ...messageData,
    chatId,
    id: messageData.id, // Используем ID из сообщения как ключ
    timestamp: new Date(messageData.timestamp) // Преобразуем в Date
  });
};

/**
 * Добавляет несколько сообщений в базу данных
 * @param {string} chatId - ID чата
 * @param {Array} messages - Массив сообщений
 */
export const addMessages = async (chatId, messages) => {
  const db = await initDB();
  const tx = db.transaction('messages', 'readwrite');
  
  for (const msg of messages) {
    await tx.store.put({
      ...msg,
      chatId,
      id: msg.id,
      timestamp: new Date(msg.timestamp)
    });
  }
  
  await tx.done;
};

/**
 * Получает сообщения для конкретного чата
 * @param {string} chatId - ID чата
 * @returns {Promise<Array>} - Массив сообщений
 */
export const getMessagesByChat = async (chatId) => {
  const db = await initDB();
  const tx = db.transaction('messages', 'readonly');
  const index = tx.store.index('chatId');
  const messages = await index.getAll(chatId);
  
  return messages.sort((a, b) => a.timestamp - b.timestamp);
};

/**
 * Получает последние N сообщений для чата
 * @param {string} chatId - ID чата
 * @param {number} limit - Максимальное количество сообщений
 * @returns {Promise<Array>} - Массив сообщений
 */
export const getRecentMessages = async (chatId, limit = 100) => {
  const db = await initDB();
  const tx = db.transaction('messages', 'readonly');
  const index = tx.store.index('chatId');
  
  // Получаем все сообщения и сортируем по времени
  let messages = await index.getAll(chatId);
  messages.sort((a, b) => b.timestamp - a.timestamp);
  
  return messages.slice(0, limit).reverse();
};