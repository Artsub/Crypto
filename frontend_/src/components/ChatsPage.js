// src/ChatsPage.jsx

import React, { useState, useEffect } from 'react';
import {
  Container,
  Card,
  Button,
  Modal,
  Form,
  Row,
  Col,
  Alert,
  ButtonGroup
} from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';
import api from './ApiTokenInter';
import DiffieHellman from './DiffieHellman/DiffieHellman'; // <-- Импорт класса DH
/* global BigInt */

const ChatsPage = () => {
  const [chats, setChats] = useState([]);
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [showConnectModal, setShowConnectModal] = useState(false);
  const [chatIdToConnect, setChatIdToConnect] = useState('');
  const [newChat, setNewChat] = useState({
    name: 'MyChat',
    cryptAlgorithm: 'MARS',
    cryptPadding: 'Zeros',
    cryptMode: 'ECB'
  });
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  const [isDHBusy, setIsDHBusy] = useState(false);           // флаг, что DH-обмен идёт
  const [dhError, setDhError] = useState('');                // сообщение об ошибке DH
  const navigate = useNavigate();

  const fetchChats = async () => {
    try {
      const response = await api.get('/chats/');
      setChats(response.data);
    } catch (err) {
      setError('Ошибка при загрузке чатов');
      console.error(err);
    }
  };

  useEffect(() => {
    fetchChats();
  }, []);


  // =============================================
  //  HELPERS для DH-обмена (сериализация ключей)
  // =============================================

  // BigInt -> Base64
  function bigIntToBase64(bigint) {
    const hex = bigint.toString(16);
    const paddedHex = hex.length % 2 === 0 ? hex : '0' + hex;
    const byteArray = new Uint8Array(
      paddedHex.match(/.{1,2}/g).map(byte => parseInt(byte, 16))
    );
    const binaryString = String.fromCharCode(...byteArray);
    return btoa(binaryString);
  }

  // Base64 -> BigInt
  function base64ToBigInt(base64) {
    try {
      const binaryStr = atob(base64);
      const hex = Array.from(binaryStr)
        .map(c => c.charCodeAt(0).toString(16).padStart(2, '0'))
        .join('');
      return BigInt('0x' + hex);
    } catch (e) {
      console.error('Error converting base64 to BigInt:', e);
      throw new Error('Invalid public key format');
    }
  }

  /**
   *  initDHAndNavigate
   *
   *  1) Создаёт DiffieHellman(2048)
   *  2) Кодирует свой publicKey → base64
   *  3) Отправляет POST /chats/{chatId}/dh/send_public_key
   *  4) Пытается сразу получить чужой ключ (GET /chats/{chatId}/dh/get_public_key)
   *     Если 404 — ждёт, делая poll каждые 2 секунды, пока не придёт ответ 200.
   *  5) Когда чужой ключ есть — вычисляет общий secret: dh.computeSharedSecret(...)
   *  6) Сохраняет sharedSecret в sessionStorage (или localStorage)
   *  7) Делает navigate(`/chat/${chatId}`)
   */
  const initDHAndNavigate = async (chatId) => {
    setIsDHBusy(true);
    setDhError('');

    try {
      // 1) Экземпляр DH (2048 бит)
      const dh = new DiffieHellman(2048);

      // 2) Кодируем publicKey
      const myPublicKeyB64 = bigIntToBase64(dh.publicKey);

      // 3) Отправляем его на сервер
      await api.post(`/chats/${chatId}/dh/send_public_key`, {
        public_key_b64: myPublicKeyB64
      });

      // Вспомогательная функция: пытается сразу получить чужой publicKey
      const fetchPartnerKey = async () => {
        try {
          const resp = await api.get(`/chats/${chatId}/dh/get_public_key`);
          // 200 → { public_key_b64: "...." }
          return resp.data.public_key_b64;
        } catch (e) {
          // Если 404, то ещё нет ключа партнёра
          if (e.response && e.response.status === 404) {
            return null;
          }
          // Иная ошибка
          console.error('Ошибка при запросе чужого ключа:', e);
          throw e;
        }
      };

      // 4) Пытаемся сразу получить чужой ключ
      let partnerKeyB64 = await fetchPartnerKey();

      // Если ещё нет − запускаем poll (каждые 2 секунды, максимум 10 попыток)
      let attempts = 0;
      while (!partnerKeyB64 && attempts < 10) {
        attempts += 1;
        await new Promise(res => setTimeout(res, 2000));
        partnerKeyB64 = await fetchPartnerKey();
      }

      if (!partnerKeyB64) {
        throw new Error('Публичный ключ партнёра не получен за отведённое время');
      }

      // 5) Вычисляем общий secret
      const theirPublicKey = base64ToBigInt(partnerKeyB64);
      const sharedSecret = dh.computeSharedSecret(theirPublicKey);

      // 6) Сохраняем sharedSecret для текущего чата (например, в sessionStorage)
      sessionStorage.setItem(
        `dh_shared_secret_${chatId}`,
        sharedSecret.toString()
      );

      // 7) Перенаправляем в персональный чат
      navigate(`/chat/${chatId}`);
    } catch (err) {
      console.error('DH handshake failed:', err);
      setDhError('Не удалось установить общий секрет (DH)');
    } finally {
      setIsDHBusy(false);
    }
  };


  // =============================================
  //  РУТИНА ЧАТОВ (создание, удаление, подключение)
  // =============================================

  const handleCreateChat = async () => {
    try {
      await api.post('/chats/', newChat);
      setShowCreateModal(false);
      setNewChat({
        name: 'MyChat',
        cryptAlgorithm: 'MARS',
        cryptPadding: 'Zeros',
        cryptMode: 'ECB'
      });
      setSuccess('Чат успешно создан');
      fetchChats();
      setTimeout(() => setSuccess(''), 3000);
    } catch (err) {
      setError('Ошибка при создании чата');
      console.error(err);
    }
  };

  const handleDeleteChat = async (chatId) => {
    if (!window.confirm('Вы уверены, что хотите удалить этот чат?')) return;
    try {
      await api.delete(`/chats/${chatId}`);
      setSuccess('Чат успешно удален');
      fetchChats();
      setTimeout(() => setSuccess(''), 3000);
    } catch (err) {
      setError('Ошибка при удалении чата');
      console.error(err);
    }
  };

  const handleConnectToChat = async () => {
    if (!chatIdToConnect) {
      setError('Введите ID чата');
      return;
    }

    try {
      await api.patch(`/chats/${chatIdToConnect}/connect`);
      setSuccess(`Успешно подключено к чату ${chatIdToConnect}`);
      setShowConnectModal(false);
      fetchChats();
      setTimeout(() => setSuccess(''), 3000);
      setChatIdToConnect('');
    } catch (err) {
      setError(err.response?.data?.detail || 'Ошибка подключения к чату');
      console.error(err);
    }
  };

  const handleDisconnectFromChat = async (chatId) => {
    try {
      await api.patch(`/chats/${chatId}/disconnect`);
      setSuccess('Успешно отключено от чата');
      fetchChats();
      setTimeout(() => setSuccess(''), 3000);
    } catch (err) {
      setError(err.response?.data?.detail || 'Ошибка отключения от чата');
      console.error(err);
    }
  };


  /**
   *  handleOpenChat — вызывается при клике на «Открыть».
   *  Если DH-обмен уже идёт (isDHBusy), не даём зайти дважды.
   *  Иначе — сразу запускаем initDHAndNavigate.
   */
  const handleOpenChat = (chatId) => {
    if (isDHBusy) return;
    setDhError('');
    initDHAndNavigate(chatId);
  };

  const isChatCreator = (chat) => {
    const userId = localStorage.getItem('user_id');
    return chat.creator_id === parseInt(userId);
  };

  const isConnectedToChat = (chat) => {
    const userId = localStorage.getItem('user_id');
    return chat.receiver_id === parseInt(userId);
  };


  // =============================================
  //  РЕНДЕР
  // =============================================

  return (
    <Container className="mt-4">
      <Row className="mb-3">
        <Col>
          <h2>Мои чаты</h2>
          <ButtonGroup>
            <Button variant="primary" onClick={() => setShowCreateModal(true)}>
              Создать новый чат
            </Button>
            <Button variant="success" onClick={() => setShowConnectModal(true)}>
              Подключиться к чату
            </Button>
          </ButtonGroup>
        </Col>
      </Row>

      {error && (
        <Alert variant="danger" onClose={() => setError('')} dismissible>
          {error}
        </Alert>
      )}
      {success && (
        <Alert variant="success" onClose={() => setSuccess('')} dismissible>
          {success}
        </Alert>
      )}
      {dhError && (
        <Alert variant="warning" onClose={() => setDhError('')} dismissible>
          {dhError}
        </Alert>
      )}
      {isDHBusy && (
        <Alert variant="info">Устанавливаем защищённое соединение…</Alert>
      )}

      <Row>
        {chats.map((chat) => (
          <Col md={4} key={chat.id} className="mb-3">
            <Card>
              <Card.Body>
                <Card.Title>{chat.name}</Card.Title>
                <Card.Text>
                  ID Чата: {chat.id}
                  <br />
                  Алгоритм: {chat.cryptAlgorithm}
                  <br />
                  Режим: {chat.cryptMode}
                  <br />
                  Паддинг: {chat.cryptPadding}
                  <br />
                  ID Создателя: {chat.creator_id}
                  <br />
                  ID Получателя: {chat.receiver_id}
                  <br />
                  Статус:{' '}
                  {isChatCreator(chat)
                    ? 'Вы создатель'
                    : isConnectedToChat(chat)
                    ? 'Вы подключены'
                    : 'Не подключен'}
                  <br />
                </Card.Text>
                <div className="d-flex flex-wrap gap-2">
                  <Button
                    variant="primary"
                    onClick={() => handleOpenChat(chat.id)}
                    disabled={isDHBusy || !isConnectedToChat(chat)}
                  >
                    Открыть
                  </Button>

                  {!isChatCreator(chat) && (
                    <>
                      {isConnectedToChat(chat) ? (
                        <Button
                          variant="warning"
                          onClick={() => handleDisconnectFromChat(chat.id)}
                          disabled={isDHBusy}
                        >
                          Отключиться
                        </Button>
                      ) : (
                        <Button
                          variant="success"
                          onClick={() => {
                            setChatIdToConnect(chat.id.toString());
                            handleConnectToChat();
                          }}
                          disabled={isDHBusy}
                        >
                          Подключиться
                        </Button>
                      )}
                    </>
                  )}

                  {isChatCreator(chat) && (
                    <Button
                      variant="danger"
                      onClick={() => handleDeleteChat(chat.id)}
                      disabled={isDHBusy}
                    >
                      Удалить
                    </Button>
                  )}
                </div>
              </Card.Body>
            </Card>
          </Col>
        ))}
      </Row>

      {/* --- Модалка создания чата --- */}
      <Modal
        show={showCreateModal}
        onHide={() => setShowCreateModal(false)}
      >
        <Modal.Header closeButton>
          <Modal.Title>Создать новый чат</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group className="mb-3">
              <Form.Label>Название чата</Form.Label>
              <Form.Control
                type="text"
                value={newChat.name}
                onChange={(e) =>
                  setNewChat({ ...newChat, name: e.target.value })
                }
              />
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>Алгоритм</Form.Label>
              <Form.Select
                value={newChat.cryptAlgorithm}
                onChange={(e) =>
                  setNewChat({ ...newChat, cryptAlgorithm: e.target.value })
                }
              >
                <option value="MARS">MARS</option>
                <option value="Serpent">Serpent</option>
              </Form.Select>
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>Режим</Form.Label>
              <Form.Select
                value={newChat.cryptMode}
                onChange={(e) =>
                  setNewChat({ ...newChat, cryptMode: e.target.value })
                }
              >
                <option value="ECB">ECB</option>
                <option value="CBC">CBC</option>
                <option value="PCBC">PCBC</option>
                <option value="CFB">CFB</option>
                <option value="OFB">OFB</option>
                <option value="CTR">CTR</option>
                <option value="Random Delta">Random Delta</option>
              </Form.Select>
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>Паддинг</Form.Label>
              <Form.Select
                value={newChat.cryptPadding}
                onChange={(e) =>
                  setNewChat({ ...newChat, cryptPadding: e.target.value })
                }
              >
                <option value="Zeros">Zeros</option>
                <option value="ANSIX923">ANSIX923</option>
                <option value="PKCS7">PKCS7</option>
                <option value="ISO10126">ISO10126</option>
              </Form.Select>
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button
            variant="secondary"
            onClick={() => setShowCreateModal(false)}
            disabled={isDHBusy}
          >
            Отмена
          </Button>
          <Button
            variant="primary"
            onClick={handleCreateChat}
            disabled={isDHBusy}
          >
            Создать
          </Button>
        </Modal.Footer>
      </Modal>

      {/* --- Модалка подключения к чату --- */}
      <Modal
        show={showConnectModal}
        onHide={() => setShowConnectModal(false)}
      >
        <Modal.Header closeButton>
          <Modal.Title>Подключиться к чату</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group>
              <Form.Label>ID чата</Form.Label>
              <Form.Control
                type="text"
                value={chatIdToConnect}
                onChange={(e) => setChatIdToConnect(e.target.value)}
                disabled={isDHBusy}
              />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button
            variant="secondary"
            onClick={() => setShowConnectModal(false)}
            disabled={isDHBusy}
          >
            Отмена
          </Button>
          <Button
            variant="success"
            onClick={handleConnectToChat}
            disabled={isDHBusy}
          >
            Подключиться
          </Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
};

export default ChatsPage;
