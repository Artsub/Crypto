import React from "react";
import { BrowserRouter as Router, Routes, Route, Link } from "react-router-dom";
import { Navbar, Nav, Container } from "react-bootstrap";
import RegisterForm from "./components/RegisterForm";
import LoginForm from "./components/LoginForm";
import Logout from "./components/Logout";
import ChatsPage from "./components/ChatsPage";
import ChatMessagesPage from './components/ChatMessagesPage';
import { useAuth } from "./components/AuthContext";
import "./styles/styles.css";

function App() {
  const { user } = useAuth();

  return (
    <div className="Maroon">
      <Router>
        <Navbar bg="dark" variant="dark" expand="lg" style={{ height: "80px" }}>
          <Container>
            <Navbar.Toggle aria-controls="basic-navbar-nav" />
            <Navbar.Collapse id="basic-navbar-nav">
              <Nav className="me-auto">
                {!user && (
                  <>
                    <Nav.Link as={Link} to="/login">Войти</Nav.Link>
                    <Nav.Link as={Link} to="/register">Зарегистрироваться</Nav.Link>
                  </>
                )}
                {user && (
                  <>
                    <Nav.Link as={Link} to="/logout">Выйти</Nav.Link>
                    <Nav.Link as={Link} to="/chats">Мои чаты</Nav.Link>
                  </>
                )}
              </Nav>
            </Navbar.Collapse>
          </Container>
        </Navbar>

        <Container className="mt-4">
          <Routes>
            <Route path="/" element={<RegisterForm />} />
            <Route path="/register" element={<RegisterForm />} />
            <Route path="/login" element={<LoginForm />} />
            <Route path="/logout" element={<Logout />} />
            <Route path="/chats" element={<ChatsPage />} />
            <Route path="/chat/:chatId" element={<ChatMessagesPage />} />
          </Routes>
        </Container>
      </Router>
    </div>
  );
}

export default App;
