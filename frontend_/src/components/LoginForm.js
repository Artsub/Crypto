import React, { useState } from "react";
import axios from "axios";
import { useNavigate } from "react-router-dom";
import { Form, Button, Alert, Container } from "react-bootstrap";
import CryptoJS from "crypto-js";
import { useAuth } from "./AuthContext";

const LoginForm = () => {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [errorMessage, setErrorMessage] = useState("");
  const navigate = useNavigate();
  const { login } = useAuth();

  const handleLogin = async (e) => {
    e.preventDefault();
    const hashedPassword = CryptoJS.SHA256(password).toString();

    const formData = new URLSearchParams();
    formData.append("grant_type", "password");
    formData.append("username", username);
    formData.append("password", hashedPassword);
    formData.append("scope", "");
    formData.append("client_id", "string");
    formData.append("client_secret", "string");

    try {
      const response = await axios.post("http://127.0.0.1:8000/token", formData, {
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
      });

      const { access_token, user_id, username } = response.data;
      login(access_token, username, user_id);

      navigate("/chats");
    } catch (error) {
      setErrorMessage("Ошибка входа: " + (error.response?.data?.detail || "Unknown error"));
    }
  };

  return (
    <Container className="p-3">
      <h2 className="mt-4">Войти</h2>
      {errorMessage && <Alert variant="danger">{errorMessage}</Alert>}
      <Form onSubmit={handleLogin}>
        <Form.Group className="mb-3">
          <Form.Label>Имя пользователя</Form.Label>
          <Form.Control
            type="text"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
            required
            placeholder="Введите имя"
          />
        </Form.Group>

        <Form.Group className="mb-3">
          <Form.Label>Пароль</Form.Label>
          <Form.Control
            type="password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            required
            placeholder="Введите пароль"
          />
        </Form.Group>

        <Button variant="primary" type="submit">
          Войти
        </Button>
      </Form>
    </Container>
  );
};

export default LoginForm;
