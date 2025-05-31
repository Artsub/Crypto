// src/ChatMessagesPage.jsx
import React, { useState, useEffect, useRef } from 'react';
import { useParams } from 'react-router-dom';
import { Container, Form, Button, Spinner } from 'react-bootstrap';
import api from './ApiTokenInter';
import { addMessages, getRecentMessages } from './db';
import { parseJwt } from './utils';

const ChatMessagesPage = () => {
  const { chatId } = useParams();
  const [message, setMessage] = useState('');
  const [sending, setSending] = useState(false);
  const [loading, setLoading] = useState(false);
  const [messages, setMessages] = useState([]);
  const [userId, setUserId] = useState(null);
  const messagesEndRef = useRef(null);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: "smooth" });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  useEffect(() => {
    const token = localStorage.getItem('access_token');
    if (token) {
      const payload = parseJwt(token);
      if (payload?.user_id) {
        setUserId(payload.user_id);
      }
    }
  }, []);

  useEffect(() => {
    loadInitialMessages();
  }, [chatId, userId]);

  const loadInitialMessages = async () => {
    try {
      const localMessages = await getRecentMessages(chatId);
      setMessages(localMessages);
    } catch (err) {
      console.error('Ошибка загрузки локальных сообщений:', err);
    }
  };

  const fetchMessages = async () => {
    if (loading) return;
    
    setLoading(true);
    try {
      const response = await api.get(`/chats/${chatId}/messages`);
      const serverMessages = response.data.messages;
      
      await addMessages(chatId, serverMessages);
      
      const updatedMessages = await getRecentMessages(chatId);
      setMessages(updatedMessages);
    } catch (err) {
      console.error('Ошибка загрузки сообщений:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleSendMessage = async () => {
    if (!message.trim() || sending) return;

    setSending(true);

    try {
      await api.post(`/chats/${chatId}/send_message`, { text: message });
      
      await fetchMessages();
    } catch (err) {
      console.error('Ошибка при отправке сообщения:', err);
    } finally {
      setSending(false);
      setMessage('');
    }
  };

  const handleKeyPress = (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSendMessage();
    }
  };

  const formatMessage = (msg) => {
    const isMe = msg.sender_id === userId;
    return {
      ...msg,
      sender: isMe ? 'me' : `user#${msg.sender_id}`,
      status: isMe ? 'sent' : 'received'
    };
  };

  return (
    <Container className="mt-4">
      <div
        style={{
          height: '400px',
          border: '1px solid #ccc',
          borderRadius: '5px',
          marginBottom: '1rem',
          padding: '10px',
          overflowY: 'scroll',
          display: 'flex',
          flexDirection: 'column'
        }}
      >
        {messages.length === 0 && !loading && (
          <div className="text-center text-muted">
            Нет сообщений. Нажмите "Получить сообщения"
          </div>
        )}

        {messages.map((msg) => {
          const formatted = formatMessage(msg);
          return (
            <div 
              key={msg.id}
              style={{
                alignSelf: formatted.sender === 'me' ? 'flex-end' : 'flex-start',
                backgroundColor: formatted.sender === 'me' ? '#d1ecf1' : '#f8f9fa',
                padding: '5px 10px',
                borderRadius: '10px',
                marginBottom: '5px',
                maxWidth: '80%'
              }}
            >
              <div>
                <strong>{formatted.sender}:</strong> {formatted.text}
              </div>
              <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                <small className="text-muted">
                  {new Date(formatted.timestamp).toLocaleTimeString([], { 
                    hour: '2-digit', 
                    minute: '2-digit' 
                  })}
                </small>
              </div>
            </div>
          );
        })}
        <div ref={messagesEndRef} />
      </div>

      <div className="d-flex mb-3">
        <Button 
          variant="outline-primary" 
          onClick={fetchMessages}
          disabled={loading}
          className="me-2"
        >
          {loading ? (
            <>
              <Spinner animation="border" size="sm" className="me-2" />
              Загрузка...
            </>
          ) : 'Получить сообщения'}
        </Button>
      </div>

      <Form onSubmit={(e) => { e.preventDefault(); handleSendMessage(); }}>
        <Form.Group className="d-flex">
          <Form.Control
            as="textarea"
            rows={2}
            placeholder="Введите сообщение"
            value={message}
            onChange={(e) => setMessage(e.target.value)}
            disabled={sending}
            onKeyPress={handleKeyPress}
          />
          <Button 
            variant="primary" 
            type="submit" 
            disabled={sending || !message.trim()}
            style={{ marginLeft: '10px' }}
          >
            {sending ? 'Отправка...' : 'Отправить'}
          </Button>
        </Form.Group>
      </Form>
    </Container>
  );
};

export default ChatMessagesPage;