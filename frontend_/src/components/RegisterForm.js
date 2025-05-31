import React, { useState } from "react";
import axios from "axios";
import { useNavigate } from "react-router-dom";
import { Form, Button, Alert, Container } from "react-bootstrap";
import CryptoJS from "crypto-js";

const RegisterForm = () => {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [errorMessage, setErrorMessage] = useState("");
  const navigate = useNavigate(); // Для перенаправления после успешной регистрации

  const handleSubmit = async (e) => {
    e.preventDefault();

    const usernameRegex = /^[A-Za-z0-9]+$/;
    if (username.length < 3 || !usernameRegex.test(username)) {
      setErrorMessage("Имя пользователя должно содержать не менее 3 символов и состоять только из букв и цифр.");
      return;
    }

    if (password.length < 5) {
      setErrorMessage("Пароль должен содержать не менее 5 символов.");
      return;
    }

    const hashedPassword = CryptoJS.SHA256(password).toString();
    const userData = {
      username: username,
      password: hashedPassword,
    };

    try {
      await axios.post("http://127.0.0.1:8000/register", userData);
      navigate("/login"); 
    } catch (error) {
      setErrorMessage("Ошибка регистрации: " + (error.response?.data?.detail || "Unknown error"));
    }
  };

  return (
    <Container className="p-3">
      <h2 className="mt-4">Регистрация</h2>
      {errorMessage && <Alert variant="danger">{errorMessage}</Alert>}
      <Form onSubmit={handleSubmit}>
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
          Зарегистрироваться
        </Button>
      </Form>
    </Container>
  );
};

export default RegisterForm;